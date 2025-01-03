#include "drawing.h"
#pragma warning( disable : 4244)

void Render::Line(float fromX, float fromY, float toX, float toY, float thickness, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(fromX, fromY), ImVec2(toX, toY), color, thickness);
}

void Render::OutLinedRect(float x, float y, int w, int h, float Thickness, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, ImDrawListFlags_None, Thickness);
}

void Render::FilledRect(float x, float y, int w, int h, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color);
}

void Render::CenteredOutlinedRect(float x, float y, float w, float h, ImColor color)
{
	Render::OutLinedRect(x - w / 2, y - h / 2, w, h, 1, color);
}

void Render::CenteredFilledRect(float x, float y, float w, float h, ImColor color)
{
	Render::FilledRect(x - w / 2, y - h / 2, w, h, color);
}

void Render::OutLinedCircle(float x, float y, float rad, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), rad, color, 360);
}

void Render::FilledCircle(float x, float y, float rad, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), rad, color, 360);
}

void Render::OutLinedText(const char* text, float x, float y, ImDrawList* drawList, ImColor color)
{
	ImColor Shade;
	Shade.Value = ImVec4(color.Value.x * 0.25, color.Value.y * 0.25, color.Value.z * 0.25, color.Value.w * 0.75);
	drawList->AddText(ImVec2(x + 1, y + 1), Shade, text);
	drawList->AddText(ImVec2(x, y), color, text);
}
