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

	for (int i = 0; i <= hooks::GlobalVars->maxClients; i++)
	{
		auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

		if (!ent->isValidState() || (ent->isTeammate() && !cfg.visuals.Friendly))
			continue;

		// TODO: Use Skeletons to define bounds (and use getRenderOrgin for better positions)
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
				DrawBoundingRect(bbox, (bool)cfg.visuals.esp.boxType);
				break;
			case Box3d:
				DrawBoundingBox(bbox);
				break;
			case Corners:
				break;
			default:
				break;
			}
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
	// Draw the front face
	Render::Line(bbox.flb.x, bbox.flb.y, bbox.flt.x, bbox.flt.y); // flb -> flt
	Render::Line(bbox.flt.x, bbox.flt.y, bbox.frt.x, bbox.frt.y); // flt -> frt
	Render::Line(bbox.frt.x, bbox.frt.y, bbox.frb.x, bbox.frb.y); // frt -> frb
	Render::Line(bbox.frb.x, bbox.frb.y, bbox.flb.x, bbox.flb.y); // frb -> flb

	// Draw the back face
	Render::Line(bbox.blb.x, bbox.blb.y, bbox.blt.x, bbox.blt.y); // blb -> blt
	Render::Line(bbox.blt.x, bbox.blt.y, bbox.brt.x, bbox.brt.y); // blt -> brt
	Render::Line(bbox.brt.x, bbox.brt.y, bbox.brb.x, bbox.brb.y); // brt -> brb
	Render::Line(bbox.brb.x, bbox.brb.y, bbox.blb.x, bbox.blb.y); // brb -> blb

	// Connect front and back faces
	Render::Line(bbox.flb.x, bbox.flb.y, bbox.blb.x, bbox.blb.y); // flb -> blb
	Render::Line(bbox.flt.x, bbox.flt.y, bbox.blt.x, bbox.blt.y); // flt -> blt
	Render::Line(bbox.frb.x, bbox.frb.y, bbox.brb.x, bbox.brb.y); // frb -> brb
	Render::Line(bbox.frt.x, bbox.frt.y, bbox.brt.x, bbox.brt.y); // frt -> brt
}

void ESP::DrawHealthBar(BBox bbox, int health)
{
	bbox.h -= 2; // for pixel correction, its one pixel off from the top and one pixel off from the bottom, so we gotta fix that :)

	math::Vector BarPos = math::Vector(bbox.topLeft.x + bbox.w + 2.f, bbox.topRight.y + 1.f /*Pixel Correction*/).floor();
	float BarHeight = bbox.h * health / 100; // get health percentage out of 100, and apply it to the height to get bar's height (TODO: add percentage of max health instead of just "100")

	Render::FilledRect(BarPos.x, BarPos.y, 2, bbox.h, ImColor(0, 0, 0)); // black bg bar
	Render::FilledRect(BarPos.x, BarPos.y, 2, BarHeight, ImColor(0, 255, 0)); // green health bar
}
