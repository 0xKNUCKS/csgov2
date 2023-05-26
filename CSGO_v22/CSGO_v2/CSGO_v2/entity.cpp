#include "entity.h"



// Ent Class
bool ent_t::isDormant()
{
	return enty->isDormant();
}

int ent_t::GetHealth()
{
	return enty->health();
}

IClientNetworkable* ent_t::Get_CN(int index)
{
    return globals::g_interfaces.ClientEntity->GetClientNetworkable(index);
}

uintptr_t ent_t::GetEnt(int index)
{
	if (index == -1)
		index = this_index;

	enty = reinterpret_cast<gEntity*>(globals::g_interfaces.ClientEntity->GetClientEntity(index));

    return (uintptr_t)enty;
}

bool ent_t::isValidState()
{
	// dont do this, NetVars use preformance and doing this constantly will drop your frames to the floor... smh.
	//ent_t::init();
	//const auto LocalPlayer = globals::g_interfaces.ClientEntity->GetClientEntity(globals::g_interfaces.Engine->GetLocalPlayerIdx());

	if (!LocalPlayer.Get())
		return 0;
	if (!reinterpret_cast<uintptr_t*>(ent_t::GetEnt()))
		return 0;
	if (ent_t::GetEnt() == LocalPlayer.Get())
		return 0;
	if (ent_t::GetHealth() <= 0)
		return 0;
	if (!ent_t::isAlive())
		return 0;

	return 1;
}

math::Vector ent_t::GetPos()
{
	return enty->getAbsOrigin();
}

//int ent_t::GetTeamId()
//{
//	//ent_t::init();
//
//	// until we fix the NetVars preformance issue
//	return *(int*)(enty + offsets::m_iTeamNum);
//}

bool ent_t::isTeammate()
{
	return LocalPlayer.GetTeamId() == ent_t::GetTeamId();
}

math::Vector ent_t::GetBonePos(int boneId)
{
	return enty->GetBonePosFromChache(boneId);
	//return ent_t::Get_CN()->GetIClientUnknown()->GetClientRenderable()->SetupBones();
}

bool ent_t::isAlive()
{
	return enty->isAlive();
}

math::Vector ent_t::GetAimAtAngles()
{
	math::Vector source = LocalPlayer->getEyePosition();
	math::Vector destination = GetBonePos(8); // Will add the option to use another bone later

	math::Vector retAngle;
	math::Vector delta = source - destination;
	float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
	retAngle.x = (float)(atan(delta.z / hyp) * (180.0f / PI));
	retAngle.y = (float)(atan(delta.y / delta.x) * (180.0f / PI));
	retAngle.z = 0.f;
	if (delta.x >= 0.f)
		retAngle.y += 180.f;

	return retAngle;
}

std::string ent_t::GetName()
{
	player_info_s pinfo;
	if (globals::g_interfaces.Engine->getPlayerInfo(this_index, pinfo))
		return pinfo.name;
	else
		return std::string();
}

float ent_t::DistTo(ent_t ent)
{
	math::Vector From = LocalPlayer.GetPos();
	math::Vector To = ent.GetPos();

	return sqrt(pow(To.x - From.x, 2) + pow(To.y - From.y, 2) + pow(To.z - From.z, 2));
}

math::Vector ent_t::GetEyePos()
{
	return enty->getEyePosition();
}

//int ent_t::Flags()
//{
//	// until we fix the NetVars preformance issue.
//	return *(int*)(enty + offsets::m_fFlags);
//}
//
//bool ent_t::isScoped()
//{
//	return *(bool*)(enty + offsets::m_bIsScoped);
//}
//
//math::Vector& ent_t::GetViewingAngles()
//{
//	// LOL: https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/public/PlayerState.h#L32
//	return *(math::Vector*)(enty + offsets::deadFlag + 0x4);
//}

// Local Player Class

uintptr_t localplayer_t::Get()
{
	this_index = globals::g_interfaces.Engine->GetLocalPlayerIdx();
	localplayer_t::Local = localplayer_t::GetEnt(this_index);

	return localplayer_t::Local;
}
