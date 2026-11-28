#pragma once

namespace Main {
class Session;
class Account {
public:
	Session *current = nullptr;
	Session *maybeSession() const { return current; }
};
}
