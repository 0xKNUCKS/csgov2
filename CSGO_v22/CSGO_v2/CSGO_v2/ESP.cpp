#include "ESP.h"
#include "hook.h"

void ESP::Render()
{
	if (!cfg.visuals.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;


	math::Vector sOrgin;
	for (int i = 1; i <= hooks::GlobalVars->maxClients; i++)
	{
		ent_t ent(i);

		if (!ent.isValidState())
			continue;

		math::Vector pos = ent.GetPos();
		if (!utils::WolrdToScreen(pos, sOrgin))
			continue;

		Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, sOrgin.x, sOrgin.y, 2.0f);
	}
}