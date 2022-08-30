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

    this_ent = (uintptr_t)globals::g_interfaces.ClientEntity->GetClientEntity(index);

    return this_ent;
}

void ent_t::init()
{
    offsets::m_iHealth =		(uintptr_t)globals::g_NetVars.FindOffset("CBaseEntity", "m_iHealth"); // "m_iHealth", "DT_BasePlayer"
    offsets::m_bSpotted =		(uintptr_t)globals::g_NetVars.FindOffset("DT_BaseEntity", "m_bSpotted");
    offsets::m_iTeamNum =		(uintptr_t)globals::g_NetVars.FindOffset("CBaseEntity", "m_iTeamNum");
	offsets::m_bAlive =			(uintptr_t)globals::g_NetVars.FindOffset("DT_PlayerResource", "m_bAlive");
	offsets::m_iKills =			(uintptr_t)globals::g_NetVars.FindOffset("DT_PlayerResource", "m_iKills");
	offsets::m_vecViewOffset =	(uintptr_t)globals::g_NetVars.FindOffset("CBasePlayer", "m_vecViewOffset[0]");
	offsets::m_fFlags =			(uintptr_t)globals::g_NetVars.FindOffset("DT_BasePlayer", "m_fFlags");
	//offsets::LocalPlayer = (ent_t*)g_interfaces->ClientEntity->GetClientEntity(1);
}

bool ent_t::isValidState()
{
	ent_t::init();

	if (!globals::g_interfaces.ClientEntity->GetClientEntity(globals::g_interfaces.Engine->GetLocalPlayerIdx()))
		return 0;
	if (!reinterpret_cast<uintptr_t*>(ent_t::GetEnt()))
		return 0;
	if (ent_t::GetEnt() == (uintptr_t)globals::g_interfaces.ClientEntity->GetClientEntity(globals::g_interfaces.Engine->GetLocalPlayerIdx()))
		return 0;
	if (ent_t::GetHealth() <= 0)
		return 0;
	if (!ent_t::isAlive())
		return 0;

	return 1;
}

math::Vector ent_t::GetPos()
{
	enty = (gEntity*)GetEnt();
	return enty->getAbsOrigin();
}

int ent_t::GetTeamId()
{
	ent_t::init();

	return *(int*)(ent_t::GetEnt() + offsets::m_iTeamNum);
}

bool ent_t::isTeammate()
{
	return LocalPlayer.GetTeamId() == ent_t::GetTeamId();
}

math::Vector ent_t::GetBonePos(int boneId)
{
	enty = (gEntity*)GetEnt();
	return enty->GetBonePos(boneId);
	//return ent_t::Get_CN()->GetIClientUnknown()->GetClientRenderable()->SetupBones();
}

bool ent_t::isAlive()
{
	enty = (gEntity*)GetEnt();
	return enty->isAlive();
}

math::Vector ent_t::GetAimAtAngles()
{
	math::Vector source = LocalPlayer.GetBonePos(8);
	math::Vector destination = ent_t::GetBonePos(8);

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
	globals::g_interfaces.Engine->getPlayerInfo(this_index, pinfo);

	return pinfo.name;
}

float ent_t::DistTo(ent_t ent)
{
	math::Vector From = LocalPlayer.GetPos();
	math::Vector To = ent.GetPos();

	return sqrt(pow(To.x - From.x, 2) + pow(To.y - From.y, 2) + pow(To.z - From.z, 2));
}

math::Vector ent_t::GetEyePos()
{
	enty = (gEntity*)GetEnt();
	return enty->getEyePosition();
}

int ent_t::Flags()
{
	return *(int*)(ent_t::GetEnt() + offsets::m_fFlags);
}

// Local Player Class

uintptr_t localplayer_t::Get()
{
	localplayer_t::Local = localplayer_t::GetEnt(this_index);

	return localplayer_t::Local;
}
