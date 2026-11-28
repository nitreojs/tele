#include "tele/tele_sealed_duress_store.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLockFile>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTextStream>

#include <cstdlib>

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	auto checks = 0;
	const auto require = [&](bool pass, const char *name) {
		++checks;
		if (!pass) {
			QTextStream(stderr) << "FAILED: " << name << '\n';
			std::exit(1);
		}
	};
	using Error = Tele::SealedDuressError;
	const auto context = QByteArray("profile alpha");
	const auto password = QByteArray("54682719");
	const auto verifier = Tele::MakeSealedDuressVerifier(password, context);
	require(verifier && verifier->salt.size() == 32 && verifier->digest.size() == 32, "verifier sizes");
	require(Tele::CheckSealedDuressVerifier(password, context, *verifier), "correct passphrase");
	require(!Tele::CheckSealedDuressVerifier("wrong", context, *verifier), "wrong passphrase");
	require(!Tele::CheckSealedDuressVerifier(password, "profile beta", *verifier), "wrong profile");
	require(!Tele::MakeSealedDuressVerifier({}, context), "empty password");
	require(!Tele::MakeSealedDuressVerifier(QByteArray(1025, 'a'), context), "password bound");
	require(!Tele::MakeSealedDuressVerifier(QByteArray::fromHex("c080"), context), "invalid UTF8 password");
	require(!Tele::MakeSealedDuressVerifier(password, QByteArray(257, 'a')), "context bound");
	QTemporaryDir temporary;
	require(temporary.isValid(), "temporary root");
	const auto root = temporary.path();
	const auto write = [&](const QString &relative, const QByteArray &bytes) {
		QFile file(QDir(root).filePath(relative));
		require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "write fixture");
	};
	const auto read = [&](const QString &relative) {
		QFile file(QDir(root).filePath(relative));
		return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
	};
	const auto exists = [&](const QString &relative) {
		return QFileInfo::exists(QDir(root).filePath(relative));
	};
	const auto account = [&](const Tele::SealedDuressState &state, const QString &name) -> const Tele::SealedDuressAccount* {
		for (const auto &entry : state.accounts) {
			if (entry.relativeDirectory == name) return &entry;
		}
		return nullptr;
	};
	require(Tele::LoadSealedDuress(root, context).error == Error::Missing, "missing state");
	for (const auto &name : {"../user_escape", "user_a/x", "user_a\\x", "user_", "other_a", "user_a.", "user_a ", "user_a:ads"}) {
		require(Tele::EnrollSealedDuress(root, context, password, { QString::fromLatin1(name) }).error == Error::InvalidInput, "invalid manifest path");
	}
	QStringList tooMany;
	for (auto i = 0; i <= Tele::kSealedDuressAccountLimit; ++i) tooMany += QStringLiteral("user_%1").arg(i);
	require(Tele::EnrollSealedDuress(root, context, password, tooMany).error == Error::InvalidInput, "manifest count bound");
	auto state = Tele::EnrollSealedDuressWithVerifier(root, context, *verifier, { "user_a", "user_b" });
	require(state.error == Error::None && state.state && state.state->armed && !state.state->quarantined, "enrollment");
	require(state.state->generation.size() == 16 && state.state->accounts.size() == 2, "state bounds");
	require(state.state->activationId.isEmpty() && state.state->activatedAtUTC == 0, "no activation before trigger");
	auto invalidVerifier = *verifier;
	invalidVerifier.digest.chop(1);
	require(Tele::EnrollSealedDuressWithVerifier(root, context, invalidVerifier, { "user_a" }, state.state->generation).error == Error::InvalidInput, "two-phase verifier sizes");
	require(Tele::EnrollSealedDuress(root, context, password, { "user_a" }).error == Error::Changed, "enroll CAS");
	const auto firstGeneration = state.state->generation;
	state = Tele::SetSealedDuressArmed(root, context, false, firstGeneration);
	require(state.error == Error::None && !state.state->armed && state.state->generation != firstGeneration, "disarm changes generation");
	require(Tele::TriggerSealedDuress(root, context, state.state->generation).error == Error::InvalidInput, "disarmed trigger refused");
	require(Tele::SetSealedDuressArmed(root, context, true, firstGeneration).error == Error::Changed, "stale mutation");
	state = Tele::SetSealedDuressArmed(root, context, true, state.state->generation);
	require(state.error == Error::None && state.state->armed, "arm");
	require(Tele::RegisterSealedDuressAccount(root, context, "user_A", state.state->generation).error == Error::InvalidInput, "case alias rejected");
	for (const auto &name : { "user_a", "user_b" }) {
		require(QDir(root).mkpath(name), "account fixture directory");
		for (const auto suffix : { 's', '0', '1' }) write(QString::fromLatin1(name) + "/tele_sealed_keys" + QChar(suffix), "PRIVATE KEY FIXTURE");
		write(QString::fromLatin1(name) + "/unrelated", "KEEP");
	}
	state = Tele::TriggerSealedDuress(root, context, state.state->generation);
	require(state.error == Error::None && state.state->quarantined, "trigger persisted quarantine");
	const auto firstActivation = state.state->activationId;
	const auto firstActivationTime = state.state->activatedAtUTC;
	require(firstActivation.size() == 16 && firstActivationTime > 0, "trigger records activation");
	for (const auto &entry : state.state->accounts) {
		require(entry.blocked && !entry.cleanupPending, "account stays blocked after cleanup");
		for (const auto suffix : { 's', '0', '1' }) require(!exists(entry.relativeDirectory + "/tele_sealed_keys" + QChar(suffix)), "all key variants removed");
		require(read(entry.relativeDirectory + "/unrelated") == "KEEP", "unrelated file preserved");
	}
	auto loaded = Tele::LoadSealedDuress(root, context);
	require(loaded.error == Error::None && loaded.state->quarantined, "quarantine survives reload");
	state = Tele::SetSealedDuressArmed(root, context, false, state.state->generation);
	require(state.error == Error::None && state.state->quarantined && account(*state.state, "user_a")->blocked, "disarm preserves deny");
	state = Tele::RegisterSealedDuressAccount(root, context, "user_c", state.state->generation);
	require(state.error == Error::None && !account(*state.state, "user_c")->blocked, "new keyless account stays available");
	require(state.state->activationId == firstActivation && state.state->activatedAtUTC == firstActivationTime, "registration preserves activation");
	state = Tele::EnrollSealedDuress(root, context, password, { "user_a" }, state.state->generation);
	require(state.error == Error::None && state.state->accounts.size() == 3 && account(*state.state, "user_b")->blocked, "reenroll preserves omitted blocked account");
	require(state.state->activationId == firstActivation && state.state->activatedAtUTC == firstActivationTime, "enrollment preserves activation");
	auto called = false;
	auto recovered = Tele::RecoverSealedDuressAccount(root, context, "user_a", state.state->generation, [&] {
		called = true;
		const auto inside = Tele::LoadSealedDuress(root, context);
		require(inside.state && account(*inside.state, "user_a")->blocked, "writer observes persisted deny without lock deadlock");
		write("user_a/tele_sealed_keyss", "RESTORED");
		return true;
	});
	require(called && recovered.error == Error::None && !account(*recovered.state, "user_a")->blocked, "authenticated writer unblocks one account");
	require(account(*recovered.state, "user_b")->blocked && !account(*recovered.state, "user_c")->blocked && recovered.state->quarantined, "known blocked account stays denied and new account stays available");
	require(recovered.state->activationId == firstActivation && recovered.state->activatedAtUTC == firstActivationTime, "recovery preserves activation");
	recovered = Tele::RetrySealedDuressCleanup(root, context, recovered.state->generation);
	require(recovered.error == Error::None && read("user_a/tele_sealed_keyss") == "RESTORED", "cleanup never removes recovered key");
	auto failed = Tele::RecoverSealedDuressAccount(root, context, "user_b", recovered.state->generation, [] { return false; });
	require(failed.error == Error::WriterFailed && account(*failed.state, "user_b")->blocked, "failed writer remains blocked");
	require(QDir(root).mkpath("user_bad/tele_sealed_keyss"), "cleanup obstruction");
	state = Tele::RegisterSealedDuressAccount(root, context, "user_bad", failed.state->generation);
	state = Tele::TriggerSealedDuress(root, context, state.state->generation);
	require(state.error == Error::CleanupFailed && state.state && state.state->quarantined && account(*state.state, "user_bad")->cleanupPending, "cleanup failure retains persisted deny");
	require(state.state->activationId != firstActivation && state.state->activatedAtUTC >= firstActivationTime, "new trigger replaces activation");
	called = false;
	failed = Tele::RecoverSealedDuressAccount(root, context, "user_bad", state.state->generation, [&] { called = true; return true; });
	require(failed.error == Error::CleanupFailed && !called, "cleanup must finish before recovery writer");
	require(QDir(root).rmdir("user_bad/tele_sealed_keyss"), "remove temporary obstruction");
	state = Tele::RetrySealedDuressCleanup(root, context, failed.state->generation);
	require(state.error == Error::None && !account(*state.state, "user_bad")->cleanupPending, "cleanup retry");
	const auto names = QDir(root).entryList({ "tele_sealed_duress_*.bin" }, QDir::Files);
	require(names.size() == 1, "one context-bound state file");
	const auto stateName = names.front();
	const auto original = read(stateName);
	const auto rehash = [&](QByteArray bytes) {
		bytes.chop(32);
		bytes += QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
		return bytes;
	};
	for (auto n : { 0, 1, 7, 8, 42, 124, 166 }) {
		write(stateName, original.left(n));
		require(Tele::LoadSealedDuress(root, context).error == Error::Corrupt, "truncated state");
	}
	for (auto offset : { 8, 9, 41, 42, 75, 147, 148, 149, 150 }) {
		auto bytes = original;
		bytes[offset] = char(-1);
		write(stateName, rehash(bytes));
		require(Tele::LoadSealedDuress(root, context).error == Error::Corrupt, "structurally invalid state with valid checksum");
	}
	for (const auto offset : { 59, 75 }) {
		auto bytes = original;
		const auto count = (offset == 59) ? 16 : 8;
		for (auto i = 0; i != count; ++i) bytes[offset + i] = char(0);
		write(stateName, rehash(bytes));
		require(Tele::LoadSealedDuress(root, context).error == Error::Corrupt, "activation ID/time consistency");
	}
	write(stateName, QByteArray(1024 * 1024 + 1, 'x'));
	require(Tele::LoadSealedDuress(root, context).error == Error::Corrupt, "oversized state");
	write(stateName, original);
	QLockFile lock(QDir(root).filePath(stateName + ".lock"));
	require(lock.tryLock(0), "test transaction lock");
	require(Tele::SetSealedDuressArmed(root, context, false, state.state->generation).error == Error::Busy, "concurrent mutation refused");
	require(Tele::LoadSealedDuress(root, context).error == Error::None, "load has no mutation lock");
	lock.unlock();
	called = false;
	recovered = Tele::RecoverSealedDuressAccount(root, context, "user_b", state.state->generation, [&] {
		called = true;
		auto replaced = read(stateName);
		replaced[43] ^= 1;
		write(stateName, rehash(replaced));
		return true;
	});
	require(called && recovered.error == Error::Changed && account(*recovered.state, "user_b")->blocked, "recovery rereads generation after writer");
	write(stateName, original);
	{
		QTemporaryDir firstUse;
		require(firstUse.isValid(), "first-use temporary root");
		require(QDir(firstUse.path()).mkpath("user_a") && QDir(firstUse.path()).mkpath("user_b"), "first-use account directories");
		QFile existing(QDir(firstUse.path()).filePath("user_a/tele_sealed_keyss"));
		require(existing.open(QIODevice::WriteOnly) && existing.write("OLD KEY") == 7, "existing identity fixture");
		existing.close();
		auto first = Tele::EnrollSealedDuressWithVerifier(firstUse.path(), context, *verifier, { "user_a", "user_b" });
		require(first.error == Error::None, "mixed used and unused enrollment");
		first = Tele::TriggerSealedDuress(firstUse.path(), context, first.state->generation);
		require(first.error == Error::None && account(*first.state, "user_a")->blocked, "used account blocked");
		require(!account(*first.state, "user_b")->blocked && !account(*first.state, "user_b")->cleanupPending, "unused B needs no nonexistent backup");
		first = Tele::EnrollSealedDuressWithVerifier(firstUse.path(), context, *verifier, { "user_a", "user_b" }, first.state->generation);
		require(first.error == Error::None && account(*first.state, "user_a")->blocked, "reenrollment does not forget wiped A");
		first = Tele::RegisterSealedDuressAccount(firstUse.path(), context, "user_c", first.state->generation);
		require(first.error == Error::None && !account(*first.state, "user_c")->blocked, "new C may create its first identity");
		QFile fresh(QDir(firstUse.path()).filePath("user_b/tele_sealed_keys0"));
		require(fresh.open(QIODevice::WriteOnly) && fresh.write("NEW KEY") == 7, "unused B creates a fresh identity fixture");
		fresh.close();
		first = Tele::TriggerSealedDuress(firstUse.path(), context, first.state->generation);
		require(first.error == Error::None && account(*first.state, "user_a")->blocked
			&& account(*first.state, "user_b")->blocked && !account(*first.state, "user_c")->blocked, "next trigger protects B while retaining wiped A");
		QFile unsafe(QDir(firstUse.path()).filePath("user_unsafe"));
		require(unsafe.open(QIODevice::WriteOnly) && unsafe.write("KEEP") == 4, "unsafe account path fixture");
		unsafe.close();
		first = Tele::RegisterSealedDuressAccount(firstUse.path(), context, "user_unsafe", first.state->generation);
		require(first.error == Error::None && account(*first.state, "user_unsafe")->blocked, "unsafe existing account path remains denied");
		first = Tele::TriggerSealedDuress(firstUse.path(), context, first.state->generation);
		require(first.error == Error::CleanupFailed && account(*first.state, "user_unsafe")->cleanupPending, "unsafe path remains blocked after failed cleanup");
		require(QFileInfo::exists(unsafe.fileName()), "unsafe non-key path was not deleted");
	}
	{
		QTemporaryDir keyless;
		require(keyless.isValid(), "keyless temporary root");
		auto first = Tele::EnrollSealedDuressWithVerifier(keyless.path(), context, *verifier, { "user_unused" });
		require(first.error == Error::None, "keyless enrollment");
		first = Tele::TriggerSealedDuress(keyless.path(), context, first.state->generation);
		require(first.error == Error::None && !first.state->quarantined
			&& !account(*first.state, "user_unused")->blocked, "entirely keyless profile has no recovery deadlock");
		require(first.state->activationId.size() == 16 && first.state->activatedAtUTC > 0, "keyless trigger still records optional-action activation");
	}
	QTextStream(stdout) << checks << " duress-store checks passed.\n";
	return 0;
}
