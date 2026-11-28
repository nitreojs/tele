#pragma once

#include "tele/tele_sealed_store.h"

namespace Tele {
inline auto FacadeDuressBlocked = false;
inline auto FacadeRecoveryCalls = 0;

inline SealedStoreError CheckSealedDuressStorage(not_null<Main::Session*>) {
	return FacadeDuressBlocked ? SealedStoreError::DuressBlocked : SealedStoreError::None;
}

inline SealedStoreError RecoverSealedDuressStorage(
		not_null<Main::Session*>,
		Fn<bool()> authenticatedWriter) {
	++FacadeRecoveryCalls;
	if (!authenticatedWriter()) {
		return SealedStoreError::WriteFailed;
	}
	FacadeDuressBlocked = false;
	return SealedStoreError::None;
}
}
