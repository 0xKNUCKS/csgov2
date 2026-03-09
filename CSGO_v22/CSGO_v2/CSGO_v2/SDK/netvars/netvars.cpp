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
    offsets::m_vecVelocity = findAndValidate("DT_BasePlayer", "m_vecVelocity[0]");

    if (!allOk)
        Log::Err("NetVars", "Some netvar offsets failed to resolve - features may crash");

    // PrintNetVars dumps every single netvar in the game - very slow.
    // Uncomment only when you specifically need to find netvar names/tables.
    //#ifdef _DEBUG
    //    PrintNetVars(globals::g_interfaces.BaseClient->GetAllClasses());
    //#endif

    return allOk;
}
