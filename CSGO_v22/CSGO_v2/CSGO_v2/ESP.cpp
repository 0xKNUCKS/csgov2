#include "ESP.h"
#include "hook.h"

void ESP::Render()
{
	if (!cfg.visuals.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	math::Vector sOrgin;
	for (int i = 0; i <= globals::EntList.Size(); i++)
	{
		if (!globals::EntList[i].isValidState() || !globals::EntList[i].GetPos().notNull())
			continue;

		math::Vector pos = globals::EntList[i].GetPos();
		if (!utils::WolrdToScreen(pos, sOrgin))
			continue;

		Render::Line(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y, sOrgin.x, sOrgin.y, 2.0f);
	}
}