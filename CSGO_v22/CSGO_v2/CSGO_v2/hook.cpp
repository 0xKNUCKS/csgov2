#include "hook.h"
#include "../ext/AnimationLib/Animation.h"
#include "hooksManager.h"
#include "localplayer.h"

// FINALLLYY FIXED IT :skull:
void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) noexcept
{
	const auto result = hooks::d3dDeviceHk.getOriginal<long, index::d3d9Device::EndScene>(pDevice)(pDevice, pDevice);

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

		//if (aimbot::Target.ent) {
		//	auto Ent = aimbot::Target.ent;
		//	math::Vector HeadPos = Ent->getBonePosFromChache(8);
		//	math::Vector sHeadPos = math::Vector();
		//	if (utils::WolrdToScreen(HeadPos, sHeadPos))
		//	{
		//		auto DisplayCenter = ImGui::GetIO().DisplaySize / 2;
		//		Render::OutLinedCircle(sHeadPos.x, sHeadPos.y, 20, ImColor(255, 0, 0));
		//		Render::OutLinedText(std::format("Target's name: {}\nTarget's Index: {}\nTarget's FOV: {}\nDistance: {}", Ent->getName(), Ent-/>index(), /aimbot::Target.fov, aimbot::Target.distance).c_str(), DisplayCenter.x, DisplayCenter.y, ImGui::GetBackgroundDrawList(), /ImColor(255, 255, /255));
		//	}
		//}
	}

	// Render the ESP
	ESP::Render();

	// Render the gui
	gui::Render();

	// Draw the Mouse Tracer
	if (cfg.settings.mouseTracer.Enabled && (cfg.settings.mouseTracer.AlwaysOn && gui::bOpen))
	{
		ImVec2 mousePos = ImGui::GetIO().MousePos;
		static std::vector<ImVec2> mousePoints = {};

		mousePoints.insert(mousePoints.begin(), mousePos);
		mousePoints.resize(cfg.settings.mouseTracer.TrailLength); // limit the points to a certrain value (in a way the "length")

		for (int i = 0; i < mousePoints.size(); i++)
		{
			ImVec2 Point = mousePoints[i];
			float scale = (float)(1.0f - (float)i / (mousePoints.size() - 1));

			ImVec4 FirstColor = cfg.settings.mouseTracer.Color;
			ImVec4 SecondColor = cfg.settings.mouseTracer.SecondColor;
			ImColor FinalColor = ImColor(SecondColor + (FirstColor - SecondColor) * ImVec4(scale, scale, scale, 0));
			FinalColor.Value.w = 0.85f * scale;

			if (i > 0) {
				ImGui::GetForegroundDrawList()->AddLine(mousePoints[i - 1] - ImVec2(0.5, 0.5), mousePoints[i] - ImVec2(0.5, 0.5),
					FinalColor, cfg.settings.mouseTracer.TrailThickness * scale);
			}
		}
	}

	// End the frame
	gui::EndFrame();

	// Unload code (dosent work)
	if (gui::bUnloaded) {
#ifdef _DEBUG
		// Free the console because one is created when using debug build
		::ShowWindow(GetConsoleWindow(), SW_HIDE);
		FreeConsole();
#endif // _DEBUG

		// Unload the hooks
		hooks::Destroy();
		// Unload dx and imgui etc
		gui::Destroy();
		// the end, now nothing is executing at all (hooks are unloaded and everything), atleast shouldnt be... (I hope there is no memory leaks here...)
	}

	return result;
}

HRESULT __stdcall hkReset(IDirect3DDevice9* Device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = hooks::d3dDeviceHk.getOriginal<HRESULT, index::d3d9Device::Reset>(Device, params)(Device, Device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool* bSendPacket) noexcept
{
	hooks::BaseClientHk.callOriginal<void, index::BaseClient::CreateMove>(sequence_number, input_sample_frametime, active);

	if (!bSendPacket)
		return;

	CUserCmd* cmd = hooks::input->getUserCmd(0, sequence_number);

	if (!cmd || !cmd->command_number)
		return;

	static int lastTick = cmd->tick_count;

	// TODO: fix this bs, no work without sv_cheats on, also distance no work too, bad bad bad (also add keys pls thanks)
	hooks::input->isCameraInThirdPerson = cfg.visuals.misc.ThirdPerson && LocalPlayer.Get();

	if (cfg.misc.exploits.InfDuck)
		cmd->buttons |= cmd->IN_BULLRUSH;

	if (cmd->buttons & cmd->IN_ATTACK)
		cmd->viewangles = globals::g_interfaces.Engine->GetViewAngles();

	//aimbot::Run(cmd);
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

	VerifiedUserCmd* verified = hooks::input->getVerifiedUserCmd(sequence_number);
	if (verified)
		*verified = VerifiedUserCmd(*cmd);
}

__declspec(naked) void __stdcall hkCreateMoveProxy(int sequenceNumber, float inputSampleTime, bool active) noexcept
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
		call hkCreateMove
		pop ebx
		pop ebp
		ret 0Ch
	}
}

void __stdcall hkFrameStageNotify(ClientFrameStage_t curStage) noexcept
{
	using enum ClientFrameStage_t;

	hooks::BaseClientHk.callOriginal<void, index::BaseClient::FrameStageNotify>(curStage);

	switch (curStage)
	{
	case FRAME_START:
		// to be used for WorldToScreen.
		globals::game::viewMatrix = globals::g_interfaces.Engine->WorldToScreenMatrix();
		break;
		break;
	}
}

float __stdcall hkGetScreenAspectRatio(int viewportWidth, int viewportHeight) noexcept
{
	globals::aspectRatio = cfg.visuals.misc.AspectRatio > 0.f ? cfg.visuals.misc.AspectRatio : hooks::EngineHk.callOriginal<float, index::Engine::GetScreenAspectRatio>(viewportWidth, viewportHeight);
	return globals::aspectRatio;
}

float __stdcall hkGetViewModelFOV() noexcept
{
	return cfg.visuals.viewmodel.ViewModelFOV;
}

void __stdcall hkOverrideView(CViewSetup* pSetup)
{
	hooks::ClientModeHk.callOriginal<void, index::ClientMode::OverrideView>(pSetup);

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

	// store the zoom sens value
	static float temp_zoom_sensitivity_ratio_mouse = globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->GetFloat();
	globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(temp_zoom_sensitivity_ratio_mouse); // restore the zoom sens ratio

	// if currently zooming
	if (LocalPlayer.Get() && cfg.visuals.misc.noZoon && ( LocalPlayer->isScoped() || pSetup->fov != cfg.visuals.misc.camFOV)) {
		pSetup->fov = cfg.visuals.misc.camFOV;
		globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(0.f);
		
	}
	else {
		pSetup->fov += (cfg.visuals.misc.camFOV - 90.0f);
	}

	globals::camFOV = pSetup->fov;
}

bool __stdcall hkShouldDrawViewModel() noexcept
{
	auto result = hooks::ClientModeHk.callOriginal<bool, index::ClientMode::ShouldDrawViewModel>();

	if (!result && cfg.visuals.viewmodel.AlwaysDraw)
		result = 1;

	return result;
}

void __stdcall hkLockCursor() noexcept
{
	if (gui::bOpen) {
		globals::g_interfaces.Surface->UnlockCursor();
		return;
	}

	hooks::SurfaceHk.callOriginal<void, index::Surface::LockCursor>();
}

bool hooks::Setup()
{
	// Globals Initialization
	ClientMode = **reinterpret_cast<void***>((*reinterpret_cast<unsigned int**>(globals::g_interfaces.BaseClient))[10] + 5);
	input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[16] + 1);
	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[11] + 10);


	// manually call if youre gonna use DETOUR hooking
	if (MH_Initialize())
		return 0;//throw std::runtime_error("Unable to initialize Hooks");

	// gui::Device hooks
	{
		d3dDeviceHk.init(gui::device, DETOUR);
		d3dDeviceHk.hook(index::d3d9Device::EndScene, hkEndScene);
		d3dDeviceHk.hook(index::d3d9Device::Reset, hkReset);
	}

	// g_ClientMode hooks
	{
		ClientModeHk.init(ClientMode, DETOUR);
		ClientModeHk.hook(index::ClientMode::GetViewModelFOV, hkGetViewModelFOV);
		ClientModeHk.hook(index::ClientMode::OverrideView, hkOverrideView);
		ClientModeHk.hook(index::ClientMode::ShouldDrawViewModel, hkShouldDrawViewModel);
	}

	// globals::g_interfaces.BaseClient hooks
	{
		BaseClientHk.init(globals::g_interfaces.BaseClient, DETOUR);
		BaseClientHk.hook(index::BaseClient::CreateMove, hkCreateMoveProxy);
		BaseClientHk.hook(index::BaseClient::FrameStageNotify, hkFrameStageNotify);
	}

	// globals::g_interfaces.Engine hooks
	{
		EngineHk.init(globals::g_interfaces.Engine, DETOUR);
		EngineHk.hook(index::Engine::GetScreenAspectRatio, hkGetScreenAspectRatio);
	}

	// globals::g_interfaces.Surface
	{
		SurfaceHk.init(globals::g_interfaces.Surface, DETOUR);
		SurfaceHk.hook(index::Surface::LockCursor, hkLockCursor); // virtual void LockCursor() = 0; - Index 67
	}

	// super duper crash dont uncomment
	{
		//if (MH_CreateHook(
		//	VirtualFunction(svCheatsCVar, 13),
		//	&hkSvCheatsGetBool,
		//	reinterpret_cast<void**>(&oSvCheatsGetBool)
		//)) return 0;//throw std::runtime_error("Unable to hook GetScreenAspectRatio");
	}

	gui::DestroyDirectX();
	return 1;
}
