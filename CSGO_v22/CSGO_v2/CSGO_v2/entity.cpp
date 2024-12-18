#include "entity.h"
#include "Globals.h"
#include "localplayer.h"

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
    math::Vector destination = getBonePosFromChache(8); // Will add the option to use another bone later

    math::Vector retAngle = {};
    math::Vector delta = source - destination;

    const float hyp = std::hypot(delta.x, delta.y);
    retAngle.x = atan2(delta.z, hyp) * RAD_TO_DEG; // pitch
    retAngle.y = atan2(delta.y, delta.x) * RAD_TO_DEG; // yaw
    retAngle.z = 0.f;

    return retAngle.normalize();
}
