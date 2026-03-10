#pragma once
#include "imgui.h"
#include "imgui_notify.h"

// Simple notification wrapper — call from anywhere in the cheat
// Notifications render on the game overlay regardless of menu state
namespace Notify
{
	inline void Success(const char* msg, int ms = 3000) {
		ImGui::InsertNotification({ ImGuiToastType_Success, ms, msg });
	}

	inline void Info(const char* msg, int ms = 3000) {
		ImGui::InsertNotification({ ImGuiToastType_Info, ms, msg });
	}

	inline void Warn(const char* msg, int ms = 4000) {
		ImGui::InsertNotification({ ImGuiToastType_Warning, ms, msg });
	}

	inline void Error(const char* msg, int ms = 5000) {
		ImGui::InsertNotification({ ImGuiToastType_Error, ms, msg });
	}
}
