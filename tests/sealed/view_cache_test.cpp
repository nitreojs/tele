#include "tele/tele_sealed_view_cache.h"

#include <cstdlib>
#include <iostream>

int main() {
	using Cache = Tele::details::SealedViewPlaintextCache;
	using Proof = Tele::details::SealedViewProof;
	auto checks = 0;
	const auto require = [&](bool value, const char *name) {
		++checks;
		if (!value) {
			std::cerr << "FAILED: " << name << '\n';
			std::exit(1);
		}
	};
	auto cache = Cache();
	require(!cache.find("absent") && cache.size() == 0 && cache.bytes() == 0, "empty cache");
	auto source = QByteArray("private fixture");
	require(cache.put("a", source).empty(), "first insertion");
	source[0] = 'X';
	require(cache.find("a") && *cache.find("a") == "private fixture", "cache owns independent plaintext bytes");
	cache.erase("a");
	require(!cache.find("a") && !cache.size() && !cache.bytes(), "erase removes plaintext and accounting");
	require(source == "Xrivate fixture", "cleanup never alters the caller's bytes");
	for (auto i = 0; i != Cache::EntryLimit; ++i) {
		require(cache.put(QByteArray::number(i), "x").empty(), "within entry bound");
	}
	require(cache.size() == Cache::EntryLimit && cache.bytes() == Cache::EntryLimit, "entry bound accounting");
	require(cache.find("0") != nullptr, "touch least-recent entry");
	const auto evicted = cache.put("next", "new");
	require(evicted.size() == 1 && evicted.front() == "1", "least recently used entry evicted");
	require(cache.find("0") && !cache.find("1") && cache.find("next"), "recent plaintext survives bounded eviction");
	require(cache.size() == Cache::EntryLimit && cache.bytes() == Cache::EntryLimit + 2, "replacement size accounted");
	cache.clear();
	require(!cache.size() && !cache.bytes() && !cache.find("next"), "clear drops every entry");
	const auto large = QByteArray(Cache::ByteLimit / 2 + 1, 'L');
	require(cache.put("large-a", large).empty(), "first byte-bounded entry");
	const auto removed = cache.put("large-b", large);
	require(removed.size() == 1 && removed.front() == "large-a" && !cache.find("large-a"), "byte budget evicts before exceeding bound");
	require(cache.bytes() == large.size() && cache.bytes() <= Cache::ByteLimit, "byte bound retained");
	require(cache.put("oversize", QByteArray(Cache::ByteLimit + 1, 'x')).empty() && !cache.find("oversize"), "oversized plaintext rejected");
	require(cache.find("large-b") != nullptr, "rejected value does not evict valid content");
	require(cache.put(QByteArray(257, 'k'), "x").empty() && cache.size() == 1, "oversized cache key rejected");
	require(cache.put(QByteArray(), "x").empty() && cache.size() == 1, "empty cache key rejected");
	cache.clear();
	require(cache.put("empty", QByteArray()).empty() && cache.find("empty") && cache.find("empty")->isEmpty(), "authenticated empty plaintext is distinct from a cache miss");
	const auto replaced = cache.put("empty", "replacement");
	require(replaced.empty() && cache.size() == 1 && cache.bytes() == 11 && *cache.find("empty") == "replacement", "same-key replacement preserves bounds");
	cache.clear();
	auto proof = Proof{
		.epoch = 42,
		.cipherDigest = QByteArray(32, 'd'),
		.ownPublic = QByteArray(Tele::kSealedPublicKeySize, 'o'),
		.expectedPublic = QByteArray(Tele::kSealedPublicKeySize, 's'),
		.context = QByteArray(17, 'c'),
	};
	require(proof.matches(proof), "same authenticated publication stamp");
	auto changed = proof;
	++changed.epoch;
	require(!proof.matches(changed) && !changed.matches(proof), "lock mode or key epoch change rejects stale result");
	changed = proof;
	changed.cipherDigest[0] = 'e';
	require(!proof.matches(changed), "edited ciphertext rejects stale result");
	changed = proof;
	changed.ownPublic[0] = 'p';
	require(!proof.matches(changed), "identity change rejects stale result");
	changed = proof;
	changed.expectedPublic[0] = 't';
	require(!proof.matches(changed), "verified sender pin change rejects stale result");
	changed = proof;
	changed.context[0] = 'n';
	require(!proof.matches(changed), "account network or direction context change rejects stale result");
	for (auto field : { 0, 1, 2, 3, 4 }) {
		changed = proof;
		switch (field) {
		case 0: changed.epoch = 0; break;
		case 1: changed.cipherDigest.chop(1); break;
		case 2: changed.ownPublic.chop(1); break;
		case 3: changed.expectedPublic.chop(1); break;
		case 4: changed.context.chop(1); break;
		}
		require(!changed.matches(changed), "incomplete stamp cannot authorize publication");
	}
	std::cout << checks << " view cache and publication-stamp checks passed. Native visibility, view clearing, selection and clipboard routes require application testing.\n";
}
