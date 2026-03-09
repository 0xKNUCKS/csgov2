#include "GUI.h"
#include "dx9/Drawing/drawing.h"
#include "SDK/Globals/Globals.h"
#include "lib/Hooks/hook.h"
#include "lib/Error/Log.h"
#include <iostream>
#include <format>

#include "Animation.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "SDK/Entity/localplayer.h"

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
	if (!SetupWindowClass("fffheavy001")) {
		Log::Fatal("GUI", "Failed to create window class");
		return false;
	}

	if (!SetupWindow("dumWin")) {
		Log::Fatal("GUI", "Failed to create window");
		DestroyWindowClass();
		return false;
	}

	if (!SetupDirectX()) {
		Log::Fatal("GUI", "Failed to create D3D device");
		DestroyWindow();
		DestroyWindowClass();
		return false;
	}

	DestroyWindow();
	DestroyWindowClass();

	return true;
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
	menu::SetupTheme();

	ImGui_ImplWin32_Init(gui::Window);
	ImGui_ImplDX9_Init(device);

	gui::init = true;
}

void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Restore the input
	globals::g_interfaces.InputSystem->EnableInput(1);

	// Restore the original WindowProc to the game's window
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
	if (!ImGui::GetCurrentContext())
		return;

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

// render our menu
void gui::Render() noexcept
{
	static Animation animPopUp(0.5f, EaseOutBack, EaseOutSine);
	animPopUp.Update();
	animPopUp.Switch(gui::bOpen);

	static Animation windowFade(0.5f, EaseOutQuart, Linear);
	windowFade.Update();
	windowFade.Switch(gui::bOpen);

	// Don't render anything when fully closed (no ghost windows)
	bool isAnimating = animPopUp.getValue() > 0.01f || windowFade.getValue() > 0.01f;
	if (!gui::bOpen && !isAnimating)
		return;

	auto xWindowPadding = ImGui::GetStyle().WindowPadding.x * 3;
	auto xWindowSize = (menu::kColumnWidth * 2) + xWindowPadding;
	auto windowSize = ImVec2(xWindowSize, xWindowSize * 1.25f);
	auto animatedSize = windowSize * animPopUp.getValue();
	static auto windowPos = ImVec2((ImGui::GetIO().DisplaySize - windowSize) / 2);
	ImGui::SetNextWindowSize(animatedSize);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once); // Only once

	float savedAlpha = ImGui::GetStyle().Alpha;
	ImGui::GetStyle().Alpha = windowFade.getValue();

	std::string playerName = LocalPlayer.Get() ? LocalPlayer->getName() : "Player";
	ImGui::Begin(std::format("cockbalt.solutions - Welcome {}!", playerName).c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	{
		// if the windowPos is updating
		bool isUpdating = false;

		// if the menu is closed, and its not animating
		if (!gui::bOpen && animPopUp.getValue() < 0.2f) {
			// reset the position back after changing it for animation
			ImGui::SetWindowPos(windowPos);
			windowPos = ImGui::GetWindowPos();
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
				menu::LeftGroup("General", 10)
					.Hotkey(cfg.aimbot.Key)
					.Checkbox("Enabled", &cfg.aimbot.Enabled)
					.Checkbox("Silent", &cfg.aimbot.Silent)
					.Slider("FOV", &cfg.aimbot.FOV, 0, 180, "%.1f")
					.Slider("Smooth", &cfg.aimbot.Smooth, 1, 10.0f, cfg.aimbot.Smooth > 1 ? "%.2f" : "None")
					.Slider("Smooth X", &cfg.aimbot.SmoothX, 0.1f, 3.0f, "%.2f", "Pitch smooth multiplier")
					.Slider("Smooth Y", &cfg.aimbot.SmoothY, 0.1f, 3.0f, "%.2f", "Yaw smooth multiplier")
					.Checkbox("Aim At Friendly", &cfg.aimbot.FriendlyFire)
					.Checkbox("Visibility Check", &cfg.aimbot.VisibilityCheck)
					.Checkbox("Auto Shoot", &cfg.aimbot.AutoShoot)
					.End();

				menu::RightGroup("Target", 3)
					.Combo("Aim Bone", &cfg.aimbot.AimBone, "Head\0Neck\0Chest\0Stomach\0")
					.Slider("Auto Shoot FOV", &cfg.aimbot.AutoShootFov, 0.5f, 10.0f, "%.1f",
						"How close aim must be to target to auto-fire")
					.Checkbox("FOV Circle", &cfg.aimbot.DrawFov)
					.End();

				menu::LeftGroup("Performance", 1)
					.SliderInt("Max Players Scan", &cfg.aimbot.MaxPlayersInFov, 2, 20, "%d",
						"Max Amount of Players Scanned inside of the aim FOV")
					.End();

				menu::RightGroup("Recoil Control", 6)
					.Checkbox("Enabled##RCS", &cfg.aimbot.RCS)
					.Checkbox("Standalone", &cfg.aimbot.StandaloneRCS,
						"RCS works even without an aimbot target")
					.Slider("Pitch (X)", &cfg.aimbot.RCSAmountX, 0.0f, 2.0f, "%.2f",
						"Vertical recoil compensation (2.0 = full)")
					.Slider("Yaw (Y)", &cfg.aimbot.RCSAmountY, 0.0f, 2.0f, "%.2f",
						"Horizontal recoil compensation (2.0 = full)")
					.SliderInt("Start Bullet", &cfg.aimbot.RCSStartBullet, 1, 10, "%d",
						"Start compensating after N shots")
					.Slider("Smoothing", &cfg.aimbot.RCSSmooth, 1.0f, 10.0f, "%.1f",
						"How smoothly to apply RCS (1 = instant)")
					.End();

				menu::EndRow();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Visuals"))
			{
				menu::LeftGroup("Player", 8)
					.Checkbox("Enabled", &cfg.visuals.Enabled)
					.CheckboxCombo("Bounding Box", &cfg.visuals.esp.BoundingBox, "##ESPboxType", &cfg.visuals.esp.boxType, "Outlined\0Filled\0Box3d\0Corners\0")
					.Checkbox("Show Skeleton", &cfg.visuals.esp.Skeleton)
					.Checkbox("Health Bar", &cfg.visuals.esp.HealthBar)
					.Checkbox("Snap Lines", &cfg.visuals.esp.Lines)
					.Checkbox("Display Name", &cfg.visuals.esp.Name)
					.Checkbox("Show Dormant", &cfg.visuals.esp.Dormant, "Show players that are not updated by the server. (kinda useless)")
					.Checkbox("Show Friendly", &cfg.visuals.Friendly)
					.End();

				menu::RightGroup("Misc", 10)
					.Checkbox("Third Person", &cfg.visuals.misc.ThirdPerson)
					.Slider("Distance", &cfg.visuals.misc.TPDistance, 0.1f, 5.0f)
					.Slider("Aspect Ratio", &cfg.visuals.misc.AspectRatio, 0.0f, 3.0f)
					.Slider("Cam Fov", &cfg.visuals.misc.camFOV, 40.f, 160.0f)
					.Checkbox("Steady Cam", &cfg.visuals.misc.SteadyCam, "Terminates the shaking effects in your Camera.")
					.Checkbox("No Zoom", &cfg.visuals.misc.NoZoom, "Eliminates the zoom effect when using Scoping.")
					.Space()
					.SubSection("View Model", [](auto& s) {
						s.Slider("FOV", &cfg.visuals.viewmodel.ViewModelFOV, 60.f, 140.0f)
						 .Checkbox("Always Draw", &cfg.visuals.viewmodel.AlwaysDraw);
					})
					.End();

				menu::EndRow();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{
				menu::LeftGroup("Movement", 3)
					.Checkbox("Bunny Hop", &cfg.misc.movement.BunnyHop)
					.Checkbox("Auto-Strafe", &cfg.misc.movement.Strafe)
					.Checkbox("Air Duck", &cfg.misc.movement.AirDuck)
					.End();

				menu::LeftGroup("Exploits", 1)
					.Checkbox("Infinite Duck", &cfg.misc.exploits.InfDuck)
					.End();

				menu::EndRow();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Settings"))
			{
				menu::Checkbox("Show Debug Window", &cfg.settings.ShowDebug);
				if (menu::Button("Unload [Pause]"))
					bUnloaded = true;
				menu::Slider("Animation Speed", &cfg.settings.AnimSpeed, 0.5f, 4.f, "%.2f",
					"Modify the menu's animation speed.\n including the fade-in and out speed, etc");

				ImGui::Spacing();

				static char cfgName[64] = "default";
				menu::Section("Config")
					.InputText("##CfgName", cfgName, sizeof(cfgName))
					.Button("Save", [&]{ cfg.Save(cfgName); })
					.SameLine()
					.Button("Load", [&]{ cfg.Load(cfgName); })
					.SameLine()
					.Button("Reset", [&]{ cfg.Reset(); })
					.ListBox("Configs", "##CfgList", Config::ListConfigs(), cfgName,
						[&](const std::string& name) {
							strncpy_s(cfgName, name.c_str(), sizeof(cfgName) - 1);
							cfg.Load(name);
						})
					.End();

				menu::Gap();

				static int curOption = 1;
				const float ThicknessOptions[3] = { 1.f, 4.f, 8.f };
				cfg.settings.mouseTracer.TrailThickness = ThicknessOptions[curOption];

				menu::Section("Mouse Tracer")
					.Checkbox("Enabled##MouseTracer", &cfg.settings.mouseTracer.Enabled, "Creates a trail behind your mouse tracing it!")
					.SliderInt("Trail Length", &cfg.settings.mouseTracer.TrailLength, 15, 100)
					.Text("Trail Thickness")
					.Combo("Thickness", &curOption, "Slim\0Thick\0Bold\0")
					.ColorPicker("Color", cfg.settings.mouseTracer.Color)
					.ColorPicker("Second Color", cfg.settings.mouseTracer.SecondColor)
					.Checkbox("Always On", &cfg.settings.mouseTracer.AlwaysOn, "Always show the tracer, even when the menu is closed.")
					.End();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

#ifdef _DEBUG
	if (gui::bOpen) {
		ImGui::Begin("Style Editor");
		ImGui::ShowStyleEditor();
		ImGui::End();
	}
#endif // _DEBUG

	if (cfg.settings.ShowDebug && gui::bOpen) {
		gui::DebugWindow();
	}

	// Restore alpha to what it was before we modified it
	ImGui::GetStyle().Alpha = savedAlpha;
}

void gui::DebugWindow() noexcept
{
	ImGui::Begin("Debug##DebugGame", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable AA", &hooks::viewRealAngles);
		if (ImGui::Button("Run ClientCmdUnrestricted"))
		{
			globals::g_interfaces.Engine->ClientCmdUnrestricted("say Hello World!");
			globals::g_interfaces.Engine->ClientCmdUnrestricted("echo [Info] Hit Miss Wowowow!!");
		}
	}

	if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		math::Vector origin = LocalPlayer.Get() ? LocalPlayer->getAbsOrigin() : math::Vector{ 0, 0, 0 };
		ImGui::Text("Origin: %.1f, %.1f, %.1f", origin.x, origin.y, origin.z);

		if (LocalPlayer.Get()) {
			bool onGround = LocalPlayer->flags() & PlayerFlag_OnGround;
			bool crouched = LocalPlayer->flags() & PlayerFlag_Crouched;
			bool partialGround = LocalPlayer->flags() & PlayerFlag_PartialGround;
			ImGui::Text("Flags: OnGround=%d  Crouched=%d  PartialGround=%d", onGround, crouched, partialGround);
		}

		ImGui::Text("LocalPlayerIdx: %d", globals::g_interfaces.Engine->GetLocalPlayerIdx());
		ImGui::Text("IsInGame: %d", globals::g_interfaces.Engine->IsInGame());
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Text("Offset: %.1f, %.1f, %.1f", hooks::input->cameraOffset.x, hooks::input->cameraOffset.y, hooks::input->cameraOffset.z);
		ImGui::Text("ThirdPerson: %d", hooks::input->isCameraInThirdPerson);
		if (ImGui::Button("Toggle Third Person"))
			hooks::input->isCameraInThirdPerson = !hooks::input->isCameraInThirdPerson;
		ImGui::SliderFloat("Camera Z", &hooks::input->cameraOffset.z, 0, 800);
	}

	if (ImGui::CollapsingHeader("Interfaces"))
	{
		ImGui::Text("BaseClient   = 0x%p", globals::g_interfaces.BaseClient);
		ImGui::Text("ClientEntity = 0x%p", globals::g_interfaces.ClientEntity);
		ImGui::Text("Engine       = 0x%p", globals::g_interfaces.Engine);
	}

	if (ImGui::CollapsingHeader("GlobalVars"))
	{
		ImGui::Text("absoluteframetime: %f", hooks::GlobalVars->absoluteframetime);
		ImGui::Text("curtime:           %f", hooks::GlobalVars->curtime);
		ImGui::Text("frametime:         %f", hooks::GlobalVars->frametime);
		ImGui::Text("maxClients:        %d", hooks::GlobalVars->maxClients);
	}

	if (globals::g_cmd && ImGui::CollapsingHeader("UserCmd"))
	{
		ImGui::SliderFloat3("AimDirection", &globals::g_cmd->aimdirection.x, 0, 1000);
		ImGui::SliderFloat("SideMove", &globals::g_cmd->sidemove, 0, 1000);
		ImGui::SliderFloat("ForwardMove", &globals::g_cmd->forwardmove, 0, 1000);
		ImGui::Text("tick_count: %d", globals::g_cmd->tick_count);
		ImGui::SliderFloat3("viewangles", &globals::g_cmd->viewangles.x, -180, 180);
	}

	ImGui::Text("Window: [%.0f, %.0f] %.0fx%.0f", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

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

	// if the menu is enabled, then disable the input, vice versa
	g_interfaces.InputSystem->EnableInput(!gui::bOpen);

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


