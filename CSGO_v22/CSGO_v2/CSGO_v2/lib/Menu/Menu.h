#pragma once
#include "imgui.h"
#include "lib/Configs/config.h"
#include "lib/utils/utils.h"
#include <format>
#include <functional>
#include <vector>
#include <string>

struct Hotkey;

namespace menu
{
	// --- Constants ---
	inline constexpr float kColumnWidth = 270.f;

	// --- Theme ---
	void SetupTheme();

	// Forward declare
	class GroupBuilder;

	// --- Layout ---
	GroupBuilder LeftGroup(const char* name, int lines);
	GroupBuilder RightGroup(const char* name, int lines);
	GroupBuilder Section(const char* name, float width = kColumnWidth);
	void Gap();

	// --- Standalone widgets (for use outside groups) ---
	void Checkbox(const char* label, bool* v, const char* tooltip = nullptr);
	void Slider(const char* label, float* v, float min, float max, const char* fmt = "%.2f", const char* tooltip = nullptr);
	void SliderInt(const char* label, int* v, int min, int max, const char* fmt = "%d", const char* tooltip = nullptr);
	void Combo(const char* label, int* current, const char* items, const char* tooltip = nullptr);
	void ColorPicker(const char* label, CfgColor& color, bool alpha = false);
	void HotkeySelector(::Hotkey& hotKey);
	void HelpMarker(const char* desc, bool sameLine = true);
	bool Button(const char* label);

	// --- Low-level outline groups (for advanced/nested use) ---
	void BeginOutlineGroup(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndOutlineGroup();

	// =========================================================
	// GroupBuilder — fluent API for declaring menu content.
	// All ImGui details are hidden behind this interface.
	// =========================================================
	class GroupBuilder
	{
	public:
		// --- Widgets ---
		GroupBuilder& Checkbox(const char* label, bool* v, const char* tooltip = nullptr);
		GroupBuilder& Slider(const char* label, float* v, float min, float max, const char* fmt = "%.2f", const char* tooltip = nullptr);
		GroupBuilder& SliderInt(const char* label, int* v, int min, int max, const char* fmt = "%d", const char* tooltip = nullptr);
		GroupBuilder& Combo(const char* label, int* current, const char* items, const char* tooltip = nullptr);
		GroupBuilder& CheckboxCombo(const char* cbLabel, bool* v, const char* comboId, int* current, const char* items, const char* tooltip = nullptr);
		GroupBuilder& ColorPicker(const char* label, CfgColor& color, bool alpha = false);
		GroupBuilder& Hotkey(::Hotkey& hotKey);
		GroupBuilder& Text(const char* text);
		GroupBuilder& Button(const char* label, std::function<void()> onClick = nullptr);
		GroupBuilder& InputText(const char* id, char* buf, size_t bufSize);

		// --- Containers ---
		GroupBuilder& ListBox(const char* label, const char* id,
			const std::vector<std::string>& items, const char* selectedName,
			std::function<void(const std::string&)> onSelect);

		// Nested outline section inside a group
		GroupBuilder& SubSection(const char* name, std::function<void(GroupBuilder&)> content);

		// --- Layout ---
		GroupBuilder& SameLine();
		GroupBuilder& Space();

		// Escape hatch — drop into raw ImGui when needed
		GroupBuilder& Custom(std::function<void()> fn);

		// Close the group/section
		void End();

	private:
		friend GroupBuilder menu::LeftGroup(const char*, int);
		friend GroupBuilder menu::RightGroup(const char*, int);
		friend GroupBuilder menu::Section(const char*, float);

		enum class Type { Group, Section, Inline };
		Type type_;
		explicit GroupBuilder(Type t) : type_(t) {}
	};
}
