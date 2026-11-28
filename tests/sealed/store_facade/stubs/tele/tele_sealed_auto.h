#pragma once

namespace Main {
class Session;
}

namespace Tele {
inline int FacadeKeyNotifications = 0;

inline void NotifySealedAutomaticKeysChanged(not_null<Main::Session*>) {
	++FacadeKeyNotifications;
}
}
