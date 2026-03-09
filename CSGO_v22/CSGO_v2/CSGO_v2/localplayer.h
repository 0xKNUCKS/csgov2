#pragma once
#include "entity.h"

class localplayer_t
{
public:
    gEntity* Get();

    gEntity* operator->() {
        return Get();
    }
};

inline localplayer_t LocalPlayer;
