#include "ControlHandler.h"
#include <string>
#define KEYDOWN(vk_code) (::GetFocus() && (::GetAsyncKeyState(vk_code) && 0x8000) ? 1 : 0)
#define KEYUP(vk_code) (::GetFocus() && (::GetAsyncKeyState(vk_code) && 0x8000) ? 0 : 1)
#include "InputCfg.h"
#include <ctime>
#include "RagDollMath.h"
void ControlHandler::GetInput()
{

	DirectionCheck();
	if (s->actorState != ActorState::Dead && s->actorState != ActorState::Unconscious)
	{
		GroundCheck();
		PunchGrabCheck();
		DuckCheck();
		LiftCheck();
		FallCheck();
		IdleCheck();
		if (s->onGround)
		{
			RunCheck();
			JumpCheck();
		}
		else if (s->leftCanClimb || s->rightCanClimb)
		{
			ClimbCheck();
		}
		else
		{
			JumpCheck();
		}
	}
	else
	{
		ResetVariables();
		ReviveCheck();
	}
	if (s->actorState != s->lastActorState)
	{
		s->stateChange = true;
	}
	else
	{
		s->stateChange = false;
	}

	switch (s->actorState)
	{
	case ActorState::Stand:
		movementHandler->Stand();
		break;
	case ActorState::Run:
		movementHandler->Run();
		break;
	case ActorState::Jump:
		movementHandler->Jump();
		break;
	case ActorState::Climb:
		movementHandler->Climb();
		break;
	case ActorState::Fall:
		movementHandler->Fall();
		break;
	case ActorState::Unconscious:
		movementHandler->Unconsious();
		break;
	default:
		break;
	}
	s->lastActorState = s->actorState;
// 	if (s->actorState == ActorState::Jump)
// 	{
// 		s->applyedForce = 0.5f;
// 	}
// 	else
// 	{
// 		s->applyedForce = PxClamp((float)(s->applyedForce + (s->deltaTime / 2.f)), (float) 0.01f, (float)1.f);
// 		s->inputSpamForceModifier = PxClamp((float)(s->inputSpamForceModifier + (s->deltaTime / 2.f)), (float) 0.01f, (float)1.f);
// 	}

	if (((s->actorState == ActorState::Dead) || (s->actorState == ActorState::Unconscious)) || (s->actorState == ActorState::Fall))
	{
		s->applyedForce = 0.1f;
	}
	else
	{
		if (s->applyedForce < 1.f)
		{
			s->applyedForce += s->deltaTime / 2.f;
		}
		if (s->applyedForce > 1.f)
		{
			s->applyedForce = 1.f;
		}
	}
}

void ControlHandler::DirectionCheck()
{

	PxVec3 a = PxVec3(0);
	PxVec3 b = PxVec3(0);

	if (KEYDOWN(UP))
	{
		a = -PxForward;
	}
	if (KEYDOWN(LEFT))
	{
		b = -PxRight;
	}
	if (KEYDOWN(DOWN))
	{
		a = PxForward;
	}
	if (KEYDOWN(RIGHT))
	{
		b = PxRight;
	}

	s->rawDirection = (a + b).getNormalized();
	if (s->rawDirection != PxVec3(0))
	{
		s->direction = s->rawDirection;
	}
}

void ControlHandler::ClimbCheck()
{
	if ((bodyHandler->leftGrabJoint == NULL) && s->leftCanClimb)
	{
		s->leftCanClimb = false;
	}
	if ((bodyHandler->rightGrabJoint == NULL) && s->rightCanClimb)
	{
		s->rightCanClimb = false;
	}
	if (((bodyHandler->leftGrabJoint != NULL) && s->leftCanClimb) || ((bodyHandler->rightGrabJoint != NULL) && s->rightCanClimb))
	{
		
		if (KEYDOWN(JUMP))
		{
			s->liftSelf = true;
		}
		else
		{
			s->liftSelf = false;
		}

		s->actorState = ActorState::Climb;

		/*
		bool enableDoubleClick = false;
		if (KEYDOWN(JUMP))
		{
			s->bJumpKeyDown = true;
		}

		if (s->bJumpKeyDown && KEYUP(JUMP))
		{
			s->bJumpKeyDown = false;
			time_t t = jumpLastClickTime;
			jumpLastClickTime = std::time(0);
			enableDoubleClick = jumpLastClickTime - t <= 1;
			//printf("jumpLastClickTime:%I64d , t:%I64d\n", jumpLastClickTime, t);
			s->liftSelf = true;
		}
		else
		{
			s->liftSelf = false;
		}
		
		//if (s->bJumpKeyDown && KEYUP(JUMP))// double jump
		if(enableDoubleClick)
		{
			enableDoubleClick = false;
			printf("double click\n");
			s->bJumpKeyDown = false;
			s->grabDelay = 0.1f;
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
			s->grabJump = true;
			s->actorState = ActorState::Jump;
		}
		else
		{
			s->actorState = ActorState::Climb;
		}
		
		*/
	}
}

void ControlHandler::IdleCheck()
{
	//if (((base.actor.RewiredPlayer.GetButton("Duck") || base.actor.RewiredPlayer.GetButton("Lift")) || (base.actor.RewiredPlayer.GetButton("Kick") || base.actor.RewiredPlayer.GetButton("Jump"))) || (((base.actor.RewiredPlayer.GetButton("Left Punch / Grab") || base.actor.RewiredPlayer.GetButton("Right Punch / Grab")) || (base.actor.RewiredPlayer.GetButton("Left Punch / Grab") || (base.actor.RewiredPlayer.GetAxis("Move Horizontal") != 0f))) || ((base.actor.RewiredPlayer.GetAxis("Move Vertical") != 0f) || (base.actor.actorState != ActorState::Stand))))
	if ((KEYDOWN(JUMP)) ||  KEYDOWN(LEFTPUNCH) ||  KEYDOWN(RIGHTPUNCH) || KEYDOWN(LEFTPUNCH) || KEYDOWN(LEFT)|| KEYDOWN(RIGHT)|| KEYDOWN(UP) || KEYDOWN(DOWN) || (s->actorState != ActorState::Stand))
	{
		if (s->idle)
		{
			s->idleTimer = Random((float) 0.f, (float)4.f);
			s->idleTimer = 0.f;
		}
		s->idle = false;
	}
	else
	{
		if (s->idleTimer < 30.f)
		{
			s->idleTimer = PxClamp((float)(s->idleTimer + deltaTime), (float)-60.f, (float) 30.f);
		}
		if ((s->idleTimer >= 10.f) && (s->idleTimer < 20.f))
		{
			s->idle = true;
// 			if (UnityEngine.Random.Range(1, 400) == 1)
// 			{
// 				s->lookDirection = new Vector3(UnityEngine.Random.Range((float)-1f, (float)1f), UnityEngine.Random.Range((float)-0.2f, (float)1f), UnityEngine.Random.Range((float)-1f, (float)1f));
// 			}
		}
		if (s->idleTimer >= 20.f)
		{
			s->run = true;
			s->idle = true;
// 			if (UnityEngine.Random.Range(1, 400) == 1)
// 			{
// 				s->lookDirection = new Vector3(UnityEngine.Random.Range((float)-1f, (float)1f), UnityEngine.Random.Range((float)-0.2f, (float)1f), UnityEngine.Random.Range((float)-1f, (float)1f));
// 			}
		}
	}
}

void ControlHandler::LiftCheck()
{
	if (KEYDOWN(LIFT))
	{
		s->lift = true;
	}
	else
	{
		s->lift = false;
	}
// 	if ((((KEYDOWN(LIFT) && onGround) && leftGrab) || ((KEYDOWN(LIFT) && onGround) && rightGrab)) && (UnityEngine.Random.Range((float)0f, (float)4f) <= 1f))
// 	{
// 		// actor.effectsHandeler.Vert();
// 		// VoiceTag.SetVoiceTag(actor.effectsHandeler.gameObject, VoiceTag.VoiceType.VoiceVert);
// 		// actor.effectsHandeler.Invoke("ResetVoiceBoxTag", 0.1f);
// 	}
	if ((s->duck && s->lift) && ((s->actorState == ActorState::Stand) || (s->actorState == ActorState::Run)))
	{
		s->jumpDelay = 0.5f;
		s->groundCheckDelay = 0.1f;
		s->actorState = ActorState::Jump;
	}
	if (((!s->leftPunch && !s->leftGrab) && (!s->rightPunch && !s->rightGrab)) && s->lift)
	{
		movementHandler->ArmActionCheering();
	}
}

void ControlHandler::RunCheck()
{
	if (s->applyedForce > 0.5f)
	{
		if (s->rawDirection.x != 0.f || s->rawDirection.z != 0.f)
		{
			s->lookDirection = s->direction = s->rawDirection;
			s->actorState = ActorState::Run;
		}
		else
		{
			s->actorState = ActorState::Stand;
		}
	}
	else
	{
		s->actorState = ActorState::Stand;
	}
}

void ControlHandler::JumpCheck()
{
	if (s->jumpDelay > 0.f)
	{
		s->jumpDelay -= s->deltaTime;
	}
	if (KEYDOWN(JUMP))
	{
		s->bJumpKeyDown = true;
		s->jumpTimer = 0.f;
	}
	if ((s->stamina <= 10.f) && !s->idle)
	{
		s->run = false;
	}
	if ((!s->duck && !s->kickDuck) && KEYDOWN(JUMP))
	{	
		if ((s->jumpTimer > 0.4f) && (s->stamina >= 10.f))
		{
			s->run = true;
			s->runTimer = 1.f;
			s->slideDirection = s->direction;
		}
		s->jumpTimer += s->deltaTime;
	}
	else if (s->runTimer >= 0.f)
	{
		s->runTimer -= s->deltaTime;
	}
	else
	{
		s->runTimer = 0.f;
		if (!s->idle)
		{
			s->run = false;
		}
	}
	if (s->bJumpKeyDown && KEYUP(JUMP))
	{
		s->bJumpKeyDown = false;
		if (s->jumpTimer <= 0.8f)
		{
			s->jump = true;
			if (((s->jumpDelay <= 0.f) && (s->actorState != ActorState::Jump)) && (s->actorState != ActorState::Fall))
			{
				s->fallTimer -= 0.4f;
				s->jumpDelay = 0.8f;
				s->groundCheckDelay = 0.1f;
				s->actorState = ActorState::Jump;
			}
		}
		s->canJump = true;
	}
	else
	{
		s->jump = false;
	}
// 	if (((GetButton("Jump") && GetButton("Duck")) || (GetButton("Jump") && GetButton("Kick"))) && (s->canJump && (s->jumpTimer <= 0.4f)))
// 	{
// 		s->jump = true;
// 		if (((s->jumpDelay <= 0.f) && (s->actorState != ActorState::Jump)) && (s->actorState != ActorState::Fall))
// 		{
// 			s->jumpDelay = 0.4f;
// 			s->groundCheckDelay = 0.1f;
// 			s->canJump = false;
// 			s->actorState = ActorState::Jump;
// 		}
// 	}
}

void ControlHandler::DuckCheck()
{
	if (KEYDOWN(DUCK))
	{
		s->bDuckKeyDown = true;
		s->duckTimer += s->deltaTime;
		if (((s->actorState == ActorState::Jump) || (s->actorState == ActorState::Fall)) || (s->actorState == ActorState::Climb))
		{
			if (s->duckTimer >= 0.2f)
			{
				if (!s->duck)
				{
					s->headbutt = true;
				}
				s->duck = true;
			}
		}
		else if (s->duckTimer >= 0.2f)
		{
			s->duck = true;
		}
	}
	if (s->bDuckKeyDown && KEYUP(DUCK))
	{
		s->bDuckKeyDown = false;	 
		s->duck = false;
		if ((s->headbuttDelay <= 0.f) && (s->duckTimer < 0.5f))
		{
			s->headbutt = true;
			s->headbuttDelay = 1.f;
		}
		s->duckTimer = 0.f;
	}
	if (s->headbuttDelay >= 0.f)
	{
		s->headbuttDelay -= s->deltaTime;
	}
	if (s->headbutt)
	{
		s->headbuttTimer += s->deltaTime;
		if (s->headbuttTimer <= 0.2f)
		{
			if (!s->duck)
			{
				movementHandler->HeadActionReadying();
			}
			else
			{
				movementHandler->HeadActionHeadbutting();
			}
		}
		else if ((s->headbuttTimer > 0.2f) && (s->headbuttTimer <= 0.4f))
		{
			if (!s->duck)
			{
				movementHandler->HeadActionHeadbutting();
			}
		}
		else
		{
			movementHandler->HeadActionResetting();
			s->headbutt = false;
			s->headbuttTimer = 0.f;
		}
	}
// 	if ((s->duck && (actorState == ActorState::Jump)) || (s->duck && (actorState == ActorState::Fall)))
// 	{
// 		bodyHandeler.tPart[BodyHandeler.Part.Head].tag = "Body (Harmful)";
// 		IEnumerator enumerator = bodyHandeler.tPart[BodyHandeler.Part.Head].GetComponentInChildren<Transform>().GetEnumerator();
// 		try
// 		{
// 			while (enumerator.MoveNext())
// 			{
// 				Transform current = (Transform)enumerator.Current;
// 				if (current.name != "helper_voiceBox")
// 				{
// 					current.tag = "Body (Harmful)";
// 				}
// 			}
// 		}
// 		finally
// 		{
// 			IDisposable disposable = enumerator as IDisposable;
// 			if (disposable == null)
// 			{
// 			}
// 			disposable.Dispose();
// 		}
// 	}
// 	else if (!controlHandeler.s->headbutt)
// 	{
// 		bodyHandeler.tPart[BodyHandeler.Part.Head].tag = "Body (High Priority)";
// 		IEnumerator enumerator2 = bodyHandeler.tPart[BodyHandeler.Part.Head].GetComponentInChildren<Transform>().GetEnumerator();
// 		try
// 		{
// 			while (enumerator2.MoveNext())
// 			{
// 				Transform transform2 = (Transform)enumerator2.Current;
// 				if (transform2.name != "helper_voiceBox")
// 				{
// 					transform2.tag = "Untagged";
// 				}
// 			}
// 		}
// 		finally
// 		{
// 			IDisposable disposable2 = enumerator2 as IDisposable;
// 			if (disposable2 == null)
// 			{
// 			}
// 			disposable2.Dispose();
// 		}
// 	}
// 	if ((kickDuck && (actorState == ActorState::Jump)) || (kickDuck && (actorState == ActorState::Fall)))
// 	{
// 		bodyHandeler.tPart[BodyHandeler.Part.Hips].tag = "Body (Harmful)";
// 		bodyHandeler.tPart[BodyHandeler.Part.LeftLeg].tag = "Body (Harmful)";
// 		bodyHandeler.tPart[BodyHandeler.Part.LeftThigh].tag = "Body (Harmful)";
// 		bodyHandeler.tPart[BodyHandeler.Part.RightLeg].tag = "Body (Harmful)";
// 		bodyHandeler.tPart[BodyHandeler.Part.RightThigh].tag = "Body (Harmful)";
// 	}
// 	else if (!controlHandeler.kickDuck)
// 	{
// 		bodyHandeler.tPart[BodyHandeler.Part.Hips].tag = "Body (Medium Priority)";
// 		bodyHandeler.tPart[BodyHandeler.Part.LeftLeg].tag = "Body (Low Priority)";
// 		bodyHandeler.tPart[BodyHandeler.Part.LeftThigh].tag = "Body (Low Priority)";
// 		bodyHandeler.tPart[BodyHandeler.Part.RightLeg].tag = "Body (Low Priority)";
// 		bodyHandeler.tPart[BodyHandeler.Part.RightThigh].tag = "Body (Low Priority)";
// 	}
}

void ControlHandler::GroundCheck()
{
	if (s->bBallOnGround || s->bLeftFootOnGround || s->bRightFootOnGround)
	{
		if (s->groundCheckDelay <= 0.f)
		{
			s->onGround = true;
		}
		else if (s->groundCheckDelay > 0.f)
		{
			s->onGround = false;
			s->groundCheckDelay -= s->deltaTime;
		}
	}
	else
	{
		s->onGround = false;
	}
}

void ControlHandler::FallCheck()
{
	if (!s->onGround)
	{
		if (s->fallTimer < 1.5f)
		{
			s->fallTimer += s->deltaTime;
		}
		if (s->leftCanClimb || s->rightCanClimb)
		{
			s->fallTimer = 0.f;
		}
		if (s->actorState != ActorState::Fall)
		{
			if ((s->fallTimer > 0.2f) && (s->fallTimer < 1.5f))
			{
				s->actorState = ActorState::Jump;
			}
			if (s->fallTimer >= 1.5f)
			{
				s->actorState = ActorState::Fall;
			}
		}
// 		s->rightGrab = true;
// 		s->leftGrab = true;
		if (bodyHandler->leftGrabJoint == NULL)
		{
			s->leftPunchGrabTarget = s->cUpperIntrest;
		}
		if (bodyHandler->rightGrabJoint == NULL)
		{
			s->rightPunchGrabTarget = s->cUpperIntrest;
		}
		movementHandler->ArmActionGrabbing(Side::Left, s->leftPunchGrabTarget);
		movementHandler->ArmActionGrabbing(Side::Right, s->rightPunchGrabTarget);
	}
	else
	{
		s->fallTimer = 0.f;
	}
}

void ControlHandler::ResetVariables()
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
	s->kickDuck = false;
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

void ControlHandler::ReviveCheck()
{
// 	if (((base.actor.RewiredPlayer.GetButtonDown("Jump") || base.actor.RewiredPlayer.GetButtonDown("Duck")) || (base.actor.RewiredPlayer.GetButtonDown("Kick") || base.actor.RewiredPlayer.GetButtonDown("Lift"))) || (base.actor.RewiredPlayer.GetButtonDown("Left Punch / Grab") || base.actor.RewiredPlayer.GetButtonDown("Right Punch / Grab")))
// 	{
// 		base.actor.statusHandeler.healthDamage -= 5f;
// 	}
}



void ControlHandler::PunchGrabCheck()
{
	if (s->grabDelay > 0.f)
	{
		s->grabDelay -= s->deltaTime;
	}
	LeftPunchGrabCheck();
	RightPunchGrabCheck();
}


void ControlHandler::LeftPunchGrabCheck()
{
	std::string str = "LeftPunchGragCheck";
	if (s->armActionDelay > 0.f)
	{
		s->armActionDelay -= s->deltaTime;
	}
	// 	if (actor.RewiredPlayer.KEYDOWN("Left Punch / Grab"))
	if (KEYDOWN(LEFTPUNCH))
	{
		s->bLeftPunchKeyDown = true;
		if (s->leftArmActionTimer > 0.2f)
		{
			str += "-1";
			s->leftGrab = true;
			if (bodyHandler->leftGrabJoint == NULL)
			{
				s->leftPunchGrabTarget = s->cUpperIntrest;
			}
			movementHandler->ArmActionGrabbing(Side::Left, s->leftPunchGrabTarget);
		}
		else
		{
			str += "-2";
			movementHandler->ArmActionReadying(Side::Left);
			s->leftPunchGrabTarget = s->cUpperIntrest;
		}
		if (s->leftArmActionTimer < 1.f)
		{
			str += "-3";
			s->leftArmActionTimer += s->deltaTime;
		}
	}
	//	if (actor.RewiredPlayer.KEYDOWNUp("Left Punch / Grab"))
	if (s->bLeftPunchKeyDown && KEYUP(LEFTPUNCH))
	{
		s->bLeftPunchKeyDown = false;
		if ((!s->leftPunch && (s->armActionDelay <= 0.f)) && (s->leftArmActionTimer <= 0.2f))
		{
			str += "-4";
			s->leftPunch = true;
			s->armActionDelay = 0.2f;
		}
		s->leftGrab = false;
		if (bodyHandler->leftGrabJoint != NULL)
		{
			str += "-5";
			bodyHandler->leftGrabJoint->release();
			bodyHandler->leftGrabJoint = NULL;
			bodyHandler->leftGrabRigidbody = NULL;
		}
		s->leftArmActionTimer = 0.f;
	}
	if (s->leftPunch)
	{
		s->leftPunchTimer += s->deltaTime;
		if (s->leftPunchTimer < 0.1f)
		{
			str += "-6";
			movementHandler->ArmActionReadying(Side::Left);
			s->leftPunchGrabTarget = s->cUpperIntrest;
		}
		if ((s->leftPunchTimer >= 0.2f) && (s->leftPunchTimer < 0.25f))
		{
			str += "-7";
		}
		if ((s->leftPunchTimer >= 0.2f) && (s->leftPunchTimer < 0.3f))
		{
			if (s->isBehindUpper)
			{
				str += "-8";
				//ArmActionElbowing(Side::Left, s->leftPunchGrabTarget);
			}
			else
			{
				str += "-9";
				movementHandler->ArmActionPunching(Side::Left, s->leftPunchGrabTarget);
			}
		}
		if ((s->leftPunchTimer >= 0.3f) && (s->leftPunchTimer < 0.5f))
		{
			str += "-10";
			movementHandler->ArmActionPunchResetting(Side::Left);
		}
		if (s->leftPunchTimer >= 0.5f)
		{
			str += "-11";
			s->leftPunch = false;
			s->leftPunchTimer = 0.f;
			s->grabDelay = 0.2f;
		}
	}
	if (s->leftPunch || s->leftGrab)
	{
		str += "-12";
		s->leftArmOverride = true;
	}
	if (!s->leftPunch && !s->leftGrab)
	{
		str += "-13";
		s->leftArmOverride = false;
	}
	else
	{
		//printf("%s\n", str.c_str());
	}
}

void ControlHandler::RightPunchGrabCheck()
{
	std::string str = "RightPunchGragCheck";
	if (s->armActionDelay > 0.f)
	{
		s->armActionDelay -= s->deltaTime;
	}
	// 	if (actor.RewiredPlayer.KEYDOWN("Left Punch / Grab"))
	if (KEYDOWN(RIGHTPUNCH))
	{
		s->bRightPunchKeyDown = true;
		if (s->rightArmActionTimer > 0.2f)
		{
			str += "-1";
			s->rightGrab = true;
			if (bodyHandler->rightGrabJoint == NULL)
			{
				s->rightPunchGrabTarget = s->cUpperIntrest;
			}
			movementHandler->ArmActionGrabbing(Side::Right, s->rightPunchGrabTarget);
		}
		else
		{
			str += "-2";
			movementHandler->ArmActionReadying(Side::Right);
			s->rightPunchGrabTarget = s->cUpperIntrest;
		}
		if (s->rightArmActionTimer < 1.f)
		{
			str += "-3";
			s->rightArmActionTimer += s->deltaTime;
		}
	}
	if (s->bRightPunchKeyDown && KEYUP(RIGHTPUNCH))
	{
		s->bRightPunchKeyDown = false;
		if ((!s->rightPunch && (s->armActionDelay <= 0.f)) && (s->rightArmActionTimer <= 0.2f))
		{
			str += "-4";
			s->rightPunch = true;
			s->armActionDelay = 0.2f;
		}
		s->rightGrab = false;
		if (bodyHandler->rightGrabJoint != NULL)
		{
			str += "-5";
			bodyHandler->rightGrabJoint->release();
			bodyHandler->rightGrabJoint = NULL;
			bodyHandler->rightGrabRigidbody = NULL;
		}
		s->rightArmActionTimer = 0.f;
	}
	if (s->rightPunch)
	{
		s->rightPunchTimer += s->deltaTime;
		if (s->rightPunchTimer < 0.1f)
		{
			str += "-6";
			movementHandler->ArmActionReadying(Side::Right);
			s->rightPunchGrabTarget = s->cUpperIntrest;
		}
		if ((s->rightPunchTimer >= 0.2f) && (s->rightPunchTimer < 0.25f))
		{
			str += "-7";
		}
		if ((s->rightPunchTimer >= 0.2f) && (s->rightPunchTimer < 0.3f))
		{
			if (s->isBehindUpper)
			{
				str += "-8";
				//ArmActionElbowing(Side::Left, s->leftPunchGrabTarget);
			}
			else
			{
				str += "-9";
				movementHandler->ArmActionPunching(Side::Right, s->rightPunchGrabTarget);
			}
		}
		if ((s->rightPunchTimer >= 0.3f) && (s->rightPunchTimer < 0.5f))
		{
			str += "-10";
			movementHandler->ArmActionPunchResetting(Side::Right);
		}
		if (s->rightPunchTimer >= 0.5f)
		{
			str += "-11";
			s->rightPunch = false;
			s->rightPunchTimer = 0.f;
			s->grabDelay = 0.2f;
		}
	}
	if (s->rightPunch || s->rightGrab)
	{
		str += "-12";
		s->rightArmOverride = true;
	}
	if (!s->rightPunch && !s->rightGrab)
	{
		str += "-13";
		s->rightArmOverride = false;
	}
}

void ControlHandler::GrabCheck(Side side, PxTransform collisionTransform, PxRigidBody * collisionRigidbody)
{
	if (side == Side::Left)
	{
		if (s->leftGrab && s->grabDelay <= 0.f && bodyHandler->leftGrabJoint == NULL)
		{
			if (collisionRigidbody != NULL)
			{
				bodyHandler->leftGrabJoint = gPxHandler->createD6JointAutomatically(bodyHandler->leftHand, PxTransform::createIdentity(), collisionRigidbody);
			}
			else
			{
				PxTransform t = bodyHandler->leftHand->getGlobalPose() * PxTransform::createIdentity();
				bodyHandler->leftGrabJoint = PxD6JointCreate(*gPhysics, bodyHandler->leftHand, PxTransform::createIdentity(), NULL, t);
			}
			bodyHandler->leftGrabRigidbody = collisionRigidbody;
			s->leftCanClimb = true;
			gPxHandler->setJointMotion(bodyHandler->leftGrabJoint, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
			bodyHandler->leftGrabJoint->setBreakForce(30000.f, 30000.f);
		}
	}
	else
	{
		if (s->rightGrab && s->grabDelay <= 0.f && bodyHandler->rightGrabJoint == NULL)
		{
			if (collisionRigidbody != NULL)
			{
				bodyHandler->rightGrabJoint = gPxHandler->createD6JointAutomatically(bodyHandler->rightHand, PxTransform::createIdentity(), collisionRigidbody);
			}
			else
			{
				PxTransform t = bodyHandler->rightHand->getGlobalPose() * PxTransform::createIdentity();
				bodyHandler->rightGrabJoint = PxD6JointCreate(*gPhysics, bodyHandler->rightHand, PxTransform::createIdentity(), NULL, t);
			}
			bodyHandler->rightGrabRigidbody = collisionRigidbody;
			s->rightCanClimb = true;
			gPxHandler->setJointMotion(bodyHandler->rightGrabJoint, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
			bodyHandler->rightGrabJoint->setBreakForce(30000.f, 30000.f);
		}
	}
}