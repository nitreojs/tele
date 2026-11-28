#pragma once
#include "storage/storage_domain.h"
namespace Main { class Domain {
public:
	Storage::Domain value;
	Storage::Domain &local() { return value; }
}; }
