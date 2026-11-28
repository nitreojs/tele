#include "storage/details/storage_file_utilities.h"
#include <cstring>

namespace Storage::details {
bool ReadFile(FileReadDescriptor &result,const QString &name,const QString &directory) {
	QFile file(directory+name+'s');
	if (!file.open(QIODevice::ReadOnly)) return false;
	result.data = file.readAll();
	result.buffer.setBuffer(&result.data); result.buffer.open(QIODevice::ReadOnly);
	result.stream.setDevice(&result.buffer); result.stream.setVersion(QDataStream::Qt_5_1);
	return true;
}
void FileWriteDescriptor::writeEncrypted(EncryptedDescriptor &plain,const std::shared_ptr<QByteArray> &key) {
	plain.finish();
	auto bytes=plain.data;
	const auto length=quint32(bytes.size());
	std::memcpy(bytes.data(),&length,4);
	bytes.resize((bytes.size()+15)&~15);
	const auto digest=QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).left(16);
	for (auto i=0;i<bytes.size();++i) bytes[i]=bytes[i]^key->at(i%key->size());
	encrypted=digest+bytes;
}
bool FileWriteDescriptor::finish() {
	if (FailWrite) return false;
	QSaveFile file(path+name+'s');
	if (!file.open(QIODevice::WriteOnly)) return false;
	QDataStream stream(&file);stream.setVersion(QDataStream::Qt_5_1);stream << encrypted;
	return stream.status()==QDataStream::Ok && file.commit();
}
bool DecryptLocal(EncryptedDescriptor &result,const QByteArray &encrypted,const std::shared_ptr<QByteArray> &key) {
	if (encrypted.size()<32 || encrypted.size()%16) return false;
	auto plain=encrypted.mid(16);
	for(auto i=0;i<plain.size();++i) plain[i]=plain[i]^key->at(i%key->size());
	if (QCryptographicHash::hash(plain,QCryptographicHash::Sha256).left(16)!=encrypted.left(16)) return false;
	quint32 size=0;std::memcpy(&size,plain.constData(),4);
	if (size<4 || size>plain.size() || size<=plain.size()-16) return false;
	plain.resize(size);result.data=std::move(plain);
	result.buffer.setBuffer(&result.data);result.buffer.open(QIODevice::ReadOnly);result.buffer.seek(4);
	result.stream.setDevice(&result.buffer);result.stream.setVersion(QDataStream::Qt_5_1);return true;
}
}
