#include "tele/tele_sealed_text.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTextStream>
#include <QtCore/QtEndian>

#include <openssl/rand.h>

#include <algorithm>
#include <cstdlib>

namespace {

int Checks = 0;

void Check(bool passed, const char *name) {
	++Checks;
	if (!passed) {
		QTextStream(stderr) << "FAILED after " << Checks << " checks: " << name << '\n';
		std::exit(1);
	}
}

QByteArray Context(quint64 sender = 100, quint64 recipient = 200, bool test = false) {
	auto result = QByteArray(17, char(0));
	result[0] = char(test);
	qToBigEndian(sender, result.data() + 1);
	qToBigEndian(recipient, result.data() + 9);
	return result;
}

QByteArray TokenBytes(const QString &token) {
	return QByteArray::fromBase64(token.mid(15).toLatin1(), QByteArray::Base64UrlEncoding);
}

QString Token(const QByteArray &bytes) {
	return QStringLiteral("tele-sealed:v2:") + QString::fromLatin1(bytes.toBase64(
		QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

void RngFailure(const QString &path) {
	QFile file(path);
	Check(file.open(QIODevice::ReadOnly), "official public-key fixture readable");
	const auto vector = QJsonDocument::fromJson(file.readAll()).object()["vectors"].toArray()[0].toObject();
	const auto privateKey = QByteArray::fromHex(vector["skSm"].toString().toLatin1());
	const auto publicKey = QByteArray::fromHex(vector["pkRm"].toString().toLatin1());
	Check(privateKey.size() == 66 && publicKey.size() == 133, "official P521 key dimensions");
	Check(RAND_set_DRBG_type(nullptr, "tele-sealed-missing-drbg", nullptr, nullptr, nullptr) == 1, "isolated entropy failure configured");
	auto sample = QByteArray(32, Qt::Uninitialized);
	Check(RAND_bytes(reinterpret_cast<unsigned char*>(sample.data()), int(sample.size())) != 1, "entropy fault reaches new group random source");
	const auto rich = Tele::SealedRichText{ QStringLiteral("private"), {{0, 7, QStringLiteral("**")}} };
	Check(!Tele::EncryptSealedRichParts(privateKey, publicKey, rich, Context()), "formatted encryption fails closed without group randomness");
	Check(!Tele::EncryptSealedRichParts(privateKey, publicKey, {QString(2000, QChar('x')), {}}, Context()), "multipart encryption fails closed without group randomness");
	Check(!Tele::EncryptSealedRichParts(privateKey, publicKey, {QStringLiteral("private"), {}}, Context()), "legacy short path fails closed without HPKE randomness");
}

void Format() {
	using namespace Tele;
	const auto emoji = QString::fromUtf8("\xF0\x9F\x98\x80");
	const auto rich = SealedRichText{
		QStringLiteral("Hello ") + emoji + QString::fromUtf8(" \xD0\xBC\xD0\xB8\xD1\x80"),
		{{0, 5, QStringLiteral("**\\__")}, {6, 2, QStringLiteral("custom-emoji://123456789?987654321")}},
	};
	Check(ValidateSealedRichText(rich), "native composite formatting and custom emoji IDs accepted structurally");
	const auto encoded = EncodeSealedRichText(rich);
	Check(encoded.has_value(), "rich text serialization");
	const auto decoded = DecodeSealedRichText(*encoded);
	Check(decoded && *decoded == rich, "UTF16 offsets, nonASCII text and opaque native IDs roundtrip exactly");
	Check(EncodeSealedRichText(*decoded) == encoded, "rich payload encoding canonical");
	auto reverse = rich;
	std::reverse(reverse.tags.begin(), reverse.tags.end());
	Check(EncodeSealedRichText(reverse) == encoded, "tag order canonicalized without losing nested formatting");
	for (auto cut = qsizetype(); cut < encoded->size(); ++cut) {
		Check(!DecodeSealedRichText(encoded->left(cut)), "truncated rich payload rejected");
	}
	auto bytes = *encoded;
	bytes.append(char(0));
	Check(!DecodeSealedRichText(bytes), "trailing rich bytes rejected");
	bytes = *encoded;
	bytes[0] ^= 1;
	Check(!DecodeSealedRichText(bytes), "rich magic validated");
	bytes = *encoded;
	bytes[8] = 2;
	Check(!DecodeSealedRichText(bytes), "unknown rich payload version rejected");
	bytes = *encoded;
	bytes[13] = '\xFF';
	Check(!DecodeSealedRichText(bytes), "malformed text UTF8 rejected");
	bytes = *encoded;
	qToBigEndian(quint32(0xFFFFFFFF), bytes.data() + 9);
	Check(!DecodeSealedRichText(bytes), "rich text length bounded before allocation");
	Check(!EncodeSealedRichText({}), "empty rich text rejected");
	Check(!EncodeSealedRichText({QString(QChar(0xD800)), {}}), "isolated high surrogate rejected");
	Check(!EncodeSealedRichText({QString(QChar(0xDC00)), {}}), "isolated low surrogate rejected");
	for (const auto &tag : std::vector<SealedRichTag>{
		{-1, 1, QStringLiteral("**")}, {0, 0, QStringLiteral("**")}, {0, -1, QStringLiteral("**")},
		{2, 1, QStringLiteral("**")}, {0, 3, QStringLiteral("**")}, {0, 1, QStringLiteral("**")},
		{1, 1, QStringLiteral("**")}, {0, 2, {}}, {0, 2, QStringLiteral("bad\nID")},
		{0, 2, QString(QChar(0xD800))}, {0, 2, QString(kSealedRichTagIdLimit + 1, QChar('x'))}}) {
		Check(!EncodeSealedRichText({emoji, {tag}}), "invalid UTF16 span or tag ID rejected");
	}
	Check(EncodeSealedRichText({emoji, {{0, 2, QStringLiteral("**")}}}).has_value(), "whole nonBMP scalar formatting accepted");
	auto duplicate = rich;
	duplicate.tags.push_back(duplicate.tags.front());
	Check(!EncodeSealedRichText(duplicate), "duplicate rich spans rejected");
	Check(!EncodeSealedRichText({QStringLiteral("four"), {{0, 3, QStringLiteral("**")}, {2, 2, QStringLiteral("__")}}}), "crossing native tag runs rejected");
	Check(!EncodeSealedRichText({QStringLiteral("four"), {{0, 4, QStringLiteral("**")}, {1, 2, QStringLiteral("__")}}}), "nested separate native tag runs rejected in favor of composite IDs");
	auto many = SealedRichText{QString(kSealedRichDictionaryLimit + 1, QChar('a')), {}};
	for (auto i = 0; i < kSealedRichDictionaryLimit; ++i) many.tags.push_back({i, 1, QString::number(i)});
	Check(ValidateSealedRichText(many), "dictionary bound accepted");
	many.tags.push_back({kSealedRichDictionaryLimit, 1, QStringLiteral("extra")});
	Check(!ValidateSealedRichText(many), "dictionary overflow rejected");
	many = {QString(kSealedRichTagLimit + 1, QChar('a')), {}};
	for (auto i = 0; i < kSealedRichTagLimit; ++i) many.tags.push_back({i, 1, QStringLiteral("**")});
	Check(ValidateSealedRichText(many), "span bound accepted");
	many.tags.push_back({kSealedRichTagLimit, 1, QStringLiteral("**")});
	Check(!ValidateSealedRichText(many), "span overflow rejected");
	const auto maximum = EncodeSealedRichText({QString(kSealedRichPayloadLimit - 17, QChar('a')), {}});
	Check(maximum && maximum->size() == kSealedRichPayloadLimit, "exact serialized payload bound accepted");
	Check(!EncodeSealedRichText({QString(kSealedRichPayloadLimit - 16, QChar('a')), {}}), "serialized payload one byte over rejected");
	auto state = quint32(0x514A72D1);
	for (auto iteration = 0; iteration < 512; ++iteration) {
		bytes.resize(iteration);
		for (auto &value : bytes) {
			state ^= state << 13; state ^= state >> 17; state ^= state << 5;
			value = char(state);
		}
		const auto parsed = DecodeSealedRichText(bytes);
		Check(!parsed || EncodeSealedRichText(*parsed) == bytes, "bounded binary parser maintains canonical roundtrip");
	}
}

void Protocol() {
	using namespace Tele;
	const auto alice = GenerateSealedIdentity();
	const auto bob = GenerateSealedIdentity();
	const auto eve = GenerateSealedIdentity();
	Check(alice && bob && eve, "three independent identities");
	const auto context = Context();
	const auto plain = SealedRichText{QString(1024, QChar('a')), {}};
	const auto legacy = EncryptSealedRichParts(alice->privateKey, bob->publicKey, plain, context);
	Check(legacy && legacy->size() == 1 && legacy->front().startsWith(QStringLiteral("tele-sealed:v1:")), "plain 1024-byte input preserves legacy v1");
	const auto binary = ParseSealedEnvelope(legacy->front());
	Check(binary && SealedDecrypt(bob->privateKey, alice->publicKey, *binary, context) == plain.text.toUtf8(), "legacy output readable by unchanged engine");
	Check(!ParseSealedRichPartHeader(legacy->front()), "legacy token not misread as multipart header");
	const auto longPlain = EncryptSealedRichParts(alice->privateKey, bob->publicKey, {QString(1025, QChar('a')), {}}, context);
	Check(longPlain && longPlain->size() == 2, "long plain input splits automatically");
	const auto rich = SealedRichText{QString::fromUtf8("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 ") + QString(2500, QChar('x')), {{0, 6, QStringLiteral("**\\__")}}};
	const auto tokens = EncryptSealedRichParts(alice->privateKey, bob->publicKey, rich, context);
	Check(tokens && tokens->size() > 1, "formatted rich text fragmented");
	const auto again = EncryptSealedRichParts(alice->privateKey, bob->publicKey, rich, context);
	Check(again && again != tokens, "each send gets fresh group and HPKE encapsulation");
	const auto payload = EncodeSealedRichText(rich);
	const auto digest = QCryptographicHash::hash(*payload, QCryptographicHash::Sha512);
	auto recipient = std::vector<SealedRichPart>();
	auto sender = std::vector<SealedRichPart>();
	for (auto index = size_t(); index < tokens->size(); ++index) {
		const auto &token = (*tokens)[index];
		const auto header = ParseSealedRichPartHeader(token);
		Check(header && header->index == int(index) && header->count == int(tokens->size()), "public part index and count parsed");
		Check(token.size() == 3202 && token.size() < 4096, "fixed-length rich token fits Telegram transport");
		const auto raw = TokenBytes(token);
		Check(raw.size() == 35 + 2355 && raw[0] == 2 && raw[35] == 1, "only 35 public metadata bytes precede unchanged HPKE envelope");
		Check(!raw.contains(digest) && !raw.contains(*payload), "plaintext hash and serialized body absent from public wire");
		const auto peer = DecryptSealedRichPart(bob->privateKey, alice->publicKey, token, context);
		const auto own = DecryptSealedRichPart(alice->privateKey, alice->publicKey, token, context);
		Check(peer && own, "recipient and sender authenticate each part");
		Check(peer->bytes == own->bytes && peer->digest == digest && peer->size == payload->size(), "private full hash and exact size authenticated inside each part");
		Check(peer->binding != own->binding, "merge binding distinguishes recipient and sender copies");
		recipient.push_back(*peer); sender.push_back(*own);
	}
	Check(MergeSealedRichParts(recipient) == rich, "recipient complete rich payload merges");
	Check(MergeSealedRichParts(sender) == rich, "sender complete rich payload merges");
	std::reverse(recipient.begin(), recipient.end());
	Check(MergeSealedRichParts(recipient) == rich, "out-of-order transport reassembles by authenticated index");
	const auto single = EncryptSealedRichParts(alice->privateKey, bob->publicKey, {QStringLiteral("bold"), {{0, 4, QStringLiteral("**")}}}, context);
	Check(single && single->size() == 1 && ParseSealedRichPartHeader(single->front()), "short formatted text uses one v2 part");
	const auto singlePart = DecryptSealedRichPart(bob->privateKey, alice->publicKey, single->front(), context);
	const auto singleRich = singlePart ? MergeSealedRichParts({*singlePart}) : std::nullopt;
	Check(singleRich && singleRich->text == QStringLiteral("bold"), "one-part rich payload authenticates and assembles");
	const auto saved = EncryptSealedRichParts(alice->privateKey, alice->publicKey, rich, Context(100, 100));
	auto notes = std::vector<SealedRichPart>();
	Check(saved.has_value(), "Saved Messages rich send");
	for (const auto &token : *saved) {
		const auto part = DecryptSealedRichPart(alice->privateKey, alice->publicKey, token, Context(100, 100));
		Check(part.has_value(), "Saved Messages rich part read");
		notes.push_back(*part);
	}
	Check(MergeSealedRichParts(notes) == rich, "Saved Messages complete rich assembly");
	for (const auto &badContext : {QByteArray(), QByteArray(16, char(0)), QByteArray(18, char(0)), Context(0, 200), Context(100, 0)}) {
		Check(!EncryptSealedRichParts(alice->privateKey, bob->publicKey, rich, badContext), "invalid base context rejected on send");
		Check(!DecryptSealedRichPart(bob->privateKey, alice->publicKey, tokens->front(), badContext), "invalid base context rejected on read");
	}
	auto badContext = context; badContext[0] = 2;
	Check(!EncryptSealedRichParts(alice->privateKey, bob->publicKey, rich, badContext), "unknown Telegram network rejected");
	Check(!DecryptSealedRichPart(bob->privateKey, alice->publicKey, tokens->front(), Context(100, 200, true)), "wrong network authentication fails");
	Check(!DecryptSealedRichPart(bob->privateKey, alice->publicKey, tokens->front(), Context(200, 100)), "wrong direction authentication fails");
	Check(!DecryptSealedRichPart(eve->privateKey, alice->publicKey, tokens->front(), context), "non-recipient cannot read part");
	Check(!DecryptSealedRichPart(bob->privateKey, eve->publicKey, tokens->front(), context), "unverified sender cannot authenticate part");
	for (const auto &token : {tokens->front() + '=', tokens->front() + '\n', tokens->front().mid(1), QStringLiteral(" ") + tokens->front()}) {
		Check(!ParseSealedRichPartHeader(token), "noncanonical multipart token rejected");
	}
	for (const auto offset : {0, 1, 33, 34, 35, 36, 170, 302}) {
		auto raw = TokenBytes(tokens->front()); raw[offset] ^= 1;
		Check(!DecryptSealedRichPart(bob->privateKey, alice->publicKey, Token(raw), context), "public header and recipient ciphertext mutations fail closed");
	}
	for (const auto count : {0, 65, 255}) {
		auto raw = TokenBytes(tokens->front()); raw[34] = char(count);
		Check(!ParseSealedRichPartHeader(Token(raw)), "invalid public part count rejected");
	}
	auto raw = TokenBytes(tokens->front()); raw[33] = raw[34];
	Check(!ParseSealedRichPartHeader(Token(raw)), "part index outside count rejected");
	raw = TokenBytes(tokens->front());
	for (auto i = 1; i <= 32; ++i) raw[i] = 0;
	Check(!ParseSealedRichPartHeader(Token(raw)), "zero group identifier rejected");
	raw = TokenBytes(tokens->front()); raw[35 + 267 + 1044 + 3] ^= 1;
	Check(DecryptSealedRichPart(bob->privateKey, alice->publicKey, Token(raw), context).has_value(), "unused sender record does not falsely claim recipient integrity");
	Check(!DecryptSealedRichPart(alice->privateKey, alice->publicKey, Token(raw), context), "tampered selected sender record rejected");
	Check(!MergeSealedRichParts({}), "empty group rejected");
	auto changed = recipient; changed.pop_back();
	Check(!MergeSealedRichParts(changed), "incomplete group never yields partial plaintext");
	changed = recipient; changed.back() = changed.front();
	Check(!MergeSealedRichParts(changed), "duplicate identical index rejected rather than ambiguously selected");
	changed = recipient; changed.back().header.group[0] ^= 1;
	Check(!MergeSealedRichParts(changed), "mixed group identifiers rejected");
	changed = recipient; changed.back().binding[0] ^= 1;
	Check(!MergeSealedRichParts(changed), "mixed account context or key binding rejected");
	changed = recipient; changed.back().digest[0] ^= 1;
	Check(!MergeSealedRichParts(changed), "conflicting private payload hashes rejected");
	changed = recipient; for (auto &part : changed) part.digest[0] ^= 1;
	Check(!MergeSealedRichParts(changed), "full payload hash independently checked before decoding");
	changed = recipient; changed.back().size += 1;
	Check(!MergeSealedRichParts(changed), "conflicting exact lengths rejected");
	changed = recipient; changed.front().bytes[0] ^= 1;
	Check(!MergeSealedRichParts(changed), "changed fragment contents rejected");
	changed = recipient; changed.back().bytes.chop(1);
	Check(!MergeSealedRichParts(changed), "noncanonical fragment length rejected");
	changed = recipient; changed.front() = sender.front();
	Check(!MergeSealedRichParts(changed), "recipient and sender record bindings cannot mix");
	const auto maximum = SealedRichText{QString(kSealedRichPayloadLimit - 17, QChar('a')), {}};
	const auto maxTokens = EncryptSealedRichParts(alice->privateKey, bob->publicKey, maximum, context);
	Check(maxTokens && maxTokens->size() == kSealedRichPartLimit, "exact maximum payload produces64 bounded parts");
	auto maxParts = std::vector<SealedRichPart>();
	for (const auto &token : *maxTokens) {
		const auto part = DecryptSealedRichPart(bob->privateKey, alice->publicKey, token, context);
		Check(part && part->bytes.size() == kSealedRichFragmentLimit, "maximum group fragments have canonical capacity");
		maxParts.push_back(*part);
	}
	Check(MergeSealedRichParts(maxParts) == maximum, "maximum64part payload authenticated end to end");
}

}

int main(int argc, char **argv) {
	QCoreApplication application(argc, argv);
	if (application.arguments().value(1) == QStringLiteral("--rng-failure")) {
		RngFailure(application.arguments().value(2));
	} else {
		Format();
		Protocol();
	}
	QTextStream(stdout) << Checks << " rich text codec checks passed.\n";
	return 0;
}
