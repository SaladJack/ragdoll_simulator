#include "PxPhysicsAPI.h"

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"
#include "../SnippetUtils/SnippetUtils.h"
#include <math.h>
#include <string>
#include <vector>
#include "Actor.h"
#include "RagDollMath.h"



Actor* actor;
std::vector<Actor*> pcActors;
std::vector<Actor*> totalActor;
std::vector<PxVec3> pcActorsPos =
{
 	PxVec3(2.0f, 0.f, 2.0f),
	PxVec3(-2.0f,0.f,-1.0f),
	PxVec3(-2.5f,0.f,-1.5f),
// 	PxVec3(0.f,0.f,-2.f),
// 	PxVec3(-3.f,0.f,2.0f),
// 	PxVec3(-2.f,0.f,5.0f),
// 	PxVec3(-4.f,0.f,-5.0f),
// 	PxVec3(-5.f,0.f,-2.0f),
// 	PxVec3(-3.f,0.f,-3.0f),
// 	PxVec3(9.22598545333f,0.f,9.30229571486f),
// 	PxVec3(-5.23768311283f,0.f,-8.1240132685f),
// 	PxVec3(-0.85532513853f,0.f,-12.6584166422f),
// 	PxVec3(-12.2318076553f,0.f,1.89217101435f),
// 	PxVec3(-13.5421525435f,0.f,-14.6742329515f),
// 	PxVec3(11.8586799465f,0.f,14.1335616898f),
};

int gPcNum =  (int)pcActorsPos.size();

PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) && PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	
	
	// 筛出id位
	PxU32 fw0 = filterData0.word0 & ACTOR_FILTER_SHIFT;
	PxU32 fw1 = filterData0.word1 & ACTOR_FILTER_SHIFT;
	PxU32 sw0 = filterData1.word0 & ACTOR_FILTER_SHIFT;
	PxU32 sw1 = filterData1.word1 & ACTOR_FILTER_SHIFT;
	
	// 两物体属于同一Ragdoll
	if (fw0 != 0 && sw0 != 0 && fw0 == sw0)
	{
		// @See PxHandler::IgnoreCollision
		if (filterData0.word1 && filterData1.word1 && filterData0.word0 != filterData1.word0)
		{
			if ((filterData0.word0 & ~filterData1.word1) && (filterData1.word0 & ~filterData0.word1))
			{
				return PxFilterFlag::eSUPPRESS;
			}
		}
	}

	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{   
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	return PxFilterFlag::eDEFAULT;
}


void createObstacles();


class RagDollSimulationEventCallback : public PxSimulationEventCallback
{
	// Implements PxSimulationEventCallback
	virtual void							onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void							onTrigger(PxTriggerPair* pairs, PxU32 count) {}
	virtual void							onConstraintBreak(PxConstraintInfo*, PxU32) {}
	virtual void							onWake(PxActor**, PxU32) {}
	virtual void							onSleep(PxActor**, PxU32) {}
};

void RagDollSimulationEventCallback::onContact(const PxContactPairHeader & pairHeader, const PxContactPair * pairs, PxU32 nbPairs)
{

	for (PxU32 i = 0; i < nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];

		// 对应Unity中的OnCollisionEnter
		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			for(Actor* a : totalActor)
				a->OnCollisionEnter(pairHeader, cp);
		}

		// 对应Unity中的OnCollisionStay
		else if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
		{
			for (Actor* a : totalActor)
				a->OnCollisionStay(pairHeader, cp);
		}

		// 对应Unity中的OnCollisionExit
		else if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			for (Actor* a : totalActor)
				a->OnCollisionExit(pairHeader, cp);
		}

	}
}

void initPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gPxHandler->Init(FilterShader, new RagDollSimulationEventCallback());
	
	// 创建ragdoll
	actor = new Actor();
	actor->Init();
	
	totalActor.push_back(actor);

	for (int i = 0; i < gPcNum; ++i)
	{
		Actor* a = new Actor(pcActorsPos[i], true);
		a->Init();
		pcActors.push_back(a);
		totalActor.push_back(a);
	}

	createObstacles();

	printf("number of total actor:%d\n", (int)totalActor.size());
}


PxTransform t = PxTransform(0,0,0);
bool bCameraFollowRagdoll = false;
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gPxHandler->Update();
	//actor->Update();
	//for (Actor* a : pcActors)
	for (Actor* a : totalActor)
	{
		a->Update();
	}
	
	//if (bCameraFollowRagdoll)
	{
		if (t == PxTransform(0, 0, 0) || !(t == actor->mBodyHandler->stomach->getGlobalPose())) {
			t = actor->mBodyHandler->stomach->getGlobalPose();
			extern void updateCameraDsc(const PxTransform& pos);
			updateCameraDsc(PxTransform(t));
		}
	}
	//gPxHandler->gScene->unlockRead();
	//gPxHandler->gScene->unlockWrite();
}

void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gPxHandler->Release();
	printf("SnippetJoint done.\n");
}

void keyPress(const char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case '.':	gPxHandler->createDynamic(camera, PxSphereGeometry(0.2f), 10.0f, gPxHandler->mNormalMaterial,camera.rotate(PxVec3(0, 0, -1)) * 10);	break;
	case 'T':	bCameraFollowRagdoll = !bCameraFollowRagdoll;
	}
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}


void createObstacles()
{
	PxReal w = 0.4;

	w = 0.4;
	gPxHandler->createObstacle(PxTransform(-2.0f, w / 2, 0.f), PxBoxGeometry(w, w, w), 150.f);
	w = 1.6f;
	gPxHandler->createObstacle(PxTransform(5.f, 4.f, 0.f), PxBoxGeometry(w, w * 3.f, w), 400.f);

	gPxHandler->createChain(PxTransform(PxVec3(0.0f, 20.0f, -20.0f)), 5, PxBoxGeometry(0.5f, 0.1f, 0.1f), 0.6f,
		[](PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1) -> PxJoint* 
	{
		PxD6Joint* j = PxD6JointCreate(*gPxHandler->mPhysics, a0, t0, a1, t1);
		j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
		j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
		j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
		j->setDrive(PxD6Drive::eSLERP, PxD6JointDrive(0, 1000, FLT_MAX, true));
		return j;
	});

	for (int i = 0; i < gPxHandler->mObstacles.size(); ++i)
	{
		//actor->AddObstacle(gPxHandler->mObstacles[i]);
		for (Actor* a : totalActor)
		{
			a->AddObstacle(gPxHandler->mObstacles[i]);
		}
	}
}
