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
    return Entities.at(idx);
}

void CEntityList::Clear() noexcept
{
    if (!Entities.empty())
        memset(&Entities, 0, sizeof(ent_t) * 0x100);
}

void CEntityList::Update() noexcept
{
    MaxClients = hooks::GlobalVars->maxClients;

    for (int i = 1; i <= MaxClients; i++)
    {
        this->this_index = i;
        this->GetEnt();

        // note: BoneMatrix is from ent_t which is protected
        if (!this->enty->SetupBones(this->BoneMatrix, 128, 0x100, 0.f))
            memset(&this->BoneMatrix, 0, sizeof(math::Matrix3x4) * 128);

        this->Entities.at(i) = this->GetEnt();
    }
}