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

		if (!ent->isValidState() ||
			(ent->isTeammate() && !cfg.visuals.Friendly) ||
			(ent->isDormant() && !cfg.visuals.esp.Dormant))
			continue;

		// if the player is dormant, render it's ESP in low opacity
		baseOpacity = ent->isDormant() ? 0.15f : 1.f;

		// TODO: Use Skeletons to define bounds (and use getRenderOrgin for better positions)
		BBox bbox = ent->GetBoundingBox();

		if (!bbox.isValid)
			continue;

		if (cfg.visuals.esp.Skeleton)
			DrawSkeleton(ent);

		if (cfg.visuals.esp.Lines)
			DrawLine(bbox);

		if (cfg.visuals.esp.Name)
			DrawName(bbox, ent->getName());

		if (cfg.visuals.esp.HealthBar)
			DrawHealthBar(bbox, ent->health());

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
	
	}
}

void ESP::DrawLine(BBox bbox)
{
	Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, norm(bbox.topLeft.x) + (bbox.w / 2), norm(bbox.bottomLeft.y), 1.0f, ImColor(1.f, 1.f, 1.f, baseOpacity));
}

void ESP::DrawBoundingRect(BBox bbox, bool filled)
{
	if (filled)
		Render::FilledRect(bbox.topLeft.x, bbox.topLeft.y, bbox.w, bbox.h, ImColor(1.f, 1.f, 1.f, 0.35f * baseOpacity));

	// Render the actual BOX!!!! ouh am geee woowwww
	Render::OutLinedRect(bbox.topLeft.x, bbox.topLeft.y, bbox.w, bbox.h, 1.f, ImColor(1.f, 1.f, 1.f, baseOpacity));

}

void ESP::DrawBoundingBox(BBox bbox)
{
	// Draw the front face
	Render::Line(bbox.flb.x, bbox.flb.y, bbox.flt.x, bbox.flt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // flb -> flt
	Render::Line(bbox.flt.x, bbox.flt.y, bbox.frt.x, bbox.frt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // flt -> frt
	Render::Line(bbox.frt.x, bbox.frt.y, bbox.frb.x, bbox.frb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // frt -> frb
	Render::Line(bbox.frb.x, bbox.frb.y, bbox.flb.x, bbox.flb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // frb -> flb

	// Draw the back face
	Render::Line(bbox.blb.x, bbox.blb.y, bbox.blt.x, bbox.blt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // blb -> blt
	Render::Line(bbox.blt.x, bbox.blt.y, bbox.brt.x, bbox.brt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // blt -> brt
	Render::Line(bbox.brt.x, bbox.brt.y, bbox.brb.x, bbox.brb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // brt -> brb
	Render::Line(bbox.brb.x, bbox.brb.y, bbox.blb.x, bbox.blb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // brb -> blb

	// Connect front and back faces
	Render::Line(bbox.flb.x, bbox.flb.y, bbox.blb.x, bbox.blb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // flb -> blb
	Render::Line(bbox.flt.x, bbox.flt.y, bbox.blt.x, bbox.blt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // flt -> blt
	Render::Line(bbox.frb.x, bbox.frb.y, bbox.brb.x, bbox.brb.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // frb -> brb
	Render::Line(bbox.frt.x, bbox.frt.y, bbox.brt.x, bbox.brt.y,
		1.f, ImColor(1.f, 1.f, 1.f, baseOpacity)); // frt -> brt
}

void ESP::DrawHealthBar(BBox bbox, int health)
{
	math::Vector barPos = math::Vector(bbox.topRight.x + 2.f, bbox.bottomLeft.y);
	float barHeight = bbox.h * health / 100; // get health percentage % and apply it to the bar's height

	Render::FilledRect(barPos.x, barPos.y, 2, -bbox.h /*hehe*/, ImColor(0.f, 0.f, 0.f, baseOpacity)); // fixed black bg bar
	Render::FilledRect(barPos.x, barPos.y, 2, -barHeight /*hehe*/, ImColor(1.f, 1.f, 1.f, baseOpacity)); // actual health bar
}

void ESP::DrawName(BBox bbox, std::string name)
{
	ImVec2 nameSize = ImGui::CalcTextSize(name.c_str());
	math::Vector namePos = math::Vector(bbox.topLeft.x + bbox.w / 2 - nameSize.x/2, bbox.bottomLeft.y);

	Render::OutLinedText(name.c_str(), namePos.x, namePos.y, ImGui::GetBackgroundDrawList(), ImColor(1.f, 1.f, 1.f, baseOpacity));
}

void ESP::DrawSkeleton(gEntity* entity)
{
	auto model = entity->getModel();
	if (!model)
		return;

	auto studioHDR = globals::g_interfaces.ModelInfo->GetStudioModel(model);
	if (!studioHDR)
		return;

	for (int i = 0; i < studioHDR->numbones; i++) {
		auto bone = studioHDR->pBone(i);

		if (!bone || !(bone->flags & BONE_USED_BY_HITBOX) || bone->parent == -1)
			continue;

		math::Vector childBone = entity->getBonePosFromChache(i);
		math::Vector parentBone = entity->getBonePosFromChache(bone->parent);

		math::Vector childScreenPos, parentScreenPos;
		if (!utils::WorldToScreen(childBone, childScreenPos) ||
			!utils::WorldToScreen(parentBone, parentScreenPos))
			continue;

		Render::Line(childScreenPos.x, childScreenPos.y,
					parentScreenPos.x, parentScreenPos.y, 1.f, ImColor(1.f, 1.f, 1.f, baseOpacity));
	}
}
