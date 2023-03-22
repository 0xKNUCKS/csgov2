#include "ESP.h"
#include "hook.h"

void ESP::Render()
{
	if (!cfg.visuals.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	for (int i = 0; i <= globals::EntList.Size(); i++)
	{
		if (!globals::EntList[i].isValidState())
			continue;

		if (cfg.visuals.esp.Lines)
			DrawLine(globals::EntList[i]);
		
		if (cfg.visuals.esp.BoudningBox)
			DrawBoundingBox(globals::EntList[i]);
	}
}

void ESP::DrawLine(ent_t& ent)
{
	math::Vector sOrginPos = math::Vector();

	if (!utils::WolrdToScreen(ent.GetPos(), sOrginPos))
		return;

	Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, sOrginPos.x, sOrginPos.y, 2.0f);
}

void ESP::DrawSkeleton(ent_t& ent)
{
	for (size_t x = 0; x <= ent.Bones.size(); x++)
	{
		std::pair<math::Vector, math::Vector> Bones = ent.Bones[x];
		math::Vector sBone, sParent;
		if (!utils::WolrdToScreen(Bones.first, sBone) || !utils::WolrdToScreen(Bones.second, sParent))
			continue;

		Render::Line(sBone.x, sBone.y, sParent.x, sParent.y);
	}
}

// Made this custom function because theres something wrong with imgui's rect, as you can see in this image: https://i.imgur.com/XrYPSnB.png (theyre both with the same args and values)
// and its not the best btw, should use vertex
void DrawRect(float x, float y, float w, float h, ImColor color = ImColor(255,255,255))
{
	Render::Line(x, y, x + w, y, 1, color);
	Render::Line(x + w, y, x + w, y + h, 1, color);
	Render::Line(x + w, y + h, x, y + h, 1, color);
	Render::Line(x, y + h, x, y, 1, color);
}

void ESP::DrawBoundingBox(ent_t& ent)
{
	math::Vector sOrginPos = math::Vector();
	math::Vector sHeadPos = math::Vector();
	math::Vector HeadPos = ent.GetBonePos(8);

	if (!utils::WolrdToScreen(ent.GetPos(), sOrginPos))
		return;
	HeadPos.z *= 1.15; // to increase the box height a little bit.
	if (!utils::WolrdToScreen(HeadPos, sHeadPos))
		return;

	// Bounding Box
	float BoxHeight = sOrginPos.y - sHeadPos.y;
	float BoxWidth = BoxHeight / 2;
	DrawRect(sOrginPos.x - (BoxWidth / 2), sOrginPos.y - BoxHeight, BoxWidth, BoxHeight);

	// Health Bar
	float BarHeight = BoxHeight * ent.GetHealth() / 100;
	float BarDelta = BoxHeight - BarHeight;
	DrawRect(sOrginPos.x + (BoxWidth / 2) + 2, (sOrginPos.y - BoxHeight) + BarDelta, 1, BarHeight, ImColor(0, 250, 10));
}	