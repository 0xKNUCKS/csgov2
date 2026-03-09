#pragma once
#include <cstddef>
#include "lib/Hooks/Manager/hooksManager.h"
#include "SDK/Classes/Input/Input.h"
#include "SDK/Classes/GlobalVars/GlobalVars.h"

namespace index
{
	enum d3d9Device : unsigned int
	{
		Reset = 16,
		EndScene = 42
	};
	enum ClientMode : unsigned int
	{
		OverrideView = 18,
		ShouldDrawViewModel = 27,
		GetViewModelFOV = 35
	};
	enum BaseClient : unsigned int
	{
		CreateMove = 22,
		FrameStageNotify = 37
	};
	enum Engine : unsigned int
	{
		GetScreenAspectRatio = 101
	};
	enum Surface : unsigned int
	{
		LockCursor = 67
	};
}


/*ToDo: Make a proper Hooking class*/
namespace hooks
{
	inline bool viewRealAngles = false;
	inline bool setupComplete = false; // true once Setup() finishes - hooks should no-op until then
	inline HMODULE hModule = nullptr;

	// Hooking.
	bool Setup();

	void Destroy() noexcept;
	void Unload() noexcept;

	constexpr void* VirtualFunction(void* thisptr, size_t indx) noexcept
	{
		return (*static_cast<void***>(thisptr))[indx];
	}

	// globals
	inline void* ClientMode = nullptr;
	inline CInput* input = nullptr;
	inline CGlobalVarsBase* GlobalVars = nullptr;

	// Cached from CreateMove for safe use in OverrideView (avoids virtual calls during map load)
	inline math::Vector cachedEyePos = {};

	// Hooks
	inline hookManager d3dDeviceHk;
	inline hookManager ClientModeHk;
	inline hookManager BaseClientHk;
	inline hookManager EngineHk;
	inline hookManager SurfaceHk;

}