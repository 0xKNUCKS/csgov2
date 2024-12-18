#include "entity.h"
#include "Globals.h"
#include "localplayer.h"
#include <cmath>

std::string gEntity::getName()
{
    player_info_s pinfo;
    if (this != nullptr && globals::g_interfaces.Engine->getPlayerInfo(this->index(), pinfo))
        return pinfo.name;
    else
        return std::string();
}

bool gEntity::isValidState()
{
    if (LocalPlayer.Get() == nullptr)
        return 0;
    if (this == nullptr)
        return 0;
    if (this == LocalPlayer.Get())
        return 0;
    if (this->health() <= 0)
        return 0;
    if (!this->isAlive())
        return 0;

    return 1;
}

bool gEntity::isTeammate()
{
    return LocalPlayer->getTeamID() == this->getTeamID();
}

math::Vector gEntity::getAimAtAngles()
{
    math::Vector source = LocalPlayer->getEyePosition();
    math::Vector destination = getBonePosFromChache(8); // Ensure this returns the correct bone position

    math::Vector delta = destination - source;
    float hypotenuse = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    math::Vector angles;
    angles.x = std::atan2(-delta.z, hypotenuse) * RAD_TO_DEG; // pitch
    angles.y = std::atan2(delta.y, delta.x) * RAD_TO_DEG; // yaw
    angles.z = 0.0f;

    // Normalize angles to ensure they are within the correct range
    angles.normalizeDeg();

    return angles;
}
