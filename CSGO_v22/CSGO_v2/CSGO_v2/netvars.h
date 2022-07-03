#pragma once
#include "interface.h"

//inline void PrintNetVars(ClientClass* clientclass)
//{
//    RecvTable* table;
//    ClientClass* currnode = clientclass;
//
//    for (auto currnode = clientclass; currnode; currnode = currnode->m_pNext)
//    {
//        table = currnode->m_pRecvTable;
//
//        for (int i = 0; i < table->m_nProps; i++)
//        {
//            RecvProp prop = table->m_pProps[i];
//
//            printf("[ %s ]\n%s\n\n", table->m_pNetTableName, prop.m_pVarName);
//        }
//    }
//}

class NetVars_t
{
public:
    intptr_t FindOffset(const char* tablename, const char* netvarName);

private:
    interfaces_t pinterf;

    inline intptr_t GetOffset(RecvTable* table, const char* tablename, const char* netvarName);

    inline intptr_t GetNetVarOffset(const char* tablename, const char* netvarName, ClientClass* clientclass);
};

//extern NetVars_t g_NetVars;
//std::unique_ptr<const NetVars_t> g_NetVars;