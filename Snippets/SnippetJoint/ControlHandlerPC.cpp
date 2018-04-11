#include "ControlHandlerPC.h"
#include "RagDollMath.h"
void ControlHandlerPC::DirectionCheck()
{
	if (startRunDelay == -1.f)
	{
		startRunDelay = Random(0.5, 1.0);
	}

	if (startRunDelay > 0)
	{
		startRunDelay -= s->deltaTime;
		return;
	}
	

	if (directionCheckDelay < 6.f)
		directionCheckDelay += s->deltaTime;
	else 
	{
		directionCheckDelay = 0.f;
		PxVec3 p = PxVec3(0, 0, -2);
		p += PxVec3(Random(-1, 1), 0, Random(-1, 1));
		s->rawDirection = p - bodyHandler->chest->getGlobalPose().p;
		s->rawDirection.y = 0;
	}
	s->rawDirection = s->rawDirection.getNormalized();
	s->run = false;
}

void ControlHandlerPC::ResetVariables()
{
	s->leftArmOverride = false;
	s->rightArmOverride = false;
	s->leftLegOverride = false;
	s->rightLegOverride = false;
	s->jumpDelay = 1.f;
	s->leftGrab = false;
	s->rightGrab = false;
	s->leftPunch = false;
	s->rightPunch = false;
	s->lift = false;
	s->liftSelf = false;
	s->grabJump = false;
	s->duck = false;
	if (bodyHandler->leftGrabJoint != NULL)
	{
		bodyHandler->leftGrabJoint->release();
		bodyHandler->leftGrabJoint = NULL;
		bodyHandler->leftGrabRigidbody = NULL;
	}
	if (bodyHandler->rightGrabJoint != NULL)
	{
		bodyHandler->rightGrabJoint->release();
		bodyHandler->rightGrabJoint = NULL;
		bodyHandler->rightGrabRigidbody = NULL;
	}
}

void ControlHandlerPC::ReviveCheck()
{

}

void ControlHandlerPC::JumpCheck()
{
	if (s->jumpDelay > 0.f)
	{
		s->jumpDelay -= s->deltaTime;
	}
	else
	{
		s->jump = false;
	}
}

void ControlHandlerPC::LiftCheck()
{
}

void ControlHandlerPC::DuckCheck()
{
}

void ControlHandlerPC::PunchGrabCheck()
{
// 	if (s->grabDelay > 0.f)
// 	{
// 		s->grabDelay -= s->deltaTime;
// 	}
// 	if (Random(0, 1) > 0.999f)
// 	{
// 		s->leftPunch = true;
// 	}
// 	if (Random(0, 1) > 0.999f)
// 	{
// 		s->rightPunch = true;
// 	}
// 	
// 	LeftPunchGrabCheck();
// 	RightPunchGrabCheck();
}


void ControlHandlerPC::ClimbCheck()
{
	if (s->leftGrabJoint == NULL && s->leftCanClimb)
	{
		s->leftCanClimb = false;
	}

	if (s->rightGrabJoint == NULL && s->rightCanClimb)
	{
		s->rightCanClimb = false;
	}
}


void ControlHandlerPC::LeftPunchGrabCheck()
{
	if (s->armActionDelay > 0.f)
	{
		s->armActionDelay -= s->deltaTime;
	}
	if (s->leftPunch)
	{
		s->leftPunchTimer += s->deltaTime;
		if (s->leftPunchTimer < 0.2f)
		{
			movementHandler->ArmActionReadying(Side::Left);
			s->leftPunchGrabTarget = s->cUpperIntrest;
		}
		if ((s->leftPunchTimer >= 0.2f) && (s->leftPunchTimer < 0.3f))
		{
			movementHandler->ArmActionPunching(Side::Left, s->leftPunchGrabTarget);
		}
		if ((s->leftPunchTimer >= 0.3f) && (s->leftPunchTimer < 0.5f))
		{
			movementHandler->ArmActionPunchResetting(Side::Left);
		}
		if (s->leftPunchTimer >= 0.5f)
		{
			s->leftPunch = false;
			s->leftPunchTimer = 0.f;
		}
	}
	if (s->leftPunch || s->leftGrab)
	{
		s->leftArmOverride = true;
	}
	if (!s->leftPunch && !s->leftGrab)
	{
		s->leftArmOverride = false;
	}
}
void ControlHandlerPC::RightPunchGrabCheck()
{
	if (s->armActionDelay > 0.f)
	{
		s->armActionDelay -= s->deltaTime;
	}
	if (s->rightPunch)
	{
		s->rightPunchTimer += s->deltaTime;
		if (s->rightPunchTimer < 0.2f)
		{
			movementHandler->ArmActionReadying(Side::Right);
			s->rightPunchGrabTarget = s->cUpperIntrest;
		}
		if ((s->rightPunchTimer >= 0.2f) && (s->leftPunchTimer < 0.3f))
		{
			movementHandler->ArmActionPunching(Side::Right, s->rightPunchGrabTarget);
		}
		if ((s->rightPunchTimer >= 0.3f) && (s->leftPunchTimer < 0.5f))
		{
			movementHandler->ArmActionPunchResetting(Side::Right);
		}
		if (s->rightPunchTimer >= 0.5f)
		{
			s->rightPunch = false;
			s->rightPunchTimer = 0.f;
		}
	}
	if (s->rightPunch || s->rightGrab)
	{
		s->rightArmOverride = true;
	}
	if (!s->rightPunch && !s->rightGrab)
	{
		s->rightArmOverride = false;
	}
}