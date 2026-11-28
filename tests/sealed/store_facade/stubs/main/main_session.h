#pragma once
#include "data/data_peer_id.h"
#include "main/main_account.h"
#include "storage/storage_account.h"
namespace Main { class Session {
public:
	Session() { accountValue.current = this; }
	Storage::Account storage;
	Account accountValue;
	quint64 owner = 1;
	UserId user = { 111 };
	Storage::Account &local() { return storage; }
	Account &account() { return accountValue; }
	quint64 uniqueId() const { return owner; }
	UserId userId() const { return user; }
}; }
