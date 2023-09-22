#pragma once
#pragma comment(lib, "libMinHook.x86.lib")
#include <Windows.h>
#include <intrin.h>
#include <MinHook.h>
#include "classes.h"
#include "interface.h"
#include "GUI.h"
#include "drawing.h"
#include "Globals.h"
#include "aimbot.h"
#include "Misc.h"
#include "ESP.h"
#include "Input.h"
#include "GlobalVars.h"
#include "ViewSetup.h"
#include "hooksManager.h"

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
	// Hooking.
	bool Setup();

	void Destroy() noexcept;

	constexpr void* VirtualFunction(void* thisptr, size_t indx) noexcept
	{
		return (*static_cast<void***>(thisptr))[indx];
	}

	// globals
	inline void* ClientMode = nullptr;
	inline CInput* input = nullptr;
	inline CGlobalVarsBase* GlobalVars = nullptr;

	// Hooks
	inline hookManager d3dDeviceHk;
	inline hookManager ClientModeHk;
	inline hookManager BaseClientHk;
	inline hookManager EngineHk;
	inline hookManager SurfaceHk;
}