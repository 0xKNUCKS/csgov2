#include "GUI.h"
#include "Drawing.h"
#include "Globals.h"
#include "hook.h"
#include <iostream>
#include <format>

#include "../ext/AnimationLib/Animation.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ext/ImGui/imgui_internal.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

// Window Process
LRESULT CALLBACK WindowProcess(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

// Setup and init the Window Class
bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	gui::WindowClass.cbSize = sizeof(WNDCLASSEX);
	gui::WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	gui::WindowClass.lpfnWndProc = DefWindowProc;
	gui::WindowClass.cbClsExtra = 0;
	gui::WindowClass.cbWndExtra = 0;
	gui::WindowClass.hInstance = GetModuleHandle(NULL);
	gui::WindowClass.hIcon = NULL;
	gui::WindowClass.hCursor = NULL;
	gui::WindowClass.hbrBackground = NULL;
	gui::WindowClass.lpszMenuName = NULL;
	gui::WindowClass.lpszClassName = windowClassName;
	gui::WindowClass.hIconSm = NULL;

	// register class
	::RegisterClassEx(&gui::WindowClass);

	return 1;
}

// Destroy the Window Class after finishing from using it
void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		gui::WindowClass.lpszClassName,
		gui::WindowClass.hInstance
	);
}

// Create a dummy window
bool gui::SetupWindow(const char* windowName) noexcept
{
	// create a temp window
	gui::Window = CreateWindow(
		gui::WindowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		NULL,
		NULL,
		gui::WindowClass.hInstance,
		NULL
	);

	if (!gui::Window)
		return 0;

	return 1;
}

// Destroy it after finishing from it
void gui::DestroyWindow() noexcept
{
	if (gui::Window)
		DestroyWindow(gui::Window);
}

// as it says lol
bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return 0;

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = (CreateFn)(GetProcAddress(
		handle,
		"Direct3DCreate9"
	));

	if (!create)
		return 0;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return 0;

	D3DDISPLAYMODE displayMode;
	if (d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode) < 0)
		return 0;

	D3DPRESENT_PARAMETERS params = {};
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = displayMode.Format;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = gui::Window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (gui::d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		gui::Window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&params,
		&gui::device
	) < 0) {
		DestroyDirectX();
		return 0;
	}

	return 1;
}

void gui::DestroyDirectX() noexcept
{
	if (gui::device)
	{
		gui::device->Release();
		gui::device = NULL;
	}

	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}

// setup dummy device
bool gui::Setup()
{
	if (!SetupWindowClass("fffheavy001"))
		throw std::runtime_error("Failed to Create Window Class.");
	
	if (!SetupWindow("dumWin"))
		throw std::runtime_error("Failed to Create Window.");
	
	if (!SetupDirectX())
		throw std::runtime_error("Failed to Create Device.");
	
	DestroyWindow();
	DestroyWindowClass();

	return 1;
}

// all callback function to Enumare through windows and find the correct one used for "EnumWindows"
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	gui::Window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	gui::Window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return gui::Window;
}

void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	do
	{
		gui::Window = ::FindWindow("Valve001", NULL);
	} while (gui::Window == NULL);

	gui::oWindowProc = reinterpret_cast<WNDPROC>(
		SetWindowLongA(
		gui::Window,
		GWL_WNDPROC,
		(LONG_PTR)WindowProcess));

	ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	ui::SetupTheme();

	ImGui_ImplWin32_Init(gui::Window);
	ImGui_ImplDX9_Init(device);

	gui::init = true;
}

void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongA(
		gui::Window,
		GWLP_WNDPROC,
		(LONG_PTR)(gui::oWindowProc)
	);

	DestroyDirectX();
}

void gui::NewFrame() noexcept
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndFrame() noexcept
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

// render our menu
void gui::Render() noexcept
{

#ifdef _DEBUG
	ImGui::Begin("Style Editor");
	ImGui::ShowStyleEditor();
	ImGui::End();
#endif // _DEBUG

	static Animation animPopUp(0.5f, EaseOutBack, EaseOutSine);
	animPopUp.Update();
	animPopUp.Switch(gui::bOpen);

	static Animation windowFade(1.f, EaseOutQuart, Linear);
	windowFade.Update();
	windowFade.Switch(gui::bOpen);

	auto xWindowPadding = ImGui::GetStyle().WindowPadding.x * 3;
	auto xWindowSize = (270 * 2) + xWindowPadding; // 270 because thats the width size i use for my groups, * 2 for 2 collums
	auto windowSize = ImVec2(xWindowSize, xWindowSize * 1.25f);
	auto animatedSize = windowSize * animPopUp.getValue();
	static auto windowPos = ImVec2((ImGui::GetIO().DisplaySize - windowSize) / 2);
	ImGui::SetNextWindowSize(animatedSize);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once); // Only once

	ImGui::Begin(std::format("cockbalt.solutions - Welcome {}!", LocalPlayer.GetName()).c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	{
		ImGui::GetStyle().Alpha = windowFade.getValue();

		// if the windowPos is updating
		bool isUpdating = false;

		// if the menu is closed, and its not animating
		if (!gui::bOpen && animPopUp.getValue() < 0.2f) {
			// reset the position back after changing it for animation
			ImGui::SetWindowPos(windowPos);
			windowPos = ImGui::GetWindowPos();
			ImGui::GetStyle().Alpha = 0.f;
			isUpdating = true;
		}
		else if (animPopUp.getValue() == 1.f) { // if the menu is up, and the animation is also finished
			windowPos = ImGui::GetWindowPos();
			isUpdating = true;
		}

		// if the menu is being dragged, also update the position
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
			&& ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			windowPos = ImGui::GetWindowPos();
			isUpdating = true;
		}

		if (!isUpdating)
		{
			ImGui::SetWindowPos(windowPos + (windowSize - animatedSize) / 2);
		}

		{
			static Animation txtFade(0.25f, Linear, Linear);
			txtFade.Update();
			animPopUp.getValue() == 1.f ? txtFade.Switch(1) : txtFade.Switch(0);

			const char* txt = "(Build: " __DATE__ " - " __TIME__ ")";
			ImVec2 txtSize = ImGui::CalcTextSize(txt);
			Render::OutLinedText(txt, (ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - (txtSize.x)), (ImGui::GetWindowPos().y +ImGui::GetWindowSize().y + 3), ImGui::GetForegroundDrawList(), ImColor(1.f, 1.f, 1.f, txtFade.getValue()));
		}

		if (ImGui::BeginTabBar("##TabsBar"))
		{
			if (ImGui::BeginTabItem("Aim"))
			{
				ui::BeginGroup(ImVec2(270, 170), "General");

				ImGui::Checkbox("Enabled", &cfg.aimbot.Enabled);
				ImGui::Checkbox("Silent", &cfg.aimbot.Silent);
				ImGui::SliderFloat("##FOVval", &cfg.aimbot.FOV, 0, 180, "FOV %.1f");
				ImGui::SliderFloat("##Smoothval", &cfg.aimbot.Smooth, 1, 10.0f, cfg.aimbot.Smooth > 1 ? "Smooth %.2f" : "Smooth None");
				ImGui::Checkbox("Aim At Friendly", &cfg.aimbot.FriendlyFire);
				ui::EndGroup();

				ImGui::SameLine();

				// Right Side
				ui::BeginGroup(ImVec2(270, 70), "Visuals");
				ImGui::Checkbox("FOV Circle", &cfg.aimbot.DrawFov);
				ui::EndGroup();

				ImGui::EndTabItem();

				ui::BeginGroup(ImVec2(270, 70), "Performance");

				ImGui::SliderInt("##MaxPlayersScanval", &cfg.aimbot.MaxPlayersInFov, 2, 20, "Max Players Scan %d");
				ui::HelpMarker("Max Amount of Players Scanned inside of the aim FOV");
				ui::EndGroup();
			}

			if (ImGui::BeginTabItem("Visuals"))
			{
				ui::BeginGroup(ImVec2(270, 150), "Player");
				ImGui::Checkbox("Enabled", &cfg.visuals.Enabled);
				ImGui::Checkbox("Show Friendly", &cfg.visuals.Friendly);
				ImGui::Checkbox("Snap Lines", &cfg.visuals.esp.Lines);
				ImGui::Checkbox("Bouding Box", &cfg.visuals.esp.BoudningBox);
				ImGui::Checkbox("Health Bar", &cfg.visuals.esp.HealthBar);
				ui::EndGroup();

				ui::BeginGroup(ImVec2(270, 265), "Misc");
				ImGui::Checkbox("Third Person", &cfg.visuals.misc.ThirdPerson);
				ImGui::SliderFloat("##ThirdPersonDistance", &cfg.visuals.misc.TPDistance, 0.0f, 3.0f, "Distance %.2f");
				ImGui::SliderFloat("##AspectRatio", &cfg.visuals.misc.AspectRatio, 0.0f, 3.0f, "Aspect Ratio %.2f");
				ImGui::SliderFloat("##CAMFOV", &cfg.visuals.misc.camFOV, 40, 160.0f, "Cam Fov %.2f");
				ImGui::Checkbox("Steady Cam", &cfg.visuals.misc.SteadyCam); ui::HelpMarker("Terminates the shaking effects in your Camera.");
				ImGui::Checkbox("No Zoom", &cfg.visuals.misc.noZoon); ui::HelpMarker("Eliminates the zoom effect when using Scoping.");
				ImGui::Spacing();

				ui::BeginOutlineGroup("View Model");
				ImGui::SliderFloat("##ViewModelFOV", &cfg.visuals.viewmodel.ViewModelFOV, 60, 140.0f, "FOV %.2f");
				ImGui::Checkbox("Always Draw", &cfg.visuals.viewmodel.AlwaysDraw);
				ui::EndOutlineGroup();

				ui::EndGroup();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{
				ui::BeginGroup(ImVec2(270, 150), "Movement");
				ImGui::Checkbox("Bunny Hop", &cfg.misc.movement.BunnyHop);
				ImGui::Checkbox("Auto-Strafe", &cfg.misc.movement.Strafe);
				ImGui::Checkbox("Air Duck", &cfg.misc.movement.AirDuck);
				ui::EndGroup();

				ui::BeginGroup(ImVec2(270, 80), "Exploits");
				ImGui::Checkbox("Infinite Duck", &cfg.misc.exploits.InfDuck);
				ui::EndGroup();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Settings"))
			{
				//ImGui::Checkbox("Stream Proof", &cfg.settings.StreamProof); // No Work :)
				ImGui::Checkbox("Show Debug Window", &cfg.settings.ShowDebug);
				if (ImGui::Button("Unload [Pause]")) // the "Pause" key will also unload it :)
				{
					bUnloaded = true;
					return;
				}
				ImGui::SliderFloat("##Animations_Speed", &cfg.settings.AnimSpeed, 0.5f, 4.f, "Animations's Speed %.2f");

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	if (cfg.settings.ShowDebug) {
		gui::DebugWindow();
	}
	
}

void gui::DebugWindow() noexcept
{
	ImGui::Begin("DBG Window##DebugGame", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::Button("Use globals::g_interfaces.Engine->ClientCmdUnrestricted"))
	{
		globals::g_interfaces.Engine->ClientCmdUnrestricted("say Hello World!");
		globals::g_interfaces.Engine->ClientCmdUnrestricted("echo [Info] Hit Miss Wowowow!!");
	}

	ImGui::Text(globals::g_interfaces.InputSystem->ButtonCodeToString(ButtonCode_t::KEY_INSERT));

	ImGui::Checkbox("Enable AA", &hooks::viewRealAngles);

	//ImGui::Text("FOV: %.1f\nEntity: %d\nDistance: %.1f", aimbot::Target.fov, aimbot::Target.ent.GetEnt(), aimbot::Target.distance);
	//ImGui::Text("LocalPlayer Name = %s", LocalPlayer.GetName());
	//ImGui::Spacing();
	//ImGui::Text("LocalPlayer.GetTeamId() = %d", LocalPlayer.Get() ? LocalPlayer.GetTeamId() : 99);
	//ImGui::Spacing();
	//ImGui::Text("offsets::m_vecViewOffset = %d", (uintptr_t)globals::g_NetVars.FindOffset("CBasePlayer", "m_vecViewOffset[0]"));
	////ImGui::Spacing();
	//gEntity* enty = (gEntity*)LocalPlayer.Get();
	////math::Vector Pos = LocalPlayer.Get() ? LocalPlayer.GetBonePos(8) : math::Vector{0, 0, 0};
	//math::Vector Pos = LocalPlayer.Get() ? LocalPlayer.GetBonePos(8) : math::Vector{0, 0, 0};
	//ImGui::Text("Local Head Pos: x.%.1f, y.%.1f, z.%.1f", Pos.x, Pos.y, Pos.z);
	//ImGui::Spacing();
	math::Vector Orgin = LocalPlayer.GetEnt() ? LocalPlayer.GetPos() : math::Vector{0, 0, 0};
	ImGui::Text("Local Player Orgin: x.%.1f, y.%.1f, z.%.1f", Orgin.x, Orgin.y, Orgin.z);
	bool Flag = LocalPlayer.Get() ? LocalPlayer.Flags() & PlayerFlag_OnGround : 0;
	ImGui::Text("Local Player OnGround Flag: %d", Flag);
	bool Flag2 = LocalPlayer.Get() ? LocalPlayer.Flags() & PlayerFlag_Crouched : 0;
	ImGui::Text("Local Player Crouched Flag: %d", Flag2);
	bool Flag3 = LocalPlayer.Get() ? LocalPlayer.Flags() & PlayerFlag_PartialGround : 0;
	ImGui::Text("Local Player PartialGround Flag: %d", Flag2);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.Engine->GetLocalPlayerIdx() = %d", globals::g_interfaces.Engine->GetLocalPlayerIdx());
	ImGui::Spacing();
	ImGui::Text("globals::g_interfaces.Engine->IsInGame() = %d", globals::g_interfaces.Engine->IsInGame());
	ImGui::Spacing();
	ImGui::Text("g_interfaces.BaseClient = 0x%d", globals::g_interfaces.BaseClient);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.ClientEntity = 0x%d", globals::g_interfaces.ClientEntity);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.Engine = 0x%d", globals::g_interfaces.Engine);
	ImGui::Spacing();
	ImGui::Text("input->cameraOffset = %.1f, %.1f, %.1f", hooks::input->cameraOffset.x, hooks::input->cameraOffset.y, hooks::input->cameraOffset.z);
	ImGui::Text("input->isCameraInThirdPerson = %d", hooks::input->isCameraInThirdPerson);
	if (ImGui::Button("Enable/Disable Third Person"))
		hooks::input->isCameraInThirdPerson = !hooks::input->isCameraInThirdPerson;
	ImGui::SliderFloat("Camera Z axis", &hooks::input->cameraOffset.z, 0, 800);
	ImGui::Text("GlobalVars debug");
	ImGui::Text("hooks::GlobalVars->absoluteframetime = %f\nhooks::GlobalVars->curtime = %f\nhooks::GlobalVars->frametime = %f\nhooks::GlobalVars->maxClients %d\n",
		hooks::GlobalVars->absoluteframetime, hooks::GlobalVars->curtime, hooks::GlobalVars->frametime, hooks::GlobalVars->maxClients);

	ImGui::Spacing();
	ImGui::Text("WindowPos[%f, %f], WindowSize[%f, %f]", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

	ImGui::Spacing();
	if (globals::g_cmd)
	{
		ImGui::SliderFloat3("cmd->AimDirection", &globals::g_cmd->aimdirection.x, 0, 1000);
		ImGui::SliderFloat("cmd->SideMove", &globals::g_cmd->sidemove, 0, 1000);
		ImGui::SliderFloat("cmd->ForwardMove", &globals::g_cmd->forwardmove, 0, 1000);
		ImGui::Text("globals::g_cmd->tick_count: %d", globals::g_cmd->tick_count);
		ImGui::SliderFloat3("cmd->viewangles", &globals::g_cmd->viewangles.x, -180, 180);
	}

	ImGui::End();
}

using namespace globals;

LRESULT CALLBACK WindowProcess(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	// toggle menu wewe
	if (GetAsyncKeyState(gui::menuKey) & 1)
		gui::bOpen = !gui::bOpen;

	// Unload key "wewe"
	if (GetAsyncKeyState(gui::unloadKey) & 1)
		gui::bUnloaded = true;

	// pass messages to imgui, to be able to click and stuff
	if (gui::bOpen &&
		ImGui_ImplWin32_WndProcHandler(
			hWnd,
			msg,
			wParam,
			lParam
		)) {
		return true;
	}

	return CallWindowProc(
		gui::oWindowProc,
		hWnd,
		msg,
		wParam,
		lParam
	);
}


