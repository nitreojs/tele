#pragma once
namespace Storage { class Domain {
public:
	bool passcode = true;
	bool hasLocalPasscode() const { return passcode; }
}; }
