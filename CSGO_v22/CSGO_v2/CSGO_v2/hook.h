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

	// Functions Declarations
	using tCreateMove =			  void(__thiscall*)(void*, int, float, bool) noexcept;
	using tEndScene =			  long(__thiscall*)(void*, LPDIRECT3DDEVICE9) noexcept;
	using tReset =				  HRESULT(__thiscall*)(void*, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
	using tFrameStageNotify =	  void(__thiscall*)(void*, ClientFrameStage_t) noexcept;
	using tGetScreenAspectRatio = float(__thiscall*)(void*, int viewportWidth, int viewportHeight) noexcept;
	using tGetViewModelFOV =	  float(__thiscall*)(void*) noexcept;
	using tOverrideView =		  void(__thiscall*)(void*, CViewSetup* pSetup) noexcept;
	using tShouldDrawViewModel =  bool(__thiscall*)(void*) noexcept;
	using tConVarHook =			  bool(__thiscall*)(void*) noexcept;
	using tLockCursor =			  void(__thiscall*)(void*) noexcept;

	// globals
	inline void* g_ClientMode = nullptr;
	inline CInput* input = nullptr;
	inline CGlobalVarsBase* GlobalVars = nullptr;
	inline uintptr_t* CAM_THINK = nullptr;
	inline void* svCheatsCVar = nullptr;

	// Original Functions Declarations
	inline tCreateMove oCreateMove = nullptr;
	inline tGetViewModelFOV oGetViewModelFOV = nullptr;

	inline tEndScene oEndScene = nullptr;
	inline tReset oReset = nullptr;

	inline tFrameStageNotify oFrameStageNotify = nullptr;
	inline tGetScreenAspectRatio oGetScreenAspectRatio = nullptr;
	inline tOverrideView oOverrideView = nullptr;
	inline tShouldDrawViewModel oShouldDrawViewModel = nullptr;

	inline tConVarHook oSvCheatsGetBool = nullptr;

	inline tLockCursor oLockCursor = nullptr;

	// Hooked Functions Declarations
    long __stdcall	  hkEndScene(LPDIRECT3DDEVICE9 pDevice) noexcept;
	HRESULT __stdcall hkReset(IDirect3DDevice9* Device, D3DPRESENT_PARAMETERS* params) noexcept;
	void __stdcall	  hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool* bSendPacket) noexcept;
	void __stdcall	  hkCreateMoveProxy(int sequenceNumber, float inputSampleTime, bool active) noexcept;
	void __stdcall	  hkFrameStageNotify(ClientFrameStage_t curStage) noexcept;
	float __stdcall	  hkGetScreenAspectRatio(int viewportWidth, int viewportHeight) noexcept;
	float __stdcall	  hkGetViewModelFOV() noexcept;
	void __stdcall	  hkOverrideView(CViewSetup* pSetup);
	bool __stdcall	  hkShouldDrawViewModel() noexcept;
	bool __stdcall	  hkSvCheatsGetBool(void* pConVar) noexcept;
	void __stdcall	  hkLockCursor() noexcept;
}