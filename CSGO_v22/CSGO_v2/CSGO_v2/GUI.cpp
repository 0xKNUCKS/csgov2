#include "GUI.h"
#include "Drawing.h"
#include "Globals.h"
#include "hook.h"
#include <iostream>
#include <format>

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

// dx shit
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

void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		gui::WindowClass.lpszClassName,
		gui::WindowClass.hInstance
	);
}

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

void gui::DestroyWindow() noexcept
{
	if (gui::Window)
		DestroyWindow(gui::Window);
}

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

// setup device
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
		gui::Window = GetProcessWindow();
	} while (gui::Window == NULL);

	gui::oWindowProc = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(
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


// render our menu
void gui::Render() noexcept
{

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (gui::bOpen)
		Render::FilledRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, ImColor(0, 0, 0, 50));

	Render::OutLinedRect(0, 0, 100, 100);

	if (cfg.aimbot.DrawFov && cfg.aimbot.Enabled)
		Render::OutLinedCircle(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2, cfg.aimbot.FOV);

	ESP::Render();

	// Menu Code
if (gui::bOpen) {

#ifdef _DEBUG
	ImGui::Begin("Style Editor");
	ImGui::ShowStyleEditor();
	ImGui::End();
#endif // _DEBUG

	ImGui::SetNextWindowSize(ImVec2(540, 650)); // res/2 = x = 270, y = 325

	ImGui::Begin(std::format("cockbalt.solutions - Welcome {}!", LocalPlayer.GetName()).c_str(), &gui::bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	{
		ImVec2 txtSize = ImGui::CalcTextSize("(Build: " __DATE__ " - " __TIME__ ")");
		Render::OutLinedText("(Build: " __DATE__ " - " __TIME__ ")", (ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - (txtSize.x)), (ImGui::GetWindowPos().y + ImGui::GetWindowSize().y + 3), ImGui::GetForegroundDrawList());
	}

	if (ImGui::BeginTabBar("##TabsBar"))
	{
		if (ImGui::BeginTabItem("Aim"))
		{
			ui::BeginGroup(ImVec2(270, 330), "General");

			ImGui::Checkbox("Enabled", &cfg.aimbot.Enabled);
			ImGui::Spacing();
			ImGui::SliderFloat("##FOVval", &cfg.aimbot.FOV, 0, 180, "FOV %.1f"); ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SliderFloat("Smooth", &cfg.aimbot.Smooth, 0, 10.0f, cfg.aimbot.Smooth ? "%.3f" : "None");
			ImGui::Spacing();
			ImGui::Checkbox("Aim At Friendly", &cfg.aimbot.FriendlyFire);
			ImGui::Spacing();

			ui::EndGroup();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Visuals"))
		{
			ui::BeginGroup(ImVec2(270, 150), "Player");
			ImGui::Checkbox("Enabled", &cfg.visuals.Enabled);
			ImGui::Text("Not yet ;)!!");
			ui::EndGroup();

			ui::BeginGroup(ImVec2(270, 150), "Misc");
			ImGui::Checkbox("Draw FOV Circle", &cfg.aimbot.DrawFov);
			ui::EndGroup();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Misc"))
		{
			ImGui::Checkbox("Bunny Hop", &cfg.misc.movement.BunnyHop);
			ImGui::Spacing();
			if (ImGui::Button("UNLOAD"))
			{
				exit(0);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Settings"))
		{
			ImGui::Checkbox("Stream Proof", &cfg.settings.StreamProof);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();


	ImGui::Begin("DBG Window##DebugGame", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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
	math::Vector Orgin = LocalPlayer.Get() ? LocalPlayer.GetPos() : math::Vector{ 0, 0, 0 };
	ImGui::Text("Local Player Orgin: x.%.1f, y.%.1f, z.%.1f", Orgin.x, Orgin.y, Orgin.z);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.Engine->GetLocalPlayerIdx() = %d", globals::g_interfaces.Engine->GetLocalPlayerIdx());
	ImGui::Spacing();
	ImGui::Text("globals::g_interfaces.Engine->IsInGame() = %d", globals::g_interfaces.Engine->IsInGame());
	ImGui::Spacing();
	ImGui::Text("g_interfaces.BaseClient = 0x%d", globals::g_interfaces.BaseClient);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.ClientEntity = 0x%d", globals::g_interfaces.ClientEntity);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.DebugOverlay = 0x%d", globals::g_interfaces.DebugOverlay);
	ImGui::Spacing();
	ImGui::Text("g_interfaces.Engine = 0x%d", globals::g_interfaces.Engine);
	ImGui::Spacing();
	//ImGui::Text("g_interfaces.Engine->GetClientVersion() = %d", globals::g_interfaces.Engine->GetClientVersion());

	ImGui::Spacing();
	ImGui::Text("WindowPos[%f, %f], WindowSize[%f, %f]", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

	ImGui::End();
}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

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

	// pass messages to imgui, to be able to click and stuff
	if (gui::bOpen && ImGui_ImplWin32_WndProcHandler( // we only want to do it if the menu is open.
		hWnd,
		msg,
		wParam,
		lParam
	)) return 1L;

		return CallWindowProc(
			gui::oWindowProc,
			hWnd,
			msg,
			wParam,
			lParam
		);
}


