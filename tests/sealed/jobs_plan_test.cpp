#include "tele/tele_sealed_jobs_plan.h"

#include <array>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {

int Checks = 0;
int Failed = 0;

void Check(bool condition, const char *name) {
	++Checks;
	if (!condition) { ++Failed; std::cerr << name << '\n'; }
}

}

int main() {
	using namespace Tele;
	static_assert(!std::is_copy_constructible_v<SealedDecryptBudget>);
	static_assert(!std::is_copy_assignable_v<SealedDecryptBudget>);
	using Purpose = SealedDecryptPurpose;
	using Pool = SealedDecryptPool;
	auto budget = SealedDecryptBudget();
	Check(kSealedDecryptJobLimit == 6 && kSealedDecryptReservedJobLimit == 3
		&& kSealedDecryptSharedJobLimit == 3 && budget.active() == 0,
		"six total slots split into three normal-only and three shared");
	Check(!budget.release(Pool::Reserved) && !budget.release(Pool::Shared) && budget.active() == 0,
		"empty pool releases cannot underflow counters");
	for (auto round = 0; round < 1000; ++round) {
		for (auto active = 0; active < 3; ++active) {
			Check(budget.acquire() == Pool::Reserved, "default normal work uses reserved pool first");
			Check(budget.active() == active + 1 && budget.active(Pool::Shared) == 0,
				"normal reservation leaves shared search capacity untouched");
			Check(budget.available(Purpose::Search), "search can still acquire after normal reserved work");
		}
		for (auto active = 0; active < 3; ++active) {
			Check(budget.acquire(Purpose::Search) == Pool::Shared, "search admits only into shared pool");
			Check(budget.active() == active + 4 && budget.active(Pool::Reserved) == 3,
				"search never consumes normal-only reservations");
		}
		Check(!budget.available() && !budget.available(Purpose::Search)
			&& !budget.acquire() && !budget.acquire(Purpose::Search) && budget.active() == 6,
			"all paths share one six-job ceiling");
		Check(budget.release(Pool::Reserved) && budget.available()
			&& !budget.available(Purpose::Search), "reserved release wakes normal work without granting search an extra slot");
		Check(!budget.acquire(Purpose::Search) && budget.active() == 5,
			"search cannot borrow freed normal-only capacity");
		Check(budget.acquire() == Pool::Reserved, "normal work reuses freed reserved capacity before shared");
		for (auto i = 0; i < 3; ++i) {
			Check(budget.release(Pool::Shared), "shared completion releases one slot");
			Check(budget.available(Purpose::Search), "shared release permits search progress");
		}
		Check(!budget.release(Pool::Shared) && budget.active() == 3, "empty shared pool cannot decrement reserved work");
		for (auto i = 0; i < 3; ++i) Check(budget.release(Pool::Reserved), "reserved completion releases its own pool");
		Check(budget.active() == 0 && !budget.release(Pool::Reserved), "all reservations drain without underflow");
	}
	{
		auto pools = std::vector<Pool>();
		for (auto i = 0; i < 6; ++i) {
			const auto pool = budget.acquire(Purpose::Normal);
			Check(pool && *pool == (i < 3 ? Pool::Reserved : Pool::Shared), "normal-only load may use all six slots in reserved-first order");
			pools.push_back(*pool);
		}
		Check(!budget.acquire(Purpose::Normal) && !budget.acquire(Purpose::Search), "seventh job rejected from either purpose");
		for (const auto pool : pools) Check(budget.release(pool), "normal-only load returns each remembered pool");
	}
	{
		for (auto i = 0; i < 3; ++i) Check(budget.acquire(Purpose::Search) == Pool::Shared, "search-first workload fills exactly shared capacity");
		Check(!budget.acquire(Purpose::Search) && budget.active() == 3 && budget.available(), "three normal slots survive saturated search");
		for (auto i = 0; i < 3; ++i) Check(budget.acquire() == Pool::Reserved, "normal decrypts proceed during saturated search");
		for (auto i = 0; i < 3; ++i) Check(budget.release(Pool::Shared) && budget.release(Pool::Reserved), "interleaved purpose completions drain independently");
	}
	for (const auto purpose : {static_cast<Purpose>(-1), static_cast<Purpose>(2)}) {
		Check(!budget.available(purpose) && !budget.acquire(purpose) && budget.active() == 0,
			"unknown job purpose fails closed");
	}
	Check(!budget.release(static_cast<Pool>(2)) && budget.active() == 0, "unknown pool release rejected");
	std::cout << Checks << " actual shared decrypt-budget checks, " << Failed
		<< " failed. Native dispatch, RAII release and availability notification require integration testing.\n";
	return Failed ? 1 : 0;
}
