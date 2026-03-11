#include "KeyValues.h"
#include "lib/utils/utils.h"
#include "lib/Error/Log.h"

KeyValues::KeyValuesFromStringAddr KeyValues::s_fromStringFn = 0;
KeyValues::FindKeyFn KeyValues::s_findKeyFn = nullptr;
KeyValues::SetStringFn KeyValues::s_setStringFn = nullptr;
bool KeyValues::s_found = false;

bool KeyValues::FindFunctions()
{
	if (s_found) return true;

	HMODULE hClient = GetModuleHandleA("client.dll");
	if (!hClient) return false;

	// KeyValuesFromString — "E8 ? ? ? ? 83 C4 04 89 45 D8"
	auto callSig = utils::PatternScan(hClient, "E8 ? ? ? ? 83 C4 04 89 45 D8");
	if (callSig) {
		auto rel = *reinterpret_cast<int32_t*>(callSig + 1);
		s_fromStringFn = reinterpret_cast<std::uintptr_t>(callSig + 5 + rel);
		Log::Info("KeyValues", "FromString at {:#x}", s_fromStringFn);
	}

	// KeyValues::FindKey — "E8 ? ? ? ? F7 45"
	auto findKeySig = utils::PatternScan(hClient, "E8 ? ? ? ? F7 45");
	if (findKeySig) {
		auto rel = *reinterpret_cast<int32_t*>(findKeySig + 1);
		s_findKeyFn = reinterpret_cast<FindKeyFn>(findKeySig + 5 + rel);
		Log::Info("KeyValues", "FindKey at {:#x}", (uintptr_t)s_findKeyFn);
	}

	// KeyValues::SetString — "E8 ? ? ? ? 89 77 38"
	auto setStringSig = utils::PatternScan(hClient, "E8 ? ? ? ? 89 77 38");
	if (setStringSig) {
		auto rel = *reinterpret_cast<int32_t*>(setStringSig + 1);
		s_setStringFn = reinterpret_cast<SetStringFn>(setStringSig + 5 + rel);
		Log::Info("KeyValues", "SetString at {:#x}", (uintptr_t)s_setStringFn);
	}

	s_found = (s_fromStringFn != 0 && s_findKeyFn != nullptr && s_setStringFn != nullptr);
	if (!s_found)
		Log::Err("KeyValues", "Pattern scan failed: FromString={:#x} FindKey={:#x} SetString={:#x}",
			s_fromStringFn, (uintptr_t)s_findKeyFn, (uintptr_t)s_setStringFn);

	return s_found;
}

// Naked wrapper for KeyValuesFromString's non-standard calling convention
static __declspec(naked) KeyValues* __cdecl CallFromString(std::uintptr_t fn, const char* name)
{
	__asm {
		push ebp
		mov ebp, esp

		push 0                  // third arg on stack (unused)
		mov edx, 0             // value = nullptr
		mov ecx, [ebp + 12]    // name
		call [ebp + 8]         // fn
		add esp, 4             // clean up push 0

		pop ebp
		ret
	}
}

KeyValues* KeyValues::FromString(const char* name)
{
	if (!FindFunctions()) return nullptr;
	return CallFromString(s_fromStringFn, name);
}

KeyValues* KeyValues::FindKey(const char* keyName, bool create)
{
	if (!s_findKeyFn) return nullptr;
	return s_findKeyFn(this, keyName, create);
}

void KeyValues::SetString(const char* keyName, const char* value)
{
	if (!s_findKeyFn || !s_setStringFn) return;
	auto* key = s_findKeyFn(this, keyName, true);
	if (key)
		s_setStringFn(key, value);
}

void KeyValues::SetStringValue(const char* value)
{
	if (!s_setStringFn) return;
	s_setStringFn(this, value);
}
