#pragma once

#include "tele/tele_sealed_registry.h"
#include "main/main_session.h"
#include <QtCore/QFileInfo>

namespace Tele {

inline QByteArray RegistryFacadeContext() {
	return SealedRegistryProfileContext(QStringLiteral("store facade"));
}

inline SealedRegistryError RegisterSealedKeyStore(not_null<Main::Session*> session) {
	const auto path = QFileInfo(session->storage.path);
	return RegistryRegister(path.absolutePath(), RegistryFacadeContext(), path.fileName());
}

inline SealedRegistryError RegisteredSealedKeyWrite(
		not_null<Main::Session*> session,
		const std::function<bool()> &writer) {
	const auto path = QFileInfo(session->storage.path);
	return RegistryWrite(path.absolutePath(), RegistryFacadeContext(), path.fileName(), writer);
}

}
