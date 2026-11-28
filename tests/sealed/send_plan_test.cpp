#include "tele/tele_sealed_send_plan.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>

#include <iostream>
#include <limits>

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
	const auto digest = QCryptographicHash::hash(QByteArray("public ciphertext fixture"), QCryptographicHash::Sha256);
	const auto changedDigest = QCryptographicHash::hash(QByteArray("changed ciphertext fixture"), QCryptographicHash::Sha256);
	const auto own = QByteArray(133, 'a');
	const auto peer = QByteArray(133, 'b');
	const auto reply = QByteArray("synthetic serialized reply");
	const auto text = SealedSendProof{
		.account = 100,
		.chat = 200,
		.digest = digest,
		.reply = reply,
		.own = own,
		.peer = peer,
		.epoch = 9,
		.when = 5000,
	};
	Check(SealedSendProofLive(text, 9, 5000), "fresh proof is live");
	Check(SealedSendProofLive(text, 9, 5000 + kSealedSendProofLifetime), "exact lifetime boundary is inclusive");
	Check(!SealedSendProofLive(text, 9, 5001 + kSealedSendProofLifetime), "expired proof rejected");
	Check(!SealedSendProofLive(text, 9, 4999), "backward clock rejected");
	Check(!SealedSendProofLive(text, 10, 5000), "changed epoch rejected");
	Check(SealedSendProofMatches(text, 100, 200, 9, 5000, own, peer), "complete identity binding matches");
	Check(!SealedSendProofMatches(text, 101, 200, 9, 5000, own, peer), "different account rejected");
	Check(!SealedSendProofMatches(text, 100 | 0x0100000000000000ULL, 200, 9, 5000, own, peer), "test network account rejected");
	Check(!SealedSendProofMatches(text, 100, 201, 9, 5000, own, peer), "different chat rejected");
	Check(!SealedSendProofMatches(text, 100, 200, 9, 5000, QByteArray(133, 'c'), peer), "changed own identity rejected");
	Check(!SealedSendProofMatches(text, 100, 200, 9, 5000, own, QByteArray(133, 'c')), "changed verified pin rejected");
	Check(!SealedSendProofMatches(text, 100, 200, 9, 5000, own, QByteArray()), "missing verified pin rejected");
	Check(SealedSendTextMatches(text, digest, reply), "exact encrypted text and reply match");
	Check(!SealedSendTextMatches(text, changedDigest, reply), "changed encrypted text rejected");
	Check(!SealedSendTextMatches(text, digest, QByteArray("other reply")), "changed reply rejected");
	Check(!SealedSendTextMatches(text, digest, QByteArray()), "omitted reply rejected");
	Check(!SealedSendFileContentMatches(text, digest), "text authorization cannot authorize file content");
	Check(!SealedSendUploadedFileMatches(text, 700), "text authorization cannot authorize uploaded file");
	auto changed = text;
	changed.fileId = 700;
	Check(!SealedSendTextMatches(changed, digest, reply), "uploaded file ID cannot appear on text proof");
	changed = text;
	changed.account = 0;
	Check(!SealedSendProofLive(changed, 9, 5000), "missing account rejected");
	changed = text;
	changed.chat = 0;
	Check(!SealedSendProofLive(changed, 9, 5000), "missing chat rejected");
	for (const auto size : { 0, 31, 33 }) {
		changed = text;
		changed.digest = QByteArray(size, 'x');
		Check(!SealedSendProofLive(changed, 9, 5000), "incorrect digest width rejected");
	}
	changed = text;
	changed.own.clear();
	Check(!SealedSendProofLive(changed, 9, 5000), "absent own key rejected");
	changed = text;
	changed.peer.clear();
	Check(!SealedSendProofLive(changed, 9, 5000), "absent peer key rejected");
	changed = text;
	changed.when = -1;
	Check(!SealedSendProofLive(changed, 9, 0), "negative creation time rejected");
	Check(!SealedSendProofLive(changed, 9, -1), "negative current time rejected");
	changed.when = std::numeric_limits<qint64>::min();
	Check(!SealedSendProofLive(changed, 9, std::numeric_limits<qint64>::max()), "extreme negative time rejected before subtraction");
	changed.when = 0;
	Check(!SealedSendProofLive(changed, 9, std::numeric_limits<qint64>::max()), "extreme elapsed time safely expires");
	changed.when = std::numeric_limits<qint64>::max() - kSealedSendProofLifetime;
	Check(SealedSendProofLive(changed, 9, std::numeric_limits<qint64>::max()), "large positive timestamp boundary remains valid");
	changed = text;
	changed.epoch = 0;
	Check(!SealedSendProofLive(changed, 0, 5000), "uninitialized epoch rejected");
	auto file = text;
	file.file = true;
	file.reply.clear();
	Check(SealedSendProofLive(file, 9, 5000), "fresh file proof is live");
	Check(SealedSendFileContentMatches(file, digest), "exact encrypted file content matches");
	Check(!SealedSendFileContentMatches(file, changedDigest), "changed encrypted file content rejected");
	Check(!SealedSendTextMatches(file, digest, QByteArray()), "file authorization cannot authorize text");
	Check(!SealedSendUploadedFileMatches(file, 700), "unbound upload ID rejected");
	file.fileId = 700;
	Check(SealedSendUploadedFileMatches(file, 700), "exact bound uploaded file ID matches");
	Check(!SealedSendUploadedFileMatches(file, 701), "different upload ID rejected");
	Check(!SealedSendUploadedFileMatches(file, 0), "zero upload ID rejected");
	Check(SealedSendProofMatches(file, 100, 200, 9, 5000, own, peer), "file still requires current account and keys");
	Check(!SealedSendProofMatches(file, 100, 201, 9, 5000, own, peer), "upload ID cannot transfer authorization to another chat");
	Check(!SealedSendProofMatches(file, 100, 200, 10, 5000, own, peer), "mode change invalidates a previously bound upload");
	Check(!SealedSendProofMatches(file, 100, 200, 9, 5001 + kSealedSendProofLifetime, own, peer), "bound upload also expires");
	{
		auto upload = file;
		upload.fileId = 0;
		auto name = QStringLiteral("file_2026-11-28_19-30-10.bin");
		Check(PrepareSealedFileUpload(upload, digest, 800, name), "native generated internal filename accepted with exact ciphertext proof");
		Check(name == QStringLiteral("message.teleenc") && upload.fileId == 800, "authorized upload canonicalizes transport filename and binds ID");
		Check(SealedSendUploadedFileMatches(upload, 800), "normalized upload matches final transport proof");
		name = QStringLiteral("native-second-uploader-check.bin");
		Check(PrepareSealedFileUpload(upload, digest, 800, name), "second native uploader gate accepts same upload ID");
		Check(name == QStringLiteral("message.teleenc") && upload.fileId == 800, "second uploader gate retains canonical transport name and ID");
		name = QStringLiteral("must-stay-unchanged.bin");
		Check(!PrepareSealedFileUpload(upload, digest, 801, name), "different upload ID cannot rebind existing ciphertext proof");
		Check(name == QStringLiteral("must-stay-unchanged.bin") && upload.fileId == 800, "rebind rejection preserves filename and bound ID");
		Check(!PrepareSealedFileUpload(upload, changedDigest, 800, name), "changed ciphertext cannot normalize transport filename");
		Check(name == QStringLiteral("must-stay-unchanged.bin") && upload.fileId == 800, "changed ciphertext rejection preserves filename and bound ID");
		Check(!PrepareSealedFileUpload(upload, digest, 0, name), "zero native upload ID rejected");
		Check(name == QStringLiteral("must-stay-unchanged.bin") && upload.fileId == 800, "zero ID rejection preserves filename and bound ID");
		upload.fileId = 0;
		upload.file = false;
		Check(!PrepareSealedFileUpload(upload, digest, 800, name), "text proof cannot authorize filename normalization");
		Check(name == QStringLiteral("must-stay-unchanged.bin") && !upload.fileId, "text proof rejection preserves filename and unbound ID");
		upload.file = true;
		upload.reply = reply;
		Check(!PrepareSealedFileUpload(upload, digest, 800, name), "reply-bearing file proof cannot authorize filename normalization");
		Check(name == QStringLiteral("must-stay-unchanged.bin") && !upload.fileId, "reply rejection preserves filename and unbound ID");
	}
	file.reply = reply;
	Check(!SealedSendFileContentMatches(file, digest), "file reply cannot bypass file restrictions");
	Check(!SealedSendUploadedFileMatches(file, 700), "uploaded file reply cannot bypass restrictions");
	std::cout << Checks << " checks, " << Failed << " failed\n";
	return Failed ? 1 : 0;
}
