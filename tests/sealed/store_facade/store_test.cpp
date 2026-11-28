#include "tele/tele_sealed_store.h"
#include "tele/tele_sealed_auto.h"
#include "tele/tele_sealed_duress.h"
#include "tele/tele_sealed_registry_native.h"
#include "core/application.h"
#include "main/main_session.h"
#include "storage/details/storage_file_utilities.h"
#include <QTemporaryDir>
#include <cstdio>

using namespace Tele;
using namespace Storage::details;
int checks=0;
void check(bool value,const char *name) {
	++checks; if(!value) {std::fprintf(stderr,"FAILED: %s\n",name);std::exit(1);}
}
QByteArray bytes(const QString &path) {
	QFile f(path);check(f.open(QIODevice::ReadOnly),"read file");return f.readAll();
}
void write(const QString &path,const QByteArray &data) {
	QFile f(path);check(f.open(QIODevice::WriteOnly),"write file");check(f.write(data)==data.size(),"write all");
}
int main(int argc,char **argv) {
	QCoreApplication app(argc,argv);
	QTemporaryDir directory;check(directory.isValid(),"temporary dir");
	Main::Session session;session.storage.path=directory.path()+"/user_test";check(QDir().mkpath(session.storage.path),"account directory");
	auto path=session.storage.path+"/tele_sealed_keyss";
	check(LoadSealedKeys(&session).error==SealedStoreError::Missing,"missing before creation");
	Core::App().value.value.passcode=false;
	check(CreateSealedKeys(&session).error==SealedStoreError::PasscodeRequired,"passcode guard");
	check(!QFileInfo::exists(path),"no generation without passcode");
	Core::App().value.value.passcode=true;Core::App().locked=true;
	check(CreateSealedKeys(&session).error==SealedStoreError::Locked,"lock guard");Core::App().locked=false;
	FailWrite=true;
	check(CreateSealedKeys(&session).error==SealedStoreError::WriteFailed,"identity creation fails closed on disk error");
	check(!QFileInfo::exists(path),"failed new key never exposed/persisted");FailWrite=false;
	check(FacadeKeyNotifications==0,"failed key writes do not publish success");
	check(LoadSealedRegistry(directory.path(),RegistryFacadeContext()).directories.contains("user_test"),"failed private write remains in durable registry");
	auto keys=CreateSealedKeys(&session);check(keys.error==SealedStoreError::None&&keys.identity.has_value(),"create key");
	check(FacadeKeyNotifications==1,"successful identity creation invalidates automatic key cache");
	check(SawSync,"sync write requested");
	auto initial=bytes(path);
	auto loaded=LoadSealedKeys(&session);check(loaded.identity->privateKey==keys.identity->privateKey,"private identity roundtrip");
	check(CreateSealedKeys(&session).identity->publicKey==keys.identity->publicKey,"creation idempotent");
	check(RestoreSealedKeys(&session,*keys.identity)==SealedStoreError::None,"restore same identity no-op");
	check(FacadeKeyNotifications==1,"idempotent creation and restore do not publish key changes");
	auto other=GenerateSealedIdentity();check(other.has_value(),"other identity");
	check(RestoreSealedKeys(&session,*other)==SealedStoreError::Changed,"restore cannot replace identity");
	auto publicKey=other->publicKey;
	check(SaveVerifiedSealedPeer(&session,222,publicKey,{})==SealedStoreError::None,"pin peer key");
	check(FacadeKeyNotifications==2,"new verified pin invalidates automatic key cache");
	auto pinned=bytes(path);
	check(LoadSealedKeys(&session).peers.at(222)==publicKey,"peer key roundtrip");
	check(SaveVerifiedSealedPeer(&session,222,keys.identity->publicKey,publicKey)==SealedStoreError::InvalidPeer,"reject own pub as peer");
	check(SaveVerifiedSealedPeer(&session,111,publicKey,{})==SealedStoreError::InvalidPeer,"reject self peer");
	check(SaveVerifiedSealedPeer(&session,0,publicKey,{})==SealedStoreError::InvalidPeer,"reject zero peer");
	auto third=GenerateSealedIdentity();check(third.has_value(),"third identity");
	check(SaveVerifiedSealedPeer(&session,222,third->publicKey,{})==SealedStoreError::Changed,"stale verification fails");
	FailWrite=true;
	check(SaveVerifiedSealedPeer(&session,222,third->publicKey,publicKey)==SealedStoreError::WriteFailed,"replacement disk failure");
	check(bytes(path)==pinned,"failed replacement leaves previous state");FailWrite=false;
	check(FacadeKeyNotifications==2,"failed pin replacement leaves cache generation unchanged");
	check(SaveVerifiedSealedPeer(&session,222,third->publicKey,publicKey)==SealedStoreError::None,"verified replacement succeeds");
	check(FacadeKeyNotifications==3,"verified replacement invalidates automatic key cache");
	check(ForgetSealedPeer(&session,222,publicKey)==SealedStoreError::Changed,"stale forget fails");
	check(ForgetSealedPeer(&session,222,third->publicKey)==SealedStoreError::None,"forget peer");
	check(FacadeKeyNotifications==4,"pin removal invalidates automatic key cache");
	check(LoadSealedKeys(&session).peers.empty(),"forgotten peer absent");
	session.owner=2;check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"account owner mismatch");session.owner=1;
	write(path,QByteArray());
	check(CreateSealedKeys(&session).error==SealedStoreError::Corrupt,"zero file not replaced");
	check(RestoreSealedKeys(&session,*keys.identity)==SealedStoreError::Corrupt,"restore cannot overwrite corruption");
	check(bytes(path).isEmpty(),"corrupt file untouched");
	write(path,initial.left(initial.size()-1));
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"truncated record");
	write(path,QByteArray::fromHex("fffffffe"));
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"untrusted allocation length");
	write(path,QByteArray(200000,'x'));
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"oversize record");
	write(path,initial);check(LoadSealedKeys(&session).identity->publicKey==keys.identity->publicKey,"restore fixture");
	Core::App().locked=true;check(LoadSealedKeys(&session).error==SealedStoreError::Locked,"cannot load while locked");Core::App().locked=false;
	auto writePayload = [&](quint32 magic,quint32 version,quint32 count,const std::vector<std::pair<quint64,QByteArray>> &entries,bool extra=false) {
		EncryptedDescriptor plain(256);
		plain.stream << magic << version << quint64(session.owner);
		plain.stream.writeRawData(keys.identity->privateKey.constData(),Tele::kSealedPrivateKeySize);
		plain.stream << count;
		for(const auto &[id,key] : entries) {
			plain.stream << id;plain.stream.writeRawData(key.constData(),key.size());
		}
		if(extra) plain.stream << quint8(1);
		FileWriteDescriptor f("tele_sealed_keys",session.storage.path+"/",true);
		f.writeEncrypted(plain,session.storage.key);check(f.finish(),"write malformed plaintext fixture");
	};
	writePayload(0x7E1E5EA1,1,4097,{});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject oversized peer count");
	writePayload(0,1,0,{});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject unknown magic");
	writePayload(0x7E1E5EA1,2,0,{});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject unknown version");
	writePayload(0x7E1E5EA1,1,2,{{222,publicKey},{222,publicKey}});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject duplicate peer entries");
	writePayload(0x7E1E5EA1,1,1,{{0,publicKey}});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject zero stored peer");
	writePayload(0x7E1E5EA1,1,1,{{111,publicKey}});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject self stored peer");
	writePayload(0x7E1E5EA1,1,1,{{222,QByteArray(Tele::kSealedPublicKeySize,char(0))}});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject low order stored public key");
	writePayload(0x7E1E5EA1,1,1,{{222,keys.identity->publicKey}});
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject own public key as stored peer");
	writePayload(0x7E1E5EA1,1,0,{},true);
	check(LoadSealedKeys(&session).error==SealedStoreError::Corrupt,"reject trailing plaintext data");
	write(path,initial);
	Core::App().value.value.passcode=false;
	check(LoadSealedKeys(&session).error==SealedStoreError::PasscodeRequired,"cannot read after passcode removal");
	Core::App().value.value.passcode=true;
	QTemporaryDir restoredDir;check(restoredDir.isValid(),"restore tempdir");
	Main::Session restored;restored.storage.path=restoredDir.path()+"/user_restore";check(QDir().mkpath(restored.storage.path),"restore account directory");
	check(RestoreSealedKeys(&restored,*keys.identity)==SealedStoreError::None,"restore missing identity");
	check(LoadSealedKeys(&restored).identity->publicKey==keys.identity->publicKey,"restored identity matches");
	check(LoadSealedKeys(&restored).peers.empty(),"restored identity requires fresh peer verification");
	FacadeDuressBlocked=true;
	check(LoadSealedKeys(&session).error==SealedStoreError::DuressBlocked,"duress facade blocks key loading");
	check(CreateSealedKeys(&session).error==SealedStoreError::DuressBlocked,"duress facade blocks key creation");
	check(SaveVerifiedSealedPeer(&session,222,publicKey,{})==SealedStoreError::DuressBlocked,"duress facade blocks pin updates");
	check(ForgetSealedPeer(&session,222,publicKey)==SealedStoreError::DuressBlocked,"duress facade blocks pin removal");
	FailWrite=true;
	check(RestoreSealedKeys(&session,*keys.identity)==SealedStoreError::WriteFailed,"failed recovery writer stays blocked");
	check(FacadeDuressBlocked,"failed recovery cannot clear facade block");
	FailWrite=false;
	check(RestoreSealedKeys(&session,*keys.identity)==SealedStoreError::None,"recovery delegates verified writer through facade");
	check(!FacadeDuressBlocked&&FacadeRecoveryCalls==2,"successful recovery invokes writer and clears facade block");
	session.account().current=nullptr;
	check(LoadSealedKeys(&session).error==SealedStoreError::Unavailable,"logged-out session cannot load private keys");
	check(RestoreSealedKeys(&session,*keys.identity)==SealedStoreError::Unavailable,"logged-out session cannot restore private keys");
	session.account().current=&session;
	session.storage.key.reset();check(LoadSealedKeys(&session).error==SealedStoreError::Unavailable,"missing account key");
	std::printf("Store facade fixtures passed: %d checks. Full app storage integration still needs manual validation.\n",checks);
}
