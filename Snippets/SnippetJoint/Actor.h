#pragma once
#include "MovementHandler.h"
#include "ControlHandler.h"
#include "ControlHandlerPC.h"
#include "BodyHandler.h"
#include "StateHandler.h" 
#include "PxHandler.h"
#include <functional>
class Actor
{
public:
	Actor(PxVec3 pos = PxVec3(0), bool bIsPC = false)
	{
		mPxHandler = gPxHandler;
		mBodyHandler = new BodyHandler(pos);
		mBodyHandler->SetActor(this);
		mStateHandler = new StateHandler();
		mMovementHandler = new MovementHandler(mBodyHandler,mStateHandler);
		mIsPC = bIsPC;
		if (bIsPC)
		{
			mControlHandler = new ControlHandlerPC(mBodyHandler, mMovementHandler, mStateHandler);
		}
			
		else
			mControlHandler = new ControlHandler(mBodyHandler,mMovementHandler,mStateHandler);
	}


public:
	void Init();
	void Update();
	// 与建筑物建立碰撞相关的联系
	void AddObstacle(PxRigidDynamic * p);
	void OnCollisionEnter(const PxContactPairHeader& pairHeader, const PxContactPair& cp);
	void OnCollisionStay(const PxContactPairHeader& pairHeader, const PxContactPair& cp);
	void OnCollisionExit(const PxContactPairHeader& pairHeader, const PxContactPair& cp);
private:
	void OnCollision(const PxActor* actor, const PxContactPairHeader& pairHeader, const PxContactPair& cp, std::function<void(PxActor*)> f0 = NULL, std::function<bool(PxVec3)> f1 = NULL);

public:
	MovementHandler* mMovementHandler;
	ControlHandler* mControlHandler;
	BodyHandler* mBodyHandler;
	StateHandler* mStateHandler;
	PxHandler* mPxHandler;

public:
	bool mIsPC = false;
};
