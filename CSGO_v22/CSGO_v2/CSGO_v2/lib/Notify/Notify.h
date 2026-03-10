#pragma once
#include "imgui.h"
#include "imgui_notify.h"

#ifdef _DEBUG
#include "lib/Error/AuditLog.h"
#endif

// Simple notification wrapper — call from anywhere in the cheat
// Notifications render on the game overlay regardless of menu state
// In debug builds, also logs to AuditLog for the in-menu audit tab
namespace Notify
{
	inline void Success(const char* msg, int ms = 3000) {
		ImGui::InsertNotification({ ImGuiToastType_Success, ms, msg });
#ifdef _DEBUG
		AuditLog::Info(msg);
#endif
	}

	inline void Info(const char* msg, int ms = 3000) {
		ImGui::InsertNotification({ ImGuiToastType_Info, ms, msg });
#ifdef _DEBUG
		AuditLog::Info(msg);
#endif
	}

	inline void Warn(const char* msg, int ms = 4000) {
		ImGui::InsertNotification({ ImGuiToastType_Warning, ms, msg });
#ifdef _DEBUG
		AuditLog::Warn(msg);
#endif
	}

	inline void Error(const char* msg, int ms = 5000) {
		ImGui::InsertNotification({ ImGuiToastType_Error, ms, msg });
#ifdef _DEBUG
		AuditLog::Error(msg);
#endif
	}
}
