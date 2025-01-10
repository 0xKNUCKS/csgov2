#pragma once
#include "netvars.h"
#include <iostream>
#include <vector>
#include "math.h"

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a,b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

#define NETVAR_DECL(name, type, offset) \
    type name() noexcept \
{\
    return *(type*)(this + offset); \
}

// bounding box
struct BBox {
    // 2D points
    math::Vector topLeft;
    math::Vector topRight;
    math::Vector bottomLeft;
    math::Vector bottomRight;

    // 3D points
    math::Vector flb; // Front Left Bottom
    math::Vector flt; // Front Left Top
    math::Vector frb; // Front Right Bottom
    math::Vector frt; // Front Right Top
    math::Vector blb; // Back Left Bottom
    math::Vector blt; // Back Left Top
    math::Vector brb; // Back Right Bottom
    math::Vector brt; // Back Right Top

    // Dimensions
    int w, h;

    bool isValid = false;
};

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
    inline uintptr_t m_bSpotted ; //=  g_NetVars->FindOffset("DT_BaseEntity", "m_bSpotted");
    inline uintptr_t m_iTeamNum ; //=  g_NetVars->FindOffset("DT_BaseEntity", "m_iTeamNum");
    inline uintptr_t m_iKills;
    inline uintptr_t m_fFlags;
    inline uintptr_t m_bIsScoped;
    inline uintptr_t deadFlag;
    inline uintptr_t m_vecVelocity;
}

// fuck this shit idek wtf is this, pasted from NEPS. sad.
class gEntity
{
public:
    // virtual functions from the entity
    VIRTUAL_METHOD(void, release, 1, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(ClientClass*, getClientClass, 2, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(void, preDataUpdate, 6, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
    VIRTUAL_METHOD(void, postDataUpdate, 7, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
    VIRTUAL_METHOD(bool, isDormant, 9, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(int, index, 10, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(void, setDestroyedOnRecreateEntities, 13, (), (this + sizeof(uintptr_t) * 2))
    
    VIRTUAL_METHOD(math::Vector&, getRenderOrigin, 1, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(bool, shouldDraw, 3, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(const model_t*, getModel, 8, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(const math::Matrix3x4&, toWorldTransform, 32, (), (this + sizeof(uintptr_t)))
    
    VIRTUAL_METHOD(int&, handle, 2, (), (this))
    VIRTUAL_METHOD(void*, getCollideable, 3, (), (this))
    VIRTUAL_METHOD(const math::Vector&, getAbsOrigin, 10, (), (this))
    VIRTUAL_METHOD(const math::Vector&, getAbsAngle, 11, (), (this))
    VIRTUAL_METHOD(void, setModelIndex, 75, (int index), (this, index))
    VIRTUAL_METHOD(int, health, 122, (), (this))
    VIRTUAL_METHOD(bool, isAlive, 156, (), (this))
    VIRTUAL_METHOD(bool, isPlayer, 158, (), (this))
    VIRTUAL_METHOD(bool, isWeapon, 166, (), (this))
    VIRTUAL_METHOD(void, updateClientSideAnimation, 224, (), (this))
    VIRTUAL_METHOD(int, getWeaponSubType, 282, (), (this))
    VIRTUAL_METHOD(void, getObserverMode, 294, (), (this))
    VIRTUAL_METHOD(float, getSpread, 453, (), (this))
    VIRTUAL_METHOD(void, getWeaponType, 455, (), (this))
    VIRTUAL_METHOD(void*, getWeaponData, 461, (), (this))
    VIRTUAL_METHOD(int, getMuzzleAttachmentIndex1stPerson, 468, (gEntity* viewModel), (this, viewModel))
    VIRTUAL_METHOD(int, getMuzzleAttachmentIndex3rdPerson, 469, (), (this))
    VIRTUAL_METHOD(float, getInaccuracy, 483, (), (this))
    VIRTUAL_METHOD(void, updateInaccuracyPenalty, 484, (), (this))
    
    // NetVars
    NETVAR_DECL(getTeamID, int, offsets::m_iTeamNum)
    NETVAR_DECL(flags, int, offsets::m_fFlags)
    NETVAR_DECL(isScoped, bool, offsets::m_bIsScoped)
    NETVAR_DECL(getViewingAngles, math::Vector, offsets::deadFlag + 0x4)
    NETVAR_DECL(getVelocity, math::Vector, offsets::m_vecVelocity)

    // custom functions

    // Validate this entity
    bool isValidState();

    bool isTeammate();

    math::Vector getAimAtAngles();

    std::string getName();

    BBox GetBoundingBox();

    auto getEyePosition() noexcept
    {
        math::Vector v;
        VirtualMethod::call<void, 285>(this, std::ref(v));
        return v;
    }

    bool SetupBones(math::Matrix3x4* out, int maxBones, int boneMask, float currentTime) noexcept
    {
        return VirtualMethod::call<bool, 13>(this + sizeof(uintptr_t), out, maxBones, boneMask, currentTime);
    }

    math::UtlVector<math::Matrix3x4>& boneCache() noexcept { return *(math::UtlVector<math::Matrix3x4>*)((uintptr_t)this + 0x2914); }
    math::Vector getBonePosFromChache(int bone) noexcept { return boneCache()[bone].GetVecOrgin(); }
};

static_assert(sizeof(gEntity) == 1);

