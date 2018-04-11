#pragma once
#include "BasicPxVec.h"
#include "CommonEnum.h"
#include "CommonPragma.h"
class StateHandler
{
private:

public:
	static StateHandler* GetInstance()
	{
		static StateHandler* m_pInstance;
		if (m_pInstance == NULL)
			m_pInstance = new StateHandler();
		return m_pInstance;
	}
public:
	StateHandler()
	{
	}

public:

	ActorState actorState = ActorState::Stand;
	ActorState lastActorState;


	float totalTime = 0.0f;

	float applyedForce = 1.0f;
	//float applyedForce = 0.050f;
	float inputSpamForceModifier = 1.f;
	float stamina = 0.f;
	float staminaDamage = 0.f;
	float cycleSpeed = 0.23f;
	bool stateChange = true;
	float cycleTimer = 0.f;
	float runForce = 0.f;
	float data_scale = 0.5;
	const PxVec3 runVectorForce10 = PxVec3(0.0f, 9.0f, 0.0f);

	const PxVec3 runVectorForce5 = PxVec3(0.0f, 8.0f, 0.0f);

	const PxVec3 runVectorForce2 = PxVec3(0.0f, 4.0f, 0.0f);

	const PxVec3 counterForce = PxVec3(0.0f, 1.0f, 0.0f);

	PxVec3 direction = PxForward;

	PxVec3 rawDirection = direction;

	bool onGround = true;

	float offGroundDelay = 0.2f;

	float groundCheckDelay = 0.f;

	bool onMovingPlatform = false;

	bool leftCanClimb = false;

	bool rightCanClimb = false;

	bool inWater = false;

	PxVec3 velocity = PxVec3(0);

	float smoothTime = 0.2f;

	PxVec3 lookDirection = PxForward;

	PxVec3 moveDirection = PxForward;

	PxVec3 lockedDirection;

	bool leftArmOverride = false;

	bool rightArmOverride = false;

	bool leftLegOverride = false;

	bool rightLegOverride = false;

	float jumpDelay = 0.f;

	bool leftGrab = false;

	bool rightGrab = false;

	bool leftPunch = false;

	bool rightPunch = false;

	bool lift = false;

	bool liftSelf = false;

	bool jump = false;

	bool grabJump = false;

	float grabJumpCounter = false;

	bool duck = false;

	bool leftKick = false;

	bool rightKick = false;

	bool kickDuck = false;

	float grabDelay = 0.f;

	float reviveDelay = 0.f;

	float idleTimer = 0.f;

	bool idle = false;

	float liftTimer = 0.f;

	bool run = true;

	float runTimer = 0.f;

	float jumpTimer = 0.f;

	float fallTimer = 0.f;

	PxVec3 slideDirection = PxVec3(0);

	bool canJump = true;

	bool canDive = true;

	bool headbutt = false;

	float headbuttTimer = 0.f;

	bool ForceUpdate = false;

	float armActionDelay = 0.f;

	float punchTimer = 0.f;

	float leftArmActionTimer = 0.f;
	float rightArmActionTimer = 0.f;

	float leftPunchTimer = 0.f;
	float rightPunchTimer = 0.f;

	bool isBehindUpper = false;

	float airSpeed = 0.25f;

	float headbuttDelay = 0.f;
	float duckTimer = 0.f;

	PxRigidBody* leftPunchGrabTarget = NULL;
	PxRigidBody* rightPunchGrabTarget = NULL;

	PxRigidBody* cUpperIntrest = NULL;

	PxD6Joint* leftGrabJoint = NULL;
	PxD6Joint* rightGrabJoint = NULL;

	const float deltaTime = 1.f / 60.f;
	const float simulateTime = 1.f / 60.f;

	//////////////////////////////
	bool bBallOnGround = false;
	bool bLeftFootOnGround = false;
	bool bRightFootOnGround = false;
	bool bJumpKeyDown = false;
	bool bLeftPunchKeyDown = false;
	bool bRightPunchKeyDown = false;
	bool bDuckKeyDown = false;
	//////////////////////////////
};
//#define gStateHandler StateHandler::GetInstance()