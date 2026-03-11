#pragma once
#include <Windows.h>
#include <cstdint>

// KeyValues - Source engine key-value pair system
// Uses KeyValuesFromString + findKey/setString found via pattern scan
class KeyValues
{
public:
	// Create a KeyValues from shader name (value is always nullptr)
	static KeyValues* FromString(const char* name);

	// Find a sub-key by name, optionally creating it
	KeyValues* FindKey(const char* keyName, bool create);

	// Set a string value on this key (calls FindKey internally)
	void SetString(const char* keyName, const char* value);

	// Set the string value directly on this KeyValues node
	void SetStringValue(const char* value);

	// Find all function pointers (called automatically)
	static bool FindFunctions();

private:
	using KeyValuesFromStringAddr = std::uintptr_t;  // raw address, called via inline asm
	using FindKeyFn = KeyValues*(__thiscall*)(KeyValues*, const char*, bool);
	using SetStringFn = void(__thiscall*)(KeyValues*, const char*);

	static KeyValuesFromStringAddr s_fromStringFn;
	static FindKeyFn s_findKeyFn;
	static SetStringFn s_setStringFn;
	static bool s_found;
};
