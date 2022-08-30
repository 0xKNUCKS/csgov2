#include "ESP.h"

void ESP::Render()
{
	if (!cfg.visuals.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	Render::OutLinedCircle(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2, 25, ImColor(255, 0, 0));

	math::Vector sOrgin;
	for (int i = 1; i <= globals::g_interfaces.Engine->GetMaxClients(); i++)
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