#include "MovementHandler.h"
#include <string>	
#include "RagDollMath.h"
void MovementHandler::Stand()
{
	//s->applyedForce /= 20.0f;
	//AlignToVector(bodyHandeler->head, bodyHandeler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 2.5f * s->applyedForce);
	AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 2.5f * s->applyedForce);
	AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 2.5f * s->applyedForce);
	AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 4.0f * s->applyedForce);
	AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 4.0f * s->applyedForce);
	AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 4.0f * s->applyedForce);
	AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 4.0f * s->applyedForce);
	AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 3.0f * s->applyedForce);

	if (!s->leftKick)
	{
		AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 3 * s->applyedForce);
		AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 3.0f * s->applyedForce);
		bodyHandler->leftFoot->addForce(-s->counterForce * s->applyedForce, PxForceMode::eVELOCITY_CHANGE);
	}
	if (!s->rightKick)
	{
		AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 3.0f * s->applyedForce);
		AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 3.0f * s->applyedForce);
		bodyHandler->rightFoot->addForce(-s->counterForce * s->applyedForce, PxForceMode::eVELOCITY_CHANGE);
	}
	bodyHandler->chest->addForce(s->counterForce * s->applyedForce, PxForceMode::eVELOCITY_CHANGE);
	bodyHandler->ball->setAngularVelocity(PxVec3(0));
}

void MovementHandler::Run()
{
	if (s->run)
	{
		s->cycleSpeed = 0.18f;
	}
	else
	{
		s->cycleSpeed = 0.23f;//0.23f;
	}
	if (s->stateChange)
	{
		if (Random(0.f,2.f) == 1.f)
		{
			leftLegPose = Pose::Bent;
			rightLegPose = Pose::Straight;
			leftArmPose = Pose::Straight;
			rightArmPose = Pose::Bent;
		}
		else
		{
			leftLegPose = Pose::Straight;
			rightLegPose = Pose::Bent;
			leftArmPose = Pose::Bent;
			rightArmPose = Pose::Straight;
		}
		s->stateChange = false;
	}
	
	RunCycleUpdate();
	RunCycleRotateball();
	RunCyclePoseBody();
	if ((!s->duck || !s->run) && (!s->kickDuck || !s->run))
	{
		if (!s->duck && !s->kickDuck)
		{
			if (!s->leftArmOverride)
			{
				RunCyclePoseArm(Side::Left, leftArmPose);
			}
			if (!s->rightArmOverride)
			{
				RunCyclePoseArm(Side::Right, rightArmPose);
			}
		}
		if (!s->leftLegOverride)
		{
			RunCyclePoseLeg(Side::Left, leftLegPose);
		}
		if (!s->rightLegOverride)
		{
			RunCyclePoseLeg(Side::Right, rightLegPose);
		}
	}
}

void MovementHandler::Fall()
{
	float num = 0.2f;
	AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 2.f);
	if (!s->duck && !s->kickDuck)
	{
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), s->direction, 0.1f, 4.f);
		AlignToVector(bodyHandler->chest, -bodyHandler->chest->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 4.f);
	}
	else if (s->duck)
	{
		AlignToVector(bodyHandler->chest, -bodyHandler->chest->getGlobalPose().q.rotate(PxForward), bodyHandler->chest->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
	}
	else if (s->kickDuck)
	{
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), bodyHandler->chest->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
	}
	AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), s->direction, 0.1f, 2.f);
	AlignToVector(bodyHandler->chest, -bodyHandler->chest->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 2.f);
	AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxUp), s->direction, 0.1f, 2.f);
	AlignToVector(bodyHandler->waist, -bodyHandler->waist->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 2.f);
	AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxUp), s->direction, 0.1f, 2.f);
	AlignToVector(bodyHandler->hips, -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 2.f);
	AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 2.f);
	AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 2.f);
	AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 2.f);
	AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 2.f);
	if ((s->rawDirection.x != 0.f) || (s->rawDirection.z != 0.f))
	{
		bodyHandler->chest->addForce(s->direction * num, PxForceMode::eVELOCITY_CHANGE);
	}
}

void MovementHandler::Jump()
{
	float num = 0.6f * s->inputSpamForceModifier;
	float num2 = 60.f * s->inputSpamForceModifier;//90.f * s->inputSpamForceModifier;
	s->applyedForce = 0.2f;
	std::string str = "Jump";
	if (s->stateChange)
	{
		s->stateChange = false;
		if (s->cUpperIntrest != NULL)
		{
			if (s->cUpperIntrest->getGlobalPose().p.y < bodyHandler->chest->getGlobalPose().p.y)
			{
				if (s->grabDelay < 0.25f)
				{
					s->grabDelay = 0.25f;
				}
			}
			else
			{
				s->grabDelay = 0.f;
			}
		}
		else
		{
			s->grabDelay = 0.25f;
		}
		if (s->run)
		{
			num = 0.8f * s->inputSpamForceModifier;
		}
		else
		{
			num = 0.6f * s->inputSpamForceModifier;
		}
		if ((s->jump || s->grabJump) && (s->stamina >= 0.f))
		{
			if (s->grabJump)
			{
				s->grabDelay = 0.4f;
				s->grabJumpCounter++;
				s->staminaDamage += 40.f * s->grabJumpCounter;
				ResetLeftGrab();
				ResetRightGrab();
				bodyHandler->chest->addForce(s->direction * num * 2.f, PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->waist->addForce(s->direction * num * 2.f, PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->hips->addForce(s->direction * num * 2.f, PxForceMode::eVELOCITY_CHANGE);
				if (bodyHandler->chest->getGlobalPose().p.y >= (bodyHandler->hips->getGlobalPose().p.y + 0.1f))
				{
					str += "-1";
					PxVec3 force = (((PxUp * num2) * 1.5f) / 3.f);
					bodyHandler->chest->addForce(force, PxForceMode::eVELOCITY_CHANGE);
					bodyHandler->waist->addForce(force, PxForceMode::eVELOCITY_CHANGE);
					bodyHandler->hips->addForce(force, PxForceMode::eVELOCITY_CHANGE);
				}
				else
				{
					str += "-2";
					PxVec3 vector6 = ((PxUp * num2) / 3.f);
					bodyHandler->chest->addForce(vector6, PxForceMode::eVELOCITY_CHANGE);
					bodyHandler->waist->addForce(vector6, PxForceMode::eVELOCITY_CHANGE);
					bodyHandler->hips->addForce(vector6, PxForceMode::eVELOCITY_CHANGE);
				}
			}
			else
			{
				bodyHandler->chest->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->waist->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->hips->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);

				if (bodyHandler->hips->getLinearVelocity().y < 2.f)
				{
					// 原地起跳走这里
					if (bodyHandler->chest->getGlobalPose().p.y >= (bodyHandler->hips->getGlobalPose().p.y + 0.1f))
					{
						str += "-3";
						PxVec3 vector10 = ((PxUp * num2) / 3.f);
						bodyHandler->chest->addForce(vector10, PxForceMode::eVELOCITY_CHANGE);
						bodyHandler->waist->addForce(vector10, PxForceMode::eVELOCITY_CHANGE);
						bodyHandler->hips->addForce(vector10, PxForceMode::eVELOCITY_CHANGE);
					}
					else
					{
						str += "-4";
						PxVec3 vector11 = (((PxUp * num2) * 0.5f) / 3.f);
						bodyHandler->chest->addForce(vector11, PxForceMode::eVELOCITY_CHANGE);
						bodyHandler->waist->addForce(vector11, PxForceMode::eVELOCITY_CHANGE);
						bodyHandler->hips->addForce(vector11, PxForceMode::eVELOCITY_CHANGE);
					}
				}
			}
			//effectsHandeler.AudioEvent(EffectsHandeler.SoundType.Jump, bodyHandeler->hips, UnityEngine.Random.Range((float) 0.9f, (float) 1.1f), 0.5f);
		}
		s->jump = false;
		s->grabJump = false;
	}
	// 边走边跳
	if ((s->rawDirection.x != 0.f) || (s->rawDirection.z != 0.f))
	{
		str += "-5";
		// 		bodyHandeler->chest->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);
		// 		bodyHandeler->waist->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);
		// 		bodyHandeler->hips->addForce((s->direction * num), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->chest->addForce((s->direction * num / 5.f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->waist->addForce((s->direction * num / 5.f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->hips->addForce((s->direction * num / 5.f), PxForceMode::eVELOCITY_CHANGE);

	}

	AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 4.f);
	AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 4.f);

	if (!s->duck && !s->kickDuck)
	{
		str += "-6";
		bodyHandler->chest->addForce((PxUp * 2.f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->hips->addForce((PxDown * 2.f), PxForceMode::eVELOCITY_CHANGE);
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), s->direction + PxVec3(0.f, -1.f, 0.f), 0.1f, 10.f);
		AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), s->direction + PxVec3(0.f, 0.f, 0.f), 0.1f, 10.f);
		AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), s->direction + PxVec3(0.f, 1.f, 0.f), 0.1f, 10.f);
		if (!s->leftArmOverride)
		{
			AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), ((bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) + bodyHandler->chest->getGlobalPose().q.rotate(PxForward)) / 4.f), 0.1f, 4.f);
			AlignToVector(bodyHandler->leftForarm, bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), ((bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) + -bodyHandler->chest->getGlobalPose().q.rotate(PxForward)) / 4.f), 0.1f, 4.f);
		}
		if (!s->rightArmOverride)
		{
			AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), ((-bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) + bodyHandler->chest->getGlobalPose().q.rotate(PxForward)) / 4.f), 0.1f, 4.f);
			AlignToVector(bodyHandler->rightForarm, bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), ((-bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) + -bodyHandler->chest->getGlobalPose().q.rotate(PxForward)) / 4.f), 0.1f, 4.f);
		}
		AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp) + -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp) + bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp) + -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp) + bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
	}
	else if (s->duck)
	{
		str += "-7";
		if (s->run)
		{
			bodyHandler->chest->addForce(s->slideDirection, PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->waist->addForce((s->slideDirection / 2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->hips->addForce((-s->slideDirection / 2.f), PxForceMode::eVELOCITY_CHANGE);
		}
		else
		{
			bodyHandler->chest->addTorque(bodyHandler->chest->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->waist->addTorque(bodyHandler->waist->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->hips->addTorque(bodyHandler->hips->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
		}
		AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), -PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), -PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), -PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->chest->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), bodyHandler->chest->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		if (!s->leftArmOverride)
		{
			AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), (bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 2.f), 0.01f, 8.f);
			AlignToVector(bodyHandler->leftForarm, bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), -s->slideDirection, 0.01f, 4.f);
		}
		if (!s->rightArmOverride)
		{
			AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), (-bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 2.f), 0.01f, 8.f);
			AlignToVector(bodyHandler->rightForarm, bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), -s->slideDirection, 0.01f, 4.f);
		}
	}
	else if (s->kickDuck)
	{
		str += "-8";
		if (s->run)
		{
			bodyHandler->chest->addForce((-s->slideDirection / 2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->waist->addForce((s->slideDirection / 2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->hips->addForce(s->slideDirection, PxForceMode::eVELOCITY_CHANGE);
		}
		else
		{
			bodyHandler->chest->addTorque(bodyHandler->chest->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->waist->addTorque(bodyHandler->waist->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->hips->addTorque(bodyHandler->hips->getGlobalPose().q.rotate(PxLeft*2.f), PxForceMode::eVELOCITY_CHANGE);
		}
		AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 4.f);
		AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), -bodyHandler->leftThigh->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), -bodyHandler->rightLeg->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
	}
	//printf("%s\n", str.c_str());
}

void MovementHandler::Climb()
{
	std::string str = "Climb";
	if ((s->rawDirection.x != 0.f) || (s->rawDirection.z != 0.f))
	{
		str += "-1";
		bodyHandler->chest->addForce((s->direction * s->airSpeed), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->waist->addForce((s->direction * s->airSpeed), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->hips->addForce((s->direction * s->airSpeed), PxForceMode::eVELOCITY_CHANGE);
	}
	if (!s->duck)
	{
		str += "-2";
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 4.f);
		AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 4.f);
		AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 4.f);
	}
	float num = 0.6f;
	if (s->liftSelf)
	{
		str += "-3";
		//num = 1.2f;
		num = 2.4f;
	}
	PxVec3 zero = PxVec3(0);
	if (!s->duck)
	{
		str += "-4";
		bodyHandler->head->addForce(PxVec3(0.f, 2.f * num, 0.f), PxForceMode::eVELOCITY_CHANGE);
	}
	else
	{
		str += "-5";
		bodyHandler->waist->addForce(PxVec3(0.f, 2.f * num, 0.f), PxForceMode::eVELOCITY_CHANGE);
	}
	if (bodyHandler->leftGrabJoint != NULL)
	{ 
		if (!s->duck)
		{
			str += "-6";
			bodyHandler->head->addForce(PxVec3(0.f, 1.5f * num, 0.f), PxForceMode::eVELOCITY_CHANGE);
			//AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), -bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			if (s->liftSelf)
			{
				AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), -bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			}
			else
			{
				AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			}

			if (bodyHandler->leftGrabRigidbody != NULL)
			{
				str += "-7";
				if (bodyHandler->leftGrabRigidbody->isRigidDynamic())
					bodyHandler->leftGrabRigidbody->addForce(PxVec3(0.f, -24.f * num, 0.f), PxForceMode::eIMPULSE);
			}
		}
		else if (bodyHandler->leftGrabRigidbody != NULL)
		{
			str += "-8";
			if (bodyHandler->leftGrabRigidbody->isRigidDynamic())
				bodyHandler->leftGrabRigidbody->addForce(PxVec3(0.f, -32.f * num, 0.f), PxForceMode::eIMPULSE);
		}
		AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		PxVec3 vector2 = (bodyHandler->rightHand->getGlobalPose().p - PxVec3(0.f, 0.5f, 0.f) - bodyHandler->leftFoot->getGlobalPose().p).getNormalized();
		bodyHandler->leftThigh->addForce((-vector2 * 2.f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->leftFoot->addForce((vector2 * 2.f), PxForceMode::eVELOCITY_CHANGE);
	}
	if (bodyHandler->rightGrabJoint != NULL)
	{
		if (!s->duck)
		{
			str += "-9";
			bodyHandler->head->addForce(PxVec3(0.f, 1.5f * num, 0.f), PxForceMode::eVELOCITY_CHANGE);
			//AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), -bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			if (s->liftSelf)
			{
				AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), -bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			}
			else
			{
				AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), 0.1f, 10.f);
			}

			if (bodyHandler->rightGrabRigidbody != NULL)
			{
				str += "-10";
				if (bodyHandler->rightGrabRigidbody->isRigidDynamic())
					bodyHandler->rightGrabRigidbody->addForce(PxVec3(0.f, -24.f * num, 0.f), PxForceMode::eIMPULSE);
			}
		}
		else if (bodyHandler->rightGrabRigidbody != NULL)
		{
			str += "-11";
			if (bodyHandler->rightGrabRigidbody->isRigidDynamic())
				bodyHandler->rightGrabRigidbody->addForce(PxVec3(0.f, -32.f * num, 0.f), PxForceMode::eIMPULSE);
		}
		AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 4.f);
		AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxUp), 0.1f, 4.f);
		PxVec3 vector3 = (bodyHandler->leftHand->getGlobalPose().p - PxVec3(0.f, 0.5f, 0.f) - bodyHandler->rightFoot->getGlobalPose().p).getNormalized();
		bodyHandler->rightThigh->addForce((-vector3 * 2.f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->rightFoot->addForce((vector3 * 2.f), PxForceMode::eVELOCITY_CHANGE);
	}
	if ((bodyHandler->rightGrabJoint != NULL) && (bodyHandler->leftGrabJoint != NULL))
	{
		str += "-12";
		zero = PxVec3(0.f, 1.f * num, 0.f);
	}
	else if ((bodyHandler->rightGrabJoint != NULL) || (bodyHandler->leftGrabJoint != NULL))
	{
		str += "-13";
		zero = PxVec3(0.f, 0.5f * num, 0.f);
	}
	//if (((actor.RewiredPlayer.GetAxis("Move Horizontal") != 0.f) || (actor.RewiredPlayer.GetAxis("Move Vertical") != 0.f)) && !s->duck)
	if ((s->rawDirection.x != 0.f || s->rawDirection.z != 0.f) || !s->duck)
	{
		str += "-14";
		bodyHandler->chest->addForce((-s->direction * 0.25f), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->ball->addForce((s->direction * 0.25f), PxForceMode::eVELOCITY_CHANGE);
	}
//	printf("%s\n", str.c_str());
}

// 眩晕时什么都不用做
void MovementHandler::Unconsious()
{
}

void MovementHandler::RunCycleUpdate()
{
	if (s->cycleTimer < s->cycleSpeed)
	{
		s->cycleTimer += s->deltaTime;
	}
	else
	{
		s->cycleTimer = 0.0f;
		int num = (int)leftArmPose;
		num++;
		leftArmPose = (Pose)((num <= 3) ? num : 0);
		int num2 = (int)rightArmPose;
		num2++;
		rightArmPose = (Pose)((num2 <= 3) ? num2 : 0);
		int num3 = (int)leftLegPose;
		num3++;
		leftLegPose = (Pose)((num3 <= 3) ? num3 : 0);
		int num4 = (int)rightLegPose;
		num4++;
		rightLegPose = (Pose)((num4 <= 3) ? num4 : 0);
	}
}

void MovementHandler::RunCyclePoseBody()
{
	if (s->run)
	{
		s->runForce = PxClamp(s->runForce += s->deltaTime, 0.f, 1.f);
	}
	else
	{
		s->runForce = PxClamp(s->runForce -= s->deltaTime, 0.f, 1.f);
	}
	if (!s->duck && !s->kickDuck)
	{
		bodyHandler->chest->addForce(((PxVec3(0.f, 10.f, 0.f) + ((s->direction * s->runForce) * 4.f)) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->hips->addForce(((PxVec3(0.f, -5.f, 0.f) + -((s->direction * s->runForce) * 4.f)) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->ball->addForce((PxVec3(0.f, -5.f, 0.f) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
		AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 2.5f * s->applyedForce);
		AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxUp), PxUp + ((bodyHandler->chest->getGlobalPose().q.rotate(PxForward) * s->runForce)), 0.1f, 2.5f * s->applyedForce);
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), ((s->direction / 4.f)) + PxVec3(0.f, -1.f, 0.f), 0.1f, 8.f * s->applyedForce);
		AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), PxUp, 0.1f, 12.f * s->applyedForce);//
		AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), ((s->direction / 2.f)) + PxVec3(0.f, -0.5f, 0.f), 0.1f, 8.f * s->applyedForce);
		AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), s->direction, 0.1f, 8.f * s->applyedForce);
	}
	else
	{
		if (s->duck)
		{
			if (s->run)
			{
				s->applyedForce = 0.5f;
				AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->chest, -bodyHandler->chest->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->waist, -bodyHandler->waist->getGlobalPose().q.rotate(PxForward), PxUp + ((-s->slideDirection / 2.f)), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->hips, -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), PxUp + -s->slideDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				bodyHandler->chest->addForce((s->slideDirection * 1.6f), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->waist->addForce((s->slideDirection * 1.6f), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->hips->addForce((-s->slideDirection * 1.6f), PxForceMode::eVELOCITY_CHANGE);
			}
			else
			{
				bodyHandler->hips->addForce(((PxVec3(0.f, 4.f, 0.f) + bodyHandler->hips->getGlobalPose().q.rotate(PxUp)) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->ball->addForce(((PxVec3(0.f, -4.f, 0.f) + -bodyHandler->hips->getGlobalPose().q.rotate(PxUp)) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
				AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxForward) + PxVec3(0.f, 0.5f, 0.f), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxUp), bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxUp), s->direction, 0.1f, 8.f * s->applyedForce);
			}
		}
		if (s->kickDuck)
		{
			if (s->run)
			{
				s->applyedForce = 0.5f;
				AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), s->lookDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxForward), PxUp, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxForward), PxUp + ((-s->slideDirection / 2.f)), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxForward), PxUp + -s->slideDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->leftThigh, bodyHandler->leftThigh->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->leftLeg, bodyHandler->leftLeg->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->rightThigh, bodyHandler->rightThigh->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				AlignToVector(bodyHandler->rightLeg, bodyHandler->rightLeg->getGlobalPose().q.rotate(PxUp), PxDown, 0.1f, 10.f);
				bodyHandler->chest->addForce((-s->slideDirection * 1.8f), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->waist->addForce((s->slideDirection * 1.8f), PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->hips->addForce((s->slideDirection * 1.8f), PxForceMode::eVELOCITY_CHANGE);
			}
			else
			{
				bodyHandler->leftLeg->addForce(PxUp, PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->leftFoot->addForce(PxDown, PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->rightLeg->addForce(PxUp, PxForceMode::eVELOCITY_CHANGE);
				bodyHandler->rightFoot->addForce(PxDown, PxForceMode::eVELOCITY_CHANGE);
				AlignToVector(bodyHandler->head, bodyHandler->head->getGlobalPose().q.rotate(PxForward), -s->lookDirection, 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->chest, bodyHandler->chest->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward) + PxVec3(0.f, 0.5f, 0.f), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->waist, bodyHandler->waist->getGlobalPose().q.rotate(PxUp), -bodyHandler->hips->getGlobalPose().q.rotate(PxForward), 0.1f, 8.f * s->applyedForce);
				AlignToVector(bodyHandler->hips, bodyHandler->hips->getGlobalPose().q.rotate(PxUp), -s->direction, 0.1f, 8.f * s->applyedForce);
			}
		}
	}
}


void MovementHandler::RunCyclePoseArm(Side side, Pose pose)
{
	
	PxVec3 zero = PxVec3(0);
	PxTransform transform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2;
	PxTransform transform3;
	PxRigidDynamic* part = NULL;
	PxRigidDynamic* rigidbody2 = NULL;
	switch (side)
	{
	case Side::Left:
		//printf("RunCyclePoseArm Left\n");
		transform2 = bodyHandler->leftArm->getGlobalPose();
		transform3 = bodyHandler->leftForarm->getGlobalPose();
		part = bodyHandler->leftArm;
		rigidbody2 = bodyHandler->leftForarm;
		zero = bodyHandler->chest->getGlobalPose().q.rotate(PxLeft);
		break;

	case Side::Right:
		transform2 = bodyHandler->rightArm->getGlobalPose();
		transform3 = bodyHandler->rightForarm->getGlobalPose();
		part = bodyHandler->rightArm;
		rigidbody2 = bodyHandler->rightForarm;
		zero = -bodyHandler->chest->getGlobalPose().q.rotate(PxLeft);
		break;
	}
	if (!s->duck)
	{
		// 调小一点防止左右摇晃幅度过大
		//zero *= 0.5f;
		// 不需要水平方向的力，让手臂惯性摆动即可
		switch (pose)
		{
		case Pose::Bent:
			//AlignToVector(part, transform2.q.rotate(PxUp), transform.q.rotate(PxUp) + zero, 0.1f, 5.f * s->applyedForce);
			AlignToVector(part, transform2.q.rotate(PxUp), transform.q.rotate(PxUp), 0.1f, 5.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), -s->direction, 0.1f, 5.f * s->applyedForce);
			break;

		case Pose::Forward:
			//AlignToVector(part, transform2.q.rotate(PxUp), s->direction + -zero*1.5f, 0.1f, 5.f * s->applyedForce);
			AlignToVector(part, transform2.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), (s->direction + -transform.q.rotate(PxUp)) + -zero, 0.1f, 5.f * s->applyedForce);
			break;

		case Pose::Straight:
			AlignToVector(part, transform2.q.rotate(PxUp), transform.q.rotate(PxUp), 0.1f, 5.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), transform.q.rotate(PxUp), 0.1f, 5.f * s->applyedForce);
			break;
		case Pose::Behind:
			//AlignToVector(part, transform2.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
			AlignToVector(part, transform2.q.rotate(PxUp), s->direction, 0.1f, 1.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), transform.q.rotate(PxUp), 0.1f, 5.f * s->applyedForce);
			break;
		}
	}
	else
	{
		switch (pose)
		{
		case Pose::Bent:
			AlignToVector(part, transform2.q.rotate(PxUp), -s->direction + zero, 0.1f, 5.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), -s->direction + -zero, 0.1f, 5.f * s->applyedForce);
			break;

		case Pose::Forward:
			AlignToVector(part, transform2.q.rotate(PxUp), -s->direction + (-(PxUp * s->runForce)), 0.1f, 8.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), (-s->direction + zero) + (-(PxUp * s->runForce)), 0.1f, 8.f * s->applyedForce);
			break;

		case Pose::Straight:
			AlignToVector(part, transform2.q.rotate(PxUp), -s->direction + zero, 0.1f, 5.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), -s->direction, 0.1f, 5.f * s->applyedForce);
			break;

		case Pose::Behind:
			AlignToVector(part, transform2.q.rotate(PxUp), (s->direction + zero) + (-(PxUp * s->runForce)), 0.1f, 8.f * s->applyedForce);
			AlignToVector(rigidbody2, transform3.q.rotate(PxUp), (s->direction + -zero) + (-(PxUp * s->runForce)), 0.1f, 8.f * s->applyedForce);
			break;
		}
	}
}


void MovementHandler::RunCyclePoseLeg(Side side, Pose pose)
{
	PxTransform transform = bodyHandler->hips->getGlobalPose();
	PxTransform transform2;
	PxTransform transform3;
	PxTransform transform4;
	PxRigidDynamic* part = NULL;
	PxRigidDynamic* rigidbody2 = NULL;
	PxRigidDynamic* rigidbody3 = NULL;
	switch (side)
	{
	case Side::Left:
		transform2 = bodyHandler->leftThigh->getGlobalPose();
		transform3 = bodyHandler->leftLeg->getGlobalPose();
		transform4 = bodyHandler->rightFoot->getGlobalPose();
		part = bodyHandler->leftThigh;
		rigidbody2 = bodyHandler->leftLeg;
		rigidbody3 = bodyHandler->leftFoot;
		break;

	case Side::Right:
		transform2 = bodyHandler->rightThigh->getGlobalPose();
		transform3 = bodyHandler->rightLeg->getGlobalPose();
		transform4 = bodyHandler->rightFoot->getGlobalPose();
		part = bodyHandler->rightThigh;
		rigidbody2 = bodyHandler->rightLeg;
		rigidbody3 = bodyHandler->rightFoot;
		break;
	}
	switch (pose)
	{
	case Pose::Bent:
		AlignToVector(part, -transform2.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
		AlignToVector(rigidbody2, transform3.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
		break;

	case Pose::Forward:
		AlignToVector(part, -transform2.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
		AlignToVector(rigidbody2, -transform3.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
		break;

	case Pose::Straight:
		AlignToVector(part, transform2.q.rotate(PxUp), PxUp, 0.1f, 5.f * s->applyedForce);
		AlignToVector(rigidbody2, transform3.q.rotate(PxUp), PxUp, 0.1f, 5.f * s->applyedForce);
		rigidbody3->addForce((PxDown * 2.f), PxForceMode::eVELOCITY_CHANGE);
		break;

	case Pose::Behind:
		AlignToVector(part, transform2.q.rotate(PxUp), s->direction, 0.1f, 5.f * s->applyedForce);
		AlignToVector(rigidbody2, -transform3.q.rotate(PxUp), -s->direction, 0.1f, 5.f * s->applyedForce);
		if (s->duck)
		{
			bodyHandler->hips->addForce(PxUp, PxForceMode::eVELOCITY_CHANGE);
			bodyHandler->ball->addForce(PxDown, PxForceMode::eVELOCITY_CHANGE);
			rigidbody3->addForce((PxDown * 2.f), PxForceMode::eVELOCITY_CHANGE);
			break;
		}
		bodyHandler->hips->addForce(((PxUp * 5.f) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
		bodyHandler->ball->addForce(((PxDown * 5.f) * s->applyedForce), PxForceMode::eVELOCITY_CHANGE);
		break;
	}
}


void MovementHandler::RunCycleRotateball()
{
	bodyHandler->ball->setAngularVelocity(PxVec3(0));
	if (s->onMovingPlatform)
	{
		bodyHandler->chest->addForce(s->direction, PxForceMode::eVELOCITY_CHANGE);
	}
	PxVec3 a = PxVec3(s->direction.z, 0.0f, -s->direction.x);
	a /= PxClamp(Angle(PxVec3(bodyHandler->hips->getGlobalPose().q.rotate(PxForward).x
		, 0.0f
		, bodyHandler->hips->getGlobalPose().q.rotate(PxForward).z)
		, s->direction)
		, 0.01f, 180.0f) / 40.0f;
	if (!s->run)
	{
		if (!s->duck && !s->kickDuck)
		{
			if (bodyHandler->ball->getLinearVelocity().magnitude() < 5.0f * s->applyedForce)
			{
				bodyHandler->ball->setMaxAngularVelocity(15.0f);
				bodyHandler->ball->addTorque(a * 15.0f, PxForceMode::eVELOCITY_CHANGE);
			}
		}
		else if (bodyHandler->ball->getLinearVelocity().magnitude() < 2.5f * s->applyedForce)
		{
			bodyHandler->ball->setMaxAngularVelocity(2.5f);
			bodyHandler->ball->addTorque(a * 2.5f, PxForceMode::eVELOCITY_CHANGE);
		}
	}
	else if (!s->duck && !s->kickDuck)
	{
		if (bodyHandler->ball->getLinearVelocity().magnitude() < 5.0f * s->applyedForce)
		{
			bodyHandler->ball->setMaxAngularVelocity(20.0f);
			bodyHandler->ball->addTorque(a * 20.0f, PxForceMode::eVELOCITY_CHANGE);
		}
	}
	else if (bodyHandler->ball->getLinearVelocity().magnitude() < 2.5f * s->applyedForce)
	{
		bodyHandler->ball->setMaxAngularVelocity(2.5f);
		bodyHandler->ball->addTorque(a * 2.5f, PxForceMode::eVELOCITY_CHANGE);
	}
}

void MovementHandler::ArmActionGrabbing(Side side, PxRigidBody* grabTarget)
{
	PxD6Joint* grabJoint = NULL;
	PxRigidBody* grabRigidbody = NULL;
	PxTransform grabTransform;
	PxTransform transform2 = bodyHandler->chest->getGlobalPose();
	PxTransform transform3;
	PxTransform transform4;
	PxTransform transform5;
	PxRigidDynamic* part = NULL;
	PxRigidDynamic* rigidbody3 = NULL;
	PxRigidDynamic* rigidbody4 = NULL;
	switch (side)
	{
	case Side::Left:
		transform3 = bodyHandler->leftArm->getGlobalPose();
		transform4 = bodyHandler->leftForarm->getGlobalPose();
		transform5 = bodyHandler->leftHand->getGlobalPose();
		part = bodyHandler->leftArm;
		rigidbody3 = bodyHandler->leftForarm;
		rigidbody4 = bodyHandler->leftHand;
		grabJoint = bodyHandler->leftGrabJoint;
		grabRigidbody = bodyHandler->leftGrabRigidbody;
		break;

	case Side::Right:
		transform3 = bodyHandler->rightArm->getGlobalPose();
		transform4 = bodyHandler->rightForarm->getGlobalPose();
		transform5 = bodyHandler->rightHand->getGlobalPose();
		part = bodyHandler->rightArm;
		rigidbody3 = bodyHandler->rightForarm;
		rigidbody4 = bodyHandler->rightHand;
		grabJoint = bodyHandler->rightGrabJoint;
		grabRigidbody = bodyHandler->rightGrabRigidbody;
		break;
	}
	if ((grabJoint == NULL) && (grabRigidbody == NULL))
	{
		PxVec3 zero = PxVec3(0);
		if (grabTarget != NULL)
		{
			AlignToVector(rigidbody3, transform4.q.rotate(PxUp), zero, 0.1f, 100.f);
			AlignToVector(part, transform3.q.rotate(PxUp), zero, 0.1f, 100.f);
			//zero = Vector3.Normalize(grabTarget.bounds.center - transform5.position);
			PxVec3 d = grabTarget->getWorldBounds().getCenter() - transform5.p;
			zero = d.getNormalized();
			//if (Vector3.Distance(grabTarget.bounds.center, transform3.position) > 0.45f)
			if (PxSqrt(d.x * d.x + d.y * d.y + d.z * d.z) > 0.45f)
			{
				part->addForce((-zero * 3.f), PxForceMode::eVELOCITY_CHANGE);
				rigidbody4->addForce((zero * 4.f), PxForceMode::eVELOCITY_CHANGE);
			}
			else
			{
				part->addForce((-zero * 2.f), PxForceMode::eVELOCITY_CHANGE);
				rigidbody4->addForce((zero * 6.f), PxForceMode::eVELOCITY_CHANGE);
			}
		}
		else
		{
			zero = s->direction;
			AlignToVector(rigidbody3, transform4.q.rotate(PxUp), -zero, 0.1f, 5.f);
			AlignToVector(part, transform3.q.rotate(PxUp), -zero, 0.1f, 5.f);
		}
	}

	float num = 25.f;
	if ((s->lift && (grabJoint != NULL)) && (grabRigidbody != NULL))
	{
		AlignToVector(part, -transform3.q.rotate(PxUp), PxUp + ((transform2.q.rotate(PxForward) * 0.8f)), 0.1f, 4.f);
		AlignToVector(rigidbody3, -transform4.q.rotate(PxUp), PxUp + ((transform2.q.rotate(PxForward) * 0.8f)), 0.1f, 4.f);
		if (s->onGround)
		{
			AlignToVector(part, -transform3.q.rotate(PxUp), PxUp + ((transform2.q.rotate(PxForward) * 0.8f)), 0.1f, 4.f);
			AlignToVector(rigidbody3, -transform4.q.rotate(PxUp), PxUp + ((transform2.q.rotate(PxForward) * 0.8f)), 0.1f, 4.f);
			//bodyHandler->ball->addForce((PxDown * num), PxForceMode::eIMPULSE);
			bodyHandler->ball->addForce((PxDown * num * 2.0f), PxForceMode::eIMPULSE);
			if(grabRigidbody->isRigidBody()) grabRigidbody->addForce((PxUp * num), PxForceMode::eIMPULSE);
		}
	}

}

void MovementHandler::ArmActionCheering()
{
	PxReal speed = 2.f;
	if (!s->leftGrab && !s->leftPunch)
	{
		AlignToVector(bodyHandler->leftArm, bodyHandler->leftArm->getGlobalPose().q.rotate(PxUp), ((-bodyHandler->chest->getGlobalPose().q.rotate(PxUp) + -(bodyHandler->chest->getGlobalPose().q.rotate(PxForward) / 2.f)) + (bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 4.f)), 0.1f, speed);
		AlignToVector(bodyHandler->leftForarm, bodyHandler->leftForarm->getGlobalPose().q.rotate(PxUp), -bodyHandler->chest->getGlobalPose().q.rotate(PxUp) + ((bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 4.f)), 0.1f, 10.f);
	}
	if (!s->rightGrab && !s->rightPunch)
	{
		AlignToVector(bodyHandler->rightArm, bodyHandler->rightArm->getGlobalPose().q.rotate(PxUp), ((-bodyHandler->chest->getGlobalPose().q.rotate(PxUp) + -(bodyHandler->chest->getGlobalPose().q.rotate(PxForward) / 2.f)) + (-bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 4.f)), 0.1f, speed);
		AlignToVector(bodyHandler->rightForarm, bodyHandler->rightForarm->getGlobalPose().q.rotate(PxUp), -bodyHandler->chest->getGlobalPose().q.rotate(PxUp) + ((-bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 4.f)), 0.1f, 10.f);
	}
}

void MovementHandler::ResetLeftGrab()
{
	if (s->leftGrabJoint) 
	{
		s->leftGrabJoint->release();
		s->leftGrabJoint = NULL;
	}
}

void MovementHandler::ResetRightGrab()
{
	if (s->rightGrabJoint)
	{
		s->rightGrabJoint->release();
		s->rightGrabJoint = NULL;
	}
}

void MovementHandler::ArmActionReadying(Side side)
{
	PxTransform partTransform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2;
	PxTransform transform3;
	PxRigidDynamic* part = NULL;
	PxRigidDynamic* partRigidbody = NULL;
	PxVec3 zero = PxVec3(0);
	if (side != Side::Left)
	{
		transform2 = bodyHandler->rightArm->getGlobalPose();
		transform3 = bodyHandler->rightForarm->getGlobalPose();
		part = bodyHandler->rightArm;
		partRigidbody = bodyHandler->rightForarm;
		zero = -bodyHandler->chest->getGlobalPose().q.rotate(PxLeft);
	}
	else
	{
		transform2 = bodyHandler->leftArm->getGlobalPose();
		transform3 = bodyHandler->leftForarm->getGlobalPose();
		part = bodyHandler->leftArm;
		partRigidbody = bodyHandler->leftForarm;
		zero = bodyHandler->chest->getGlobalPose().q.rotate(PxLeft);
	}
	AlignToVector(part, transform2.q.rotate(PxUp), zero, 0.01f, 10.f);
	AlignToVector(partRigidbody, transform3.q.rotate(PxUp), -partTransform.q.rotate(PxForward), 0.01f, 10.f);
}

void MovementHandler::ArmActionPunching(Side side, PxRigidBody* punchTarget)
{
	PxTransform partTransform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2;
	PxTransform transform3;
	PxRigidDynamic* rigidbody = NULL;
	PxRigidDynamic* rigidbody2 = NULL;
	if (side != Side::Left)
	{
		transform2 = bodyHandler->rightArm->getGlobalPose();
		transform3 = bodyHandler->rightHand->getGlobalPose();
		rigidbody = bodyHandler->rightArm;
		rigidbody2 = bodyHandler->rightHand;

	}
	else
	{
		transform2 = bodyHandler->leftArm->getGlobalPose();
		transform3 = bodyHandler->leftHand->getGlobalPose();
		rigidbody = bodyHandler->leftArm;
		rigidbody2 = bodyHandler->leftHand;
	}
	PxVec3 zero = PxVec3(0);
	if (punchTarget == NULL)
	{
		zero = (partTransform.p + partTransform.q.rotate(PxForward) + partTransform.q.rotate(PxUp) / 2.f - transform2.p).getNormalized();
		zero.y = 0; // 原版有y分量
		rigidbody->addForce(-zero * 15.f, PxForceMode::eVELOCITY_CHANGE); // 原版的系数是 * 4.f
		rigidbody2->addForce(zero * 15.f, PxForceMode::eVELOCITY_CHANGE); // 原版的系数是 * 4.f
	}
	else
	{
		PxVec3 d = punchTarget->getWorldBounds().getCenter() - transform2.p;
		zero = d.getNormalized();
		if (PxSqrt(d.x*d.x+d.y*d.y+d.z*d.z) > 1.f)
		{
			rigidbody->addForce((-zero * 2.f), PxForceMode::eVELOCITY_CHANGE);
			rigidbody2->addForce((zero * 6.f), PxForceMode::eVELOCITY_CHANGE);
		}
		else
		{
			rigidbody2->addForce((zero * 6.f), PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void MovementHandler::ArmActionPunchResetting(Side side)
{
	PxTransform partTransform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2;
	PxTransform transform3;
	PxRigidDynamic* part = NULL;
	PxRigidDynamic* partRigidbody = NULL;
	PxVec3 zero = PxVec3(0);
	if (side != Left)
	{
		transform2 = bodyHandler->rightArm->getGlobalPose();
		transform3 = bodyHandler->rightForarm->getGlobalPose();
		part = bodyHandler->rightArm;
		partRigidbody = bodyHandler->rightForarm;
		zero = -bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 2.f;
		// 		bodyHandeler->rightArm.PartInteractable.damageModifier = InteractableObject.Damage.Default;
		// 		bodyHandeler->rightForarm.PartInteractable.damageModifier = InteractableObject.Damage.Default;
		// 		bodyHandeler->rightHand.PartInteractable.damageModifier = InteractableObject.Damage.Default;

	}
	else
	{
		transform2 = bodyHandler->leftArm->getGlobalPose();
		transform3 = bodyHandler->leftForarm->getGlobalPose();
		part = bodyHandler->leftArm;
		partRigidbody = bodyHandler->leftForarm;
		zero = bodyHandler->chest->getGlobalPose().q.rotate(PxLeft) / 2.f;
		// 		bodyHandeler->leftArm.PartInteractable.damageModifier = InteractableObject.Damage.Default;
		// 		bodyHandeler->leftForarm.PartInteractable.damageModifier = InteractableObject.Damage.Default;
		// 		bodyHandeler->leftHand.PartInteractable.damageModifier = InteractableObject.Damage.Default;
	}
	AlignToVector(part, transform2.q.rotate(PxUp), zero + partTransform.q.rotate(PxForward), 0.01f, 2.f);
	AlignToVector(partRigidbody, transform3.q.rotate(PxUp), zero + -partTransform.q.rotate(PxForward), 0.01f, 2.f);
}


void MovementHandler::HeadActionHeadbutting()
{
	PxVec3 zero = PxVec3(0);
	PxTransform transform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2 = bodyHandler->head->getGlobalPose();
	PxRigidDynamic* rigidbody = bodyHandler->chest;
	PxRigidDynamic* rigidbody2 = bodyHandler->head;
	PxRigidBody* cUpperIntrest = s->cUpperIntrest;
	//transform2.tag = "Body (Harmful)";
	if (s->cUpperIntrest == NULL)
	{
		zero = (((transform.p + transform.q.rotate(PxForward)) + ((transform.q.rotate(PxUp) / 2.f))) - transform2.p).getNormalized();
		rigidbody->addForce((-zero * 2.f), PxForceMode::eVELOCITY_CHANGE);
		rigidbody2->addForce((zero * 4.f), PxForceMode::eVELOCITY_CHANGE);
	}
	else
	{
		PxVec3 d = cUpperIntrest->getWorldBounds().getCenter() - transform2.p;
		zero = d.getNormalized();
		if (PxSqrt(d.x * d.x + d.y * d.y + d.z * d.z) > 0.5f)
		{
			rigidbody->addForce((-zero * 4.f), PxForceMode::eVELOCITY_CHANGE);
			rigidbody2->addForce((zero * 6.f), PxForceMode::eVELOCITY_CHANGE);
		}
		else
		{
			rigidbody->addForce((-zero * 4.f), PxForceMode::eVELOCITY_CHANGE);
			rigidbody2->addForce((zero * 12.f), PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void MovementHandler::HeadActionReadying()
{
	PxVec3 zero = PxVec3(0);
	PxTransform transform = bodyHandler->chest->getGlobalPose();
	PxTransform transform2 = bodyHandler->head->getGlobalPose();
	PxRigidDynamic* rigidbody = bodyHandler->chest;
	PxRigidDynamic* rigidbody2 = bodyHandler->head;
	PxRigidBody* cUpperIntrest = s->cUpperIntrest;
	if (s->cUpperIntrest == NULL)
	{
		zero = (((transform.p + transform.q.rotate(PxForward)) + ((transform.q.rotate(PxUp) / 2.f))) - transform2.p).getNormalized();
		rigidbody2->addForce((-transform.q.rotate(PxForward) * 2.f), PxForceMode::eVELOCITY_CHANGE);
	}
	else
	{
		PxVec3 d = cUpperIntrest->getWorldBounds().getCenter() - transform2.p;
		zero = d.getNormalized();
		if (PxSqrt(d.x * d.x + d.y * d.y + d.z * d.z) > 0.9f)
		{
			rigidbody2->addForce((-zero * 2.f), PxForceMode::eVELOCITY_CHANGE);
		}
		else
		{
			rigidbody2->addForce((-zero * 2.f), PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void MovementHandler::HeadActionResetting()
{
	//PxTransform transform = bodyHandler->head;
	// transform.tag = "Body (High Priority)";
}
