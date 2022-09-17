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
        Entities.clear();
}

int CEntityList::Size() noexcept
{
    return Entities.size() - 1; // -1 because size() will return the number of items, eg 64, but we will start from 0 and go to 63
}

void CEntityList::Update() noexcept
{
    if (!globals::g_interfaces.Engine->IsInGame()) {
        Clear(); // Clear if not in game
        return;
    }

    MaxClients = hooks::GlobalVars->maxClients;
    if (Entities.size() != MaxClients) {
        if (abs(MaxClients) > Entities.max_size())
            return;
        Entities.resize(MaxClients); // Resize the container to avoid over complex code
    }

    for (int i = 1; i <= Entities.size(); i++)
    {
        ent_t ent(i);
        
        if (ent.isValidState())
        {
            if (!Update(ent))
                continue;
        }

        this->Entities[i - 1] = ent; // -1 because the vector index starts at 0
    }
}

bool CEntityList::Update(ent_t& ent) noexcept
{
    const model_t* Model = reinterpret_cast<gEntity*>(ent.GetEnt())->getModel();
    if (!Model)
        return 0;

    const auto StudioModel = globals::g_interfaces.ModelInfo->GetStudioModel(Model);
    if (!StudioModel)
        return 0;

    ent.BoneMatrix = reinterpret_cast<gEntity*>(ent.GetEnt())->boneCache();

    ent.Bones.clear();
    ent.Bones.reserve(20); // 20 Max HitBoxes

    for (int i = 0; i < StudioModel->numbones; i++)
    {
        const mstudiobone_t* Bone = StudioModel->pBone(i);

        if (!Bone || Bone->parent < 0 || !(Bone->flags & BONE_USED_BY_HITBOX))
            continue;

        ent.Bones.emplace_back(ent.BoneMatrix[i].GetVecOrgin(), ent.BoneMatrix[Bone->parent].GetVecOrgin());
    }
}