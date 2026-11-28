#pragma once
#include "main/main_domain.h"
namespace Core { class Application {
public:
	bool locked = false;
	Main::Domain value;
	bool passcodeLocked() const { return locked; }
	Main::Domain &domain() { return value; }
};
inline Application &App() { static Application app; return app; }
}
