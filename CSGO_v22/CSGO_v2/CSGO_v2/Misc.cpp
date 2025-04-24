#include "Misc.h"
#include "localplayer.h"

// Constants for CS:GO movement
constexpr float AIR_ACCELERATION = 0.025f;
constexpr float GROUND_ACCELERATION = 0.15f;
constexpr float MAX_SPEED = 350.0f;
constexpr float FRICTION = 0.95f;
constexpr float EDGE_BUG_THRESHOLD = 0.1f;

void misc::BunnyHop(CUserCmd* cmd)
{
	if (!cfg.misc.movement.BunnyHop)	
		return;

	if (!LocalPlayer.Get())
		return;

	if ((cmd->buttons & CUserCmd::IN_JUMP)) {
		// ON AIR
		if (!(LocalPlayer->flags() & PlayerFlag_OnGround))
		{
			cmd->buttons &= ~CUserCmd::IN_JUMP;

			if (cfg.misc.movement.AirDuck)
				cmd->buttons |= CUserCmd::IN_DUCK;

			if (cfg.misc.movement.Strafe)
			{
				// Get current velocity
				auto velocity = LocalPlayer->getVelocity();
				float currentSpeed = velocity.length2D();
				
				// Calculate optimal strafe angle based on current speed
				float optimalAngle = 0.0f;
				if (currentSpeed > 0.0f) {
					optimalAngle = std::atan2(velocity.y, velocity.x);
				}

				// Check for edge bug
				bool isEdgeBug = false;
				if (LocalPlayer->flags() & PlayerFlag_OnGround) {
					auto origin = LocalPlayer->getAbsOrigin();
					auto nextOrigin = origin + velocity * 0.1f;
					if (std::abs(nextOrigin.z - origin.z) < EDGE_BUG_THRESHOLD) {
						isEdgeBug = true;
					}
				}

				// Get keyboard input
				bool movingForward = (cmd->buttons & CUserCmd::IN_FORWARD) != 0;
				bool movingBackward = (cmd->buttons & CUserCmd::IN_BACK) != 0;
				bool movingLeft = (cmd->buttons & CUserCmd::IN_MOVELEFT) != 0;
				bool movingRight = (cmd->buttons & CUserCmd::IN_MOVERIGHT) != 0;

				// Determine base direction
				float baseDirection = 0.0f;
				bool usingKeyboardDirection = false;

				// Check if any movement key is pressed
				if (movingForward || movingBackward || movingLeft || movingRight) {
					usingKeyboardDirection = true;
					// Set base direction based on keyboard input
					if (movingForward) {
						if (movingLeft) baseDirection = -45.0f;
						else if (movingRight) baseDirection = 45.0f;
						else baseDirection = 0.0f;
					}
					else if (movingBackward) {
						if (movingLeft) baseDirection = -135.0f;
						else if (movingRight) baseDirection = 135.0f;
						else baseDirection = 180.0f;
					}
					else {
						if (movingLeft) baseDirection = -90.0f;
						else if (movingRight) baseDirection = 90.0f;
					}
				}

				// Advanced strafing logic
				if (ABS(cmd->mousedx) >= 2) {
					// Mouse-based strafing with acceleration
					float targetYaw = cmd->viewangles.y;
					if (usingKeyboardDirection) {
						// Use keyboard direction as base
						targetYaw += baseDirection;
					} else {
						// Use mouse direction as base
						targetYaw += (cmd->mousedx < 0 ? -90.0f : 90.0f);
					}
					
					float currentYaw = std::atan2(velocity.y, velocity.x) * 180.0f / PI;
					float deltaYaw = targetYaw - currentYaw;
					
					// Apply acceleration based on speed
					float acceleration = isEdgeBug ? GROUND_ACCELERATION : AIR_ACCELERATION;
					float speedFactor = min(currentSpeed / MAX_SPEED, 1.0f);
					float moveSpeed = 450.0f * (1.0f - speedFactor * 0.5f);
					
					// Apply movement based on keyboard input
					if (usingKeyboardDirection) {
						cmd->forwardmove = (movingForward ? moveSpeed : (movingBackward ? -moveSpeed : 0.0f));
						cmd->sidemove = (movingRight ? moveSpeed : (movingLeft ? -moveSpeed : 0.0f));
					} else {
						cmd->sidemove = cmd->mousedx < 0 ? -moveSpeed : moveSpeed;
					}
					
					// Adjust view angles for optimal strafing
					cmd->viewangles.y += deltaYaw * acceleration;
				}
				else {
					// Automatic strafing with optimal angles
					static float lastYaw = 0.0f;
					static auto lastTick = cmd->tick_count;
					
					if (cmd->tick_count != lastTick) {
						float targetYaw = optimalAngle * 180.0f / PI;
						if (usingKeyboardDirection) {
							targetYaw += baseDirection;
						}
						float deltaYaw = targetYaw - lastYaw;
						
						// Apply acceleration based on speed
						float acceleration = isEdgeBug ? GROUND_ACCELERATION : AIR_ACCELERATION;
						float speedFactor = min(currentSpeed / MAX_SPEED, 1.0f);
						float moveSpeed = 450.0f * (1.0f - speedFactor * 0.5f);
						
						// Apply movement based on keyboard input
						if (usingKeyboardDirection) {
							cmd->forwardmove = (movingForward ? moveSpeed : (movingBackward ? -moveSpeed : 0.0f));
							cmd->sidemove = (movingRight ? moveSpeed : (movingLeft ? -moveSpeed : 0.0f));
						} else {
							// Use automatic strafing when no keyboard input
							static bool direction = false;
							if (direction) {
								cmd->viewangles.y += deltaYaw * acceleration;
								cmd->sidemove = moveSpeed;
							}
							else {
								cmd->viewangles.y -= deltaYaw * acceleration;
								cmd->sidemove = -moveSpeed;
							}
							direction = !direction;
						}
						
						lastYaw = cmd->viewangles.y;
					}
					
					lastTick = cmd->tick_count;
				}

				// Normalize view angles
				cmd->viewangles.normalize();
			}
		}
	}
}
