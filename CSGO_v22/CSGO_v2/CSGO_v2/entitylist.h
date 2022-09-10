#pragma once
#include <array>
#include "entity.h"

struct CEntityList : ent_t
{
    CEntityList();
    ~CEntityList();

    int MaxClients;

    void Clear() noexcept;
    void Update() noexcept;

    ent_t& operator[](int);

protected:
    std::array<ent_t, 0x100> Entities;
};