#include "tele/tele_sealed_duress_signout_plan.h"

#include <cstdio>
#include <cstdlib>
#include <limits>

namespace {

using Device = Tele::SealedDuressSignoutDevice;
using Action = Tele::SealedDuressSignoutAction;
using Devices = std::vector<Device>;

auto Checks = 0;

void Check(bool value, const char *name) {
	++Checks;
	if (!value) {
		std::fprintf(stderr, "FAILED after %d checks: %s\n", Checks, name);
		std::exit(1);
	}
}

Tele::SealedDuressSignoutPlan Plan(
		bool here,
		const Devices &selected,
		const Devices &available,
		quint64 actualCurrent = 10,
		int actualCreated = 100) {
	return Tele::NextSealedDuressSignout(10, 100, actualCurrent,
		actualCreated, here, selected, available);
}

void Boundaries() {
	Check(Plan(false, {}, {}).action == Action::Complete, "No selected actions completes");
	Check(Plan(true, {}, {}).action == Action::LogoutLocal, "Explicit local-only rule logs out local");
	Check(Plan(true, {}, {}, 11).action == Action::Changed, "Changed current hash fails closed");
	Check(Plan(true, {}, {}, 10, 101).action == Action::Changed, "Changed current creation fails closed");
	Check(Tele::NextSealedDuressSignout(10, 0, 10, 0, true, {}, {}).action == Action::Changed,
		"Invalid current creation fails closed");
	const auto selected = Devices{ { 20, 200, false } };
	const auto available = Devices{ { 20, 200, false }, { 30, 300, false } };
	Check(Tele::NextSealedDuressSignout(0, 100, 0, 100, true, selected, available).action
		== Action::TerminateDevice, "Matching zero current hash is allowed for Telegram current-session representation");
	const auto exact = Plan(true, selected, available);
	Check(exact.action == Action::TerminateDevice && exact.hash == 20 && exact.created == 200,
		"Only exact selected remote identity is planned before local logout");
	const auto missing = Plan(true, selected, {});
	Check(missing.action == Action::MarkMissingDevice && missing.hash == 20 && missing.created == 200,
		"Missing selected device is acknowledged without a remote request");
	Check(Plan(true, selected, { { 20, 201, false } }).action == Action::Changed,
		"Reused selected hash with changed creation date fails closed");
	Check(Plan(true, { { 20, 200, true } }, available).action == Action::LogoutLocal,
		"Confirmed remote completion permits explicit local action");
	Check(Plan(false, { { 20, 200, true } }, available).action == Action::Complete,
		"Remote-only rule retains local session after completion");
	for (const auto invalid : { Device{ 0, 200, false }, Device{ 10, 200, false },
		Device{ 20, 0, false }, Device{ 20, -1, false } }) {
		Check(Plan(true, { invalid }, available).action == Action::Changed,
			"Invalid selected identity fails closed");
		Check(Plan(true, selected, { invalid }).action == Action::Changed,
			"Invalid fresh remote identity fails closed");
	}
	Check(Plan(true, { { 20, 200, false }, { 20, 200, true } }, available).action == Action::Changed,
		"Duplicate selected hash fails closed even when marked done");
	Check(Plan(true, selected, { { 20, 200, false }, { 20, 200, false } }).action == Action::Changed,
		"Duplicate fresh hash fails closed");
	auto many = Devices();
	for (auto index = 0; index != 256; ++index) {
		many.push_back({ quint64(1000 + index), 1000 + index, false });
	}
	Check(Plan(true, many, many).action == Action::TerminateDevice, "Exactly 256 remote entries are bounded and valid");
	const auto oversized = [&] {
		auto result = many;
		result.push_back({ 2000, 2000, false });
		return result;
	}();
	Check(Plan(true, oversized, many).action == Action::Changed, "Oversized selected list fails closed");
	Check(Plan(true, selected, oversized).action == Action::Changed, "Oversized fresh list fails closed");
	const auto fullHash = std::numeric_limits<quint64>::max();
	Check(Plan(false, { { fullHash, 200, false } }, { { fullHash, 200, false } }).hash == fullHash,
		"Opaque full-width session hash is preserved");
}

void SimulatedOrchestration() {
	auto selected = Devices{ { 20, 200, false }, { 30, 300, false }, { 40, 400, false } };
	auto available = Devices{ { 20, 200, false }, { 40, 400, false }, { 50, 500, false } };
	auto remoteRequests = std::vector<quint64>();
	auto localRequests = 0;
	auto missingAcknowledgments = 0;
	for (auto step = 0; step != 4; ++step) {
		const auto next = Plan(true, selected, available);
		if (next.action == Action::TerminateDevice) {
			Check(next.hash == 20 || next.hash == 40, "Simulated remote requests only target selected available devices");
			remoteRequests.push_back(next.hash);
			std::erase_if(available, [&](const auto &entry) { return entry.hash == next.hash; });
		} else if (next.action == Action::MarkMissingDevice) {
			Check(next.hash == 30, "Simulated missing identity does not start a remote request");
			++missingAcknowledgments;
		} else if (next.action == Action::LogoutLocal) {
			Check(std::all_of(selected.begin(), selected.end(), [](const auto &entry) { return entry.done; }),
				"Simulated local logout occurs only after every selected remote result is acknowledged");
			++localRequests;
			continue;
		} else {
			Check(false, "Unexpected action in successful simulated orchestration");
		}
		for (auto &entry : selected) {
			if (entry.hash == next.hash && entry.created == next.created) entry.done = true;
		}
	}
	Check(remoteRequests == std::vector<quint64>{ 20, 40 }, "Selected remote ordering is stable");
	Check(missingAcknowledgments == 1 && localRequests == 1, "Missing acknowledgment and local action each occur once");
	Check(available.size() == 1 && available[0].hash == 50, "Unselected fresh session remains untouched");
	const auto pending = Devices{ { 20, 200, false } };
	const auto fresh = Devices{ { 20, 200, false } };
	const auto failed = Plan(true, pending, fresh);
	const auto retry = Plan(true, pending, fresh);
	Check(failed.action == Action::TerminateDevice && retry.action == Action::TerminateDevice
		&& failed.hash == retry.hash,
		"Unacknowledged simulated remote failure never permits local logout");
	Check(Plan(true, pending, {}).action == Action::MarkMissingDevice,
		"Fresh snapshot confirms vanished target after an uncertain remote result");
	Check(Plan(true, { { 20, 200, true }, { 40, 400, false } },
		{ { 40, 400, false } }, 11).action == Action::Changed,
		"Current session change between remote steps stops the plan before local logout");
	Check(Plan(true, { { 20, 200, true }, { 40, 400, false } },
		{ { 40, 401, false } }).action == Action::Changed,
		"Remote identity change between steps stops the plan");
	Check(Plan(false, { { 20, 200, true } }, { { 50, 500, false } }).action == Action::Complete,
		"New unselected session is never added to a completed rule");
}

void LocalFallback() {
	const auto clear = Tele::PlanSealedDuressLocalFallback(true, true, false, true);
	Check(clear.action == Action::ClearLocal && clear.hash == 0 && clear.created == 0,
		"Expired selected same-session fallback is explicitly local and has no remote target");
	Check(Tele::PlanSealedDuressLocalFallback(false, true, false, true).action == Action::Changed,
		"Local fallback cannot act without explicit this-device selection");
	Check(Tele::PlanSealedDuressLocalFallback(true, false, false, true).action == Action::Changed,
		"Local fallback cannot clear a new or different login");
	Check(Tele::PlanSealedDuressLocalFallback(true, true, true, true).action == Action::Changed,
		"Already removed local account is not cleared twice");
	Check(Tele::PlanSealedDuressLocalFallback(true, true, false, false).action == Action::Changed,
		"Local fallback cannot clear before its bounded deadline");
	const auto selected = Devices{ { 20, 200, false } };
	const auto available = Devices{ { 20, 200, false } };
	const auto pendingRemote = Plan(true, selected, available);
	Check(pendingRemote.action == Action::TerminateDevice && pendingRemote.hash == 20,
		"Remote work retains its exact target while local fallback is waiting");
	auto simulatedRemoteTargets = std::vector<quint64>();
	auto simulatedLocalClears = 0;
	for (const auto step : { pendingRemote, clear }) {
		if (step.action == Action::TerminateDevice) {
			Check(step.hash != 0 && step.hash != 10,
				"Simulated remote dispatcher never uses zero or current-session hash");
			simulatedRemoteTargets.push_back(step.hash);
		} else if (step.action == Action::ClearLocal) {
			++simulatedLocalClears;
		}
	}
	Check(simulatedRemoteTargets == std::vector<quint64>{ 20 } && simulatedLocalClears == 1,
		"Bounded local cleanup remains separate from selected remote revocations");
	Check(Tele::PlanSealedDuressLocalFallback(true, true, true, true).action != Action::ClearLocal,
		"Simulated completed local cleanup is not retried");
	Check(Plan(true, selected, available, 11).action == Action::Changed,
		"A replacement login also stops pending remote orchestration");
	Check(Plan(true, { { 0, 200, false } }, available).action != Action::TerminateDevice,
		"A zero target is never turned into a remote termination request");
	Check(Plan(true, { { 10, 100, false } }, available).action != Action::TerminateDevice,
		"Current session is never turned into a remote termination request");
}

}

int main() {
	Boundaries();
	SimulatedOrchestration();
	LocalFallback();
	std::printf("%d actual signout-planner checks passed. No native RPC or logout was performed.\n", Checks);
}
