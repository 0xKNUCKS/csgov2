#pragma once
#include "math.h"
#include "Pad.h"

class CViewSetup
{
public:
	PAD(172);
	void* csm;

	// horizontal FOV in degrees
	float		fov;
	// horizontal FOV in degrees for in-view model
	float		fovViewmodel;

	// 3D origin of camera
	math::Vector		origin;

	// heading of camera (pitch, yaw, roll)
	math::Vector		angles;
	// local Z coordinate of near plane of camera
	float		zNear;
	// local Z coordinate of far plane of camera
	float		zFar;

	// local Z coordinate of near plane of camera ( when rendering view model )
	float		zNearViewmodel;
	// local Z coordinate of far plane of camera ( when rendering view model )
	float		zFarViewmodel;

	// The aspect ratio to use for computing the perspective projection matrix
	// (0.0f means use the viewport)
	float		m_flAspectRatio;
};
