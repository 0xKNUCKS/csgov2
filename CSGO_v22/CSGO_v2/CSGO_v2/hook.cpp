#include "hook.h"

bool hooks::Setup()
{
	if (MH_Initialize())
		return 0;//throw std::runtime_error("Unable to initialize Hooks");

	// D3D9 Hook
	if (MH_CreateHook(
		VirtualFunction(gui::device, 42),
		&hkEndScene,
		reinterpret_cast<void**>(&oEndScene)
	)) return 0;//throw std::runtime_error("Unable to hook Reset");

	// Reset Hook
	if (MH_CreateHook(
		VirtualFunction(gui::device, 16),
		&hkReset,
		reinterpret_cast<void**>(&oReset)
	)) return 0;// throw std::runtime_error("Unable to hook Reset");

	// Get ClientMode global pointer
	g_ClientMode = **reinterpret_cast<void***>((*reinterpret_cast<unsigned int**>(globals::g_interfaces.BaseClient))[10] + 5);
	
	// CreateMove Hook
	if (MH_CreateHook(
		(*static_cast<void***>(g_ClientMode))[24],
		&hkCreateMove,
		reinterpret_cast<void**>(&oCreateMove)
	)) return 0;//throw std::runtime_error("Unable to hook CreateMove");

	// GetViewModelFOV Hook
	if (MH_CreateHook(
		(*static_cast<void***>(g_ClientMode))[35],
		&hkGetViewModelFOV,
		reinterpret_cast<void**>(&oGetViewModelFOV)
	)) return 0;//throw std::runtime_error("Unable to hook GetViewModelFOV");

	// FrameStageNotify hook
	if (MH_CreateHook(
		VirtualFunction(globals::g_interfaces.BaseClient , 37),
		&hkFrameStageNotify,
		reinterpret_cast<void**>(&oFrameStageNotify)
	)) return 0;//throw std::runtime_error("Unable to hook FrameStageNotify");

	// Engine::GetScreenAspectRatio hook
	if (MH_CreateHook(
		VirtualFunction(globals::g_interfaces.Engine, 101),
		&hkGetScreenAspectRatio,
		reinterpret_cast<void**>(&oGetScreenAspectRatio)
	)) return 0;//throw std::runtime_error("Unable to hook GetScreenAspectRatio");

	// Actually hook
	if (MH_EnableHook(MH_ALL_HOOKS))
		return 0;//throw std::runtime_error("Unable to enable hooks");

	gui::DestroyDirectX();
	return 1;
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

long __stdcall hooks::hkEndScene(LPDIRECT3DDEVICE9 pDevice) noexcept
{
	const auto result = oEndScene(&pDevice, pDevice);
	if (cfg.settings.StreamProof)
	{
		// Stream proof using steam's Game Overlay
		static uintptr_t GameOverlayRetAdr = 0;

		if (!GameOverlayRetAdr)
		{
			MEMORY_BASIC_INFORMATION info;
			VirtualQuery(_ReturnAddress(), &info, sizeof(MEMORY_BASIC_INFORMATION));

			char mod[MAX_PATH];
			GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

			if (strstr(mod, "gameoverlay"))
				GameOverlayRetAdr = (uintptr_t)(_ReturnAddress());
		}

		if (GameOverlayRetAdr != (uintptr_t)(_ReturnAddress()))
			return result;
	}


	if (!gui::init)
		gui::SetupMenu(pDevice);

	gui::Render();

	return result;
}

HRESULT __stdcall hooks::hkReset(IDirect3DDevice9* Device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = oReset(Device, Device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

bool __stdcall hooks::hkCreateMove(float frametime, CUserCmd* cmd) noexcept
{
	const auto result = oCreateMove(g_ClientMode, frametime, cmd);

	if (!cmd || !cmd->command_number)
		return result;

	if (GetAsyncKeyState(VK_F9)) {
		cmd->viewangles.x = 0;
	}

	aimbot::Run(cmd);
	misc::BunnyHop(cmd);

	// if returned false CreateMove will not change the local viewAngles
	return 0;
}

void __stdcall hooks::hkFrameStageNotify(ClientFrameStage_t curStage) noexcept
{
	if (curStage == ClientFrameStage_t::FRAME_START) {
		// to be used for WorldToScreen.
		globals::game::viewMatrix = globals::g_interfaces.Engine->WorldToScreenMatrix();
	}

	return oFrameStageNotify(globals::g_interfaces.BaseClient, curStage);
}

float __stdcall hooks::hkGetScreenAspectRatio(int viewportWidth, int viewportHeight) noexcept
{
	auto aspectRatio = cfg.visuals.misc.AspectRatio > 0.f ? cfg.visuals.misc.AspectRatio : oGetScreenAspectRatio(globals::g_interfaces.Engine, viewportWidth, viewportHeight);
	return aspectRatio;
}

float __stdcall hooks::hkGetViewModelFOV() noexcept
{
	return cfg.visuals.misc.ViewModelFOV;
}