#pragma once
#include "entity.h"

class localplayer_t
{
public:
    gEntity* Get();

    localplayer_t()
    {
        Get();
    }

    gEntity* operator->() {
        return Local;
    }

protected:
    gEntity* Local;
};

inline localplayer_t LocalPlayer;
