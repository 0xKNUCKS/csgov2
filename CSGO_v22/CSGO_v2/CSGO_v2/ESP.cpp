#include "ESP.h"

void ESP::Render()
{
	if (!cfg.visuals.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	Render::OutLinedCircle(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2, 25, ImColor(255, 0, 0));

	math::Vector sOrgin;
	int maxidx = globals::g_interfaces.ClientEntity->GetHighestEntityIndex();
	for (int i = 1; i <= maxidx; i++)
	{
		ent_t ent(2);

		if (!ent.isValidState())
			continue;

		math::Vector pos = ent.GetPos();
		if (!Render::WolrdToScreen(pos, sOrgin))
			continue;

		Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, sOrgin.x, sOrgin.y);
	}
}
