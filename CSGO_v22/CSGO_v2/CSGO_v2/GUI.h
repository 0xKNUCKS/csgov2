#pragma once
#include <Windows.h>
#include <stdexcept>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "../ext/ImGui/imgui.h"
#include "../ext/ImGui/imgui_impl_dx9.h"
#include "../ext/ImGui/imgui_impl_win32.h"

#include <MinHook.h>

#include "utils.h"
#include "config.h"
#include "entity.h"
#include "UI.h"

typedef uint32_t uint150_t;

namespace gui
{
	// show/hide menu
	inline bool bOpen = true;

	inline bool init = false;

	static inline unsigned int menuKey = VK_INSERT;

	// WinAPI related stuff to make a window
	inline HWND Window = nullptr;
	inline WNDCLASSEX WindowClass = {};
	inline WNDPROC oWindowProc = nullptr;

	// dx shit
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline LPDIRECT3D9 d3d9 = nullptr;

	static uint150_t* g_methodsTable = NULL;

	bool SetupWindowClass(const char* windowClassName) noexcept;
	void DestroyWindowClass() noexcept;

	bool SetupWindow(const char* windowName) noexcept;
	void DestroyWindow() noexcept;

	bool SetupDirectX() noexcept;
	void DestroyDirectX() noexcept;

	// setup device
	bool Setup();

	void SetupMenu(LPDIRECT3DDEVICE9 device) noexcept;
	void Destroy() noexcept;

	// render our menu
	void Render() noexcept;
	void DebugWindow() noexcept;
};

