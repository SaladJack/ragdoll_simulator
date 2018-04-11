#pragma once
#include "BodyHandler.h"
#include "StateHandler.h"
#include "CommonPragma.h"

class MovementHandler
{
public:
	MovementHandler(BodyHandler* pBh, StateHandler* pBs)
	{
		bodyHandler = pBh;
		s = pBs;
	}

public:
	void Stand();
	void Jump();
	void Run();
	void Fall();
	void Climb();
	void Unconsious();
	void RunCycleUpdate();
	void RunCycleRotateball();
	void RunCyclePoseBody();
	void RunCyclePoseArm(Side side, Pose pose);
	void RunCyclePoseLeg(Side side, Pose pose);
	void ArmActionReadying(Side side);
	void ArmActionPunching(Side side, PxRigidBody* punchTarget);
	void ArmActionPunchResetting(Side side);
	void ArmActionGrabbing(Side side, PxRigidBody* grabTarget);
	void ArmActionCheering();
	void ResetLeftGrab();
	void ResetRightGrab();
	void HeadActionReadying();
	void HeadActionHeadbutting();
	void HeadActionResetting();
private:
	BodyHandler* bodyHandler;
	StateHandler* s;
	
private:
	Pose leftLegPose = Pose::Straight;
	Pose rightLegPose;
	Pose leftArmPose;
	Pose rightArmPose = Pose::Straight;

};
//#define gMovementHandeler MovementHandler::GetInstance()
