#include "Menu.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_notify.h"
#include "font_awesome_5.h"
#include "inter_regular_data.h"
#include "inter_medium_data.h"
#include <cmath>

// =========================================================
// Internal helpers — the actual ImGui rendering logic.
// GroupBuilder and standalone functions both call these.
// To restyle, change these — everything else stays the same.
// =========================================================

namespace detail
{
	// iOS-style animated toggle switch
	static bool ToggleSwitch(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const float height = ImGui::GetFrameHeight();
		const float width = height * 1.75f;
		const float radius = height * 0.5f;

		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.f ? style.ItemInnerSpacing.x + label_size.x : 0.f), pos.y + height));
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
		{
			*v = !(*v);
			ImGui::MarkItemEdited(id);
		}

		// Animate the knob position (0.0 = off, 1.0 = on)
		ImGuiStorage* storage = window->DC.StateStorage;
		const ImGuiID animId = id + ImGuiID(0xA91E); // unique sub-id for animation
		float animVal = storage->GetFloat(animId, *v ? 1.f : 0.f);
		float target = *v ? 1.f : 0.f;
		float speed = g.IO.DeltaTime * 12.f;
		if (speed > 1.f) speed = 1.f;
		animVal += (target - animVal) * speed;
		if (std::abs(animVal - target) < 0.005f) animVal = target;
		storage->SetFloat(animId, animVal);

		// Colors
		ImVec4 offBg = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
		ImVec4 onBg = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
		// Lerp background color
		ImVec4 bgColor(
			offBg.x + (onBg.x - offBg.x) * animVal,
			offBg.y + (onBg.y - offBg.y) * animVal,
			offBg.z + (onBg.z - offBg.z) * animVal,
			1.f
		);
		// Brighten on hover
		if (hovered) {
			bgColor.x += 0.05f;
			bgColor.y += 0.05f;
			bgColor.z += 0.05f;
		}

		// Draw track (pill shape)
		ImVec2 trackMin = pos;
		ImVec2 trackMax(pos.x + width, pos.y + height);
		window->DrawList->AddRectFilled(trackMin, trackMax, ImGui::ColorConvertFloat4ToU32(bgColor), radius);

		// Draw knob
		float knobRadius = radius - 2.f;
		float knobX = pos.x + radius + animVal * (width - height);
		float knobY = pos.y + radius;
		ImU32 knobColor = IM_COL32(255, 255, 255, 255);
		// Subtle shadow
		window->DrawList->AddCircleFilled(ImVec2(knobX + 0.5f, knobY + 1.f), knobRadius, IM_COL32(0, 0, 0, 40));
		window->DrawList->AddCircleFilled(ImVec2(knobX, knobY), knobRadius, knobColor);

		// Label
		if (label_size.x > 0.f) {
			ImVec2 label_pos(pos.x + width + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y);
			ImGui::RenderText(label_pos, label);
		}

		return pressed;
	}

	static void RenderCheckbox(const char* label, bool* v, const char* tooltip)
	{
		if (cfg.settings.ToggleStyle)
			ToggleSwitch(label, v);
		else
			ImGui::Checkbox(label, v);
		if (tooltip) menu::HelpMarker(tooltip);
	}

	// Get display name from label (strips ##suffix used for ImGui ID)
	static std::string DisplayName(const char* label) {
		const char* h = strstr(label, "##");
		return h ? std::string(label, h) : std::string(label);
	}

	static void RenderSlider(const char* label, float* v, float min, float max, const char* fmt, const char* tooltip)
	{
		std::string format = DisplayName(label) + " " + fmt;
		std::string id = std::format("##{}_slider", label);
		ImGui::SliderFloat(id.c_str(), v, min, max, format.c_str());
		if (tooltip) menu::HelpMarker(tooltip);
	}

	static void RenderSliderInt(const char* label, int* v, int min, int max, const char* fmt, const char* tooltip)
	{
		std::string format = DisplayName(label) + " " + fmt;
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
		std::string popupId = std::format("##{}_popup", label);

		if (ImGui::ColorButton(std::format("##{}_btn", label).c_str(),
			ImVec4(color.r, color.g, color.b, color.a))) {
			ImGui::OpenPopup(popupId.c_str());
		}
		// Only show label text if it doesn't start with ## (hidden label)
		if (label[0] != '#' || label[1] != '#') {
			ImGui::SameLine();
			ImGui::Text(label);
		}

		// Animated fade for color picker popup
		ImGuiContext& g = *GImGui;
		ImGuiStorage* storage = ImGui::GetCurrentWindow()->DC.StateStorage;
		ImGuiID animKey = ImGui::GetID(popupId.c_str()) + ImGuiID(0xC010);
		float popupAlpha = storage->GetFloat(animKey, 0.f);

		bool isOpen = ImGui::IsPopupOpen(popupId.c_str());
		float target = isOpen ? 1.f : 0.f;
		float speed = g.IO.DeltaTime * 8.f;
		if (speed > 1.f) speed = 1.f;
		popupAlpha += (target - popupAlpha) * speed;
		if (std::abs(popupAlpha - target) < 0.01f) popupAlpha = target;
		storage->SetFloat(animKey, popupAlpha);

		if (popupAlpha > 0.01f) {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, popupAlpha);
			ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 6.f);
			if (ImGui::BeginPopup(popupId.c_str())) {
				ImGui::ColorPicker4(label, (float*)&color, flags);
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);
		}
	}

	static void RenderHotkey(Hotkey& hotKey)
	{
		// Key bind button
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

		// Mode selector on the same line
		ImGui::SameLine();
		ImGui::PushItemWidth(80);
		int mode = static_cast<int>(hotKey.mode);
		ImGui::Combo(std::format("##mode_{}", (uintptr_t)&hotKey).c_str(), &mode, "Hold\0Toggle\0Always\0");
		hotKey.mode = static_cast<HotkeyMode>(mode);
		ImGui::PopItemWidth();
	}

	// Accent color used for card top lines (matches title bar accent)
	static constexpr ImU32 kCardAccent = IM_COL32(71, 143, 255, 255);  // #478FFF

	// Draw multi-layer soft shadow behind a rectangle (call on parent draw list)
	static void DrawCardShadow(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding)
	{
		// 4-layer expanding shadow with decreasing opacity
		struct ShadowLayer { float expand; ImU32 color; };
		static constexpr ShadowLayer layers[] = {
			{ 3.f,  IM_COL32(0, 0, 0, 40) },
			{ 6.f,  IM_COL32(0, 0, 0, 25) },
			{ 10.f, IM_COL32(0, 0, 0, 15) },
			{ 15.f, IM_COL32(0, 0, 0, 8)  },
		};
		for (auto& l : layers) {
			dl->AddRectFilled(
				ImVec2(min.x - l.expand, min.y - l.expand),
				ImVec2(max.x + l.expand, max.y + l.expand),
				l.color, rounding + l.expand * 0.3f);
		}
	}

	// Opens a child window sized for N widget lines + title
	static void BeginGroupChild(const char* name, int lines, float width)
	{
		float w = (width <= 0.f) ? ImGui::GetContentRegionAvail().x : width;
		float lineH = ImGui::GetFrameHeightWithSpacing();
		float titleH = lineH + 2.f;
		float h = titleH + lines * lineH + ImGui::GetStyle().WindowPadding.y;

		// --- Shadow: draw on parent draw list before creating child ---
		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		ImVec2 cardMin = screenPos;
		ImVec2 cardMax = ImVec2(screenPos.x + w, screenPos.y + h);
		float rounding = ImGui::GetStyle().ChildRounding;
		DrawCardShadow(ImGui::GetWindowDrawList(), cardMin, cardMax, rounding);

		ImGui::BeginChild(std::format("{}##{}", name, name).c_str(), ImVec2(w, h), true);

		// --- Accent line at top of card (2px) ---
		ImVec2 childPos = ImGui::GetWindowPos();
		ImDrawList* childDl = ImGui::GetWindowDrawList();
		childDl->AddRectFilled(
			childPos,
			ImVec2(childPos.x + w, childPos.y + 2.0f),
			kCardAccent, rounding, ImDrawFlags_RoundCornersTop);

		// --- Header text in medium font ---
		if (menu::g_fontMedium) ImGui::PushFont(menu::g_fontMedium);

		const char* hashPos = strstr(name, "##");
		if (hashPos) {
			ImGui::TextUnformatted(name, hashPos);
		} else {
			ImGui::Text("%s", name);
		}

		if (menu::g_fontMedium) ImGui::PopFont();

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

// Independent two-column layout state.
// Each column tracks its own Y cursor so groups stack without gaps.
static float s_leftColY  = -1.f;
static float s_rightColY = -1.f;
static float s_colStartX = 0.f;
static bool  s_colActive = false;

static void InitColumnsIfNeeded()
{
	if (!s_colActive) {
		s_leftColY  = ImGui::GetCursorPosY();
		s_rightColY = s_leftColY;
		s_colStartX = ImGui::GetCursorPosX();
		s_colActive = true;
	}
}

menu::GroupBuilder menu::LeftGroup(const char* name, int lines)
{
	InitColumnsIfNeeded();
	ImGui::SetCursorPos(ImVec2(s_colStartX, s_leftColY));
	detail::BeginGroupChild(name, lines, kColumnWidth);
	return GroupBuilder(GroupBuilder::Type::LeftGroup);
}

menu::GroupBuilder menu::RightGroup(const char* name, int lines)
{
	InitColumnsIfNeeded();
	float rightX = s_colStartX + kColumnWidth + ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetCursorPos(ImVec2(rightX, s_rightColY));
	detail::BeginGroupChild(name, lines, kColumnWidth);
	return GroupBuilder(GroupBuilder::Type::RightGroup);
}

void menu::EndRow()
{
	if (s_colActive) {
		float maxY = (s_leftColY > s_rightColY) ? s_leftColY : s_rightColY;
		ImGui::SetCursorPosY(maxY);
		s_colActive = false;
	}
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

GB& GB::CheckboxCombo(const char* cbLabel, bool* v, const char* comboId, int* current, const char* items, const char* tooltip)
{
	if (cfg.settings.ToggleStyle)
		detail::ToggleSwitch(cbLabel, v);
	else
		ImGui::Checkbox(cbLabel, v);
	ImGui::SameLine();
	ImGui::PushItemWidth(menu::kColumnWidth * 0.3f);
	ImGui::Combo(comboId, current, items);
	ImGui::PopItemWidth();
	if (tooltip) menu::HelpMarker(tooltip);
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
	// Indent to align with checkbox/toggle labels
	// Toggle switch width = height * 1.75f, then ItemInnerSpacing.x gap before label
	float toggleWidth = ImGui::GetFrameHeight() * 1.75f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + toggleWidth + ImGui::GetStyle().ItemInnerSpacing.x);
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

GB& GB::GearPopup(const char* id, std::function<void(GroupBuilder&)> content)
{
	ImGui::SameLine();

	// Small gear button with no frame, blends into the row
	std::string popupId = std::format("##gear_popup_{}", id);
	std::string btnId = std::format(ICON_FA_COG "##gear_{}", id);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.56f, 1.f, 0.3f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.56f, 1.f, 0.5f));

	if (ImGui::Button(btnId.c_str(), ImVec2(ImGui::GetFrameHeight(), 0))) {
		ImGui::OpenPopup(popupId.c_str());
	}

	ImGui::PopStyleColor(3);

	// Animated popup
	ImGuiContext& g = *GImGui;
	ImGuiStorage* storage = ImGui::GetCurrentWindow()->DC.StateStorage;
	ImGuiID animKey = ImGui::GetID(popupId.c_str()) + ImGuiID(0xFADE);
	float alpha = storage->GetFloat(animKey, 0.f);

	bool isOpen = ImGui::IsPopupOpen(popupId.c_str());
	float target = isOpen ? 1.f : 0.f;
	float speed = g.IO.DeltaTime * 10.f;
	if (speed > 1.f) speed = 1.f;
	alpha += (target - alpha) * speed;
	if (std::abs(alpha - target) < 0.01f) alpha = target;
	storage->SetFloat(animKey, alpha);

	if (alpha > 0.01f) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 6.f);

		if (ImGui::BeginPopup(popupId.c_str())) {
			GroupBuilder inner(Type::Inline);
			content(inner);
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(3);
	}

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
	case Type::LeftGroup:
		ImGui::EndChild();
		s_leftColY = ImGui::GetCursorPosY();
		break;
	case Type::RightGroup:
		ImGui::EndChild();
		s_rightColY = ImGui::GetCursorPosY();
		break;
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
	// --- Font Setup: Inter (embedded) + FontAwesome 5 icons ---
	ImFontConfig fontCfg;
	fontCfg.FontDataOwnedByAtlas = false;  // Static data, don't free()
	fontCfg.OversampleH = 2;               // Better horizontal anti-aliasing

	// Inter Regular — body text, widgets
	g_fontRegular = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
		(void*)inter_regular_data, inter_regular_size, 14.0f, &fontCfg);
	ImGui::MergeIconsWithLatestFont(14.0f, false);

	// Inter Medium — group headers, labels, title bar
	ImFontConfig medCfg;
	medCfg.FontDataOwnedByAtlas = false;
	medCfg.OversampleH = 2;
	g_fontMedium = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
		(void*)inter_medium_data, inter_medium_size, 14.0f, &medCfg);
	ImGui::MergeIconsWithLatestFont(14.0f, false);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;
	ImGui::GetStyle().ChildRounding = 6.f;
	ImGui::GetStyle().WindowRounding = 6.f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);   // Darker bg so cards stand out
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.16f, 0.20f, 1.00f);   // Slightly lighter cards
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
