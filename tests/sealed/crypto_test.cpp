#include "tele/tele_sealed_crypto.cpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTextStream>

#include <cstdio>
#include <stdexcept>

namespace {

auto Checks = 0;

void Check(bool condition, const char *label) {
	++Checks;
	if (!condition) {
		throw std::runtime_error(label);
	}
}

QByteArray Hex(const QJsonObject &object, const char *key) {
	return QByteArray::fromHex(object.value(QLatin1String(key)).toString().toLatin1());
}

QByteArray DeriveKeyPair(const QByteArray &ikm) {
	const auto prk = Tele::LabeledExtract(Tele::KemSuite(), {}, "dkp_prk", ikm);
	Check(bool(prk), "RFC DeriveKeyPair extraction");
	for (auto counter = 0; counter < 256; ++counter) {
		auto candidate = Tele::LabeledExpand(
			Tele::KemSuite(), *prk, "candidate", QByteArray(1, char(counter)), 66);
		Check(candidate && candidate->size() == 66, "RFC 66-byte HKDF expansion");
		(*candidate)[0] &= char(1);
		if (Tele::DeriveSealedPublicKey(*candidate)) {
			return *candidate;
		}
	}
	throw std::runtime_error("RFC DeriveKeyPair rejection limit");
}

void VectorTest(const QString &path) {
	QFile file(path);
	Check(file.open(QIODevice::ReadOnly), "Open official HPKE vectors");
	const auto vectors = QJsonDocument::fromJson(file.readAll())
		.object()["vectors"].toArray();
	Check(!vectors.isEmpty(), "Official auth vector exists");
	for (const auto item : vectors) {
		const auto vector = item.toObject();
		Check(vector["mode"].toInt() == 2 && vector["kem_id"].toInt() == 18
			&& vector["kdf_id"].toInt() == 3 && vector["aead_id"].toInt() == 2,
			"Official vector selects exact P521/SHA512/AES256GCM Auth suite");
		const auto recipientPrivate = Hex(vector, "skRm");
		const auto senderPrivate = Hex(vector, "skSm");
		const auto recipientPublic = Hex(vector, "pkRm");
		const auto senderPublic = Hex(vector, "pkSm");
		const auto ephemeralPrivate = Hex(vector, "skEm");
		const auto enc = Hex(vector, "enc");
		const auto info = Hex(vector, "info");
		Check(DeriveKeyPair(Hex(vector, "ikmR")) == recipientPrivate,
			"RFC recipient DeriveKeyPair rejection sampling");
		Check(DeriveKeyPair(Hex(vector, "ikmS")) == senderPrivate,
			"RFC sender DeriveKeyPair rejection sampling");
		Check(DeriveKeyPair(Hex(vector, "ikmE")) == ephemeralPrivate,
			"RFC ephemeral DeriveKeyPair rejection sampling");
		Check(Tele::DeriveSealedPublicKey(recipientPrivate) == recipientPublic,
			"RFC recipient key derivation");
		Check(Tele::DeriveSealedPublicKey(senderPrivate) == senderPublic,
			"RFC sender key derivation");
		Check(Tele::DeriveSealedPublicKey(ephemeralPrivate) == enc,
			"RFC ephemeral key derivation");
		const auto first = Tele::DiffieHellman(ephemeralPrivate, recipientPublic);
		const auto second = Tele::DiffieHellman(senderPrivate, recipientPublic);
		Check(bool(first) && bool(second), "RFC authenticated DH");
		const auto shared = Tele::KemSecret(
			*first, *second, enc, recipientPublic, senderPublic);
		Check(shared && *shared == Hex(vector, "shared_secret"),
			"RFC authenticated KEM shared secret");
		auto keys = Tele::KeySchedule(*shared, info);
		Check(bool(keys), "RFC key schedule");
		Check(*keys->key == Hex(vector, "key"), "RFC AEAD key");
		const auto baseNonce = Hex(vector, "base_nonce");
		Check(*keys->nonce == baseNonce, "RFC base nonce");
		const auto encryptions = vector["encryptions"].toArray();
		for (auto index = 0; index != encryptions.size(); ++index) {
			const auto encryption = encryptions[index].toObject();
			const auto plaintext = Hex(encryption, "pt");
			const auto aad = Hex(encryption, "aad");
			const auto ciphertext = Hex(encryption, "ct");
			const auto expectedNonce = Hex(encryption, "nonce");
			auto nonce = baseNonce;
			for (auto byte = 0; byte != 8; ++byte) {
				nonce[nonce.size() - 1 - byte] ^= char(
					(quint64(index) >> (byte * 8)) & 0xFF);
			}
			Check(nonce == expectedNonce, "RFC sequence nonce");
			*keys->nonce = nonce;
			Check(Tele::AeadSeal(*keys, plaintext, aad) == ciphertext,
				"RFC AES256GCM encryption");
			Check(Tele::AeadOpen(*keys, ciphertext, aad) == plaintext,
				"RFC AES256GCM decryption");
			if (index == 0) {
				Check(Tele::HpkeSeal(
					senderPrivate, recipientPublic, ephemeralPrivate,
					info, aad, plaintext) == ciphertext,
					"RFC single shot AuthSeal");
				Check(Tele::HpkeOpen(
					recipientPrivate, senderPublic, enc, info, aad,
					ciphertext) == plaintext,
					"RFC single shot AuthOpen");
			}
		}
	}
}

void RngFailureTest(const QString &path) {
	QFile file(path);
	Check(file.open(QIODevice::ReadOnly), "Open official vector for RNG fault test");
	const auto vector = QJsonDocument::fromJson(file.readAll())
		.object()["vectors"].toArray()[0].toObject();
	Check(RAND_set_DRBG_type(nullptr, "tele-sealed-missing-drbg", nullptr, nullptr, nullptr) == 1,
		"Configure isolated unavailable OpenSSL random generator");
	auto sample = QByteArray(32, Qt::Uninitialized);
	Check(RAND_bytes(reinterpret_cast<unsigned char*>(sample.data()), int(sample.size())) != 1,
		"Injected entropy failure reaches OpenSSL RNG");
	Check(!Tele::GenerateSealedIdentity(), "Identity generation fails closed without entropy");
	Check(!Tele::SealedEncrypt(Hex(vector, "skSm"), Hex(vector, "pkRm"),
		"private text", "prod:1234567:7654321"),
		"Chat encryption fails closed without ephemeral randomness");
	Check(!Tele::SealedEncryptFile(Hex(vector, "skSm"), Hex(vector, "pkRm"),
		"private file", "prod:1234567:7654321"),
		"File encryption fails closed without ephemeral randomness");
	Check(!Tele::ExportSealedIdentity(Hex(vector, "skSm"),
		"This is a test-only backup passphrase 12345", "prod:1234567"),
		"Backup export fails closed without salt and nonce randomness");
}

void ProtocolTest() {
	const auto alice = Tele::GenerateSealedIdentity();
	const auto bob = Tele::GenerateSealedIdentity();
	const auto eve = Tele::GenerateSealedIdentity();
	Check(bool(alice) && bool(bob) && bool(eve), "Generate three identities");
	const auto context = QByteArray("prod:1234567:7654321");
	const auto plaintext = QByteArray("Private local-only text");
	const auto encrypted = Tele::SealedEncrypt(
		alice->privateKey, bob->publicKey, plaintext, context);
	Check(bool(encrypted), "Encrypt verified private chat text");
	Check(Tele::SealedDecrypt(
		bob->privateKey, alice->publicKey, *encrypted, context) == plaintext,
		"Verified recipient reads text");
	Check(Tele::SealedDecrypt(
		alice->privateKey, alice->publicKey, *encrypted, context) == plaintext,
		"Sender reads retained sent copy");
	Check(!Tele::SealedDecrypt(
		eve->privateKey, alice->publicKey, *encrypted, context),
		"Non-recipient cannot read text");
	Check(!Tele::SealedDecrypt(
		bob->privateKey, eve->publicKey, *encrypted, context),
		"Unverified sender cannot authenticate");
	Check(!Tele::SealedDecrypt(
		bob->privateKey, alice->publicKey, *encrypted, "test:1234567:7654321"),
		"Network domain is authenticated");
	Check(!Tele::SealedDecrypt(
		bob->privateKey, alice->publicKey, *encrypted, "prod:7654321:1234567"),
		"Direction and accounts are authenticated");
	const auto second = Tele::SealedEncrypt(
		alice->privateKey, bob->publicKey, plaintext, context);
	Check(second && *encrypted != *second, "Every encryption uses fresh randomness");
	const auto note = Tele::SealedEncrypt(
		alice->privateKey, alice->publicKey, plaintext, "prod:1234567:1234567");
	Check(note && Tele::SealedDecrypt(
		alice->privateKey, alice->publicKey, *note, "prod:1234567:1234567") == plaintext,
		"Saved Messages self-recipient works");
	Check(!Tele::SealedDecrypt(
		bob->privateKey, alice->publicKey, *note, "prod:1234567:1234567"),
		"Another account cannot read private self notes");
	const auto wire = Tele::FormatSealedEnvelope(*encrypted);
	Check(wire.size() <= 4096, "Wire text fits a Telegram message");
	Check(Tele::ParseSealedEnvelope(wire) == encrypted, "Canonical wire roundtrip");
	Check(!wire.contains(QString::fromLatin1(alice->publicKey.toBase64(
		QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals))),
		"Wire hides stable sender key");
	Check(!encrypted->contains(alice->publicKey), "Binary hides stable sender key");
	Check(!encrypted->contains(bob->publicKey), "Binary hides stable recipient key");
	Check(!Tele::ParseSealedEnvelope(wire + "="), "Reject padded base64 wire");
	Check(!Tele::ParseSealedEnvelope(wire + "\n"), "Reject trailing wire whitespace");
	Check(!Tele::ParseSealedEnvelope(wire + "!"), "Reject non-base64 byte");
	Check(!Tele::ParseSealedEnvelope(wire.mid(1)), "Reject missing wire prefix");
	const auto publicToken = Tele::FormatSealedPublicKey(alice->publicKey);
	Check(Tele::ParseSealedPublicKey(publicToken) == alice->publicKey,
		"Public token roundtrip");
	Check(Tele::SealedFingerprint(alice->publicKey).size() == 159,
		"Full SHA512 fingerprint is displayed");
	Check(alice->privateKey.size() == Tele::kSealedPrivateKeySize,
		"P521 private scalar is fixed-width 66-byte big endian");
	Check(alice->publicKey.size() == Tele::kSealedPublicKeySize
		&& alice->publicKey.front() == char(4),
		"P521 public key is canonical uncompressed SEC1");
	Check(encrypted->size() == 2355 && wire.size() == 3155,
		"P521 text envelope and token retain canonical fixed size");
	Check(!Tele::IsSealedPublicKey(QByteArray(Tele::kSealedPublicKeySize, char(0))),
		"Reject zero/infinity encoding");
	auto offCurve = alice->publicKey;
	offCurve.back() ^= char(1);
	Check(!Tele::IsSealedPublicKey(offCurve), "Reject off-curve point");
	auto hybrid = alice->publicKey;
	hybrid[0] = char(6);
	Check(!Tele::IsSealedPublicKey(hybrid), "Reject hybrid point encoding");
	auto compressed = alice->publicKey.left(67);
	compressed[0] = char(2);
	Check(!Tele::IsSealedPublicKey(compressed), "Reject compressed point encoding");
	auto nonCanonical = alice->publicKey;
	nonCanonical[1] = char(2);
	Check(!Tele::IsSealedPublicKey(nonCanonical), "Reject out-of-range x coordinate");
	nonCanonical = alice->publicKey;
	nonCanonical[67] = char(2);
	Check(!Tele::IsSealedPublicKey(nonCanonical), "Reject out-of-range y coordinate");
	Check(!Tele::DeriveSealedPublicKey(QByteArray(Tele::kSealedPrivateKeySize, char(0))),
		"Reject scalar zero");
	const auto group = Tele::EcGroup(EC_GROUP_new_by_curve_name(NID_secp521r1), EC_GROUP_free);
	auto order = QByteArray(Tele::kSealedPrivateKeySize, Qt::Uninitialized);
	Check(BN_bn2binpad(EC_GROUP_get0_order(group.get()),
		reinterpret_cast<unsigned char*>(order.data()), int(order.size())) == order.size(),
		"Serialize curve order for scalar-boundary test");
	Check(!Tele::DeriveSealedPublicKey(order), "Reject scalar equal to curve order");
	Check(!Tele::DeriveSealedPublicKey(QByteArray(Tele::kSealedPrivateKeySize, char(-1))),
		"Reject scalar above curve order");
	auto one = QByteArray(Tele::kSealedPrivateKeySize, char(0));
	one.back() = char(1);
	const auto generator = Tele::DeriveSealedPublicKey(one);
	Check(generator && Tele::IsSealedPublicKey(*generator), "Accept canonical scalar one");
	Check(!Tele::DeriveSealedPublicKey(one.mid(1)), "Reject short scalar encoding");
	Check(!Tele::DeriveSealedPublicKey(QByteArray(1, char(0)) + one),
		"Reject overlong scalar encoding");
	Check(!Tele::SealedEncrypt(alice->privateKey, bob->publicKey, {}, context),
		"Reject empty text");
	Check(!Tele::SealedEncrypt(alice->privateKey, bob->publicKey,
		QByteArray(Tele::kSealedPlaintextLimit + 1, 'a'), context),
		"Reject oversized text");
	Check(!Tele::SealedEncrypt(alice->privateKey, bob->publicKey, plaintext, {}),
		"Reject missing conversation binding");
	Check(!Tele::SealedDecrypt(bob->privateKey, alice->publicKey,
		QByteArray(5000000, 'a'), context), "Reject oversized envelope before crypto");
	for (auto index = 0; index != encrypted->size(); ++index) {
		auto modified = *encrypted;
		modified[index] ^= char(1);
		const auto peer = Tele::SealedDecrypt(
			bob->privateKey, alice->publicKey, modified, context);
		const auto self = Tele::SealedDecrypt(
			alice->privateKey, alice->publicKey, modified, context);
		const auto split = Tele::kHeaderSize
			+ (encrypted->size() - Tele::kHeaderSize) / 2;
		if (index < split) {
			Check(!peer, "All selected ciphertext/header mutations fail for recipient");
		}
		if (index < Tele::kHeaderSize || index >= split) {
			Check(!self, "All selected ciphertext/header mutations fail for sender");
		}
	}
	for (const auto size : { 1, 2, 23, 1023, 1024 }) {
		const auto plain = QByteArray(size, QByteArray::fromHex("aa").front());
		const auto result = Tele::SealedEncrypt(
			alice->privateKey, bob->publicKey, plain, context);
		Check(result && result->size() == encrypted->size(),
			"Chat padding hides precise message size");
		Check(result && Tele::SealedDecrypt(
			bob->privateKey, alice->publicKey, *result, context) == plain,
			"Every allowed chat size roundtrips");
	}
	const auto copy = *alice;
	Check(copy.privateKey.constData() != alice->privateKey.constData(),
		"Identity copy owns a separate private-key buffer");
	for (const auto size : { 1, 65532, 65533, 65536, 131072, Tele::kSealedFileLimit }) {
		const auto plain = QByteArray(size, QByteArray::fromHex("a5").front());
		const auto result = Tele::SealedEncryptFile(
			alice->privateKey, bob->publicKey, plain, context);
		Check(bool(result), "Encrypt allowed binary file size");
		Check(Tele::SealedDecryptFile(
			bob->privateKey, alice->publicKey, *result, context) == plain,
			"File payload recipient roundtrip");
		Check(Tele::SealedDecryptFile(
			alice->privateKey, alice->publicKey, *result, context) == plain,
			"File payload sender roundtrip");
		Check(!Tele::SealedDecrypt(
			bob->privateKey, alice->publicKey, *result, context),
			"File does not parse as a chat message");
		Check((result->size() - Tele::kHeaderSize) / 2 % Tele::kFileBucket == 16,
			"File size bucket padding is exact");
	}
	Check(!Tele::SealedEncryptFile(alice->privateKey, bob->publicKey,
		QByteArray(Tele::kSealedFileLimit + 1, 'a'), context),
		"Reject file size above limit");
	Check(!Tele::SealedDecryptFile(
		bob->privateKey, alice->publicKey, *encrypted, context),
		"Chat does not parse as a file");
	const auto smallPadded = Tele::Pad("123", Tele::kEnvelopeVersion);
	Check(Tele::Unpad(*smallPadded, Tele::kEnvelopeVersion, 1024) == "123",
		"Inner length decodes exactly");
	(*smallPadded)[3] = char(0);
	Check(!Tele::Unpad(*smallPadded, Tele::kEnvelopeVersion, 1024),
		"Reject zero authenticated inner length");
	(*smallPadded)[0] = char(-1);
	Check(!Tele::Unpad(*smallPadded, Tele::kEnvelopeVersion, 1024),
		"Reject oversized authenticated inner length");
}

void BackupTest() {
	const auto identity = Tele::GenerateSealedIdentity();
	Check(bool(identity), "Generate backup identity");
	const auto passphrase = QString::fromUtf8("secret backup phrase 0123456789");
	const auto context = QByteArray("prod:1234567");
	const auto backup = Tele::ExportSealedIdentity(identity->privateKey, passphrase, context);
	Check(backup && backup->size() == 135, "Export account-bound 135-byte encrypted private key");
	Check(!backup->contains(identity->privateKey), "Backup never embeds plain private key");
	const auto restored = Tele::ImportSealedIdentity(*backup, passphrase, context);
	Check(restored && restored->privateKey == identity->privateKey
		&& restored->publicKey == identity->publicKey,
		"Backup restores the same account device key");
	Check(!Tele::ImportSealedIdentity(*backup, "wrong password 1234567890", context),
		"Wrong passphrase cannot decrypt backup");
	Check(!Tele::ImportSealedIdentity(*backup, passphrase, "prod:7654321"),
		"Another account cannot import the backup");
	Check(!Tele::ImportSealedIdentity(*backup, passphrase, "test:1234567"),
		"Another Telegram network cannot import the backup");
	Check(!Tele::ExportSealedIdentity(identity->privateKey, "short", context),
		"Reject short backup passphrase");
	Check(!Tele::ExportSealedIdentity(identity->privateKey, QString(1025, 'x'), context),
		"Reject oversized backup passphrase");
	Check(!Tele::ImportSealedIdentity(*backup + "x", passphrase, context),
		"Reject trailing backup data");
	Check(!Tele::ImportSealedIdentity(backup->left(backup->size() - 1), passphrase, context),
		"Reject truncated backup");
	for (const auto offset : { 0, 24, 41, 55, 75, 99, 118, 119, 134 }) {
		auto modified = *backup;
		modified[offset % modified.size()] ^= char(1);
		Check(!Tele::ImportSealedIdentity(modified, passphrase, context),
			"Backup header/ciphertext/tag corruption fails closed");
	}
	const auto second = Tele::ExportSealedIdentity(identity->privateKey, passphrase, context);
	Check(second && *second != *backup, "Backups use fresh salt and nonce");
}

}

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	if (app.arguments().size() > 1 && app.arguments()[1] == "--fixtures") {
		QFile file(app.arguments().value(2, "hpke-auth-vectors.json"));
		if (!file.open(QIODevice::ReadOnly)) return 2;
		const auto vector = QJsonDocument::fromJson(file.readAll())
			.object()["vectors"].toArray()[0].toObject();
		const auto skS = Hex(vector, "skSm");
		const auto skR = Hex(vector, "skRm");
		const auto pkR = Hex(vector, "pkRm");
		const auto context = QByteArray("prod:1234567:7654321");
		const auto plaintext = QByteArray("Private local-only text");
		const auto encrypted = Tele::SealedEncrypt(skS, pkR, plaintext, context);
		const auto note = Tele::SealedEncrypt(skS, Hex(vector, "pkSm"), plaintext, context);
		const auto binary = Tele::SealedEncryptFile(skS, pkR, plaintext, context);
		const auto backup = Tele::ExportSealedIdentity(
			skS, "This is a test-only backup passphrase 12345", "prod:1234567");
		if (!encrypted || !note || !binary || !backup) return 3;
		const auto output = QJsonObject{
			{ "text", QString::fromLatin1(encrypted->toHex()) },
			{ "note", QString::fromLatin1(note->toHex()) },
			{ "binary", QString::fromLatin1(binary->toHex()) },
			{ "backup", QString::fromLatin1(backup->toHex()) },
			{ "context", QString::fromLatin1(context.toHex()) },
			{ "plaintext", QString::fromLatin1(plaintext.toHex()) },
		};
		QFile destination(app.arguments().value(3, "interoperability-fixture.json"));
		if (!destination.open(QIODevice::WriteOnly)) return 4;
		destination.write(QJsonDocument(output).toJson(QJsonDocument::Indented));
		return 0;
	}
	QElapsedTimer timer;
	timer.start();
	try {
		if (app.arguments().value(1) == "--rng-failure") {
			RngFailureTest(app.arguments().value(2, "hpke-auth-vectors.json"));
			QTextStream(stdout) << Checks << " isolated RNG-failure checks passed.\n";
			return 0;
		}
		VectorTest(app.arguments().value(1, "hpke-auth-vectors.json"));
		ProtocolTest();
		BackupTest();
		QTextStream(stdout) << Checks << " HPKE/vector/protocol/backup checks passed in "
			<< timer.elapsed() << " ms.\n";
		return 0;
	} catch (const std::exception &error) {
		QTextStream(stderr) << "FAILED after " << Checks << " checks: " << error.what() << '\n';
		return 1;
	}
}
