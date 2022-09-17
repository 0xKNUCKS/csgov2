#pragma once
#include <vector>

class ent_t;

struct CEntityList
{
    CEntityList();
    ~CEntityList();

    int MaxClients;

    void Clear() noexcept;
    void Update() noexcept;
    int Size() noexcept;

    ent_t& operator[](int);

protected:
    std::vector<ent_t> Entities;
    bool Update(gEntity* ent) noexcept;
};