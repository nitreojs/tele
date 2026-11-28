#include "tele/tele_sealed_auto_mode.h"

#include <QtCore/QCoreApplication>

#include <iostream>

namespace {

int Checks = 0;
int Failed = 0;

void Check(bool condition, const char *name) {
	++Checks;
	if (!condition) {
		++Failed;
		std::cerr << name << '\n';
	}
}

}

int main(int argc, char **argv) {
	QCoreApplication application(argc, argv);
	using namespace Tele;
	const auto own = QByteArray(64, 'a');
	const auto remote = QByteArray::fromHex(QByteArray(128, 'b'));
	const auto id = SealedAutomaticChatId{ false, 10, 20 };
	const auto binding = SealedAutomaticBinding{ own, remote };
	auto plan = SealedAutomaticPlan();
	Check(ParseSealedAutomaticPlan(QString()).valid, "empty settings are valid");
	Check(!SealedAutomaticPlanSelected(plan, id), "empty settings select no chat");
	plan.chats.emplace(id, binding);
	const auto encoded = EncodeSealedAutomaticPlan(plan);
	const auto decoded = ParseSealedAutomaticPlan(encoded);
	Check(decoded.valid && decoded.chats == plan.chats, "round trip preserves binding");
	Check(SealedAutomaticPlanSelected(decoded, id), "selected chat is selected");
	Check(!SealedAutomaticPlanSelected(decoded, { true, 10, 20 }), "test network isolated");
	Check(!SealedAutomaticPlanSelected(decoded, { false, 11, 20 }), "account isolated");
	Check(!SealedAutomaticPlanSelected(decoded, { false, 10, 21 }), "peer isolated");
	Check(MatchesSealedAutomaticBinding(binding, own, remote, false), "matching send and incoming keys");
	Check(!MatchesSealedAutomaticBinding(binding, own, QByteArray(), false), "missing pin blocks send and incoming");
	Check(!MatchesSealedAutomaticBinding(binding, own, QByteArray(64, 'c'), false), "changed pin blocks send and incoming");
	Check(!MatchesSealedAutomaticBinding(binding, QByteArray(64, 'c'), remote, false), "changed own identity blocks send");
	Check(MatchesSealedAutomaticBinding(binding, own, QByteArray(), true), "own copy survives identity-only recovery");
	Check(MatchesSealedAutomaticBinding(binding, own, QByteArray(64, 'c'), true), "own copy does not require current peer pin");
	Check(!MatchesSealedAutomaticBinding(binding, QByteArray(64, 'c'), remote, true), "own copy still binds own identity");
	Check(!MatchesSealedAutomaticBinding(binding, QByteArray(63, 'a'), remote, true), "short own fingerprint rejected");
	Check(!MatchesSealedAutomaticBinding(binding, own, QByteArray(63, 'b'), false), "short peer fingerprint rejected");
	Check(MatchesSealedAutomaticBinding({ own, own }, own, own, false), "saved messages bind own identity twice");
	const auto object = QJsonDocument::fromJson(encoded.toUtf8()).object();
	const auto row = object.value(QStringLiteral("chats")).toArray().first().toObject();
	const auto malformed = [&](QJsonObject value, const char *name) {
		const auto result = ParseSealedAutomaticPlan(QString::fromUtf8(QJsonDocument(value).toJson()));
		Check(!result.valid && SealedAutomaticPlanSelected(result, id)
			&& SealedAutomaticPlanSelected(result, { true, 11, 21 }), name);
	};
	const auto malformedRow = [&](const QString &key, QJsonValue value, const char *name) {
		auto changed = row;
		changed.insert(key, value);
		auto parent = object;
		parent.insert(QStringLiteral("chats"), QJsonArray{ changed });
		malformed(parent, name);
	};
	malformedRow(QStringLiteral("account"), QStringLiteral("0"), "zero account rejected fail closed");
	malformedRow(QStringLiteral("account"), QStringLiteral("010"), "noncanonical account rejected");
	malformedRow(QStringLiteral("account"), 10, "numeric account rejected");
	malformedRow(QStringLiteral("account"), QStringLiteral("281474976710656"), "oversized account rejected");
	malformedRow(QStringLiteral("peer"), QStringLiteral("-1"), "negative peer rejected");
	malformedRow(QStringLiteral("peer"), QStringLiteral("+20"), "signed peer rejected");
	malformedRow(QStringLiteral("peer"), QStringLiteral(" 20"), "whitespace peer rejected");
	malformedRow(QStringLiteral("test"), QStringLiteral("false"), "string network rejected");
	malformedRow(QStringLiteral("own"), QString(), "missing own binding rejected");
	malformedRow(QStringLiteral("own"), QString::fromLatin1(QByteArray(63, 'a').toHex()), "short own binding rejected");
	malformedRow(QStringLiteral("remote"), QString::fromLatin1(remote.toHex()).toUpper(), "uppercase peer binding rejected");
	malformedRow(QStringLiteral("remote"), QString(128, 'g'), "invalid hex rejected");
	malformedRow(QStringLiteral("extra"), true, "unknown row field rejected");
	auto changed = object;
	changed.insert(QStringLiteral("chats"), QJsonArray{ row, row });
	malformed(changed, "duplicate chat rejected");
	changed = object;
	changed.insert(QStringLiteral("version"), 2);
	malformed(changed, "unknown version rejected");
	changed.insert(QStringLiteral("version"), 1.5);
	malformed(changed, "fractional version rejected");
	changed = object;
	changed.insert(QStringLiteral("extra"), 1);
	malformed(changed, "unknown root field rejected");
	changed = object;
	changed.insert(QStringLiteral("chats"), QJsonObject());
	malformed(changed, "wrong collection type rejected");
	for (const auto text : { QStringLiteral(" "), QStringLiteral("[]"), QStringLiteral("null"), QStringLiteral("{"), QStringLiteral("invalid") }) {
		const auto result = ParseSealedAutomaticPlan(text);
		Check(!result.valid && SealedAutomaticPlanSelected(result, id), "malformed nonempty config blocks selected chats");
	}
	auto rows = QJsonArray();
	for (auto i = 0; i <= kSealedAutomaticChatLimit; ++i) {
		auto next = row;
		next.insert(QStringLiteral("peer"), QString::number(i + 1));
		rows.push_back(next);
	}
	changed = object;
	changed.insert(QStringLiteral("chats"), rows);
	malformed(changed, "entry count bound enforced");
	Check(!ParseSealedAutomaticPlan(QString(kSealedAutomaticOptionLimit + 1, 'x')).valid, "input byte bound enforced");
	Check(!ParseSealedAutomaticPlan(QString(kSealedAutomaticOptionLimit / 2, QChar(0x044F))).valid, "UTF8 byte bound enforced");
	plan.chats.clear();
	Check(EncodeSealedAutomaticPlan(plan).isEmpty(), "last removal returns canonical empty settings");
	std::cout << Checks << " checks, " << Failed << " failed\n";
	return Failed ? 1 : 0;
}
