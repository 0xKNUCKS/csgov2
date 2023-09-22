#include "hooksManager.h"

void hookManager::init(uintptr_t* VMTbase, hookType type)
{
	this->type = type;
	this->VMT.init(VMTbase);
	this->ogVMT.init(this->VMT); // Set it to the real VMT and store it
}

void hookManager::init(void* VMTbase, hookType type)
{
	this->init(reinterpret_cast<uintptr_t*>(VMTbase), type);
}

void hookManager::hook(unsigned int index, void* hookedFunction, hookType type)
{
	if (type == DEFAULT) {
		type = this->type;
	}

	switch (type)
	{
	case hookType::DETOUR:
		this->typeUsed[hookType::DETOUR] = true;
		if (MH_CreateHook(
			VMT.getVirtualFunction(index),
			hookedFunction,
			reinterpret_cast<void**>(&ogVMT.getTable()[index])
		) == MH_STATUS::MH_OK) {
			if (MH_EnableHook(VMT.getVirtualFunction(index)) == MH_STATUS::MH_OK)
				this->MH_HookedFunctions.push_back(hookedFunction);
		}
		break;
	case hookType::VMT:
		this->typeUsed[hookType::VMT] = true;
		this->VMT.setIndex(index, hookedFunction);
		break;
	default: // shouldnt be possible, but just to be safe.
		hookManager::hook(index, hookedFunction, DETOUR); // holy recursion
		break;
	}
}

void hookManager::restore()
{
	if (this->typeUsed[hookType::DETOUR]) {
		for (void* function : MH_HookedFunctions) {
			MH_RemoveHook(function);
		}
	}

	if (this->typeUsed[hookType::VMT]) {
		this->VMT.swapVMT(ogVMT); // wow so ez :O
	}
}
