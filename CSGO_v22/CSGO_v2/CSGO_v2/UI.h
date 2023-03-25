#pragma once
#include "GUI.h"
#include <format>

namespace ui
{
	void BeginGroup(ImVec2 Size, const char* name, ...);
	void EndGroup();
	void SetupTheme();
	void HelpMarker(const char* desc);
}

