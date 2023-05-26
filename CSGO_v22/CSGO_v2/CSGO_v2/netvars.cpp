#include "netvars.h"
#include "Globals.h"
#include "entity.h"

intptr_t NetVars_t::FindOffset(const char* tablename, const char* netvarName)
{
    ClientClass* clientclass = globals::g_interfaces.BaseClient->GetAllClasses();

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
void NetVars_t::Init()
{
    offsets::m_bSpotted = (uintptr_t)globals::g_NetVars.FindOffset("DT_BaseEntity", "m_bSpotted");
    offsets::m_iTeamNum = (uintptr_t)globals::g_NetVars.FindOffset("DT_BaseEntity", "m_iTeamNum");
    offsets::m_iKills = (uintptr_t)globals::g_NetVars.FindOffset("DT_PlayerResource", "m_iKills");
    offsets::m_fFlags = (uintptr_t)globals::g_NetVars.FindOffset("DT_BasePlayer", "m_fFlags");
    offsets::m_bIsScoped = (uintptr_t)globals::g_NetVars.FindOffset("DT_CSPlayer", "m_bIsScoped");
    offsets::deadFlag = (uintptr_t)globals::g_NetVars.FindOffset("DT_BasePlayer", "deadflag");
    offsets::m_vecVelocity = (uintptr_t)globals::g_NetVars.FindOffset("DT_BasePlayer", "m_vecVelocity[0]");

#ifdef _DEBUG
    PrintNetVars(globals::g_interfaces.BaseClient->GetAllClasses());
#endif // _DEBUG

}