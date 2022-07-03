#pragma once
#include "netvars.h"
#include "offsets.hpp"
#include <iostream>
#include "math.h"
#include "Globals.h"

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a,b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

enum class MoveType
{
    Noclip = 8,
    Ladder = 9
};

enum PlayerFlag
{
    PlayerFlag_OnGround = 1 << 0, // At rest / on the ground
    PlayerFlag_Crouched = 1 << 1, // Player is fully crouched
    PlayerFlag_WaterJump = 1 << 2, // Player jumping out of water
    PlayerFlag_OnTrain = 1 << 3, // Player is controlling a train, so movement commands should be ignored on client during prediction
    PlayerFlag_InRain = 1 << 4, // Indicates the entity is standing in rain
    PlayerFlag_Frozen = 1 << 5, // Player is frozen for 3rd person camera
    PlayerFlag_AtControls = 1 << 6, // Player can't move, but keeps key inputs for controlling another entity
    PlayerFlag_Client = 1 << 7, // Is a player
    PlayerFlag_FakeClient = 1 << 8, // Fake client, simulated server side; don't send network messages to them. NON-PLAYER SPECIFIC (i.e. not used by GameMovement or the client.dll) - can still be applied to players, though
    PlayerFlag_InWater = 1 << 9, // In water
    PlayerFlag_Fly = 1 << 10, // Changes the SV_Movestep() behavior to not need to be on ground
    PlayerFlag_Swim = 1 << 11, // Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
    PlayerFlag_Conveyor = 1 << 12,
    PlayerFlag_NPC = 1 << 13,
    PlayerFlag_GodMode = 1 << 14,
    PlayerFlag_NoTarget = 1 << 15,
    PlayerFlag_AimTarget = 1 << 16, // Set if the crosshair needs to aim onto the entity
    PlayerFlag_PartialGround = 1 << 17, // Not all corners are valid
    PlayerFlag_StaticProp = 1 << 18, // Eetsa static prop!
    PlayerFlag_Graphed = 1 << 19, // Worldgraph has this ent listed as something that blocks a connection
    PlayerFlag_Grenade = 1 << 20,
    PlayerFlag_StepMovement = 1 << 21, // Changes the SV_Movestep() behavior to not do any processing
    PlayerFlag_NoTouch = 1 << 22, // Doesn't generate touch functions, generates Untouch() for anything it was touching when this flag was set
    PlayerFlag_BaseVelocity = 1 << 23, // Base velocity has been applied this frame (used to convert base velocity into momentum)
    PlayerFlag_WorldBrush = 1 << 24, // Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
    PlayerFlag_NPCSpecific = 1 << 25, // This is an object that NPCs should see. Missiles, for example
    PlayerFlag_KillMe = 1 << 26, // This entity is marked for death -- will be freed by game DLL
    PlayerFlag_OnFire = 1 << 27, // You know...
    PlayerFlag_Dissolving = 1 << 28, // We're dissolving!
    PlayerFlag_TransitionRagdoll = 1 << 29, // In the process of turning into a client side ragdoll
    PlayerFlag_NotBlockableByPlayer = 1 << 30 // Pusher that can't be blocked by the player
};

namespace offsets
{
    inline uintptr_t m_iHealth  ; //=  g_NetVars->FindOffset("m_iHealth", "DT_BasePlayer");
    inline uintptr_t m_vecOrigin; //=  g_NetVars->FindOffset("DT_BaseEntity", "m_vecOrigin");
    inline uintptr_t m_bSpotted ; //=  g_NetVars->FindOffset("DT_BaseEntity", "m_bSpotted");
    inline uintptr_t m_iTeamNum ; //=  g_NetVars->FindOffset("DT_BaseEntity", "m_iTeamNum");
    inline uintptr_t m_bAlive;
    inline uintptr_t m_iKills;
    inline uintptr_t m_vecViewOffset;
    inline uintptr_t m_fFlags;
    constexpr ::std::ptrdiff_t dwBoneMatrix = hazedumper::netvars::m_dwBoneMatrix;
}

class ent_t
{
public:
    int this_index = 1;

    bool isDormant();

    int GetHealth();

    bool isValidState();

    math::Vector GetPos();

    uintptr_t GetEnt(int index = -1);

    int GetTeamId();

    bool isTeammate();

    bool isAlive();

    math::Vector GetBonePos(int boneId);

    math::Vector GetAimAtAngles();

    std::string GetName();

    math::Vector GetEyePos();

    float DistTo(ent_t ent);

    int Flags();

    ent_t(int idx) { ent_t::init(); this_index = idx; enty = (gEntity*)GetEnt(); }
    ent_t() { ent_t::init(); this_index = 1; enty = (gEntity*)GetEnt();}

    void init();

private:
    uintptr_t this_ent;
    gEntity* enty;

    IClientNetworkable* Get_CN(int index);
};

class localplayer_t : public ent_t
{
public:
    uintptr_t Get();

    localplayer_t()
    {
        this_index = globals::g_interfaces.Engine->GetLocalPlayerIdx();
        Get();
    }

protected:
    uintptr_t Local;
};

inline localplayer_t LocalPlayer;

