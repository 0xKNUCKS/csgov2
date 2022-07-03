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
	gui::SetupTheme();

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

class Entity
{
public:
	VIRTUAL_METHOD(void, release, 1, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(ClientClass*, getClientClass, 2, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(void, preDataUpdate, 6, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
		VIRTUAL_METHOD(void, postDataUpdate, 7, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
		VIRTUAL_METHOD(bool, isDormant, 9, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(int, index, 10, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(void, setDestroyedOnRecreateEntities, 13, (), (this + sizeof(uintptr_t) * 2))

		VIRTUAL_METHOD(math::Vector&, getRenderOrigin, 1, (), (this + sizeof(uintptr_t)))
		VIRTUAL_METHOD(bool, shouldDraw, 3, (), (this + sizeof(uintptr_t)))
		VIRTUAL_METHOD(const math::Matrix3x4&, toWorldTransform, 32, (), (this + sizeof(uintptr_t)))

		VIRTUAL_METHOD(int&, handle, 2, (), (this))
		VIRTUAL_METHOD(const math::Vector&, getAbsOrigin, 10, (), (this))
		VIRTUAL_METHOD(const math::Vector&, getAbsAngle, 11, (), (this))
		VIRTUAL_METHOD(void, setModelIndex, 75, (int index), (this, index))
		VIRTUAL_METHOD(bool, getAttachment, 84, (int index, math::Vector& origin), (this, index, std::ref(origin)))
		VIRTUAL_METHOD(int, health, 122, (), (this))
		VIRTUAL_METHOD(bool, isAlive, 156, (), (this))
		VIRTUAL_METHOD(bool, isPlayer, 158, (), (this))
		VIRTUAL_METHOD(bool, isWeapon, 166, (), (this))
		VIRTUAL_METHOD(void, updateClientSideAnimation, 224, (), (this))
		VIRTUAL_METHOD(int, getWeaponSubType, 282, (), (this))
		VIRTUAL_METHOD(float, getSpread, 453, (), (this))
		VIRTUAL_METHOD(int, getMuzzleAttachmentIndex1stPerson, 468, (Entity* viewModel), (this, viewModel))
		VIRTUAL_METHOD(int, getMuzzleAttachmentIndex3rdPerson, 469, (), (this))
		VIRTUAL_METHOD(float, getInaccuracy, 483, (), (this))
		VIRTUAL_METHOD(void, updateInaccuracyPenalty, 484, (), (this))

		Entity* getObserverTarget() noexcept
	{
		Entity* entity = VirtualMethod::call<Entity*, 295>(this);
		if (entity)
			return entity->isPlayer() ? entity : nullptr;
		return nullptr;
	}

	math::Vector getEyePosition() noexcept
	{
		if ((uintptr_t)this == LocalPlayer.Get())
			return getAbsOrigin() + offsets::m_vecViewOffset;

		math::Vector v;
		VirtualMethod::call<void, 285>(this, std::ref(v));
		return v;
	}

	math::Vector getAimPunch() noexcept
	{
		math::Vector v;
		VirtualMethod::call<void, 346>(this, std::ref(v));
		return v;
	}

	math::UtlVector<math::Matrix3x4>& boneCache() noexcept { return *(math::UtlVector<math::Matrix3x4> *)((uintptr_t)this + 0x2914); }
	math::Vector getBonePosition(int bone) noexcept { return boneCache()[bone].GetVecOrgin(); }
};

static_assert(sizeof(Entity) == 1);

// render our menu
void gui::Render() noexcept
{

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Render::OutLinedRect(0, 0, 100, 100);

	if (cfg.aimbot.DrawFov && cfg.aimbot.Enabled)
		Render::OutLinedCircle(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2, cfg.aimbot.FOV);

	ESP::Render();

	// Menu Code
if (gui::bOpen) {
	ImGui::SetNextWindowSize(ImVec2(500, 250));

	ImGui::Begin(std::format("Cockbalt - Welcome {}!", LocalPlayer.GetName()).c_str(), &gui::bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginTabBar("##TabsBar"))
	{
		if (ImGui::BeginTabItem("Aim"))
		{
			ImGui::Checkbox("Enabled", &cfg.aimbot.Enabled);
			ImGui::Spacing();
			ImGui::SliderFloat("##FOVval", &cfg.aimbot.FOV, 0, 180, "FOV %.1f"); ImGui::SameLine(); ImGui::Checkbox("Draw", &cfg.aimbot.DrawFov);
			ImGui::Spacing();
			ImGui::SliderFloat("Smooth", &cfg.aimbot.Smooth, 0, 10.0f, cfg.aimbot.Smooth ? "%.3f" : "None");
			ImGui::Spacing();
			ImGui::Checkbox("Aim At Friendly", &cfg.aimbot.FriendlyFire);
			ImGui::Spacing();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Visuals"))
		{
			ImGui::Checkbox("Enabled", &cfg.visuals.Enabled);
			ImGui::Text("Not yet ;)!!");

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


	ImGui::Begin("DBG Window");

	ImGui::Text("FOV: %.1f\nEntity: %d\nDistance: %.1f", aimbot::Target.fov, aimbot::Target.ent.GetEnt(), aimbot::Target.distance);
	ImGui::Text("LocalPlayer Name = %s", LocalPlayer.GetName());
	ImGui::Spacing();
	ImGui::Text("LocalPlayer.GetTeamId() = %d", LocalPlayer.Get() ? LocalPlayer.GetTeamId() : 99);
	ImGui::Spacing();
	ImGui::Text("offsets::m_vecViewOffset = %d", (uintptr_t)globals::g_NetVars.FindOffset("CBasePlayer", "m_vecViewOffset[0]"));
	//ImGui::Spacing();
	gEntity* enty = (gEntity*)LocalPlayer.Get();
	//math::Vector Pos = LocalPlayer.Get() ? LocalPlayer.GetBonePos(8) : math::Vector{0, 0, 0};
	math::Vector Pos = LocalPlayer.Get() ? LocalPlayer.GetBonePos(8) : math::Vector{0, 0, 0};
	ImGui::Text("Local Head Pos: x.%.1f, y.%.1f, z.%.1f", Pos.x, Pos.y, Pos.z);
	ImGui::Spacing();
	math::Vector Orgin = LocalPlayer.Get() ? LocalPlayer.GetPos() : math::Vector{0, 0, 0};
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

void gui::SetupTheme()
{
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.5f);;
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

