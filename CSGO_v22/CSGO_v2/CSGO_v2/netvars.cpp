#include "netvars.h"

intptr_t NetVars_t::FindOffset(const char* tablename, const char* netvarName)
{
    NetVars_t::pinterf.init();
    ClientClass* clientclass = (ClientClass*)NetVars_t::pinterf.BaseClient->GetAllClasses();

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