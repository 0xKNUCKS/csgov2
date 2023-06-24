#include "hook.h"
#include "../ext/AnimationLib/Animation.h"

bool hooks::Setup()
{
	// Globals Initialization
	g_ClientMode = **reinterpret_cast<void***>((*reinterpret_cast<unsigned int**>(globals::g_interfaces.BaseClient))[10] + 5);
	input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[16] + 1);
	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[11] + 10);
	CAM_THINK = reinterpret_cast<uintptr_t*>(utils::PatternScan(GetModuleHandle("client.dll"), "85 C0 75 30 38 86"));
	svCheatsCVar = globals::g_interfaces.Cvar->FindVar("sv_cheats");

	if (MH_Initialize())
		return 0;//throw std::runtime_error("Unable to initialize Hooks");

	// gui::Device hooks
	{
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
	}

	// g_ClientMode hooks
	{
		// GetViewModelFOV Hook
		if (MH_CreateHook(
			VirtualFunction(g_ClientMode, 35),
			&hkGetViewModelFOV,
			reinterpret_cast<void**>(&oGetViewModelFOV)
		)) return 0;//throw std::runtime_error("Unable to hook GetViewModelFOV");

		// OverrideView Hook
		if (MH_CreateHook(
			VirtualFunction(g_ClientMode, 18),
			&hkOverrideView,
			reinterpret_cast<void**>(&oOverrideView)
		)) return 0;

		// ShouldDrawViewModel Hook
		if (MH_CreateHook(
			VirtualFunction(g_ClientMode, 27),
			&hkShouldDrawViewModel,
			reinterpret_cast<void**>(&oShouldDrawViewModel)
		)) return 0;
	}

	// globals::g_interfaces.BaseClient hooks
	{
		// CreateMove Hook
		if (MH_CreateHook(
			VirtualFunction(globals::g_interfaces.BaseClient, 22),
			&hkCreateMoveProxy,
			reinterpret_cast<void**>(&oCreateMove)
		)) return 0;//throw std::runtime_error("Unable to hook CreateMove");

		// FrameStageNotify hook
		if (MH_CreateHook(
			VirtualFunction(globals::g_interfaces.BaseClient, 37),
			&hkFrameStageNotify,
			reinterpret_cast<void**>(&oFrameStageNotify)
		)) return 0;//throw std::runtime_error("Unable to hook FrameStageNotify");
	}

	// globals::g_interfaces.Engine hooks
	{
		// Engine::GetScreenAspectRatio hook
		if (MH_CreateHook(
			VirtualFunction(globals::g_interfaces.Engine, 101),
			&hkGetScreenAspectRatio,
			reinterpret_cast<void**>(&oGetScreenAspectRatio)
		)) return 0;//throw std::runtime_error("Unable to hook GetScreenAspectRatio");
	}

	// super duper crash dont uncomment
	{
		//if (MH_CreateHook(
		//	VirtualFunction(svCheatsCVar, 13),
		//	&hkSvCheatsGetBool,
		//	reinterpret_cast<void**>(&oSvCheatsGetBool)
		//)) return 0;//throw std::runtime_error("Unable to hook GetScreenAspectRatio");
	}

	// Actually hook
	if (MH_EnableHook(MH_ALL_HOOKS))
		return 0;//throw std::runtime_error("Unable to enable hooks");

	gui::DestroyDirectX();
	return 1;
}

// FINALLLYY FIXED IT :skull:
void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

long __stdcall hooks::hkEndScene(LPDIRECT3DDEVICE9 pDevice) noexcept
{
	const auto result = oEndScene(&pDevice, pDevice);

	// gotta setup/init only once
	if (!gui::init)
		gui::SetupMenu(pDevice);

	// init New Frame
	gui::NewFrame();

	// Whatever bs u want to render here

	// Dark BG behind the menu
	static Animation animFade(0.5f, EaseInSine, Linear);
	animFade.Update();
	animFade.Switch(gui::bOpen);
	Render::FilledRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, ImColor(0.f, 0.f, 0.f, animFade.getValue(gui::baseFade)));

	// show our rendering's FPS
	Render::OutLinedText(std::format(" [{}fps]", (int)ImGui::GetIO().Framerate).c_str(), 0, 5, ImGui::GetBackgroundDrawList());

	// Draw the aimbot's FOV
	if (cfg.aimbot.DrawFov && cfg.aimbot.Enabled && LocalPlayer.Get()) {
		auto DispSize = ImGui::GetIO().DisplaySize;
		float r = cfg.aimbot.FOV / globals::camFOV * DispSize.x / 2;
		Render::OutLinedCircle(DispSize.x / 2, DispSize.y / 2, r);

		//if (!aimbot::TargetsArr.empty()) ESP::DrawLine(aimbot::TargetsArr[0].second);
	}

	// Render the ESP
	ESP::Render();

	// Render the gui
	gui::Render();

	// Unload code
	if (gui::bUnloaded) {
		// Free console because one is created when using debug build
#ifdef _DEBUG
		FreeConsole(); // Will NOT close the console, but will free it from the process, meaning, you can close it manually without it closing csgo with it.
		/*I did try to attempt to fix this issue and close it but it either didnt work wasnt that conveniet, one of them was using "fclose", it works, but u wont be able to use the stream stdout again if you reinject, thats the problem*/
#endif // _DEBUG

		// Unload the hooks
		hooks::Destroy();
		// Unload dx and imgui etc
		gui::Destroy();
		// return, now nothing is executing at all (hooks are unloaded and everything), atleast shouldnt be... (I hope there is no memory leaks here...)
		return result;
	}

	// End the frame
	gui::EndFrame();

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

	static int lastTick = cmd->tick_count;

	// TODO: fix this bs, no work without sv_cheats on, also distance no work too, bad bad bad (also add keys pls thanks)
	input->isCameraInThirdPerson = cfg.visuals.misc.ThirdPerson && LocalPlayer.Get();

	if (cfg.misc.exploits.InfDuck)
		cmd->buttons |= cmd->IN_BULLRUSH;

	if (cmd->buttons & cmd->IN_ATTACK)
		cmd->viewangles = globals::g_interfaces.Engine->GetViewAngles();

	aimbot::Run(cmd);
	misc::BunnyHop(cmd);
	globals::g_cmd = cmd;

	// dont repeat ticks
	if (lastTick != cmd->tick_count)
	{
		//std::cout << "AimDirection.x: " << cmd->aimdirection.x << "\nAimDirection.y: " << cmd->aimdirection.x << std::endl;
		//std::cout << "cmd->mousedx: " << cmd->mousedx << std::endl;
	}

	lastTick = cmd->tick_count;

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
		aimbot::TargetsArr = aimbot::GetTargetsArr(cfg.aimbot);
		break;
	}

	oFrameStageNotify(globals::g_interfaces.BaseClient, curStage);
}

float __stdcall hooks::hkGetScreenAspectRatio(int viewportWidth, int viewportHeight) noexcept
{
	globals::aspectRatio = cfg.visuals.misc.AspectRatio > 0.f ? cfg.visuals.misc.AspectRatio : oGetScreenAspectRatio(globals::g_interfaces.Engine, viewportWidth, viewportHeight);
	return globals::aspectRatio;
}

float __stdcall hooks::hkGetViewModelFOV() noexcept
{
	return cfg.visuals.viewmodel.ViewModelFOV;
}

void __stdcall hooks::hkOverrideView(CViewSetup* pSetup)
{
	oOverrideView(g_ClientMode, pSetup);

	if (LocalPlayer.Get())
	{
		// Show Real Angles when 3rd Person :) (no workie :( gotta use prediction, thats what im planning on, its a TODO)
		//if (hooks::input->isCameraInThirdPerson)
		//	*(math::Vector*)(LocalPlayer.Get() + offsets::deadFlag + 0x4) = math::Vector(0, 89, 0);
	}

	// not perfect but eh im okay
	//if (hooks::input->isCameraInThirdPerson) // verry shitty dont use (big TODO: IMPROVE THIRD PERSON FFS!!!)
	//	pSetup->origin += input->cameraOffset.z * cfg.visuals.misc.TPDistance;

	if (cfg.visuals.misc.SteadyCam) {
		pSetup->angles = globals::g_interfaces.Engine->GetViewAngles(); // LOL xD
	}

	if (LocalPlayer.Get() && LocalPlayer.isScoped() && cfg.visuals.misc.noZoon) {
		pSetup->fov = cfg.visuals.misc.camFOV;
	}
	else {
		pSetup->fov += (cfg.visuals.misc.camFOV - 90.0f);
	}

	globals::camFOV = pSetup->fov;
}

bool __stdcall hooks::hkShouldDrawViewModel() noexcept
{
	auto result = oShouldDrawViewModel(g_ClientMode);

	if (!result && cfg.visuals.viewmodel.AlwaysDraw)
		result = 1;

	return result;
}

bool __stdcall hooks::hkSvCheatsGetBool(void* pConVar) noexcept
{
	if ((uintptr_t*)_ReturnAddress() == CAM_THINK && cfg.visuals.misc.ThirdPerson)
		return true;

	return oSvCheatsGetBool(pConVar);
}
