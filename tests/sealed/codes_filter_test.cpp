#include "tele/tele_sealed_codes_filter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <cstdlib>

namespace {

int Checks = 0;

void Check(bool condition, const char *name) {
	++Checks;
	if (!condition) {
		qCritical().noquote() << "failed:" << name;
		std::exit(1);
	}
}

}

int main(int argc, char **argv) {
	const auto app = QCoreApplication(argc, argv);
	const auto seed = QByteArray(32, char(42));
	const auto context = QByteArray("account1-message17");
	const auto sample = QStringLiteral("Login code: 12345. Never share it.");
	const auto masked = Tele::MakeSealedCodeText(sample, seed, context);
	Check(masked.text != sample, "code replaced");
	Check(masked.text.size() == sample.size(), "UTF16 length preserved");
	Check(masked.mappings.size() == 1, "single mapping");
	Check(masked.mappings.front().original == QStringLiteral("12345"), "original mapping");
	Check(!masked.text.contains(QStringLiteral("12345")), "original digits absent");
	Check(Tele::MakeSealedCodeText(sample, seed, context).text == masked.text, "stable replacement");
	Check(Tele::MakeSealedCodeText(sample, QByteArray(32, char(43)), context).text != masked.text, "activation separation");
	Check(Tele::MakeSealedCodeText(sample, seed, QByteArray("account2-message17")).text != masked.text, "account separation");
	Check(Tele::MakeSealedCodeText(sample, seed, QByteArray("account1-message18")).text != masked.text, "message separation");
	Check(Tele::ApplySealedCodeMappings(sample, masked.mappings) == masked.text, "mapping applies");
	Check(Tele::ApplySealedCodeMappings(masked.text, masked.mappings) == masked.text, "mapped value not transformed again");
	const auto translated = QStringLiteral("Код входа: 12345. Никому не сообщайте его.");
	const auto mappedTranslation = Tele::ApplySealedCodeMappings(translated, masked.mappings);
	Check(mappedTranslation.contains(masked.mappings.front().replacement), "translation uses same digits");
	Check(!mappedTranslation.contains(QStringLiteral("12345")), "translation original removed");
	for (const auto &plain : {
		QStringLiteral("No code here"),
		QStringLiteral("2026-11-28 12:34"),
		QStringLiteral("Phone: 12345678901"),
		QStringLiteral("Only 1234 digits"),
		QStringLiteral("abc12345def"),
		QStringLiteral("код12345слово"),
		QStringLiteral("#12345"),
		QStringLiteral("1---2"),
		QStringLiteral("-12345-"),
	}) {
		Check(Tele::MakeSealedCodeText(plain, seed, context).text == plain, "non-code span unchanged");
	}
	for (const auto &original : {
		QStringLiteral("00000"),
		QStringLiteral("123456"),
		QStringLiteral("1234567"),
		QStringLiteral("12345678"),
		QStringLiteral("123-456"),
	}) {
		const auto text = QStringLiteral("😀 Код: ") + original + QStringLiteral(".");
		const auto result = Tele::MakeSealedCodeText(text, seed, context);
		Check(result.mappings.size() == 1, "code length recognized");
		Check(result.mappings.front().original == original, "complete code matched");
		Check(result.text.size() == text.size(), "nonBMP prefix offsets preserved");
		Check(result.mappings.front().replacement != original, "replacement differs");
		Check(result.mappings.front().replacement.count('-') == original.count('-'), "hyphens preserved");
	}
	const auto repeated = Tele::MakeSealedCodeText(QStringLiteral("12345 / 12345"), seed, context);
	Check(repeated.mappings.size() == 1, "same-code mapping deduplicated");
	Check(repeated.text.section('/', 0, 0).trimmed() == repeated.text.section('/', 1, 1).trimmed(), "same-code appearances agree");
	const auto multiple = Tele::MakeSealedCodeText(QStringLiteral("12345 and 67890"), seed, context);
	Check(multiple.mappings.size() == 2, "multiple code spans");
	Check(!multiple.text.contains(QStringLiteral("12345")) && !multiple.text.contains(QStringLiteral("67890")), "multiple originals removed");
	for (auto i = 0; i != 1000; ++i) {
		const auto code = QString::number(i).rightJustified(5, '0');
		const auto result = Tele::MakeSealedCodeText(code, seed, context);
		Check(result.text != code, "sampled code never maps to itself");
		Check(result.text.size() == 5, "sampled code keeps width");
		Check(result.mappings.size() == 1, "sampled code maps once");
	}
	Check(Tele::MakeSealedCodeText(sample, QByteArray(31, char(42)), context).text.isEmpty(), "bad seed fails closed");
	Check(Tele::MakeSealedCodeText(sample, seed, {}).text.isEmpty(), "empty context fails closed");
	Check(Tele::MakeSealedCodeText(sample, seed, QByteArray(129, 'x')).text.isEmpty(), "oversized context fails closed");
	qInfo() << "passed" << Checks << "decoy display checks";
	return 0;
}
