#include "entitylist.h"
#include "hook.h"

CEntityList::~CEntityList()
{
    Clear();
}
CEntityList::CEntityList()
{
    Clear();
}

ent_t& CEntityList::operator[](int idx)
{
    return Entities.at(idx-1); // - 1 because it starts at 0.
}

void CEntityList::Clear() noexcept
{
    if (!Entities.empty())
        Entities.clear();
}

void CEntityList::Update() noexcept
{
    MaxClients = hooks::GlobalVars->maxClients;

    for (int i = 1; i <= MaxClients; i++)
    {
        ent_t ent(i);

        if (!reinterpret_cast<gEntity*>(ent.GetEnt())->SetupBones(ent.BoneMatrix, 128, 0x100, 0.f))
            memset(&ent.BoneMatrix, 0, sizeof(math::Matrix3x4) * 128);

        this->Entities.push_back(ent);
    }
}