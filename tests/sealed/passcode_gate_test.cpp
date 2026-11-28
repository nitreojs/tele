#include "tele/tele_sealed_passcode_gate.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>

int main() {
	int checks = 0;
	const auto require = [&](bool value, const char *name) {
		++checks;
		if (!value) {
			std::cerr << "FAILED: " << name << '\n';
			std::exit(1);
		}
	};
	require(Tele::SealedNativePasscodeAvailable(true), "inactive true preserved");
	require(!Tele::SealedNativePasscodeAvailable(false), "inactive false preserved");
	for (const auto correct : { false, true }) {
		for (const auto matched : { false, true }) {
			auto absent = Tele::SealedPasscodeGate();
			require(!absent.mayTrigger(correct, matched), "missing native proof refuses cleanup");
			auto failed = Tele::SealedPasscodeGate();
			{
				const auto scope = Tele::SealedPasscodeGateScope(failed);
				require(!Tele::SealedNativePasscodeAvailable(false), "failed native key preserved");
			}
			require(!failed.mayTrigger(correct, matched), "failed native derivation refuses cleanup");
			auto available = Tele::SealedPasscodeGate();
			{
				const auto scope = Tele::SealedPasscodeGateScope(available);
				require(Tele::SealedNativePasscodeAvailable(true), "available native key preserved");
			}
			require(available.mayTrigger(correct, matched) == (!correct && matched), "only verified rejection with matching duress permits cleanup");
		}
	}
	auto outer = Tele::SealedPasscodeGate();
	auto inner = Tele::SealedPasscodeGate();
	{
		const auto outerScope = Tele::SealedPasscodeGateScope(outer);
		require(Tele::SealedNativePasscodeAvailable(true), "outer proof");
		{
			const auto innerScope = Tele::SealedPasscodeGateScope(inner);
			require(!Tele::SealedNativePasscodeAvailable(false), "nested failure");
		}
		require(Tele::SealedNativePasscodeAvailable(true), "outer scope restored");
	}
	require(outer.mayTrigger(false, true), "nested failure does not contaminate parent");
	require(!inner.mayTrigger(false, true), "nested failure remains blocked");
	require(!Tele::SealedNativePasscodeAvailable(false), "scope exit restores inactive behavior");
	require(outer.mayTrigger(false, true), "inactive call cannot alter completed proof");
	for (const auto first : { false, true }) {
		auto gate = Tele::SealedPasscodeGate();
		{
			const auto scope = Tele::SealedPasscodeGateScope(gate);
			require(Tele::SealedNativePasscodeAvailable(first) == first, "first result preserved");
			require(Tele::SealedNativePasscodeAvailable(!first) != first, "second result preserved");
		}
		require(!gate.mayTrigger(false, true), "any failed key check refuses cleanup");
	}
	std::cout << checks << " native-passcode gate checks passed. Native KDF and lock-screen integration require separate testing.\n";
}
