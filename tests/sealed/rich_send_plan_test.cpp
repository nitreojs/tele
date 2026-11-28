#include "tele/tele_sealed_rich_send_plan.h"

#include <array>
#include <iostream>
#include <limits>
#include <vector>

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

bool Equal(const Tele::SealedRichSendCursor &a, const Tele::SealedRichSendCursor &b) {
	return a.count == b.count && a.next == b.next
		&& a.accepted == b.accepted && a.canceled == b.canceled;
}

}

int main() {
	using namespace Tele;
	for (auto count = 1; count <= 64; ++count) {
		auto cursor = SealedRichSendCursor{ .count = count };
		Check(!TakeSealedRichTail(cursor), "tail waits for first canonical message acceptance");
		Check(AcceptSealedRichFirst(cursor), "valid group accepts first message");
		Check(!AcceptSealedRichFirst(cursor), "duplicate first acceptance cannot release tail twice");
		auto visited = std::array<int, 64>();
		while (const auto part = TakeSealedRichTail(cursor)) {
			Check(*part >= 1 && *part < count, "tail index stays within group");
			Check(cursor.next == *part + 1, "cursor advances before reentrant dispatch");
			Check(++visited[*part] == 1, "each tail index released exactly once");
			Check(!AcceptSealedRichFirst(cursor), "reentrant first callback cannot reset cursor");
		}
		for (auto index = 1; index < count; ++index) Check(visited[index] == 1, "all expected tail indices dispatched");
		const auto end = cursor;
		Check(!TakeSealedRichTail(cursor) && Equal(cursor, end), "completed cursor stays unchanged");
		Check(cursor.next == count, "completed cursor has exact terminal index");
	}
	for (const auto count : {std::numeric_limits<int>::min(), -1, 0, 65, std::numeric_limits<int>::max()}) {
		auto cursor = SealedRichSendCursor{ .count = count };
		const auto before = cursor;
		Check(!AcceptSealedRichFirst(cursor) && Equal(cursor, before), "malformed count cannot accept or mutate");
		cursor.accepted = true;
		const auto accepted = cursor;
		Check(!TakeSealedRichTail(cursor) && Equal(cursor, accepted), "malformed accepted count cannot dispatch or mutate");
	}
	for (const auto next : {std::numeric_limits<int>::min(), -1, 0, 2, 64, std::numeric_limits<int>::max()}) {
		auto cursor = SealedRichSendCursor{ .count = 64, .next = next };
		const auto before = cursor;
		Check(!AcceptSealedRichFirst(cursor) && Equal(cursor, before), "unexpected initial next index cannot release group");
	}
	for (const auto next : {std::numeric_limits<int>::min(), -1, 0, 64, 65, std::numeric_limits<int>::max()}) {
		auto cursor = SealedRichSendCursor{ .count = 64, .next = next, .accepted = true };
		const auto before = cursor;
		Check(!TakeSealedRichTail(cursor) && Equal(cursor, before), "invalid tail index rejected before increment or overflow");
	}
	{
		auto cursor = SealedRichSendCursor{ .count = 64, .canceled = true };
		const auto before = cursor;
		Check(!AcceptSealedRichFirst(cursor) && !TakeSealedRichTail(cursor) && Equal(cursor, before), "pre-accept cancellation prevents all release");
		cursor.canceled = false;
		Check(AcceptSealedRichFirst(cursor), "uncanceled fixture accepts first message");
		cursor.canceled = true;
		const auto canceled = cursor;
		Check(!TakeSealedRichTail(cursor) && Equal(cursor, canceled), "post-accept cancellation prevents first tail part");
	}
	{
		auto cursor = SealedRichSendCursor{ .count = 64 };
		Check(AcceptSealedRichFirst(cursor), "mid-tail fixture accepts first");
		Check(TakeSealedRichTail(cursor) == 1 && TakeSealedRichTail(cursor) == 2, "first two tail indices released in order");
		cursor.canceled = true;
		const auto canceled = cursor;
		for (auto i = 0; i < 4; ++i) Check(!TakeSealedRichTail(cursor) && Equal(cursor, canceled), "mid-tail cancellation cannot resume or advance");
	}
	{
		auto cursor = SealedRichSendCursor{ .count = 5 };
		auto dispatched = std::vector<int>();
		Check(AcceptSealedRichFirst(cursor), "reentrant dispatcher accepts first");
		while (const auto index = TakeSealedRichTail(cursor)) {
			dispatched.push_back(*index);
			if (const auto reentrant = TakeSealedRichTail(cursor)) dispatched.push_back(*reentrant);
		}
		Check(dispatched == std::vector<int>{1, 2, 3, 4}, "nested tail callbacks still dispatch every index once");
	}
	Check(SealedRichSendCapacity(0, 0, 0, 1, 3202), "empty queue accepts one transport part");
	Check(SealedRichSendCapacity(15, 960, 0, 64, 64 * 3202), "last group and exact1024 part bound accepted");
	Check(!SealedRichSendCapacity(16, 0, 0, 1, 3202), "full group budget rejects next group");
	Check(!SealedRichSendCapacity(std::numeric_limits<std::size_t>::max(), 0, 0, 1, 3202), "extreme group count safely rejected");
	Check(SealedRichSendCapacity(0, 1023, 0, 1, 3202), "exact final part budget accepted");
	Check(!SealedRichSendCapacity(0, 1024, 0, 1, 3202), "full part budget rejects additional part");
	Check(!SealedRichSendCapacity(0, 961, 0, 64, 64 * 3202), "aggregate part overflow rejected");
	for (const auto parts : {std::numeric_limits<int>::min(), -1, 1025, std::numeric_limits<int>::max()}) {
		Check(!SealedRichSendCapacity(0, parts, 0, 1, 3202), "invalid accumulated part count rejected");
	}
	for (const auto parts : {std::numeric_limits<int>::min(), -1, 0, 65, std::numeric_limits<int>::max()}) {
		Check(!SealedRichSendCapacity(0, 0, 0, parts, 3202), "invalid incoming part count rejected");
	}
	Check(SealedRichSendCapacity(0, 0, kSealedRichSendByteLimit - 3202, 1, 3202), "exact combined byte budget accepted");
	Check(!SealedRichSendCapacity(0, 0, kSealedRichSendByteLimit - 3201, 1, 3202), "combined byte budget one over rejected");
	Check(SealedRichSendCapacity(0, 0, 0, 1, kSealedRichSendByteLimit), "exact incoming byte bound accepted");
	Check(!SealedRichSendCapacity(0, 0, 0, 1, kSealedRichSendByteLimit + 1), "incoming byte budget one over rejected");
	Check(SealedRichSendCapacity(0, 0, kSealedRichSendByteLimit, 1, 0), "zero-byte reservation does not overflow an exact byte bound");
	for (const auto bytes : {std::numeric_limits<std::int64_t>::min(), std::int64_t(-1), kSealedRichSendByteLimit + 1, std::numeric_limits<std::int64_t>::max()}) {
		Check(!SealedRichSendCapacity(0, 0, bytes, 1, 1), "invalid accumulated byte count rejected before subtraction");
		Check(!SealedRichSendCapacity(0, 0, 0, 1, bytes), "invalid incoming bytes rejected before subtraction");
	}
	Check(!SealedRichSendCapacity(0, 0, std::numeric_limits<std::int64_t>::max(), 1, std::numeric_limits<std::int64_t>::max()), "simultaneous extreme sizes cannot overflow capacity arithmetic");
	std::cout << Checks << " actual rich-send planner checks, " << Failed << " failed. Native queue routing and lifecycle require application testing.\n";
	return Failed ? 1 : 0;
}
