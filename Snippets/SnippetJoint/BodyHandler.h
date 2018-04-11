#pragma once
#include "PxPhysicsAPI.h"
#include "CommonEnum.h"
#include "CommonPragma.h"
#include "PxHandler.h"
using namespace physx;
class BodyHandler
{
public:
	BodyHandler()
	{
		SetPxHandler(gPxHandler);
	}

	BodyHandler(PxVec3 pos = PxVec3(0))
	{
		static PxU32 id = 1;
		mPos = pos;
		mID = id++ << ACTOR_ID_SHIFT;
		SetPxHandler(gPxHandler);
	}
	void SetActor(void* a) { mActor = a; }
public:
	void SetupBodyParts();
	void SetupBodyPartsFiltering();
	void SetupBodyPartsJoints();
public:
	void createHand2forArm();
	void createForArm2Arm();
	void createArm2chest();
	void createHead2Chest();
	void createChest2Waist();
	void createWaist2Hips();
	void createStomach2Waist();
	void createCrotch2Hips();
	void createFoot2Leg();
	void createLeg2Thigh();
	void createThigh2Hips();
	void createBall2Hips();

public:
	PxRigidDynamic* head = NULL;
	PxRigidDynamic* chest = NULL;
	PxRigidDynamic* waist = NULL;
	PxRigidDynamic* stomach = NULL;
	PxRigidDynamic* hips = NULL;
	PxRigidDynamic* crotch = NULL;

	PxRigidDynamic* leftArm = NULL;
	PxRigidDynamic* leftForarm = NULL;
	PxRigidDynamic* leftHand = NULL;

	PxRigidDynamic* rightArm = NULL;
	PxRigidDynamic* rightHand = NULL;
	PxRigidDynamic* rightForarm = NULL;

	PxRigidDynamic* leftThigh = NULL;
	PxRigidDynamic* leftLeg = NULL;
	PxRigidDynamic* leftFoot = NULL;

	PxRigidDynamic* rightThigh = NULL;
	PxRigidDynamic* rightLeg = NULL;
	PxRigidDynamic* rightFoot = NULL;

	PxRigidDynamic* spring_helper = NULL;
	PxRigidDynamic* ball = NULL;

public:
	PxD6Joint* leftGrabJoint = NULL;
	PxD6Joint* rightGrabJoint = NULL;
	PxRigidBody* leftGrabRigidbody = NULL;
	PxRigidBody* rightGrabRigidbody = NULL;
	
public:
	void InitMaterials();

private:
	PxMaterial*				hlfMaterial = NULL; // hand & leg & foot
	PxMaterial*				ballMaterial = NULL; // ball
	PxMaterial*				springHelperMaterial = NULL; // spring_helper
	PxMaterial*				defaultMaterial = NULL; // default

public:
	void SetPxPhysics(PxPhysics* p) { mPhysics = p; }
	void SetPxScene(PxScene* p) { mScene = p; }
	void SetGround(PxRigidStatic* p) { mGroundPlane = p;}
	void SetAllocator(PxDefaultAllocator m) { mAllocator = m; }
	void SetPxHandler(PxHandler* p) 
	{ 
		mPxHandler = p; 
		mPhysics = mPxHandler->mPhysics; 
		mScene = mPxHandler->mScene;
		mGroundPlane = mPxHandler->mGroundPlane;
		mAllocator = mPxHandler->mAllocator;
	}

private:
	PxPhysics*				mPhysics = NULL;
	PxScene*				mScene = NULL;
	PxRigidStatic*			mGroundPlane = NULL;
	PxDefaultAllocator		mAllocator;
	PxHandler*				mPxHandler = NULL;
	PxVec3					mPos = PxVec3(0);
	PxU32					mID = 1 << ACTOR_ID_SHIFT;
	void*					mActor = NULL;
};
//#define gBodyHandler BodyHandler::GetInstance()
