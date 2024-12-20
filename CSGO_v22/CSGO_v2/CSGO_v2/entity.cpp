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

    // Getting the bounding box's min and max
    math::Vector min, max;
    auto model = this->getModel();
    min = model->mins; max = model->maxs;

    math::Vector points[] = {
        math::Vector(min.x, min.y, min.z),
        math::Vector(min.x, max.y, min.z),
        math::Vector(max.x, max.y, min.z),
        math::Vector(max.x, min.y, min.z),
        math::Vector(max.x, max.y, max.z),
        math::Vector(min.x, max.y, max.z),
        math::Vector(min.x, min.y, max.z),
        math::Vector(max.x, min.y, max.z)
    };

    math::Vector pointsTransformed[8];
    for (int i = 0; i < 8; i++)
    {
        pointsTransformed[i] = utils::VectorTransform(points[i], this->toWorldTransform());
    }

    if (!utils::WolrdToScreen(pointsTransformed[3], bbox.flb) || !utils::WolrdToScreen(pointsTransformed[5], bbox.brt)
        || !utils::WolrdToScreen(pointsTransformed[0], bbox.blb) || !utils::WolrdToScreen(pointsTransformed[4], bbox.frt)
        || !utils::WolrdToScreen(pointsTransformed[2], bbox.frb) || !utils::WolrdToScreen(pointsTransformed[1], bbox.brb)
        || !utils::WolrdToScreen(pointsTransformed[6], bbox.blt) || !utils::WolrdToScreen(pointsTransformed[7], bbox.flt))
    {
        bbox.isValid = false;
        return bbox;
    }

    math::Vector arr[] = { bbox.flb, bbox.brt, bbox.blb, bbox.frt, bbox.frb, bbox.brb, bbox.blt, bbox.flt };

    float left = bbox.flb.x;	// left
    float top = bbox.flb.y;		// top
    float right = bbox.flb.x;	// right
    float bottom = bbox.flb.y;	// bottom

    for (int i = 0; i < 8; i++)
    {
        if (left > arr[i].x)
            left = arr[i].x;
        if (top < arr[i].y)
            top = arr[i].y;
        if (right < arr[i].x)
            right = arr[i].x;
        if (bottom > arr[i].y)
            bottom = arr[i].y;
    }

    bbox.h = bottom - top;	// height
    bbox.w = -bbox.h / ceilf(globals::aspectRatio);

    // Store 2D points
    bbox.topLeft = { left, top };
    bbox.topRight = { right, top };
    bbox.bottomLeft = { left, bottom };
    bbox.bottomRight = { right, bottom };

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
