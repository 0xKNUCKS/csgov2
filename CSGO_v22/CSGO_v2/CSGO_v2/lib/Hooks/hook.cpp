#include "hook.h"
#include <Windows.h>
#include <d3d9.h>
#include "SDK/Classes/classes.h"
#include "SDK/interface/interface.h"
#include "lib/Hooks/GUI/GUI.h"
#include "lib/Error/Log.h"
#include "lib/Error/CrashLog.h"
#include "dx9/Drawing/drawing.h"
#include "SDK/Globals/Globals.h"
#include "Modules/Aimbot/aimbot.h"
#include "Modules/Misc/Misc.h"
#include "Modules/Visuals/ESP.h"
#include "SDK/Classes/ViewSetup/ViewSetup.h"
#include "SDK/Entity/localplayer.h"
#include "lib/Configs/config.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_notify.h"
#include "lib/Notify/Notify.h"
#include "Animation.h"

void hooks::Destroy() noexcept
{
	// FIRST: stop all hook logic immediately so no hook function runs against torn-down state
	setupComplete = false;

	// Remove the vectored exception handler before unloading (dangling pointer = crash)
	if (hVEH) {
		RemoveVectoredExceptionHandler(hVEH);
		hVEH = nullptr;
	}

	// Restore each hookManager properly (handles both DETOUR and VMT)
	d3dDeviceHk.restore();
	ClientModeHk.restore();
	BaseClientHk.restore();
	EngineHk.restore();
	SurfaceHk.restore();

	// Uninit minhook after all hooks are restored
	MH_Uninitialize();
}

// Runs on a separate thread so we don't tear down hooks while inside a hooked function
void hooks::Unload() noexcept
{
	// Stop all hook logic first — no hook function will run past its setupComplete check after this
	setupComplete = false;

	// Wait for any in-flight hook calls to finish (at 60fps a frame is ~16ms, give several frames)
	Sleep(500);

	// Restore WindowProc BEFORE destroying ImGui — prevents WndProc calling into freed ImGui context
	gui::Destroy();

#ifdef _DEBUG
	::ShowWindow(GetConsoleWindow(), SW_HIDE);
	FreeConsole();
#endif

	hooks::Destroy();

	// Actually unload the DLL from the process
	FreeLibraryAndExitThread(hooks::hModule, 0);
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	const auto result = hooks::d3dDeviceHk.getOriginal<long, index::d3d9Device::EndScene>(pDevice)(pDevice, pDevice);

	// Don't touch anything until hooks::Setup() has fully completed
	if (!hooks::setupComplete)
		return result;

	try {
		if (!gui::init)
			gui::SetupMenu(pDevice);

		gui::NewFrame();

		static Animation animFade(0.5f, EaseInSine, Linear);
		animFade.Update();
		animFade.Switch(gui::bOpen);
		Render::FilledRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, ImColor(0.f, 0.f, 0.f, animFade.getValue(gui::baseFade)));

		Render::OutLinedText(std::format(" [{}fps]", (int)ImGui::GetIO().Framerate).c_str(), 0, 5, ImGui::GetBackgroundDrawList());

		if (cfg.aimbot.DrawFov && cfg.aimbot.Enabled && LocalPlayer.Get()) {
			auto DispSize = ImGui::GetIO().DisplaySize;
			float r = cfg.aimbot.FOV / globals::camFOV * DispSize.x / 2;
			Render::OutLinedCircle(DispSize.x / 2, DispSize.y / 2, r);
		}

		if (LocalPlayer.Get() && cfg.misc.movement.BunnyHop) {
			auto DispSize = ImGui::GetIO().DisplaySize;
			auto centerX = DispSize.x / 2;
			auto centerY = DispSize.y / 2;

			auto velocity = LocalPlayer->getVelocity();
			float speed = velocity.length2D();

			Render::OutLinedText(std::format("Speed: {:.1f}", speed).c_str(), centerX - 50, centerY - 100, ImGui::GetBackgroundDrawList(), ImColor(255, 255, 255, 255));
		}

		ESP::Render();

		gui::Render();

		if (cfg.settings.mouseTracer.Enabled && (cfg.settings.mouseTracer.AlwaysOn || gui::bOpen))
		{
			ImVec2 mousePos = ImGui::GetIO().MousePos;
			static std::vector<ImVec2> mousePoints = {};

			mousePoints.insert(mousePoints.begin(), mousePos);
			mousePoints.resize(cfg.settings.mouseTracer.TrailLength);

			if (mousePoints.size() > 1) {
				for (size_t i = 0; i < mousePoints.size(); i++)
				{
					float scale = 1.0f - (float)i / (float)(mousePoints.size() - 1);

					ImVec4 FirstColor(cfg.settings.mouseTracer.Color.r, cfg.settings.mouseTracer.Color.g, cfg.settings.mouseTracer.Color.b, cfg.settings.mouseTracer.Color.a);
					ImVec4 SecondColor(cfg.settings.mouseTracer.SecondColor.r, cfg.settings.mouseTracer.SecondColor.g, cfg.settings.mouseTracer.SecondColor.b, cfg.settings.mouseTracer.SecondColor.a);
					ImColor FinalColor = ImColor(SecondColor + (FirstColor - SecondColor) * ImVec4(scale, scale, scale, 0));
					FinalColor.Value.w = 0.85f * scale;

					if (i > 0) {
						ImGui::GetForegroundDrawList()->AddLine(mousePoints[i - 1] - ImVec2(0.5, 0.5), mousePoints[i] - ImVec2(0.5, 0.5),
							FinalColor, cfg.settings.mouseTracer.TrailThickness * scale);
					}
				}
			}
		}

		// Render notifications (toast popups) on top of everything
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43/255.f, 43/255.f, 43/255.f, 100/255.f));
		ImGui::RenderNotifications();
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(1);

		gui::EndFrame();

		// Unload: spawn a thread so we don't tear down hooks while inside one
		if (gui::bUnloaded) {
			gui::bUnloaded = false;
			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(hooks::Unload), nullptr, 0, nullptr);
		}
	}
	catch (...) {
		CrashLog::Write("[EndScene] C++ exception caught");
		Log::Fatal("Hooks", "Exception in hkEndScene - overlay disabled");
		Log::DumpToFile("csgo_v2_errors.log");
		Notify::Error("Overlay crash caught — check csgo_v2_errors.log");
	}

	return result;
}

HRESULT __stdcall hkReset(IDirect3DDevice9* Device, D3DPRESENT_PARAMETERS* params)
{
	const auto result = hooks::d3dDeviceHk.getOriginal<HRESULT, index::d3d9Device::Reset>(Device, params)(Device, Device, params);
	if (!hooks::setupComplete)
		return result;
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool* bSendPacket)
{
	hooks::BaseClientHk.callOriginal<void, index::BaseClient::CreateMove>(sequence_number, input_sample_frametime, active);

	if (!hooks::setupComplete)
		return;

	if (!bSendPacket)
		return;

	CUserCmd* cmd = hooks::input->getUserCmd(0, sequence_number);

	if (!cmd || !cmd->command_number)
		return;

	static int lastTick = cmd->tick_count;

	// Third person: use netvar reads instead of virtual calls to avoid vtable crash during map transitions
	if (cfg.visuals.misc.ThirdPerson && globals::g_interfaces.Engine->IsInGame() && LocalPlayer.Get()) {
		// Read deadflag netvar directly (0 = alive) - avoids virtual isAlive() call
		bool alive = *(int*)((uintptr_t)LocalPlayer.Get() + offsets::deadFlag) == 0;
		if (alive) {
			hooks::input->isCameraInThirdPerson = true;
			// Eye position = abs origin + view offset
			auto viewOffset = *(math::Vector*)((uintptr_t)LocalPlayer.Get() + offsets::m_vecViewOffset);
			const auto& origin = LocalPlayer->getAbsOrigin();
			hooks::cachedEyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
		} else {
			hooks::input->isCameraInThirdPerson = false;
		}
	} else {
		hooks::input->isCameraInThirdPerson = false;
	}

	if (cfg.misc.exploits.InfDuck)
		cmd->buttons |= cmd->IN_BULLRUSH;

	if (cmd->buttons & cmd->IN_ATTACK)
		cmd->viewangles = globals::g_interfaces.Engine->GetViewAngles();

	//aimbot::Run(cmd);
	aimbot::Run(cmd);
	misc::BunnyHop(cmd);
	globals::g_cmd = cmd;

	lastTick = cmd->tick_count;

	VerifiedUserCmd* verified = hooks::input->getVerifiedUserCmd(sequence_number);
	if (verified)
		*verified = VerifiedUserCmd(*cmd);
}

__declspec(naked) void __stdcall hkCreateMoveProxy(int sequenceNumber, float inputSampleTime, bool active)
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

void __stdcall hkFrameStageNotify(ClientFrameStage_t curStage)
{
	hooks::BaseClientHk.callOriginal<void, index::BaseClient::FrameStageNotify>(curStage);

	if (!hooks::setupComplete)
		return;

	using enum ClientFrameStage_t;
	switch (curStage)
	{
	case FRAME_START:
		// to be used for WorldToScreen.
		globals::game::viewMatrix = globals::g_interfaces.Engine->WorldToScreenMatrix();
		break;
		break;
	}
}

float __stdcall hkGetScreenAspectRatio(int viewportWidth, int viewportHeight)
{
	if (!hooks::setupComplete)
		return hooks::EngineHk.callOriginal<float, index::Engine::GetScreenAspectRatio>(viewportWidth, viewportHeight);
	globals::aspectRatio = cfg.visuals.misc.AspectRatio > 0.f ? cfg.visuals.misc.AspectRatio : hooks::EngineHk.callOriginal<float, index::Engine::GetScreenAspectRatio>(viewportWidth, viewportHeight);
	return globals::aspectRatio;
}

float __stdcall hkGetViewModelFOV()
{
	if (!hooks::setupComplete)
		return hooks::ClientModeHk.callOriginal<float, index::ClientMode::GetViewModelFOV>();

	return cfg.visuals.viewmodel.ViewModelFOV;
}

void __stdcall hkOverrideView(CViewSetup* pSetup)
{
	hooks::ClientModeHk.callOriginal<void, index::ClientMode::OverrideView>(pSetup);
	if (!hooks::setupComplete) return;

	// Third person camera distance with wall collision
	if (hooks::input->isCameraInThirdPerson) {
		float dist = 150.f * cfg.visuals.misc.TPDistance;

		float pitch = pSetup->angles.x * (3.14159265f / 180.f);
		float yaw   = pSetup->angles.y * (3.14159265f / 180.f);

		math::Vector forward;
		forward.x = cosf(pitch) * cosf(yaw);
		forward.y = cosf(pitch) * sinf(yaw);
		forward.z = -sinf(pitch);

		math::Vector eyePos = hooks::cachedEyePos;
		math::Vector camPos;
		camPos.x = eyePos.x - forward.x * dist;
		camPos.y = eyePos.y - forward.y * dist;
		camPos.z = eyePos.z - forward.z * dist;

		// Trace from eye to desired camera position to avoid clipping through walls
		if (globals::g_interfaces.EngineTrace) {
			Ray_t ray(eyePos, camPos);
			ITraceFilter filter(LocalPlayer.Get());
			trace_t trace;

			__try {
				globals::g_interfaces.EngineTrace->TraceRay(ray, MASK_SOLID, filter, trace);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				pSetup->origin = camPos;
				return; // trace failed, use unclipped position
			}

			if (trace.fraction < 1.0f) {
				// Hit a wall — pull camera to the hit point with a small offset
				camPos.x = eyePos.x + (camPos.x - eyePos.x) * trace.fraction * 0.95f;
				camPos.y = eyePos.y + (camPos.y - eyePos.y) * trace.fraction * 0.95f;
				camPos.z = eyePos.z + (camPos.z - eyePos.z) * trace.fraction * 0.95f;
			}
		}

		pSetup->origin = camPos;
	}

	if (cfg.visuals.misc.SteadyCam) {
		pSetup->angles = globals::g_interfaces.Engine->GetViewAngles(); // LOL xD
	}

	// store the zoom sens value
	static float temp_zoom_sensitivity_ratio_mouse = globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->GetFloat();
	globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(temp_zoom_sensitivity_ratio_mouse); // restore the zoom sens ratio

	// if currently zooming
	if (LocalPlayer.Get() && cfg.visuals.misc.NoZoom && ( LocalPlayer->isScoped() || pSetup->fov != cfg.visuals.misc.camFOV)) {
		pSetup->fov = cfg.visuals.misc.camFOV;
		globals::g_interfaces.Cvar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(0.f);
		
	}
	else {
		pSetup->fov += (cfg.visuals.misc.camFOV - 90.0f);
	}

	globals::camFOV = pSetup->fov;
}

bool __stdcall hkShouldDrawViewModel()
{
	auto result = hooks::ClientModeHk.callOriginal<bool, index::ClientMode::ShouldDrawViewModel>();
	if (!hooks::setupComplete) return result;

	if (!result && cfg.visuals.viewmodel.AlwaysDraw)
		result = 1;

	return result;
}

void __stdcall hkLockCursor()
{
	if (!hooks::setupComplete) {
		hooks::SurfaceHk.callOriginal<void, index::Surface::LockCursor>();
		return;
	}

	if (gui::bOpen) {
		globals::g_interfaces.Surface->UnlockCursor();
		return;
	}

	hooks::SurfaceHk.callOriginal<void, index::Surface::LockCursor>();
}

// Process-wide crash handler - catches crashes on ANY thread (including game render thread)
static LONG WINAPI GlobalCrashHandler(EXCEPTION_POINTERS* ep)
{
	// Filter out non-fatal exceptions (breakpoints, first-chance, etc.)
	DWORD code = ep->ExceptionRecord->ExceptionCode;
	if (code == EXCEPTION_BREAKPOINT || code == EXCEPTION_SINGLE_STEP ||
		code == DBG_PRINTEXCEPTION_C || code == 0x406D1388 /* SetThreadName */)
		return EXCEPTION_CONTINUE_SEARCH;

	// Log DLL base address so we can compute exact crash offset
	HMODULE hSelf = nullptr;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)GlobalCrashHandler, &hSelf);

	void* addr = ep->ExceptionRecord->ExceptionAddress;
	CrashLog::Writef("[FATAL] Exception on thread %lu (code: 0x%08X, addr: 0x%p, DLL base: 0x%p, offset: 0x%X)",
		GetCurrentThreadId(), code, addr, (void*)hSelf,
		hSelf ? ((uintptr_t)addr - (uintptr_t)hSelf) : 0);

	// Log registers for access violation context
	if (code == 0xC0000005 && ep->ContextRecord) {
		auto ctx = ep->ContextRecord;
		CrashLog::Writef("[FATAL] Registers: EAX=0x%08X EBX=0x%08X ECX=0x%08X EDX=0x%08X ESI=0x%08X EDI=0x%08X EBP=0x%08X ESP=0x%08X EIP=0x%08X",
			ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx, ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp, ctx->Eip);
		// Log access violation details (read/write and target address)
		if (ep->ExceptionRecord->NumberParameters >= 2) {
			CrashLog::Writef("[FATAL] Access violation %s address 0x%08X",
				ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "reading" : "writing",
				(DWORD)ep->ExceptionRecord->ExceptionInformation[1]);
		}
	}

	Log::DumpToFile("csgo_v2_errors.log");

	return EXCEPTION_CONTINUE_SEARCH;
}

bool hooks::Setup()
{
	Log::Info("Hooks", "Starting hook setup...");

	// Install process-wide crash handler so we catch crashes on the game's render thread too
	hVEH = AddVectoredExceptionHandler(1, GlobalCrashHandler);

	// Validate critical interfaces before dereferencing them
	if (!globals::g_interfaces.BaseClient) {
		Log::Fatal("Hooks", "BaseClient is null - cannot initialize hooks");
		return false;
	}
	if (!gui::device) {
		Log::Fatal("Hooks", "D3D device is null - cannot hook rendering");
		return false;
	}

	// Globals Initialization
	CrashLog::Write("[Hooks] Resolving globals (ClientMode, Input, GlobalVars)...");
	ClientMode = **reinterpret_cast<void***>((*reinterpret_cast<unsigned int**>(globals::g_interfaces.BaseClient))[10] + 5);
	input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[16] + 1);
	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(globals::g_interfaces.BaseClient))[11] + 10);

	if (!ClientMode) { Log::Fatal("Hooks", "ClientMode pointer is null"); return false; }
	if (!input)      { Log::Fatal("Hooks", "CInput pointer is null"); return false; }
	if (!GlobalVars) { Log::Fatal("Hooks", "GlobalVars pointer is null"); return false; }

	Log::Info("Hooks", "Globals: ClientMode={:#x}, Input={:#x}, GlobalVars={:#x}",
		(uintptr_t)ClientMode, (uintptr_t)input, (uintptr_t)GlobalVars);

	// manually call if youre gonna use DETOUR hooking
	MH_STATUS mhStatus = MH_Initialize();
	if (mhStatus != MH_OK) {
		Log::Fatal("Hooks", "MH_Initialize failed (MH_STATUS: {})", (int)mhStatus);
		return false;
	}

	bool allOk = true;

	// gui::Device hooks
	CrashLog::Write("[Hooks] Hooking D3D EndScene + Reset...");
	{
		allOk &= d3dDeviceHk.init(gui::device, DETOUR);
		allOk &= d3dDeviceHk.hook(index::d3d9Device::EndScene, hkEndScene);
		allOk &= d3dDeviceHk.hook(index::d3d9Device::Reset, hkReset);
	}

	// g_ClientMode hooks
	CrashLog::Write("[Hooks] Hooking ClientMode...");
	{
		allOk &= ClientModeHk.init(ClientMode, DETOUR);
		allOk &= ClientModeHk.hook(index::ClientMode::GetViewModelFOV, hkGetViewModelFOV);
		allOk &= ClientModeHk.hook(index::ClientMode::OverrideView, hkOverrideView);
		allOk &= ClientModeHk.hook(index::ClientMode::ShouldDrawViewModel, hkShouldDrawViewModel);
	}

	// globals::g_interfaces.BaseClient hooks
	CrashLog::Write("[Hooks] Hooking BaseClient...");
	{
		allOk &= BaseClientHk.init(globals::g_interfaces.BaseClient, DETOUR);
		allOk &= BaseClientHk.hook(index::BaseClient::CreateMove, hkCreateMoveProxy);
		allOk &= BaseClientHk.hook(index::BaseClient::FrameStageNotify, hkFrameStageNotify);
	}

	// globals::g_interfaces.Engine hooks
	CrashLog::Write("[Hooks] Hooking Engine...");
	if (globals::g_interfaces.Engine) {
		allOk &= EngineHk.init(globals::g_interfaces.Engine, DETOUR);
		allOk &= EngineHk.hook(index::Engine::GetScreenAspectRatio, hkGetScreenAspectRatio);
	}
	else {
		Log::Err("Hooks", "Engine interface null - skipping Engine hooks");
	}

	// globals::g_interfaces.Surface
	CrashLog::Write("[Hooks] Hooking Surface...");
	if (globals::g_interfaces.Surface) {
		allOk &= SurfaceHk.init(globals::g_interfaces.Surface, DETOUR);
		allOk &= SurfaceHk.hook(index::Surface::LockCursor, hkLockCursor);
	}
	else {
		Log::Err("Hooks", "Surface interface null - skipping Surface hooks");
	}

	CrashLog::Write("[Hooks] Cleaning up dummy DirectX...");
	gui::DestroyDirectX();

	if (!allOk) {
		Log::Err("Hooks", "Some hooks failed to install - check errors above");
		Log::DumpToFile("csgo_v2_errors.log");
	}
	else {
		CrashLog::Write("[Hooks] All hooks installed successfully");
		Log::Info("Hooks", "All hooks installed successfully");
	}

	// Signal all hook functions that they can start running their logic
	setupComplete = allOk;

	return allOk;
}
