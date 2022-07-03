#include "drawing.h"

void Render::Line(float fromX, float fromY, float toX, float toY, float thickness, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(fromX, fromY), ImVec2(toX, toY), color, thickness);
}

void Render::OutLinedRect(float x, float y, int w, int h, float Thickness, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), color, 0, ImDrawListFlags_None, Thickness);
}

void Render::FilledRect(float x, float y, int w, int h, ImColor color)
{
	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(w, h), color);
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

void Render::ESP::DrawBox(math::Vector top, math::Vector bot, ImColor color, eBoxType Type)
{
	float Height = ABS(bot.y - top.y);
	float Width = Height / 2;

	switch (Type)
	{
	case Filled:
		Render::FilledRect(bot.x - (Width / 2), bot.y - Height, Width, Height, D3DCOLOR_RGBA(255, 255, 255, 255));
		break;
	case Outlined:
		Render::OutLinedRect(bot.x - (Width / 2), bot.y - Height, Width, Height, 1, D3DCOLOR_RGBA(255, 255, 255, 255));
		break;
	case Box3d:
		break;
	case Corners:
		break;
	default:
		break;
	}
}

bool Render::WolrdToScreen(math::Vector& in, math::Vector& out)
{
	const math::Matrix4x4& worldToScreen = globals::g_interfaces.Engine->WorldToScreenMatrix();

	float w = worldToScreen.m[3][0] * in.x + worldToScreen.m[3][1] * in.y + worldToScreen.m[3][2] * in.z + worldToScreen.m[3][3];
	out.z = 0;
	if (w > 0.01)
	{
		float inverseWidth = 1 / w;
		out.x = (ImGui::GetIO().DisplaySize.x / 2) + (0.5 * ((worldToScreen.m[0][0] * in.x + worldToScreen.m[0][1] * in.y + worldToScreen.m[0][2] * in.z + worldToScreen.m[0][3]) * inverseWidth) * ImGui::GetIO().DisplaySize.x + 0.5);
		out.y = (ImGui::GetIO().DisplaySize.y / 2) - (0.5 * ((worldToScreen.m[1][0] * in.x + worldToScreen.m[1][1] * in.y + worldToScreen.m[1][2] * in.z + worldToScreen.m[1][3]) * inverseWidth) * ImGui::GetIO().DisplaySize.y + 0.5);
		return true;
	}
	return false;
}

