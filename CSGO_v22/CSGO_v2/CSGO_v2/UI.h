#pragma once
#include "GUI.h"
#include "config.h"
#include <format>

struct Hotkey;

namespace ui
{
	void BeginGroup(ImVec2 Size, const char* name, ...);
	void EndGroup();
	void SetupTheme();
	void HelpMarker(const char* desc, bool sameLine = 1);
	void HotkeySelector(Hotkey& hotKey);

	// Pasted from: https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353
	void BeginOutlineGroup(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndOutlineGroup();
}

