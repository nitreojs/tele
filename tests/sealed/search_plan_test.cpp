#include "tele/tele_sealed_search_plan.h"

#include <QtCore/QCoreApplication>

#include <iostream>
#include <limits>

namespace {

int Checks = 0;
int Failed = 0;

void Check(bool condition, const char *name) {
	++Checks;
	if (!condition) { ++Failed; std::cerr << name << '\n'; }
}

}

int main(int argc, char **argv) {
	QCoreApplication application(argc, argv);
	using namespace Tele;
	const auto selected = SealedAutomaticChatId{ .test = false, .account = 100, .peer = 200 };
	auto plan = SealedAutomaticPlan();
	plan.chats.emplace(selected, SealedAutomaticBinding{ QByteArray(64, 'a'), QByteArray(64, 'b') });
	Check(!SealedSearchPeerBlocked(plan, false, true, true, selected), "default-off policy does not change native search");
	Check(SealedSearchPeerBlocked(plan, true, true, true, selected), "selected private chat query cannot reach cloud search");
	Check(!SealedSearchPeerBlocked(plan, true, true, true, {false, 100, 201}), "different chat remains ordinary");
	Check(!SealedSearchPeerBlocked(plan, true, true, true, {false, 101, 200}), "different account stays isolated");
	Check(!SealedSearchPeerBlocked(plan, true, true, true, {true, 100, 200}), "test and production networks stay isolated");
	Check(SealedSearchPeerBlocked(plan, true, false, true, selected), "unknown account private search fails closed");
	Check(SealedSearchPeerBlocked(plan, true, true, true, {false, 0, 200}), "zero account private query blocked");
	Check(SealedSearchPeerBlocked(plan, true, true, true, {false, 100, 0}), "zero peer private query blocked");
	Check(!SealedSearchPeerBlocked(plan, true, false, false, {}), "public channel search unaffected by private policy");
	const auto malformed = ParseSealedAutomaticPlan(QStringLiteral("{broken"));
	Check(!malformed.valid && SealedSearchPeerBlocked(malformed, true, true, true, {false, 100, 201}), "malformed nonempty plan blocks private query");
	Check(!SealedSearchPeerBlocked(malformed, false, false, true, {}), "inactive malformed fixture cannot change off behavior");
	Check(!SealedSearchPeerBlocked(malformed, true, true, false, selected), "malformed private policy does not affect channel peers");
	Check(SealedSearchSourceAllowed(0, 0, 1), "first nonempty candidate accepted");
	Check(SealedSearchSourceAllowed(1999, 0, 1), "last candidate slot accepted");
	Check(!SealedSearchSourceAllowed(2000, 0, 1), "candidate count exact cap rejects next item");
	Check(SealedSearchSourceAllowed(0, 0, kSealedSearchSourceLimit), "exact8MiB source bound accepted");
	Check(SealedSearchSourceAllowed(0, kSealedSearchSourceLimit - 1, 1), "exact combined source byte bound accepted");
	Check(!SealedSearchSourceAllowed(0, kSealedSearchSourceLimit, 1), "full source budget rejects added byte");
	Check(!SealedSearchSourceAllowed(0, kSealedSearchSourceLimit - 1, 2), "combined source budget one over rejected");
	for (const auto count : {std::numeric_limits<int>::min(), -1, 2001, std::numeric_limits<int>::max()}) {
		Check(!SealedSearchSourceAllowed(count, 0, 1), "invalid source count rejected");
	}
	for (const auto used : {std::numeric_limits<qint64>::min(), qint64(-1), kSealedSearchSourceLimit + 1, std::numeric_limits<qint64>::max()}) {
		Check(!SealedSearchSourceAllowed(0, used, 1), "invalid used bytes rejected before subtraction");
	}
	for (const auto size : {std::numeric_limits<qint64>::min(), qint64(-1), qint64(0), kSealedSearchSourceLimit + 1, std::numeric_limits<qint64>::max()}) {
		Check(!SealedSearchSourceAllowed(0, 0, size), "invalid incoming source size rejected");
	}
	Check(!SealedSearchSourceAllowed(0, std::numeric_limits<qint64>::max(), std::numeric_limits<qint64>::max()), "extreme source sizes cannot overflow subtraction");
	const auto russianUpper = QString::fromUtf8("\xD0\x9F\xD0\xA0\xD0\x98\xD0\x92\xD0\x95\xD0\xA2");
	const auto russianQuery = QString::fromUtf8("\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2");
	Check(SealedSearchTextMatches(russianUpper, NormalizeSealedSearchText(russianQuery)), "Russian case-folded substring matches");
	Check(!SealedSearchTextMatches(russianUpper, NormalizeSealedSearchText(QStringLiteral("other"))), "unrelated text does not match");
	Check(SealedSearchTextMatches(QStringLiteral("prefix HeLLo suffix"), NormalizeSealedSearchText(QStringLiteral("hello"))), "ASCII case-folded substring matches");
	const auto fullwidth = QString(QChar(0xFF26)) + QChar(0xFF2F) + QChar(0xFF2F);
	Check(NormalizeSealedSearchText(fullwidth) == QStringLiteral("foo"), "NFKC folds fullwidth characters");
	Check(SealedSearchTextMatches(fullwidth, NormalizeSealedSearchText(QStringLiteral("FOO"))), "NFKC equivalent source and query match");
	Check(SealedSearchTextMatches(QString(QChar(0xFB00)), NormalizeSealedSearchText(QStringLiteral("ff"))), "compatibility ligature matches decomposed query");
	const auto accented = QString(QChar(0x00C9));
	const auto decomposed = QStringLiteral("e") + QChar(0x0301);
	Check(SealedSearchTextMatches(accented, NormalizeSealedSearchText(decomposed)), "canonical accent equivalence survives case-folding");
	Check(!SealedSearchTextMatches(QStringLiteral("private"), {}), "empty normalized query never matches everything");
	Check(!SealedSearchTextMatches({}, NormalizeSealedSearchText(QStringLiteral("private"))), "empty source does not match");
	Check(!SealedSearchTextMatches(QStringLiteral("hello\nworld"), NormalizeSealedSearchText(QStringLiteral("hello world"))), "substring matching preserves meaningful whitespace");
	std::cout << Checks << " actual encrypted-search policy/matching checks, " << Failed
		<< " failed. Native TL routing, query persistence and worker lifecycle require application testing.\n";
	return Failed ? 1 : 0;
}
