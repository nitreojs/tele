#include "tele/tele_sealed_registry.h"
#include "tele/tele_sealed_duress_store.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLockFile>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTextStream>

#include <cstdlib>
#include <filesystem>

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	auto checks = 0;
	const auto require = [&](bool value, const char *name) {
		++checks;
		if (!value) {
			QTextStream(stderr) << "FAILED: " << name << '\n';
			std::exit(1);
		}
	};
	using Error = Tele::SealedRegistryError;
	QTemporaryDir temporary;
	require(temporary.isValid(), "temporary root");
	const auto root = temporary.path();
	const auto context = Tele::SealedRegistryProfileContext(QStringLiteral("test profile"));
	const auto path = [&](const QString &relative) { return QDir(root).filePath(relative); };
	const auto write = [&](const QString &relative, const QByteArray &bytes) {
		QFile file(path(relative));
		require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "write temporary fixture");
	};
	const auto read = [&](const QString &relative) {
		QFile file(path(relative));
		return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
	};
	const auto stateName = [&] {
		const auto names = QDir(root).entryList({ QStringLiteral("tele_sealed_registry_*.bin") }, QDir::Files);
		require(names.size() == 1, "one context registry");
		return names.front();
	};
	auto loaded = Tele::LoadSealedRegistry(root, context);
	require(loaded.error == Error::None && !loaded.exists && loaded.directories.isEmpty(), "missing registry is neutral");
	require(!Tele::RegistryHasKeys(root, context), "no warning without keys");
	require(!Tele::SealedKeyFilesPresentAt(root) && !Tele::SealedKeyFilesPresentAt(path("missing-account")), "exact known filenames absent without directory scans");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(root, context);
		require(bool(guard), "no-registry unprotection allowed");
		require(Tele::RegistryRegister(root, context, "user_race") == Error::Busy, "cleanup guard blocks in-process registration even before first write");
	}
	require(QDir(root).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty(), "default cleanup creates no files");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(path("absent-root"), context);
		require(bool(guard) && !QFileInfo::exists(path("absent-root")), "absent unused root stays unchanged");
	}
	write("unused-file-root", "UNRELATED");
	require(!Tele::SealedKeyFilesPresentAt(path("unused-file-root")), "non-directory account path contains no key filename");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(path("unused-file-root"), context);
		require(bool(guard) && read("unused-file-root") == "UNRELATED", "non-directory unused root stays unchanged");
	}
	require(QFile::remove(path("unused-file-root")), "remove non-directory root fixture");
	{
		QTemporaryDir linkTarget;
		require(linkTarget.isValid(), "symlink target fixture");
		const auto link = path("unused-linked-root");
		auto error = std::error_code();
#ifdef Q_OS_WIN
		std::filesystem::create_directory_symlink(linkTarget.path().toStdWString(), link.toStdWString(), error);
#else
		std::filesystem::create_directory_symlink(linkTarget.path().toStdString(), link.toStdString(), error);
#endif
#ifdef Q_OS_WIN
		if (error) {
			auto process = QProcess();
			process.setNativeArguments(QStringLiteral("/d /c mklink /J \"") + QDir::toNativeSeparators(link)
				+ QStringLiteral("\" \"") + QDir::toNativeSeparators(linkTarget.path()) + QStringLiteral("\""));
			process.start(QStringLiteral("cmd.exe"), {});
			if (process.waitForFinished() && process.exitCode() == 0) {
				error.clear();
			}
		}
#endif
		if (!error) {
			{
				auto guard = Tele::PrepareSealedRegistryCleanup(link, context);
				require(bool(guard), "unused linked root does not change upstream unprotection");
			}
			require(QDir(linkTarget.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty(), "unused linked root creates no registry or lock");
			require(Tele::RegistryRegister(linkTarget.path(), context, "user_a") == Error::None, "existing registry behind link fixture");
			{
				auto guard = Tele::PrepareSealedRegistryCleanup(link, context);
				require(!guard, "existing registry behind unsafe linked root fails closed");
			}
			QTemporaryDir accountRoot;
			require(accountRoot.isValid(), "account-link root fixture");
			const auto accountLink = QDir(accountRoot.path()).filePath("user_link");
#ifdef Q_OS_WIN
			auto accountProcess = QProcess();
			accountProcess.setNativeArguments(QStringLiteral("/d /c mklink /J \"") + QDir::toNativeSeparators(accountLink)
				+ QStringLiteral("\" \"") + QDir::toNativeSeparators(linkTarget.path()) + QStringLiteral("\""));
			accountProcess.start(QStringLiteral("cmd.exe"), {});
			require(accountProcess.waitForFinished() && accountProcess.exitCode() == 0, "account directory junction fixture");
#else
			std::filesystem::create_directory_symlink(linkTarget.path().toStdString(), accountLink.toStdString(), error);
			require(!error, "account directory symlink fixture");
#endif
			auto writerCalled = false;
			require(Tele::RegistryWrite(accountRoot.path(), context, "user_link", [&] { writerCalled = true; return true; }) == Error::Unavailable && !writerCalled, "private writer refuses an unsafe account directory");
			require(Tele::LoadSealedRegistry(accountRoot.path(), context).directories.contains("user_link"), "unsafe known directory is durable before refusal");
			require(Tele::EraseAllRegistered(accountRoot.path(), context) == Error::EraseFailed, "unsafe registered account blocks unprotection");
			require(QDir().rmdir(accountLink), "remove only the temporary account directory link");
			require(Tele::EraseAllRegistered(accountRoot.path(), context) == Error::None, "removed unsafe directory can be retried safely");
			require(QDir().rmdir(link), "remove only the temporary directory link");
		} else {
			QTextStream(stdout) << "Directory-symlink fixture unavailable: " << QString::fromStdString(error.message()) << '\n';
		}
	}
	for (const auto &directory : { "../user_other", "user_", "user_a/x", "user_a\\x", "other", "user_a.", "user_a:stream" }) {
		require(Tele::RegistryRegister(root, context, QString::fromLatin1(directory)) == Error::InvalidInput, "invalid directory rejected");
	}
	require(Tele::RegistryRegister(root, QByteArray(), "user_a") == Error::InvalidInput, "empty context rejected");
	require(QDir(root).mkpath("user_a") && QDir(root).mkpath("user_b"), "account directories");
	auto called = false;
	require(Tele::RegistryWrite(root, context, "user_a", [&] {
		called = true;
		const auto state = Tele::LoadSealedRegistry(root, context);
		require(state.error == Error::None && state.exists && state.directories.contains("user_a"), "registration is durable before private writer");
		require(Tele::EraseAllRegistered(root, context) == Error::Busy, "writer lease excludes unprotection");
		for (const auto suffix : { 's', '0', '1' }) write(QStringLiteral("user_a/tele_sealed_keys") + QChar(suffix), "PRIVATE FIXTURE");
		return true;
	}) == Error::None && called, "registered private write");
	require(Tele::RegistryHasKeys(root, context), "existing private keys detected");
	const auto registry = stateName();
	const auto first = read(registry);
	require(Tele::RegistryRegister(root, context, "user_a") == Error::None && read(registry) == first, "idempotent registration");
	require(Tele::RegistryRegister(root, context, "user_A") == Error::InvalidInput, "case aliases rejected");
	write("user_a/authorization", "KEEP AUTHORIZATION");
	write("user_a/wallet", "KEEP WALLET");
	require(Tele::EraseAccount(root, context, "user_a") == Error::None, "logout cleanup succeeds");
	for (const auto suffix : { 's', '0', '1' }) require(!QFileInfo::exists(path(QStringLiteral("user_a/tele_sealed_keys") + QChar(suffix))), "all three key variants deleted");
	require(read("user_a/authorization") == "KEEP AUTHORIZATION" && read("user_a/wallet") == "KEEP WALLET", "unrelated files never removed");
	require(Tele::LoadSealedRegistry(root, context).directories.contains("user_a"), "deleted account directory remains registered permanently");
	require(!Tele::RegistryHasKeys(root, context), "warning disappears after deletion");
	require(Tele::RegistryWrite(root, context, "user_b", [] { return false; }) == Error::WriterFailed, "private writer failure reported");
	require(Tele::LoadSealedRegistry(root, context).directories.contains("user_b"), "failed writer cannot orphan a directory");
	write("user_b/tele_sealed_keys0", "PARTIAL PRIVATE WRITE");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(root, context);
		require(bool(guard) && !QFileInfo::exists(path("user_b/tele_sealed_keys0")), "preflight removes a partial or delayed private file");
		require(Tele::RegistryWrite(root, context, "user_a", [] { return true; }) == Error::Busy, "unprotection retains writer exclusion until guard destruction");
		require(Tele::LoadSealedRegistry(root, context).error == Error::None, "nonlocking read works during cleanup");
	}
	require(Tele::RegistryRegister(root, context, "user_a") == Error::None, "writer resumes after protection transaction finishes");
	require(QDir(root).mkpath("user_b/tele_sealed_keyss"), "cleanup failure fixture");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(root, context);
		require(!guard && guard.error() == Error::EraseFailed, "unsafe key path prevents native unprotection");
	}
	require(Tele::LoadSealedRegistry(root, context).directories.contains("user_b"), "cleanup failure keeps restart registry");
	require(QDir(root).rmdir("user_b/tele_sealed_keyss"), "remove temporary cleanup obstruction");
	require(Tele::EraseAllRegistered(root, context) == Error::None, "restart cleanup can retry");
	const auto valid = read(registry);
	const auto rehash = [&](QByteArray bytes) {
		bytes.chop(32);
		bytes += QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
		return bytes;
	};
	for (auto size : { 0, 1, 8, 9, 41, 74 }) {
		write(registry, valid.left(size));
		require(Tele::LoadSealedRegistry(root, context).error == Error::Corrupt, "truncated registry rejected");
		called = false;
		require(Tele::RegistryWrite(root, context, "user_a", [&] { called = true; return true; }) == Error::Corrupt && !called, "unknown registry never permits private writes");
		{
			auto guard = Tele::PrepareSealedRegistryCleanup(root, context);
			require(!guard, "unknown registry never permits unprotection");
		}
	}
	for (auto offset : { 8, 9, 41, 42, 43, 44 }) {
		auto invalid = valid;
		invalid[offset] = char(-1);
		write(registry, rehash(invalid));
		require(Tele::LoadSealedRegistry(root, context).error == Error::Corrupt, "bounded fields validated despite valid checksum");
	}
	write(registry, QByteArray(1024 * 1024 + 1, 'x'));
	require(Tele::LoadSealedRegistry(root, context).error == Error::Corrupt, "oversized registry rejected");
	write(registry, valid + 'x');
	require(Tele::LoadSealedRegistry(root, context).error == Error::Corrupt, "trailing registry bytes rejected");
	write(registry, valid);
	QLockFile lock(path(registry + ".lock"));
	require(lock.tryLock(0), "external mutation lock fixture");
	require(Tele::RegistryRegister(root, context, "user_c") == Error::Busy, "external writer lock respected");
	{
		auto guard = Tele::PrepareSealedRegistryCleanup(root, context);
		require(!guard && guard.error() == Error::Busy, "busy registry blocks unprotection");
	}
	lock.unlock();
	QTemporaryDir independent;
	require(independent.isValid(), "independent temporary profile");
	require(!Tele::LoadSealedRegistry(independent.path(), context).exists, "registry scoped to explicit root");
	require(!Tele::LoadSealedRegistry(root, Tele::SealedRegistryProfileContext("another profile")).exists, "registry scoped to profile context");
	QTemporaryDir legacy;
	require(legacy.isValid() && QDir(legacy.path()).mkpath("user_old"), "legacy fixture root");
	QFile old(QDir(legacy.path()).filePath("user_old/tele_sealed_keyss"));
	require(old.open(QIODevice::WriteOnly) && old.write("OLD") == 3, "legacy private fixture");
	old.close();
	require(Tele::EraseAccount(legacy.path(), context, "user_old") == Error::None, "known legacy account is registered before logout cleanup");
	require(Tele::LoadSealedRegistry(legacy.path(), context).directories.contains("user_old") && !QFileInfo::exists(old.fileName()), "legacy cleanup remains known after restart");
	QTextStream(stdout) << checks << " registry checks passed.\n";
}
