#include "aimbot.h"

// TODO: Aimbot sometimes aim at incorrect weird stuff pls fix and thanks :) (seems to be worse with smooth too) (its probably because of invalid entites, i need to to do better entity checks) (yep the issue is 100% invalid entites, i just need better entity checks, that will be for the next update ;))
// Wrap for the aimbot function 
void aimbot::Run(CUserCmd* cmd)
{
	if (!cfg.aimbot.Enabled)
		return;

	if (!GetAsyncKeyState(cfg.aimbot.Key))
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	if (TargetsArr.empty())
		return;

	auto BestEnt = TargetsArr[0].second; // 0 for the closest/best

	if (BestEnt.isTeammate() && !cfg.aimbot.FriendlyFire)
		return;

	math::Vector AimAtAngles = BestEnt.GetAimAtAngles().normalize();
	math::Vector CurrAngles = globals::g_interfaces.Engine->GetViewAngles();

	math::Vector AimDelta = AimAtAngles - CurrAngles;
	math::Vector AimAngles = CurrAngles + (AimDelta / cfg.aimbot.Smooth);

	bool isSilent = cfg.aimbot.Silent;
	if (cmd->buttons & cmd->IN_ATTACK)
		cmd->viewangles = isSilent ? AimAtAngles : AimAngles; // if its silent aim, aim at target directly without any smooth.
	if (!isSilent)
		globals::g_interfaces.Engine->SetViewAngles(AimAngles); // if silent aim isnt on, itll also change engine's viewAngles
}

// Get an array of the best target entities for the aimbot, using the aimbot config settings
std::vector<std::pair<float, ent_t>> aimbot::GetTargetsArr(Config::Aimbot cfg)
{
	// Because used multiple times
	int EntListSize = globals::EntList.Size();

	// initilize the array
	std::vector<std::pair<float, ent_t>> Arr;
	Arr.reserve(EntListSize + 1); // +1 because the size function returns the size with -1 (for better uses in other cases)
	Arr.clear();

	// Create an array with all of the players within the fov with their fov.
	for (int i = 0; i <= EntListSize; i++)
	{
		ent_t ent = globals::EntList[i];
		if (!ent.isValidState())
			continue;

		float fov = abs((LocalPlayer->getAbsAngle() - ent.GetAimAtAngles()).normalize().length()); // entity FOV delta

		if (fov <= cfg.FOV) // if the entity's fov is in the range of the configured fov, add the entity to the array 
			Arr.push_back({ fov, ent }); // {first pair, second pair}
	}

	int ArrSize = Arr.size() - 1; // Used Multiple times, and -1 because size will not start from 0
	// After creating an array of all the players in the game with the fov, we arrange it and sort it from lowest to biggest
	for (int first = 0; first <= ArrSize; first++)
	{
		for (int next = first + 1; next <= ArrSize; next++)
		{
			if (Arr[first].first > Arr[next].first) // Compare the fov's to arrange them from lowest to biggest
			{
				auto FirstNum = Arr[first]; // store the first num in a temp var because it will be replaced
				Arr[first] = Arr[next]; // replace the first num with the next num (the smaller one)
				Arr[next] = FirstNum; // replace the next num with the first num (restore to the first num instead of the next smaller num)
				// this is a basic var swap operation, done to swap the biggest number to the smaller one,
				// this will output an array starting from the smaller numbers to the biggest
			}
		}
	}

	if (Arr.size() > cfg.MaxPlayersInFov)
	{
		// then resize the array to cut it to the amount of max players allowed to scan in a FOV before returning
		Arr.resize(cfg.MaxPlayersInFov - 1); // -1 cuz arrays start from 0 :O
	}


	// omg function done!
	return Arr;
}
