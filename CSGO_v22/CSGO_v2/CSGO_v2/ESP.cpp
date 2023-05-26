#include "ESP.h"
#include "hook.h"
#include <math.h>

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

		if (globals::EntList[i].isTeammate() && !cfg.visuals.Friendly)
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

	// TODO: improve the lines by using the new players bounding box instead of this ancient way
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

void ESP::DrawBoundingBox(ent_t& ent)
{
	// Getting the bounding box's min and max
	math::Vector min, max;
	auto model = ent->getModel();
	min = model->mins; max = model->maxs;

	math::Vector points[] = {math::Vector(min.x, min.y, min.z),
						math::Vector(min.x, max.y, min.z),
						math::Vector(max.x, max.y, min.z),
						math::Vector(max.x, min.y, min.z),
						math::Vector(max.x, max.y, max.z),
						math::Vector(min.x, max.y, max.z),
						math::Vector(min.x, min.y, max.z),
						math::Vector(max.x, min.y, max.z)
	};

	math::Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++)
	{
		pointsTransformed[i] = utils::VectorTransform(points[i], ent->toWorldTransform());
	}

	math::Vector pos = ent.GetPos();
	math::Vector flb, brt, blb, frt, frb, brb, blt, flt;


	if (!utils::WolrdToScreen(pointsTransformed[3], flb) || !utils::WolrdToScreen(pointsTransformed[5], brt)
		|| !utils::WolrdToScreen(pointsTransformed[0], blb) || !utils::WolrdToScreen(pointsTransformed[4], frt)
		|| !utils::WolrdToScreen(pointsTransformed[2], frb) || !utils::WolrdToScreen(pointsTransformed[1], brb)
		|| !utils::WolrdToScreen(pointsTransformed[6], blt) || !utils::WolrdToScreen(pointsTransformed[7], flt))
		return;

	math::Vector arr[] = {flb, brt, blb, frt, frb, brb, blt, flt};

	float left = flb.x;		// left
	float top = flb.y;		// top
	float right = flb.x;	// right
	float bottom = flb.y;	// bottom

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (top < arr[i].y)
			top = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (bottom > arr[i].y)
			bottom = arr[i].y;
	}

	int h = bottom - top;	// height
	int w = -min(left - right, h / globals::aspectRatio); /* width -> to understand it better, this is how it looks like with the default Apect Ratio: "h / 2", but im gonna use the aspect ratio to make it compatable with other aspect ratios, the goal here is to avoid ugly boxes anyways ;) (if the width is less than half of the height (thats with the default aspect ratio) its gonna look very ugly, i dont like it, so i implemented this :)*/

	// Render the actual BOX!!!! ouh am geee woowwww
	Render::OutLinedRect(norm(left), norm(top), w, h);

	//	x-y
	// .left-top   x-y
	// ---------- .right-top
	// |		|
	// |		|
	// |		|
	// |		|
	// |		|
	// |		|  x-y
	// ---------- .right-bottom
	// .left-bottom
	//  x-y


	// Health Bar (TODO: make work when not want box esp too, so it can work by itself)
	if (cfg.visuals.esp.HealthBar)
	{
		h -= 2; // for pixel correction, its one pixel off from the top and one pixel off from the bottom, so we gotta fix that :)
		math::Vector BarPos = math::Vector((left + w) + 2, top + 1 /*Pixel Correction*/).floor();
		float BarHeight = h * ent.GetHealth() / 100; // get health percentage out of 100, and apply it to the height to get bar's height (TODO: add percentage of max health instead of just "100")

		Render::FilledRect(BarPos.x, BarPos.y, 2, h, ImColor(0, 0, 0)); // black bg bar
		Render::FilledRect(BarPos.x, BarPos.y, 2, BarHeight, ImColor(0, 255, 0)); // green health bar
	}
}	