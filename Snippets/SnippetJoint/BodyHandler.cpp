#include "BodyHandler.h"
#include "RagDollCfg.h"
#include "RagDollMath.h"
#include "JointDescriptions.h"
#include "CommonPragma.h"
#include "RdObject.h"

void BodyHandler::InitMaterials()
{
	hlfMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.f);
	ballMaterial = mPhysics->createMaterial(1.f, 1.f, 0.f);
	springHelperMaterial = mPhysics->createMaterial(0.6f, 0.6f, 0.f);
	defaultMaterial = mPhysics->createMaterial(0.f, 0.f, 0.f);

	{
		hlfMaterial->setRestitutionCombineMode(PxCombineMode::eAVERAGE); // bounceCombine
		hlfMaterial->setRestitution(0.f); // bounciness
		hlfMaterial->setDynamicFriction(0.5f); // dynamicFriction
		hlfMaterial->setFrictionCombineMode(PxCombineMode::eMIN); // frictionCombine
		hlfMaterial->setStaticFriction(0.5f); // staticFriction
	}

	{
		ballMaterial->setRestitutionCombineMode(PxCombineMode::eMIN); // bounceCombine
		ballMaterial->setRestitution(0.f); // bounciness
		ballMaterial->setDynamicFriction(1.f);
		ballMaterial->setFrictionCombineMode(PxCombineMode::eAVERAGE);
		ballMaterial->setStaticFriction(1.f);
	}

	{
		springHelperMaterial->setRestitutionCombineMode(PxCombineMode::eAVERAGE); // bounceCombine
		springHelperMaterial->setRestitution(0.f); // bounciness
		springHelperMaterial->setDynamicFriction(0.6f);
		springHelperMaterial->setFrictionCombineMode(PxCombineMode::eAVERAGE);
		springHelperMaterial->setStaticFriction(0.6f);
	}


	{
		defaultMaterial->setRestitutionCombineMode(PxCombineMode::eAVERAGE); // bounceCombine
		defaultMaterial->setRestitution(0.f); // bounciness
		defaultMaterial->setDynamicFriction(0.f);
		defaultMaterial->setFrictionCombineMode(PxCombineMode::eMIN);
		defaultMaterial->setStaticFriction(0.f);
	}
}

void BodyHandler::SetupBodyParts()
{
	head =  gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + HEAD_POS_Y, mPos.z), PxSphereGeometry(HEAD_RADIUS), 5.0, defaultMaterial);

	chest = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + CHEST_POS_Y, mPos.z), PxCapsuleGeometry(CHEST_CAPSULE_RAIDUS, CHEST_CAPSULE_HALF_LEN), 10.0, defaultMaterial);

	waist = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + WAIST_POS_Y, mPos.z), PxCapsuleGeometry(WAIST_CAPSULE_RADIUS, WAIST_CAPSULE_HALF_LEN), 10.0, defaultMaterial);

	stomach = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + STOMACH_POS_Y, mPos.z + STOMACH_POS_Z), PxCapsuleGeometry(STOMACH_CAPSULE_RADIUS, STOMACH_CAPSULE_HALF_LEN), 5.0, defaultMaterial);

	hips = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + HIPS_POS_Y, mPos.z), PxCapsuleGeometry(HIPS_CAPSULE_RADIUS, HIPS_CAPSULE_HALF_LEN), 10.0, defaultMaterial);

	crotch = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + CROTCH_POS_Y, mPos.z + CROTCH_POS_Z), PxCapsuleGeometry(CROTCH_CAPSULE_RADIUS, CROTCH_CAPSULE_HALF_LEN), 5.0, defaultMaterial);

	leftArm = gPxHandler->createArmPos(PxVec3(mPos.x + ARM_POS_X, mPos.y + ARM_POS_Y, mPos.z), PxCapsuleGeometry(ARM_CAPSULE_RADIUS, ARM_CAPSULE_HALF_LEN), 5.0, Side::Left, defaultMaterial);

	leftForarm = gPxHandler->createArmPos(PxVec3(mPos.x + FOR_ARM_POS_X, mPos.y + FOR_ARM_POS_Y, mPos.z), PxCapsuleGeometry(FOR_ARM_CAPSULE_RADIUS, FOR_ARM_CAPSULE_HALF_LEN), 5.0, Side::Left, defaultMaterial);

	leftHand = gPxHandler->createArmPos(PxVec3(mPos.x + HAND_POS_X, mPos.y + HAND_POS_Y, mPos.z), PxSphereGeometry(HAND_RADIUS), 5.0, Side::Left, hlfMaterial);

	leftThigh = gPxHandler->createDynamicRelativePos(PxTransform(mPos.x + THIGH_POS_X, mPos.y + THIGH_POS_Y, mPos.z), PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))), PxCapsuleGeometry(THIGH_RADIUS, THIGH_HALF_HEIGHT), 5.0, defaultMaterial);

	leftLeg = gPxHandler->createDynamicRelativePos(PxTransform(mPos.x + LEG_POS_X, mPos.y + LEG_POS_Y, mPos.z), PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))), PxCapsuleGeometry(LEG_RADIUS, LEG_HALF_HEIGHT), 5.0, hlfMaterial);

	leftFoot = gPxHandler->createDynamic(PxTransform(mPos.x + FOOT_POS_X, mPos.y + FOOT_POS_Y, mPos.z), PxSphereGeometry(FOOT_RADIUS), 5.0, hlfMaterial);

	rightArm = gPxHandler->createArmPos(PxVec3(mPos.x + -ARM_POS_X, mPos.y + ARM_POS_Y, mPos.z), PxCapsuleGeometry(ARM_CAPSULE_RADIUS, ARM_CAPSULE_HALF_LEN), 5.0, Side::Right, defaultMaterial);

	rightForarm = gPxHandler->createArmPos(PxVec3(mPos.x + -FOR_ARM_POS_X, mPos.y + FOR_ARM_POS_Y, mPos.z), PxCapsuleGeometry(FOR_ARM_CAPSULE_RADIUS, FOR_ARM_CAPSULE_HALF_LEN), 5.0, Side::Right, defaultMaterial);

	rightHand = gPxHandler->createArmPos(PxVec3(mPos.x + -HAND_POS_X, mPos.y + HAND_POS_Y, mPos.z), PxSphereGeometry(HAND_RADIUS), 5.0, Side::Right, hlfMaterial);

	rightThigh = gPxHandler->createDynamicRelativePos(PxTransform(mPos.x + -THIGH_POS_X, mPos.y + THIGH_POS_Y, mPos.z), PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))), PxCapsuleGeometry(THIGH_RADIUS, THIGH_HALF_HEIGHT), 5.0, defaultMaterial);

	rightLeg = gPxHandler->createDynamicRelativePos(PxTransform(mPos.x + -LEG_POS_X, mPos.y + LEG_POS_Y, mPos.z), PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))), PxCapsuleGeometry(LEG_RADIUS, LEG_HALF_HEIGHT), 5.0, hlfMaterial);

	rightFoot = gPxHandler->createDynamic(PxTransform(mPos.x + -FOOT_POS_X, mPos.y + FOOT_POS_Y, mPos.z), PxSphereGeometry(FOOT_RADIUS), 5.0, hlfMaterial);

	spring_helper = gPxHandler->createDynamicEmptyShape(PxTransform(mPos.x, mPos.y + BALL_POS_Y, mPos.z + BALL_POS_Z), 10.0, springHelperMaterial);

	ball = gPxHandler->createDynamic(PxTransform(mPos.x, mPos.y + BALL_POS_Y, mPos.z + BALL_POS_Z), PxSphereGeometry(BALL_RADIUS), 10.0, ballMaterial);
}

void BodyHandler::SetupBodyPartsFiltering()
{
	// FilterMask
	gPxHandler->SetFilterMask(head, FilterGroup::eHead | mID);
	gPxHandler->SetFilterMask(chest, FilterGroup::eChest | mID);
	gPxHandler->SetFilterMask(waist, FilterGroup::eWaist | mID);
	gPxHandler->SetFilterMask(stomach, FilterGroup::eStomach | mID);
	gPxHandler->SetFilterMask(hips, FilterGroup::eHips | mID);
	gPxHandler->SetFilterMask(crotch, FilterGroup::eCrotch | mID);
	gPxHandler->SetFilterMask(ball, FilterGroup::eBall | mID);

	gPxHandler->SetFilterMask(rightArm, FilterGroup::eRightArm | mID);
	gPxHandler->SetFilterMask(rightForarm, FilterGroup::eRightForArm | mID);
	gPxHandler->SetFilterMask(rightHand, FilterGroup::eRightHand | mID);
	gPxHandler->SetFilterMask(rightThigh, FilterGroup::eRightThigh | mID);
	gPxHandler->SetFilterMask(rightLeg, FilterGroup::eRightLeg | mID);

	gPxHandler->SetFilterMask(leftArm, FilterGroup::eLeftArm | mID);
	gPxHandler->SetFilterMask(leftForarm, FilterGroup::eLeftForArm | mID);
	gPxHandler->SetFilterMask(leftHand, FilterGroup::eLeftHand | mID);
	gPxHandler->SetFilterMask(leftThigh, FilterGroup::eLeftThigh | mID);
	gPxHandler->SetFilterMask(leftLeg, FilterGroup::eLeftLeg | mID);

	gPxHandler->SetFilterMask(leftFoot, FilterGroup::eLeftFoot | mID);
	gPxHandler->SetFilterMask(rightFoot, FilterGroup::eRightFoot | mID);
	gPxHandler->SetFilterMask(spring_helper, FilterGroup::eSpringHelper | mID);

	// userData
// 	head->userData = new RdObject(FilterGroup::eHead | mID);
// 	chest->userData = new RdObject(FilterGroup::eChest | mID);
// 	waist->userData = new RdObject(FilterGroup::eWaist | mID);
// 	stomach->userData = new RdObject(FilterGroup::eStomach | mID);
// 	hips->userData = new RdObject(FilterGroup::eHips | mID);
// 	crotch->userData = new RdObject(FilterGroup::eCrotch | mID);
// 	ball->userData = new RdObject(FilterGroup::eBall | mID);
// 
// 	rightArm->userData = new RdObject(FilterGroup::eRightArm | mID);
// 	rightForarm->userData = new RdObject(FilterGroup::eRightForArm | mID);
// 	rightHand->userData = new RdObject(FilterGroup::eRightHand | mID);
// 	rightThigh->userData = new RdObject(FilterGroup::eRightThigh | mID);
// 	rightLeg->userData = new RdObject(FilterGroup::eRightLeg | mID);
// 
// 	leftArm->userData = new RdObject(FilterGroup::eLeftArm | mID);
// 	leftForarm->userData = new RdObject(FilterGroup::eLeftForArm | mID);
// 	leftHand->userData = new RdObject(FilterGroup::eLeftHand | mID);
// 	leftThigh->userData = new RdObject(FilterGroup::eLeftThigh | mID);
// 	leftLeg->userData = new RdObject(FilterGroup::eLeftLeg | mID);
// 
// 	leftFoot->userData = new RdObject(FilterGroup::eLeftFoot | mID);
// 	rightFoot->userData = new RdObject(FilterGroup::eRightFoot | mID);
// 	spring_helper->userData = new RdObject(FilterGroup::eSpringHelper | mID);

	head->userData = new RdObject(mActor, FilterGroup::eHead | mID);
	chest->userData = new RdObject(mActor, FilterGroup::eChest | mID);
	waist->userData = new RdObject(mActor, FilterGroup::eWaist | mID);
	stomach->userData = new RdObject(mActor, FilterGroup::eStomach | mID);
	hips->userData = new RdObject(mActor, FilterGroup::eHips | mID);
	crotch->userData = new RdObject(mActor, FilterGroup::eCrotch | mID);
	ball->userData = new RdObject(mActor, FilterGroup::eBall | mID);

	rightArm->userData = new RdObject(mActor, FilterGroup::eRightArm | mID);
	rightForarm->userData = new RdObject(mActor, FilterGroup::eRightForArm | mID);
	rightHand->userData = new RdObject(mActor, FilterGroup::eRightHand | mID);
	rightThigh->userData = new RdObject(mActor, FilterGroup::eRightThigh | mID);
	rightLeg->userData = new RdObject(mActor, FilterGroup::eRightLeg | mID);

	leftArm->userData = new RdObject(mActor, FilterGroup::eLeftArm | mID);
	leftForarm->userData = new RdObject(mActor, FilterGroup::eLeftForArm | mID);
	leftHand->userData = new RdObject(mActor, FilterGroup::eLeftHand | mID);
	leftThigh->userData = new RdObject(mActor, FilterGroup::eLeftThigh | mID);
	leftLeg->userData = new RdObject(mActor, FilterGroup::eLeftLeg | mID);

	leftFoot->userData = new RdObject(mActor, FilterGroup::eLeftFoot | mID);
	rightFoot->userData = new RdObject(mActor, FilterGroup::eRightFoot | mID);
	spring_helper->userData = new RdObject(mActor, FilterGroup::eSpringHelper | mID);

	// IgnoreCollision
	gPxHandler->IgnoreCollision(head, chest);

	gPxHandler->IgnoreCollision(stomach, crotch);
	gPxHandler->IgnoreCollision(stomach, chest);
	gPxHandler->IgnoreCollision(stomach, waist);
	gPxHandler->IgnoreCollision(stomach, hips);
	gPxHandler->IgnoreCollision(stomach, leftThigh);
	gPxHandler->IgnoreCollision(stomach, rightThigh);

	gPxHandler->IgnoreCollision(leftForarm, leftArm);
	gPxHandler->IgnoreCollision(leftForarm, leftHand);
	gPxHandler->IgnoreCollision(leftForarm, chest);

	gPxHandler->IgnoreCollision(leftArm, leftHand);
	gPxHandler->IgnoreCollision(leftArm, chest);
	gPxHandler->IgnoreCollision(leftArm, waist);
	
	gPxHandler->IgnoreCollision(rightForarm, rightArm);
	gPxHandler->IgnoreCollision(rightForarm, rightHand);
	gPxHandler->IgnoreCollision(rightForarm, chest);

	gPxHandler->IgnoreCollision(rightArm, rightHand);
	gPxHandler->IgnoreCollision(rightArm, chest);
	gPxHandler->IgnoreCollision(rightArm, waist);

	gPxHandler->IgnoreCollision(chest, head);
	gPxHandler->IgnoreCollision(chest, leftArm);
	gPxHandler->IgnoreCollision(chest, leftForarm);
	gPxHandler->IgnoreCollision(chest, rightArm);
	gPxHandler->IgnoreCollision(chest, rightForarm);
	gPxHandler->IgnoreCollision(chest, stomach);
	gPxHandler->IgnoreCollision(chest, waist);
	gPxHandler->IgnoreCollision(chest, crotch);

	gPxHandler->IgnoreCollision(waist, crotch);
	gPxHandler->IgnoreCollision(waist, stomach);
	gPxHandler->IgnoreCollision(waist, chest);

	gPxHandler->IgnoreCollision(hips, chest);
	gPxHandler->IgnoreCollision(hips, waist);
	gPxHandler->IgnoreCollision(hips, stomach);
	gPxHandler->IgnoreCollision(hips, crotch);
	gPxHandler->IgnoreCollision(hips, leftThigh);
	gPxHandler->IgnoreCollision(hips, leftLeg);
	gPxHandler->IgnoreCollision(hips, rightThigh);
	gPxHandler->IgnoreCollision(hips, rightLeg);
	
	gPxHandler->IgnoreCollision(leftThigh, stomach);
	gPxHandler->IgnoreCollision(leftThigh, crotch);
	gPxHandler->IgnoreCollision(leftThigh, leftLeg);
	gPxHandler->IgnoreCollision(leftThigh, hips);
	gPxHandler->IgnoreCollision(leftThigh, ball);

	gPxHandler->IgnoreCollision(rightThigh, stomach);
	gPxHandler->IgnoreCollision(rightThigh, crotch);
	gPxHandler->IgnoreCollision(rightThigh, rightLeg);
	gPxHandler->IgnoreCollision(rightThigh, hips);
	gPxHandler->IgnoreCollision(rightThigh, ball);
	
	gPxHandler->IgnoreCollision(ball, hips);
	gPxHandler->IgnoreCollision(ball, crotch);
	gPxHandler->IgnoreCollision(ball, leftThigh);
	gPxHandler->IgnoreCollision(ball, rightThigh);
	gPxHandler->IgnoreCollision(ball, stomach);
	gPxHandler->IgnoreCollision(ball, waist);
	gPxHandler->IgnoreCollision(ball, leftLeg);
	gPxHandler->IgnoreCollision(ball, rightLeg);

	gPxHandler->IgnoreCollision(crotch, ball);
	gPxHandler->IgnoreCollision(crotch, leftThigh);
	gPxHandler->IgnoreCollision(crotch, rightThigh);

	gPxHandler->IgnoreCollision(spring_helper, leftThigh);
	gPxHandler->IgnoreCollision(spring_helper, leftLeg);
	

	// AddCollision
	gPxHandler->AddCollision(mGroundPlane, leftFoot);
	gPxHandler->AddCollision(mGroundPlane, rightFoot);
	gPxHandler->AddCollision(mGroundPlane, ball);
	
	gPxHandler->AddCollision(crotch, leftForarm);
	gPxHandler->AddCollision(crotch, rightForarm);
}

void BodyHandler::SetupBodyPartsJoints()
{
	createHand2forArm();
	createForArm2Arm();
	createArm2chest();

	createHead2Chest();

	createChest2Waist();
	createWaist2Hips();

	createStomach2Waist();
	createCrotch2Hips();

	createFoot2Leg();
	createLeg2Thigh();
	createThigh2Hips();
	createBall2Hips();
}



void BodyHandler::createHead2Chest()
{
	PxD6Joint* j = gPxHandler->createD6Joint(head, PxTransform(0, -HEAD_RADIUS*0.8f, 0), chest, PxTransform(0, CHEST_CAPSULE_RAIDUS*0.8f, CHEST_CAPSULE_RAIDUS*0.5f, PxQuat(PxPi / 5, PxVec3(1, 0, 0))));
	gPxHandler->setJointMotion(j, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setDefaultJointDesc(j, 40.f, 40.f, -40.f, 40.f);
}

void BodyHandler::createHand2forArm()
{
	PxReal handOffset = HAND_RADIUS*0.5f;
	PxReal forarmOffset = -(FOR_ARM_CAPSULE_HALF_LEN + FOR_ARM_CAPSULE_RADIUS*0.8f);
	PxD6Joint *lj = gPxHandler->createD6Joint(leftHand, PxTransform(0, handOffset, 0), leftForarm, PxTransform(0, forarmOffset, 0));
	PxD6Joint *rj = gPxHandler->createD6Joint(rightHand, PxTransform(0, handOffset, 0), rightForarm, PxTransform(0, forarmOffset, 0));
	gPxHandler->setJointMotion(lj, PxD6Motion::eFREE, PxD6Motion::eLOCKED);
	gPxHandler->setJointMotion(rj, PxD6Motion::eFREE, PxD6Motion::eLOCKED);
}

void BodyHandler::createForArm2Arm()
{
	PxReal forarmOffset = (FOR_ARM_CAPSULE_HALF_LEN + FOR_ARM_CAPSULE_RADIUS) * 0.8f;
	PxReal armOffset = -(ARM_CAPSULE_HALF_LEN + ARM_CAPSULE_RADIUS)*0.6f;
	//PxD6Joint *lj = gPxHandler->createD6Joint(leftForarm, PxTransform(0, forarmOffset, 0), leftArm, PxTransform(0, armOffset, 0));
	PxD6Joint *lj = gPxHandler->createD6Joint(leftForarm, PxTransform(0, forarmOffset, 0), leftArm, PxTransform(0, armOffset, 0));
	PxD6Joint *rj = gPxHandler->createD6Joint(rightForarm, PxTransform(0, forarmOffset, 0), rightArm, PxTransform(0, armOffset, 0));

	gPxHandler->setJointMotion(lj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	lj->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
	gPxHandler->setJointMotion(rj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	rj->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);

	// 	{
	// 		PxTransform localFrame0 = lj->getLocalPose(PxJointActorIndex::eACTOR0);
	// 		PxVec3 localAxis = PxVec3(1, 0, 0);
	// 		PxVec3 localNormal = PxVec3(0, 1, 0);
	// 		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
	// 		localFrame0.q = PxQuat(rot).getNormalized();
	// 		lj->setLocalPose(PxJointActorIndex::eACTOR0, localFrame0);
	// 	}
	// 
	// 	{
	// 		PxTransform localFrame1 = lj->getLocalPose(PxJointActorIndex::eACTOR1);
	// 		PxVec3 localAxis = PxVec3(1, 0, 0);
	// 		PxVec3 localNormal = PxVec3(0, 1, 0);
	// 		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
	// 		localFrame1.q = PxQuat(rot).getNormalized();
	// 		lj->setLocalPose(PxJointActorIndex::eACTOR1, localFrame1);
	// 	}

	gPxHandler->setDefaultJointDesc(lj, 0.0f, 0.0f, 0.0f, 120.0f);
	gPxHandler->setDefaultJointDesc(rj, 0.0f, 0.0f, 0.0f, 120.0f);
}

void BodyHandler::createArm2chest()
{
	//PxReal armOffset = (ARM_CAPSULE_HALF_LEN + ARM_CAPSULE_RADIUS)*0.8f;
	PxReal armOffset = ARM_CAPSULE_HALF_LEN;
	PxReal chestOffset = (CHEST_CAPSULE_HALF_LEN + CHEST_CAPSULE_RAIDUS)*0.8f;
	PxD6Joint *lj = gPxHandler->createD6Joint(leftArm, PxTransform(0, armOffset, 0), chest, PxTransform(chestOffset, CHEST_CAPSULE_RAIDUS * 0.2f, 0, PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxD6Joint *rj = gPxHandler->createD6Joint(rightArm, PxTransform(0, armOffset, 0), chest, PxTransform(-chestOffset, CHEST_CAPSULE_RAIDUS * 0.2f, 0, PxQuat(-PxHalfPi, PxVec3(0, 0, 1))));
	gPxHandler->setJointMotion(lj, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setJointMotion(rj, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	// 	lj->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	// 	rj->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	// 

	// 根据GB的Joint节点的方向设置
	// @see unity/ConfigurableJoint Axis,SecondaryAsis
	{
		PxTransform localFrame0 = lj->getLocalPose(PxJointActorIndex::eACTOR0);
		PxVec3 localAxis = PxVec3(1, 0, 0);
		PxVec3 localNormal = PxVec3(0, -1, 0);
		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		localFrame0.q = PxQuat(rot).getNormalized();
		lj->setLocalPose(PxJointActorIndex::eACTOR0, localFrame0);
	}

	{
		PxTransform localFrame1 = lj->getLocalPose(PxJointActorIndex::eACTOR1);
		PxVec3 localAxis = PxVec3(0, 1, 0);
		PxVec3 localNormal = PxVec3(1, 0, 0);
		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		localFrame1.q = PxQuat(rot).getNormalized();
		lj->setLocalPose(PxJointActorIndex::eACTOR1, localFrame1);
	}


	{
		PxTransform localFrame0 = rj->getLocalPose(PxJointActorIndex::eACTOR0);
		PxVec3 localAxis = PxVec3(-1, 0, 0);
		PxVec3 localNormal = PxVec3(0, 1, 0);
		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		localFrame0.q = PxQuat(rot).getNormalized();
		rj->setLocalPose(PxJointActorIndex::eACTOR0, localFrame0);
	}

	{
		PxTransform localFrame1 = rj->getLocalPose(PxJointActorIndex::eACTOR1);
		PxVec3 localAxis = PxVec3(0, 1, 0);
		PxVec3 localNormal = PxVec3(1, 0, 0);
		PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		localFrame1.q = PxQuat(rot).getNormalized();
		rj->setLocalPose(PxJointActorIndex::eACTOR1, localFrame1);
	}
	// 原版angularYlimit为20.f，值调大是因为用20.f时，人在行走过程中换方向偶现手臂会卡住的情况
	gPxHandler->setDefaultJointDesc(lj, 60.0f, 90.0f, -90.0f, 90.0f); 
	gPxHandler->setDefaultJointDesc(rj, 60.0f, 90.0f, -90.0f, 90.0f);
}

void BodyHandler::createChest2Waist()
{
	PxD6Joint* j = gPxHandler->createD6Joint(chest, PxTransform(0, -(CHEST_CAPSULE_RAIDUS)*0.8f, 0), waist, PxTransform(0, WAIST_CAPSULE_RADIUS*0.8f, 0));
	gPxHandler->setJointMotion(j, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setDefaultJointDesc(j, 20.f, 20.f, -40.f, 40.f);
}

void BodyHandler::createStomach2Waist()
{
	if (!stomach) return;
	//PxD6Joint* j = gPxHandler->createD6Joint(stomach, PxTransform(0, STOMACH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -STOMACH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f), waist, PxTransform(0, -WAIST_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, WAIST_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f,PxQuat(-PxPi/6,PxVec3(1,0,0))));
	//PxFixedJoint* j = PxFixedJointCreate(*gPhysics, stomach, PxTransform(0, STOMACH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -STOMACH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f), waist, PxTransform(0, -WAIST_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, WAIST_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f, PxQuat(-PxPi / 6, PxVec3(1, 0, 0))));
	PxFixedJoint* j = PxFixedJointCreate(*mPhysics, 
		stomach, 
		PxTransform(0, STOMACH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -STOMACH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f), 
		waist, 
		PxTransform(0, -WAIST_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, WAIST_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f));
	//j->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	//gPxHandler->setJointMotion(j, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	//gPxHandler->setDefaultJointDesc(j, 0, 0, 0, 0);
}

void BodyHandler::createWaist2Hips()
{
	PxD6Joint* j = gPxHandler->createD6Joint(waist, PxTransform(0, -(WAIST_CAPSULE_RADIUS)*0.8f, 0), hips, PxTransform(0, (HIPS_CAPSULE_RADIUS)*0.8f, 0));
	gPxHandler->setJointMotion(j, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setDefaultJointDesc(j, 20.f, 20.f, -40.f, 40.f);
}

void BodyHandler::createCrotch2Hips()
{
	if (!crotch) return;
	//PxD6Joint* j = gPxHandler->createD6Joint(crotch, PxTransform(0, 0, -CROTCH_CAPSULE_RADIUS*0.8f), hips, PxTransform(0, -HIPS_CAPSULE_RADIUS*0.8f, 0));
	//PxD6Joint* j = gPxHandler->createD6Joint(crotch, PxTransform(0, CROTCH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -CROTCH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f), hips, PxTransform(0, -HIPS_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.4f, HIPS_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.4f,PxQuat(-PxPi/6,PxVec3(1,0,0))));
	// 	PxFixedJoint* j = PxFixedJointCreate(*gPhysics, crotch, PxTransform(0, CROTCH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -CROTCH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f)
	// 		, hips, PxTransform(0, -HIPS_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.4f, HIPS_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.4f, PxQuat(-PxPi / 6, PxVec3(1, 0, 0))));
	PxFixedJoint* j = PxFixedJointCreate(*mPhysics, crotch, PxTransform(0, CROTCH_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.5f, -CROTCH_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f)
		, hips, PxTransform(0, -HIPS_CAPSULE_RADIUS*PxCos(PxPi / 6)*0.4f, HIPS_CAPSULE_RADIUS*PxSin(PxPi / 6)*0.5f));

	//j->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	// 	gPxHandler->setJointMotion(j, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	// 	gPxHandler->setDefaultJointDesc(j, 0, 0, 0, 0);
}

void BodyHandler::createThigh2Hips()
{
	PxReal radius = PxPi / 12;
	PxD6Joint *lj = gPxHandler->createD6Joint(leftThigh, PxTransform(0, THIGH_HALF_HEIGHT, 0), hips, PxTransform(HIPS_CAPSULE_HALF_LEN + HIPS_CAPSULE_RADIUS*PxCos(radius)*0.8f, -HIPS_CAPSULE_RADIUS*PxSin(radius)*0.8f, 0, PxQuat(PxPi / 18, PxVec3(0, 0, 1))));
	PxD6Joint *rj = gPxHandler->createD6Joint(rightThigh, PxTransform(0, THIGH_HALF_HEIGHT, 0), hips, PxTransform(-(HIPS_CAPSULE_HALF_LEN + HIPS_CAPSULE_RADIUS*PxCos(radius)*0.8f), -HIPS_CAPSULE_RADIUS*PxSin(radius)*0.8f, 0, PxQuat(-PxPi / 18, PxVec3(0, 0, 1))));
// 	PxD6Joint *lj = gPxHandler->createD6Joint(leftThigh, PxTransform(0, THIGH_RADIUS*0.5f, 0), hips, PxTransform(HIPS_CAPSULE_HALF_LEN + HIPS_CAPSULE_RADIUS*PxCos(radius)*0.8f, -HIPS_CAPSULE_RADIUS*PxSin(radius)*0.8f, 0));
// 	PxD6Joint *rj = gPxHandler->createD6Joint(rightThigh, PxTransform(0, THIGH_RADIUS*0.5f, 0), hips, PxTransform(-(HIPS_CAPSULE_HALF_LEN + HIPS_CAPSULE_RADIUS*PxCos(radius)*0.8f), -HIPS_CAPSULE_RADIUS*PxSin(radius)*0.8f,0));
// 	PxD6Joint *lj = gPxHandler->createD6Joint(leftThigh, PxTransform(0, THIGH_HALF_HEIGHT+THIGH_RADIUS, 0), hips, PxTransform(HIPS_CAPSULE_HALF_LEN, 0, 0));
// 	PxD6Joint *rj = gPxHandler->createD6Joint(rightThigh, PxTransform(0, THIGH_HALF_HEIGHT + THIGH_RADIUS, 0), hips, PxTransform(-HIPS_CAPSULE_HALF_LEN, 0, 0));
	gPxHandler->setJointMotion(lj, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setJointMotion(rj, PxD6Motion::eLIMITED, PxD6Motion::eLOCKED);
	gPxHandler->setDefaultJointDesc(lj, 20.0f, 20.0f, -80.0, 80.0f);
	gPxHandler->setDefaultJointDesc(rj, 20.0f, 20.0f, -80.0, 80.0f);
}

void BodyHandler::createLeg2Thigh()
{
// 	PxD6Joint *lj = gPxHandler->createD6Joint(leftLeg, PxTransform(0, (LEG_HALF_HEIGHT), 0), leftThigh, PxTransform(0, -(THIGH_HALF_HEIGHT + THIGH_RADIUS*0.5f), 0, PxQuat(-PxPi / 18, PxVec3(0, 0, 1))));
// 	PxD6Joint *rj = gPxHandler->createD6Joint(rightLeg, PxTransform(0, (LEG_HALF_HEIGHT), 0), rightThigh, PxTransform(0, -(THIGH_HALF_HEIGHT + THIGH_RADIUS*0.5f), 0, PxQuat(PxPi / 18, PxVec3(0, 0, 1))));
	PxD6Joint *lj = gPxHandler->createD6Joint(leftLeg, PxTransform(0, (LEG_HALF_HEIGHT), 0), leftThigh, PxTransform(0, -(THIGH_HALF_HEIGHT + THIGH_RADIUS*0.5f), 0));
	PxD6Joint *rj = gPxHandler->createD6Joint(rightLeg, PxTransform(0, (LEG_HALF_HEIGHT), 0), rightThigh, PxTransform(0, -(THIGH_HALF_HEIGHT + THIGH_RADIUS*0.5f), 0));
	gPxHandler->setJointMotion(lj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	lj->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);

	gPxHandler->setJointMotion(rj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	rj->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
	gPxHandler->setDefaultJointDesc(lj, 0.0f, 0.0f, -90.0, 0.0f);
	gPxHandler->setDefaultJointDesc(rj, 0.0f, 0.0f, -90.0, 0.0f);

}

void BodyHandler::createFoot2Leg()
{
	PxD6Joint* lj = gPxHandler->createD6Joint(leftFoot, PxTransform::createIdentity(), leftLeg, PxTransform(0, -(LEG_HALF_HEIGHT + LEG_RADIUS*0.5), 0));
	PxD6Joint* rj = gPxHandler->createD6Joint(rightFoot, PxTransform::createIdentity(), rightLeg, PxTransform(0, -(LEG_HALF_HEIGHT + LEG_RADIUS*0.5), 0));
	gPxHandler->setJointMotion(lj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
	gPxHandler->setJointMotion(rj, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
}
void BodyHandler::createBall2Hips()
{
	if (!ball) return;
	// 	PxD6Joint* j = gPxHandler->createD6Joint(ball, PxTransform(0, BALL_RADIUS*0.5, 0), hips, PxTransform(0, -HIPS_CAPSULE_RADIUS*0.3, 0));
	// 	gPxHandler->setJointMotion(j, PxD6Motion::eFREE, PxD6Motion::eLOCKED);
	//gPxHandler->setDefaultJointDesc(j, 0, 0, 0, 0);
	{
		PxD6Joint* j = gPxHandler->createD6Joint(ball, PxTransform::createIdentity(), spring_helper, PxTransform::createIdentity());
		gPxHandler->setJointMotion(j, PxD6Motion::eFREE, PxD6Motion::eLOCKED);
		gPxHandler->setDefaultJointDesc(j, 0, 0, 0, 0);
	}

	{
		// 		PxD6Joint* j = gPxHandler->createD6Joint(spring_helper, PxTransform(0, 0, 0), crotch, PxTransform(0, 0, -CROTCH_CAPSULE_RADIUS));
		// 		gPxHandler->setJointMotion(j, PxD6Motion::eLOCKED, PxD6Motion::eLOCKED);
		// 		gPxHandler->setDefaultJointDesc(j, 0, 0, 0, 0);
		PxFixedJoint* j = PxFixedJointCreate(*mPhysics, spring_helper, PxTransform(0, 0, 0), crotch, PxTransform(0, CROTCH_CAPSULE_RADIUS * 0.1f, -CROTCH_CAPSULE_RADIUS*0.8));
	}
}

