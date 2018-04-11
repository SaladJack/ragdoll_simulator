#pragma once
#include "PxPhysicsAPI.h"
#include "CommonEnum.h"
#include <vector>
using namespace physx;
typedef PxJoint* (*JointCreateFunction)(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1);
// 用来管理Physx的一些基础配置及场景里通用的物体
class PxHandler
{
public:
	static PxHandler* GetInstance()
	{
		static PxHandler* m_pInstance;
		if (m_pInstance == NULL)
			m_pInstance = new PxHandler();
		return m_pInstance;
	}
public:
	void Init(PxSimulationFilterShader fs, PxSimulationEventCallback* callback);
	void Update();
	void Release();
public:
	PxRigidDynamic* createDynamicRelativePos(const PxTransform& t, const PxTransform& relativePose, const PxGeometry& geometry, PxReal mass, const PxMaterial* material, const PxVec3& velocity = PxVec3(0));
	PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxReal& mass, const PxMaterial* material, const PxVec3& velocity = PxVec3(0));
	PxRigidDynamic* createDynamicEmptyShape(const PxTransform& t, PxReal mass, const PxMaterial* material);
	PxRigidDynamic* createDynamicDensity(const PxTransform& t, const PxGeometry& geometry, const PxReal& density = 10.0f, const PxVec3& velocity = PxVec3(0));
	PxRigidDynamic* createObstacle(const PxTransform& t, const PxGeometry& geometry, const PxReal& density = 10.0f, const PxVec3& velocity = PxVec3(0));
	PxRigidDynamic* createArmPos(const PxVec3& p, const PxGeometry& geometry, PxReal mass, Side side, const PxMaterial* material);

public:
	PxD6Joint* createD6Joint(PxRigidDynamic* actor0, const PxTransform& tRelativeToActor0, PxRigidDynamic* actor1, const PxTransform& tRelativeToActor1);
	PxD6Joint* createD6JointAutomatically(PxRigidDynamic* actor0, const PxTransform& tRelativeToActor0, PxRigidActor* actor1);
	PxD6Joint* setDefaultJointDesc(PxD6Joint* j, float angularYLimit, float angularZLimit, float lowAngularXLimit, float highAngularXLimit);
	PxD6Joint* setJointMotion(PxD6Joint* j, PxD6Motion::Enum angularMotion, PxD6Motion::Enum motion);
	void createChain(const PxTransform& t, PxU32 length, const PxGeometry& g, PxReal separation, JointCreateFunction createJoint);
public:
	void SetFilterMask(PxRigidActor* actor, PxU32 fm);
	void AddFilterData(PxRigidActor* actor0, PxU32 fm, PxU32 fg, bool bIntersect = true);
	void IgnoreCollision(PxRigidActor* actor0, PxU32 fg0, PxRigidActor* actor1, PxU32 fg1);
	void IgnoreCollision(PxRigidActor* actor0, PxRigidActor* actor1);
	PxFilterData GetFilterData(PxRigidActor* actor);
	void AddCollision(PxRigidActor* actor0, PxRigidActor* actor1);
	void AddCollision(PxRigidActor* actor0, PxU32 fg0, PxRigidActor* actor1, PxU32 fg1);

public:
	PxDefaultAllocator		mAllocator;
	PxDefaultErrorCallback	mErrorCallback;

	PxFoundation*			mFoundation = NULL;
	PxPhysics*				mPhysics;

	PxDefaultCpuDispatcher*	mDispatcher = NULL;
	PxScene*				mScene = NULL;

	PxVisualDebuggerConnection*		mConnection = NULL;

	PxRigidStatic*			mGroundPlane = NULL;
	
	PxMaterial* mNormalMaterial = NULL;
	
	std::vector<PxRigidDynamic*> mObstacles;
	
};

const float deltaTime = 1.f / 60.f;
const float simulateTime = 1.f / 60.f;

#define gPxHandler PxHandler::GetInstance()