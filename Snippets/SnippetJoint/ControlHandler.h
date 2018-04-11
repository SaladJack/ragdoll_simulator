#pragma once
#include "BodyHandler.h"
#include "MovementHandler.h"
#include "StateHandler.h"

#include "CommonEnum.h"
#include "CommonPragma.h"


class ControlHandler
{	
public:
	ControlHandler(BodyHandler* pBh, MovementHandler* pMh, StateHandler* pSh)
	{
		bodyHandler = pBh;
		movementHandler = pMh;
		s = pSh;
		SetPxHandler(gPxHandler);
	}

	virtual ~ControlHandler()
	{
		bodyHandler = NULL;
		movementHandler = NULL;
		s = NULL;
	}

public:
	virtual void GetInput();
	virtual void DirectionCheck();
	virtual void GroundCheck();
	virtual void LiftCheck();
	virtual void IdleCheck();
	virtual void ClimbCheck();
	virtual void RunCheck();
	virtual void JumpCheck();
	virtual void DuckCheck();
	virtual void PunchGrabCheck();
	virtual void LeftPunchGrabCheck();
	virtual void RightPunchGrabCheck();
	virtual void FallCheck();
	virtual void ResetVariables();
	virtual void ReviveCheck();

	virtual void GrabCheck(Side side, PxTransform collisionTransform, PxRigidBody* collisionRigidbody);

protected:
	BodyHandler* bodyHandler;
	MovementHandler* movementHandler;
	StateHandler* s;

public:
	void SetPxPhysics(PxPhysics* p) { gPhysics = p; }
	void SetPxScene(PxScene* p) { gScene = p; }
	void SetPxHandler(PxHandler* p)
	{
		mPxHandler = p;
		gPhysics = p->mPhysics;
		gScene = p->mScene;
	}

private:
	PxPhysics*				gPhysics = NULL;
	PxScene*				gScene = NULL;
	PxHandler*				mPxHandler = NULL;
};
//#define gControlHandeler ControlHandler::GetInstance()
