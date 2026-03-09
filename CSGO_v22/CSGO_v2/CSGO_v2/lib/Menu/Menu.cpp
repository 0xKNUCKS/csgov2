#include "Menu.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// =========================================================
// Internal helpers — the actual ImGui rendering logic.
// GroupBuilder and standalone functions both call these.
// To restyle, change these — everything else stays the same.
// =========================================================

namespace detail
{
	static void RenderCheckbox(const char* label, bool* v, const char* tooltip)
	{
		ImGui::Checkbox(label, v);
		if (tooltip) menu::HelpMarker(tooltip);
	}

	static void RenderSlider(const char* label, float* v, float min, float max, const char* fmt, const char* tooltip)
	{
		std::string format = std::string(label) + " " + fmt;
		std::string id = std::format("##{}_slider", label);
		ImGui::SliderFloat(id.c_str(), v, min, max, format.c_str());
		if (tooltip) menu::HelpMarker(tooltip);
	}

	static void RenderSliderInt(const char* label, int* v, int min, int max, const char* fmt, const char* tooltip)
	{
		std::string format = std::string(label) + " " + fmt;
		std::string id = std::format("##{}_slider", label);
		ImGui::SliderInt(id.c_str(), v, min, max, format.c_str());
		if (tooltip) menu::HelpMarker(tooltip);
	}

	static void RenderCombo(const char* label, int* current, const char* items, const char* tooltip)
	{
		ImGui::Combo(std::format("##{}_combo", label).c_str(), current, items);
		if (tooltip) menu::HelpMarker(tooltip);
	}

	static void RenderColorPicker(const char* label, CfgColor& color, bool alpha)
	{
		ImGuiColorEditFlags flags = alpha ? 0 : ImGuiColorEditFlags_NoAlpha;
		if (ImGui::ColorButton(std::format("##{}_btn", label).c_str(),
			ImVec4(color.r, color.g, color.b, color.a))) {
			ImGui::OpenPopup(std::format("##{}_popup", label).c_str());
		}
		ImGui::SameLine();
		ImGui::Text(label);

		if (ImGui::BeginPopup(std::format("##{}_popup", label).c_str())) {
			ImGui::ColorPicker4(label, (float*)&color, flags);
			ImGui::EndPopup();
		}
	}

	static void RenderHotkey(Hotkey& hotKey)
	{
		if (ImGui::Button(hotKey.label.c_str(), ImVec2(70, 0))) {
			hotKey.searching = true;
		}

		if (hotKey.searching) {
			hotKey.label = "...";
			for (unsigned int i = 0x01; i <= 0xFE; i++) {
				if (GetAsyncKeyState(i) & 0x8000) {
					hotKey.searching = false;
					if (i != VK_ESCAPE)
						hotKey.virtualKey = i;
					hotKey.label = utils::VirtualKeyToString(hotKey.virtualKey);
					break;
				}
			}
		}
	}

	// Opens a child window sized for N widget lines + title
	static void BeginGroupChild(const char* name, int lines, float width)
	{
		float w = (width <= 0.f) ? ImGui::GetContentRegionAvail().x : width;
		float lineH = ImGui::GetFrameHeightWithSpacing();
		float titleH = lineH + 2.f;
		float h = titleH + lines * lineH + ImGui::GetStyle().WindowPadding.y;

		ImGui::BeginChild(std::format("{}##{}", name, name).c_str(), ImVec2(w, h), true);
		ImGui::Text(name);
		ImGui::Separator();
	}
}

// =========================================================
// Standalone widget functions (for use outside groups)
// =========================================================

void menu::Checkbox(const char* label, bool* v, const char* tooltip) { detail::RenderCheckbox(label, v, tooltip); }
void menu::Slider(const char* label, float* v, float min, float max, const char* fmt, const char* tooltip) { detail::RenderSlider(label, v, min, max, fmt, tooltip); }
void menu::SliderInt(const char* label, int* v, int min, int max, const char* fmt, const char* tooltip) { detail::RenderSliderInt(label, v, min, max, fmt, tooltip); }
void menu::Combo(const char* label, int* current, const char* items, const char* tooltip) { detail::RenderCombo(label, current, items, tooltip); }
void menu::ColorPicker(const char* label, CfgColor& color, bool alpha) { detail::RenderColorPicker(label, color, alpha); }
void menu::HotkeySelector(Hotkey& hotKey) { detail::RenderHotkey(hotKey); }

bool menu::Button(const char* label) { return ImGui::Button(label); }

void menu::HelpMarker(const char* desc, bool sameLine)
{
	if (sameLine) ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void menu::Gap()
{
	ImGui::Spacing();
	ImGui::Spacing();
}

// =========================================================
// Layout functions — return GroupBuilder for chaining
// =========================================================

menu::GroupBuilder menu::LeftGroup(const char* name, int lines)
{
	detail::BeginGroupChild(name, lines, kColumnWidth);
	return GroupBuilder(GroupBuilder::Type::Group);
}

menu::GroupBuilder menu::RightGroup(const char* name, int lines)
{
	ImGui::SameLine();
	detail::BeginGroupChild(name, lines, kColumnWidth);
	return GroupBuilder(GroupBuilder::Type::Group);
}

menu::GroupBuilder menu::Section(const char* name, float width)
{
	menu::BeginOutlineGroup(name, ImVec2(width, 0.f));
	return GroupBuilder(GroupBuilder::Type::Section);
}

// =========================================================
// GroupBuilder — fluent widget methods
// =========================================================

using GB = menu::GroupBuilder;

GB& GB::Checkbox(const char* label, bool* v, const char* tooltip)
{
	detail::RenderCheckbox(label, v, tooltip);
	return *this;
}

GB& GB::Slider(const char* label, float* v, float min, float max, const char* fmt, const char* tooltip)
{
	detail::RenderSlider(label, v, min, max, fmt, tooltip);
	return *this;
}

GB& GB::SliderInt(const char* label, int* v, int min, int max, const char* fmt, const char* tooltip)
{
	detail::RenderSliderInt(label, v, min, max, fmt, tooltip);
	return *this;
}

GB& GB::Combo(const char* label, int* current, const char* items, const char* tooltip)
{
	detail::RenderCombo(label, current, items, tooltip);
	return *this;
}

GB& GB::ColorPicker(const char* label, CfgColor& color, bool alpha)
{
	detail::RenderColorPicker(label, color, alpha);
	return *this;
}

GB& GB::Hotkey(::Hotkey& hotKey)
{
	detail::RenderHotkey(hotKey);
	return *this;
}

GB& GB::Text(const char* text)
{
	ImGui::Text(text);
	return *this;
}

GB& GB::Button(const char* label, std::function<void()> onClick)
{
	if (ImGui::Button(label) && onClick)
		onClick();
	return *this;
}

GB& GB::InputText(const char* id, char* buf, size_t bufSize)
{
	ImGui::InputText(id, buf, bufSize);
	return *this;
}

GB& GB::ListBox(const char* label, const char* id,
	const std::vector<std::string>& items, const char* selectedName,
	std::function<void(const std::string&)> onSelect)
{
	if (items.empty()) return *this;

	ImGui::Spacing();
	if (label && label[0])
		ImGui::Text(label);

	float height = items.size() * ImGui::GetTextLineHeightWithSpacing() + 4;
	if (ImGui::BeginListBox(id, ImVec2(0, height))) {
		for (auto& name : items) {
			bool selected = (selectedName && name == selectedName);
			if (ImGui::Selectable(name.c_str(), selected) && onSelect)
				onSelect(name);
		}
		ImGui::EndListBox();
	}
	return *this;
}

GB& GB::SubSection(const char* name, std::function<void(GroupBuilder&)> content)
{
	menu::BeginOutlineGroup(name);
	GroupBuilder inner(Type::Inline);
	content(inner);
	menu::EndOutlineGroup();
	return *this;
}

GB& GB::SameLine()
{
	ImGui::SameLine();
	return *this;
}

GB& GB::Space()
{
	ImGui::Spacing();
	return *this;
}

GB& GB::Custom(std::function<void()> fn)
{
	if (fn) fn();
	return *this;
}

void GB::End()
{
	switch (type_) {
	case Type::Group:   ImGui::EndChild(); break;
	case Type::Section: menu::EndOutlineGroup(); break;
	case Type::Inline:  break; // no-op for nested builders
	}
}

// =========================================================
// Outline group (from github.com/ocornut/imgui/issues/1496)
// =========================================================

static ImVector<ImRect> s_GroupPanelLabelStack;

void menu::BeginOutlineGroup(const char* name, const ImVec2& size)
{
	ImGui::BeginGroup();

	auto cursorPos = ImGui::GetCursorScreenPos();
	auto itemSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	auto frameHeight = ImGui::GetFrameHeight();
	ImGui::BeginGroup();

	ImVec2 effectiveSize = size;
	if (size.x < 0.0f)
		effectiveSize.x = ImGui::GetContentRegionAvail().x;
	else
		effectiveSize.x = size.x;
	ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::BeginGroup();
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::TextUnformatted(name);
	auto labelMin = ImGui::GetItemRectMin();
	auto labelMax = ImGui::GetItemRectMax();
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
	ImGui::BeginGroup();

	ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
	ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
	ImGui::GetCurrentWindow()->Size.x -= frameHeight;

	auto itemWidth = ImGui::CalcItemWidth();
	ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

	s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
}

void menu::EndOutlineGroup()
{
	ImGui::PopItemWidth();

	auto itemSpacing = ImGui::GetStyle().ItemSpacing;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	auto frameHeight = ImGui::GetFrameHeight();

	ImGui::EndGroup();
	ImGui::EndGroup();

	ImGui::SameLine(0.0f, 0.0f);
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

	ImGui::EndGroup();

	auto itemMin = ImGui::GetItemRectMin();
	auto itemMax = ImGui::GetItemRectMax();

	auto labelRect = s_GroupPanelLabelStack.back();
	s_GroupPanelLabelStack.pop_back();

	ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
	ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
	labelRect.Min.x -= itemSpacing.x;
	labelRect.Max.x += itemSpacing.x;
	for (int i = 0; i < 4; ++i)
	{
		switch (i)
		{
		case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
		case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
		case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
		case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
		}

		ImGui::GetWindowDrawList()->AddRect(
			frameRect.Min, frameRect.Max + ImVec2(0, itemSpacing.y),
			ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Separator)),
			halfFrame.x * 1.5f, 0, 0.5f);

		ImGui::PopClipRect();
	}

	ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
	ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
	ImGui::GetCurrentWindow()->Size.x += frameHeight;

	ImGui::Dummy(ImVec2(0.0f, 0.0f));
	ImGui::EndGroup();
}

// =========================================================
// Theme
// =========================================================

void menu::SetupTheme()
{
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.0f);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;
	ImGui::GetStyle().ChildRounding = 6.f;
	ImGui::GetStyle().WindowRounding = 6.f;

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
