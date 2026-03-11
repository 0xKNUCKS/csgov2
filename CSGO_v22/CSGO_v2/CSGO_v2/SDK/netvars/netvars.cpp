#include "netvars.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/entity.h"
#include "lib/Error/Log.h"

intptr_t NetVars_t::FindOffset(const char* tablename, const char* netvarName)
{
    ClientClass* clientclass = globals::g_interfaces.BaseClient->GetAllClasses();

    if (!clientclass)
        return 0;

    return GetNetVarOffset(tablename, netvarName, clientclass);
}

intptr_t NetVars_t::GetOffset(RecvTable* table, const char* tablename, const char* netvarName)
{
    for (int i = 0; i < table->m_nProps; i++)
    {
        RecvProp prop = table->m_pProps[i];

        if (!_stricmp(prop.m_pVarName, netvarName))
            return prop.m_Offset;

        if (prop.m_pDataTable)
        {
            intptr_t offset = GetOffset(prop.m_pDataTable, tablename, netvarName);

            if (offset)
            {
                return offset + prop.m_Offset;
            }
        }
    }

    return 0;
}

intptr_t NetVars_t::GetNetVarOffset(const char* tablename, const char* netvarName, ClientClass* clientclass)
{
    ClientClass* currnode = clientclass;

    for (currnode = clientclass; currnode; currnode = currnode->m_pNext)
    {
        if (!_stricmp(tablename, currnode->m_pRecvTable->m_pNetTableName))
        {
            return GetOffset(currnode->m_pRecvTable, tablename, netvarName);
        }
    }

    return 0;
}

// Temporary fix
bool NetVars_t::Init()
{
    bool allOk = true;

    // Helper: find offset, log only on failure
    auto findAndValidate = [&](const char* table, const char* name) -> uintptr_t {
        intptr_t offset = globals::g_NetVars.FindOffset(table, name);
        if (offset == 0) {
            Log::Err("NetVars", "Failed to find '{}.{}' - using 0", table, name);
            allOk = false;
        }
        return (uintptr_t)offset;
    };

    offsets::m_bSpotted =    findAndValidate("DT_BaseEntity", "m_bSpotted");
    offsets::m_iTeamNum =    findAndValidate("DT_BaseEntity", "m_iTeamNum");
    offsets::m_iKills =      findAndValidate("DT_PlayerResource", "m_iKills");
    offsets::m_fFlags =      findAndValidate("DT_BasePlayer", "m_fFlags");
    offsets::m_bIsScoped =   findAndValidate("DT_CSPlayer", "m_bIsScoped");
    offsets::deadFlag =      findAndValidate("DT_BasePlayer", "deadflag");
    offsets::m_vecVelocity =  findAndValidate("DT_BasePlayer", "m_vecVelocity[0]");
    offsets::m_vecViewOffset = findAndValidate("DT_BasePlayer", "m_vecViewOffset[0]");
    offsets::m_aimPunchAngle = findAndValidate("DT_BasePlayer", "m_aimPunchAngle");

    // Anti-Flash
    offsets::m_flFlashMaxAlpha = findAndValidate("DT_CSPlayer", "m_flFlashMaxAlpha");
    offsets::m_flFlashDuration = findAndValidate("DT_CSPlayer", "m_flFlashDuration");

    // Backtracking / timing
    offsets::m_flSimulationTime = findAndValidate("DT_BaseEntity", "m_flSimulationTime");
    offsets::m_nTickBase = findAndValidate("DT_BasePlayer", "m_nTickBase");

    // Weapon handling
    offsets::m_hActiveWeapon = findAndValidate("DT_BaseCombatCharacter", "m_hActiveWeapon");
    offsets::m_flNextPrimaryAttack = findAndValidate("DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
    offsets::m_iClip1 = findAndValidate("DT_BaseCombatWeapon", "m_iClip1");
    offsets::m_iShotsFired = findAndValidate("DT_CSPlayer", "m_iShotsFired");

    // ESP extras
    offsets::m_ArmorValue = findAndValidate("DT_CSPlayer", "m_ArmorValue");
    offsets::m_bHasHelmet = findAndValidate("DT_CSPlayer", "m_bHasHelmet");
    offsets::m_bIsDefusing = findAndValidate("DT_CSPlayer", "m_bIsDefusing");
    offsets::m_iAccount = findAndValidate("DT_CSPlayer", "m_iAccount");
    offsets::m_bGunGameImmunity = findAndValidate("DT_CSPlayer", "m_bGunGameImmunity");

    // Resolver / anti-aim
    offsets::m_angEyeAngles = findAndValidate("DT_CSPlayer", "m_angEyeAngles[0]");
    offsets::m_flLowerBodyYawTarget = findAndValidate("DT_CSPlayer", "m_flLowerBodyYawTarget");

    // Spectator
    offsets::m_hObserverTarget = findAndValidate("DT_BasePlayer", "m_hObserverTarget");
    offsets::m_iObserverMode = findAndValidate("DT_BasePlayer", "m_iObserverMode");

    // Skin Changer
    offsets::m_iItemDefinitionIndex = findAndValidate("DT_BaseAttributableItem", "m_iItemDefinitionIndex");
    offsets::m_iEntityQuality = findAndValidate("DT_BaseAttributableItem", "m_iEntityQuality");
    offsets::m_iItemIDHigh = findAndValidate("DT_BaseAttributableItem", "m_iItemIDHigh");
    offsets::m_iItemIDLow = findAndValidate("DT_BaseAttributableItem", "m_iItemIDLow");
    offsets::m_iAccountID = findAndValidate("DT_BaseAttributableItem", "m_iAccountID");
    offsets::m_nFallbackPaintKit = findAndValidate("DT_BaseAttributableItem", "m_nFallbackPaintKit");
    offsets::m_nFallbackSeed = findAndValidate("DT_BaseAttributableItem", "m_nFallbackSeed");
    offsets::m_flFallbackWear = findAndValidate("DT_BaseAttributableItem", "m_flFallbackWear");
    offsets::m_nFallbackStatTrak = findAndValidate("DT_BaseAttributableItem", "m_nFallbackStatTrak");
    offsets::m_szCustomName = findAndValidate("DT_BaseAttributableItem", "m_szCustomName");
    offsets::m_OriginalOwnerXuidLow = findAndValidate("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
    offsets::m_OriginalOwnerXuidHigh = findAndValidate("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");

    // Weapon iteration / viewmodel
    offsets::m_hMyWeapons = findAndValidate("DT_BaseCombatCharacter", "m_hMyWeapons");
    offsets::m_hViewModel = findAndValidate("DT_BasePlayer", "m_hViewModel[0]");
    offsets::m_nModelIndex = findAndValidate("DT_BaseEntity", "m_nModelIndex");
    offsets::m_hWeaponWorldModel = findAndValidate("DT_BaseCombatWeapon", "m_hWeaponWorldModel");

    if (!allOk)
        Log::Err("NetVars", "Some netvar offsets failed to resolve - features may crash");

    // PrintNetVars dumps every single netvar in the game - very slow.
    // Uncomment only when you specifically need to find netvar names/tables.
    //#ifdef _DEBUG
    //    PrintNetVars(globals::g_interfaces.BaseClient->GetAllClasses());
    //#endif

    return allOk;
}
