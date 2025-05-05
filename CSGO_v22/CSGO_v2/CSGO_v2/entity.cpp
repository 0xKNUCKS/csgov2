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

BBox gEntity::GetBoundingBox()
{
    BBox bbox;

    // Getting the entity's orgin and model bounds
	math::Vector origin = this->getRenderOrigin();
    auto model = this->getModel();
    math::Vector min, max;
    min = model->mins; max = model->maxs;

    math::Vector headPos = this->getBonePosFromChache(8);
    math::Vector top = { origin.x, origin.y, headPos.z + 4.f };
    math::Vector bottom = { origin.x, origin.y, origin.z - 4.f };

    math::Vector screenTop, screenBottom;
    if (!utils::WorldToScreen(top, screenTop) ||
        !utils::WorldToScreen(bottom, screenBottom)) {
        bbox.isValid = false;
        return bbox;
    }

	float height = screenBottom.y - screenTop.y;
    float width = height / globals::aspectRatio;

    // Calculate box position
    float left = screenBottom.x - (width / 2.0f);
    float right = screenBottom.x + (width / 2.0f);
    float topY = screenTop.y;
    float bottomY = screenBottom.y;

    // Store 2D points (maintaining compatibility with the original function)
    bbox.topLeft = { left, topY };
    bbox.topRight = { right, topY };
    bbox.bottomLeft = { left, bottomY };
    bbox.bottomRight = { right, bottomY };

    // Set dimensions
    bbox.h = height;
    bbox.w = width;

    // For compatibility with the original function's return points
    bbox.flb = screenBottom;  // front lower bottom
    bbox.brt = screenTop;     // back right top
    bbox.blb = { left, bottomY };  // back left bottom
    bbox.frt = { right, topY };    // front right top
    bbox.frb = { right, bottomY }; // front right bottom
    bbox.brb = { right, bottomY }; // back right bottom
    bbox.blt = { left, topY };     // back left top
    bbox.flt = { left, topY };     // front left top

    bbox.isValid = true;
    return bbox;
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
