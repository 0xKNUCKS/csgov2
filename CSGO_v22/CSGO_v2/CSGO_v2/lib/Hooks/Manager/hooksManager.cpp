#include "hooksManager.h"
#include "lib/Error/Log.h"

bool hookManager::init(uintptr_t* VMTbase, hookType type)
{
	if (!VMTbase) {
		Log::Fatal("HookMgr", "init() called with null VMT base");
		return false;
	}

	this->type = type;
	this->VMT.init(VMTbase);
	this->ogVMT.init(this->VMT); // Set it to the real VMT and store it

	if (!this->VMT.getBase() || !this->VMT.getLength()) {
		Log::Err("HookMgr", "VMT init failed - base={:#x}, length={}",
			(uintptr_t)VMTbase, this->VMT.getLength());
		return false;
	}

	Log::Info("HookMgr", "Initialized VMT at {:#x} (length: {})",
		(uintptr_t)VMTbase, this->VMT.getLength());
	return true;
}

bool hookManager::init(void* VMTbase, hookType type)
{
	return this->init(reinterpret_cast<uintptr_t*>(VMTbase), type);
}

bool hookManager::hook(unsigned int index, void* hookedFunction, hookType type)
{
	if (type == DEFAULT) {
		type = this->type;
	}

	switch (type)
	{
	case hookType::DETOUR:
		this->typeUsed[hookType::DETOUR] = true;
		{
			void* target = VMT.getVirtualFunction(index);
			if (!target) {
				Log::Err("HookMgr", "getVirtualFunction({}) returned null", index);
				return false;
			}

			MH_STATUS createStatus = MH_CreateHook(
				target,
				hookedFunction,
				reinterpret_cast<void**>(&ogVMT.getTable()[index])
			);
			if (createStatus != MH_STATUS::MH_OK) {
				Log::Err("HookMgr", "MH_CreateHook failed for index {} (MH_STATUS: {})",
					index, (int)createStatus);
				return false;
			}

			MH_STATUS enableStatus = MH_EnableHook(target);
			if (enableStatus != MH_STATUS::MH_OK) {
				Log::Err("HookMgr", "MH_EnableHook failed for index {} (MH_STATUS: {})",
					index, (int)enableStatus);
				MH_RemoveHook(target);
				return false;
			}

			this->MH_HookedFunctions.push_back(target); // store TARGET, not detour
			Log::Info("HookMgr", "Hooked index {} (DETOUR) at {:#x}", index, (uintptr_t)target);
		}
		break;
	case hookType::VMT:
		this->typeUsed[hookType::VMT] = true;
		if (!this->VMT.setIndex(index, hookedFunction)) {
			Log::Err("HookMgr", "VMT setIndex({}) failed - VirtualProtect denied", index);
			return false;
		}
		Log::Info("HookMgr", "Hooked index {} (VMT)", index);
		break;
	default: // shouldnt be possible, but just to be safe.
		return hookManager::hook(index, hookedFunction, DETOUR); // holy recursion
	}

	return true;
}

void hookManager::restore()
{
	if (this->typeUsed[hookType::DETOUR]) {
		for (void* target : MH_HookedFunctions) {
			MH_STATUS dis = MH_DisableHook(target);
			MH_STATUS rem = MH_RemoveHook(target);
			if (dis != MH_OK || rem != MH_OK)
				Log::Warn("HookMgr", "restore: disable={} remove={} for {:#x}",
					(int)dis, (int)rem, (uintptr_t)target);
		}
	}

	if (this->typeUsed[hookType::VMT]) {
		this->VMT.swapVMT(ogVMT); // wow so ez :O
	}
}
