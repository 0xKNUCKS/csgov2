#pragma once
#include <memory>
#include <vector>
#include <MinHook.h>
#include "VMT.h"

// Recomended to only use detour, but VMT is there too incase needed
enum hookType : unsigned short
{
	DETOUR,
	VMT,
	DEFAULT
};

class hookManager
{
public:
	hookManager() {}
	hookManager(hookType type) : type(type) {}
	hookManager(hookType type, uintptr_t* VMTbase) { init(VMTbase, type); }

	void init(uintptr_t* VMTbase, hookType type);
	void init(void* VMTbase, hookType type);
	void hook(unsigned int index, void* hookedFunction, hookType type = DEFAULT);
	void restore();

	template<typename retType, unsigned int index, typename ...arguments>
	auto getOriginal(arguments ... args)
	{
		//	   type				thisptr, ..Arguments
		// ex: void(__thiscall*)(void*, ...)
		return reinterpret_cast<retType(__thiscall*)(void*, arguments ...)>(this->ogVMT.getTable()[index]);
	}

	template<typename retType, unsigned int index, typename ...arguments>
	retType callOriginal(arguments ... args)
	{
		return getOriginal<retType, index>(args ...)(this->VMT.getBase(), args ...);
	}
private:
	hookType type = DETOUR;
	VMT_t VMT;
	VMT_t ogVMT;
	// to track which methods/types were used, to be able to restore hooks properly
	bool typeUsed[2] = {0};
	// track MinHook Hooked functions to be able to restore them later
	std::vector<void*> MH_HookedFunctions;
};
