#include "ESP.h"
#include "hook.h"
#include <math.h>
#include <format>
#include <string>
#include <vector>

void ESP::Render()
{
	if (!cfg.visuals.Enabled || !globals::g_interfaces.Engine->IsInGame())
		return;

	if (hooks::GlobalVars == nullptr) {
		std::cerr << "Error: GlobalVars is null." << std::endl;
		return;
	}

	math::Vector screenPosition;

	for (int i = 0; i <= hooks::GlobalVars->maxClients; i++)
	{
		auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

		if (!ent->isValidState() || (ent->isTeammate() && !cfg.visuals.Friendly))
			continue;

		if (!utils::WolrdToScreen(ent->getAbsOrigin(), screenPosition))
			continue;

		BBox bbox = ent->GetBoundingBox();

		if (!bbox.isValid)
			continue;

		if (cfg.visuals.esp.Lines)
			DrawLine(bbox);

		if (cfg.visuals.esp.BoudningBox) {
			switch ((eBoxType)cfg.visuals.esp.boxType)
			{
			case Outlined:
			case Filled:
				//DrawBoundingRect(bbox, (bool)cfg.visuals.esp.boxType);
				break;
			case Box3d:
				//DrawBoundingBox(bbox);
				break;
			case Corners:
				break;
			default:
				break;
			}
			//DrawBoundingBox(bbox);
			DrawBoundingRect(bbox, 0);
		}

		if (cfg.visuals.esp.HealthBar)
			DrawHealthBar(bbox, ent->health());
	}
}

void ESP::DrawLine(BBox bbox)
{
	Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, norm(bbox.topLeft.x) + (bbox.w / 2), norm(bbox.topLeft.y), 1.0f);
}

void ESP::DrawBoundingRect(BBox bbox, bool filled)
{
	if (filled)
		Render::FilledRect(norm(bbox.topLeft.x), norm(bbox.topLeft.y), bbox.w, bbox.h, ImColor(1.f, 1.f, 1.f, 0.35f));

	// Render the actual BOX!!!! ouh am geee woowwww
	Render::OutLinedRect(norm(bbox.topLeft.x), norm(bbox.topLeft.y), bbox.w, bbox.h);

}

void ESP::DrawBoundingBox(BBox bbox)
{
	// Array to store the converted 2D points of the 3D corners
	math::Vector screenCorners[8];

	// Define the 3D corners of the bounding box
	math::Vector boxCorners[8] = {
		bbox.flb, bbox.flt, bbox.frb, bbox.frt,
		bbox.blb, bbox.blt, bbox.brb, bbox.brt
	};

	// Convert each 3D corner to 2D screen space
	for (int i = 0; i < 8; i++)
	{
		if (!utils::WolrdToScreen(boxCorners[i], screenCorners[i]))
		{
			// If any point fails to project, exit the function
			return;
		}

		Render::OutLinedCircle(screenCorners[i].x, screenCorners[i].y, 35);
	}

	// Draw the front face
	Render::Line(screenCorners[0].x, screenCorners[0].y, screenCorners[1].x, screenCorners[1].y); // flb -> flt
	Render::Line(screenCorners[1].x, screenCorners[1].y, screenCorners[3].x, screenCorners[3].y); // flt -> frt
	Render::Line(screenCorners[3].x, screenCorners[3].y, screenCorners[2].x, screenCorners[2].y); // frt -> frb
	Render::Line(screenCorners[2].x, screenCorners[2].y, screenCorners[0].x, screenCorners[0].y); // frb -> flb

	// Draw the back face
	Render::Line(screenCorners[4].x, screenCorners[4].y, screenCorners[5].x, screenCorners[5].y); // blb -> blt
	Render::Line(screenCorners[5].x, screenCorners[5].y, screenCorners[7].x, screenCorners[7].y); // blt -> brt
	Render::Line(screenCorners[7].x, screenCorners[7].y, screenCorners[6].x, screenCorners[6].y); // brt -> brb
	Render::Line(screenCorners[6].x, screenCorners[6].y, screenCorners[4].x, screenCorners[4].y); // brb -> blb

	// Connect front and back faces
	Render::Line(screenCorners[0].x, screenCorners[0].y, screenCorners[4].x, screenCorners[4].y); // flb -> blb
	Render::Line(screenCorners[1].x, screenCorners[1].y, screenCorners[5].x, screenCorners[5].y); // flt -> blt
	Render::Line(screenCorners[2].x, screenCorners[2].y, screenCorners[6].x, screenCorners[6].y); // frb -> brb
	Render::Line(screenCorners[3].x, screenCorners[3].y, screenCorners[7].x, screenCorners[7].y); // frt -> brt

}

void ESP::DrawHealthBar(BBox bbox, int health)
{
	bbox.h -= 2; // for pixel correction, its one pixel off from the top and one pixel off from the bottom, so we gotta fix that :)

	math::Vector BarPos = math::Vector(bbox.topLeft.x + bbox.w + 2.f, bbox.topRight.y + 1.f /*Pixel Correction*/).floor();
	float BarHeight = bbox.h * health / 100; // get health percentage out of 100, and apply it to the height to get bar's height (TODO: add percentage of max health instead of just "100")

	Render::FilledRect(BarPos.x, BarPos.y, 2, bbox.h, ImColor(0, 0, 0)); // black bg bar
	Render::FilledRect(BarPos.x, BarPos.y, 2, BarHeight, ImColor(0, 255, 0)); // green health bar
}
