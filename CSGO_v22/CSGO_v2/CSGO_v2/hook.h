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

/*ToDo: Make a proper Hooking class*/
namespace hooks
{
	// Hooking.
	bool Setup();

	void Destroy() noexcept;

	constexpr void* VirtualFunction(void* thisptr, size_t indx) noexcept
	{
		return (*static_cast<void***>(thisptr))[indx];
	}

	// Functions Declarations
	using tCreateMove =		  bool(__thiscall*)(void*, float, CUserCmd*) noexcept;
	using tEndScene =		  long(__thiscall*)(void*, LPDIRECT3DDEVICE9) noexcept;
	using tReset =		   HRESULT(__thiscall*)(void*, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
	using tFrameStageNotify = void(__thiscall*)(void*, ClientFrameStage_t) noexcept;

	// Hooks Declarations
	inline tCreateMove oCreateMove = nullptr;
	inline void* g_ClientMode = nullptr;

	inline tEndScene oEndScene = nullptr;
	inline tReset oReset = nullptr;

	inline tFrameStageNotify oFrameStageNotify = nullptr;

	// Hooked Functions Declarations
    long __stdcall	  hkEndScene(LPDIRECT3DDEVICE9 pDevice) noexcept;
	HRESULT __stdcall hkReset(IDirect3DDevice9* Device, D3DPRESENT_PARAMETERS* params) noexcept;
	bool __stdcall	  hkCreateMove(float frametime, CUserCmd* cmd) noexcept;
	void __stdcall	  hkFrameStageNotify(ClientFrameStage_t curStage);
}