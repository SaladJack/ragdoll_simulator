#pragma once
#include "ControlHandler.h"
class ControlHandlerPC : public ControlHandler
{
public:
	ControlHandlerPC(BodyHandler* pBh, MovementHandler* pMh, StateHandler* pSh) :ControlHandler(pBh, pMh, pSh) {}
public:
// 	virtual void GetInput();
	virtual void DirectionCheck();
//	virtual void GroundCheck();
	virtual void LiftCheck();
//	virtual void IdleCheck();
	virtual void ClimbCheck();
//	virtual void RunCheck();
 	virtual void JumpCheck();
 	virtual void DuckCheck();
 	virtual void PunchGrabCheck();
 	virtual void LeftPunchGrabCheck();
	virtual void RightPunchGrabCheck();
	virtual void ResetVariables();
	virtual void ReviveCheck();
// 	virtual void FallCheck();
// 	virtual void GrabCheck(Side side, PxTransform collisionTransform, PxRigidBody* collisionRigidbody);
private:
	float directionCheckDelay = 0.f;
	float startRunDelay = -1.f;
};
