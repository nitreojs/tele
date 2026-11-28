#pragma once
#include "storage/storage_account.h"
#include <QtCore/QBuffer>
namespace Storage::details {
inline bool FailWrite = false;
inline bool SawSync = false;
struct FileReadDescriptor {
	QByteArray data;
	QBuffer buffer;
	QDataStream stream;
};
struct EncryptedDescriptor {
	QByteArray data;
	QBuffer buffer;
	QDataStream stream;
	EncryptedDescriptor() = default;
	explicit EncryptedDescriptor(uint32) {
		data.resize(4); buffer.setBuffer(&data); buffer.open(QIODevice::WriteOnly);
		buffer.seek(4); stream.setDevice(&buffer); stream.setVersion(QDataStream::Qt_5_1);
	}
	void finish() { stream.setDevice(nullptr); buffer.close(); buffer.setBuffer(nullptr); }
	~EncryptedDescriptor() { finish(); }
};
bool ReadFile(FileReadDescriptor &, const QString &, const QString &);
bool DecryptLocal(EncryptedDescriptor &, const QByteArray &, const std::shared_ptr<QByteArray> &);
class FileWriteDescriptor {
public:
	FileWriteDescriptor(const QString &name,const QString &path,bool sync) : name(name),path(path) { SawSync=sync; }
	void writeEncrypted(EncryptedDescriptor &,const std::shared_ptr<QByteArray> &);
	bool finish();
private:
	QString name,path;
	QByteArray encrypted;
};
}
