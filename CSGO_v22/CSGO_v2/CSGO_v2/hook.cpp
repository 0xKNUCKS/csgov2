#include "hook.h"

bool hooks::Setup()
{
	// Globals Initialization
	g_ClientMode = **reinterpret_cast<void***>((*reinterpret_cast<unsigned int**>(globals::g_interfaces.BaseClient))[10] + 5);
	input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[16] + 1);
	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[11] + 10);

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

	// GetViewModelFOV Hook
	if (MH_CreateHook(
		VirtualFunction(g_ClientMode, 35),
		&hkGetViewModelFOV,
		reinterpret_cast<void**>(&oGetViewModelFOV)
	)) return 0;//throw std::runtime_error("Unable to hook GetViewModelFOV");

	// CreateMove Hook
	if (MH_CreateHook(
		VirtualFunction(globals::g_interfaces.BaseClient, 22),
		&hkCreateMoveProxy,
		reinterpret_cast<void**>(&oCreateMove)
	)) return 0;//throw std::runtime_error("Unable to hook CreateMove");

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

void __stdcall hooks::hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool* bSendPacket) noexcept
{
	oCreateMove(globals::g_interfaces.BaseClient, sequence_number, input_sample_frametime, active);

	if (!bSendPacket)
		return;

	CUserCmd* cmd = input->getUserCmd(0, sequence_number);

	if (!cmd || !cmd->command_number)
		return;

	static bool sides = 0;

	if (viewRealAngles)
	{
		*bSendPacket = false;
		cmd->viewangles.y = 130.f;
		sides = !sides;
		cmd->viewangles.x = 89.f;

		*bSendPacket = true;
		cmd->viewangles.y = 100.f;
		sides = !sides;
		cmd->viewangles.x = 89.f;
	}

	aimbot::Run(cmd);
	misc::BunnyHop(cmd);

	VerifiedUserCmd* verified = input->getVerifiedUserCmd(sequence_number);
	if (verified)
		*verified = VerifiedUserCmd(*cmd);
}

__declspec(naked) void __stdcall hooks::hkCreateMoveProxy(int sequenceNumber, float inputSampleTime, bool active) noexcept
{
	// Create move Proxy to be able to retrieve "bSendPacket" pointer
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		lea ecx, [esp]; load stack pointer to ecx(bSendPacket)
		push ecx; push bSendPacket to the stack 
		movzx edx, active
		push edx
		push inputSampleTime
		push sequenceNumber
		call hooks::hkCreateMove
		pop ebx
		pop ebp
		ret 0Ch
	}
}

void __stdcall hooks::hkFrameStageNotify(ClientFrameStage_t curStage) noexcept
{
	using enum ClientFrameStage_t;


	switch (curStage)
	{
	case FRAME_START:
		// to be used for WorldToScreen.
		globals::game::viewMatrix = globals::g_interfaces.Engine->WorldToScreenMatrix();
		break;
	case FRAME_NET_UPDATE_END:
		globals::EntList.Update();
		break;
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