#include "PxHandler.h"
#include "JointDescriptions.h"
#include "RagDollMath.h"
#include "CommonPragma.h"
#include "RdObject.h"

#define PVD_HOST "127.0.0.1"	
#define SNIPPET_ALLOC(x)	mAllocator.allocate(x, 0, __FILE__, __LINE__)
#define	SNIPPET_FREE(x)	if(x)	{ mAllocator.deallocate(x); x = NULL;	}
#define SAMPLE_NEW(x)	new(#x, __FILE__, __LINE__) x

void PxHandler::Init(PxSimulationFilterShader fs, PxSimulationEventCallback* callback)
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
	//allocator = gAllocator;
	PxProfileZoneManager* profileZoneManager = &PxProfileZoneManager::createProfileZoneManager(mFoundation);
	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, profileZoneManager);
	PxInitExtensions(*mPhysics);

	if (mPhysics->getPvdConnectionManager())
	{
		mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS, true);
		mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
		mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
		mConnection = PxVisualDebuggerExt::createConnection(mPhysics->getPvdConnectionManager(), PVD_HOST, 5425, 10);
	}

	PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	//sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);
	mDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = mDispatcher;
	//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = fs;
	sceneDesc.flags |= PxSceneFlag::eREQUIRE_RW_LOCK;
	mScene = mPhysics->createScene(sceneDesc);

	mScene->lockWrite();
	mScene->lockRead();

	mScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 0.2f);
	// x:red, y:blue, z:green
	//gScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 5.0f);


	mScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 2.0f);
	mScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);

	mScene->setSimulationEventCallback(callback);


	
	mNormalMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	mGroundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *mNormalMaterial);
	mGroundPlane->userData = new RdObject(FilterGroup::eObstacales);
	SetFilterMask(mGroundPlane, FilterGroup::eGround);
	mScene->addActor(*mGroundPlane);

	
}

void PxHandler::Update()
{
	mScene->simulate(simulateTime);
	mScene->fetchResults(true);
}

void PxHandler::Release()
{
	mScene->release();
	mDispatcher->release();
	PxProfileZoneManager* profileZoneManager = mPhysics->getProfileZoneManager();
	if (mConnection != NULL)
		mConnection->release();
	PxCloseExtensions();
	mPhysics->release();
	profileZoneManager->release();
	mFoundation->release();
}
// PxRigidDynamic
PxRigidDynamic* PxHandler::createDynamicRelativePos(const PxTransform& t, const PxTransform& relativePose, const PxGeometry& geometry, PxReal mass, const PxMaterial* material, const PxVec3& velocity)
{
	PxRigidDynamic* dynamic = mPhysics->createRigidDynamic(PxTransform(t));
	PxShape* shape = dynamic->createShape(geometry, *material);

	shape->setLocalPose(relativePose);
	//PxRigidBodyExt::updateMassAndInertia(*dynamic, density * 1);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	dynamic->setMass(mass);
	dynamic->setSolverIterationCounts(20, 20);
	mScene->addActor(*dynamic);
	return dynamic;
}

PxRigidDynamic* PxHandler::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxReal& mass, const PxMaterial* material, const PxVec3& velocity)
{
	return createDynamicRelativePos(t, PxTransform::createIdentity(), geometry, mass, material);
}

PxRigidDynamic* PxHandler::createDynamicEmptyShape(const PxTransform& t, PxReal mass, const PxMaterial* material)
{
	PxRigidDynamic* dynamic = mPhysics->createRigidDynamic(PxTransform(t));

	// 	PxShape* shape = dynamic->createShape(PxSphereGeometry(0.1f), *material);
	// 	shape->setLocalPose(PxTransform::createIdentity());

	dynamic->setLinearVelocity(PxVec3(0));
	dynamic->setMass(mass);
	dynamic->setSolverIterationCounts(20, 20);
	mScene->addActor(*dynamic);
	return dynamic;
}
PxRigidDynamic* PxHandler::createDynamicDensity(const PxTransform& t, const PxGeometry& geometry, const PxReal& density, const PxVec3& velocity)
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*mPhysics, t, geometry, *mNormalMaterial, density);

	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);

	mScene->addActor(*dynamic);
	return dynamic;
}

PxRigidDynamic * PxHandler::createObstacle(const PxTransform & t, const PxGeometry & geometry, const PxReal & density, const PxVec3 & velocity)
{
	PxRigidDynamic* p = createDynamicDensity(t, geometry, density, velocity);
	//PxRigidDynamic* p = createDynamic(t, geometry, density, mNormalMaterial);
	//PxRigidDynamic* p = PxCreateDynamic(*mPhysics, t, geometry, *mNormalMaterial, density);
	//p->setMass(density);
	p->userData = new RdObject(FilterGroup::eObstacales);
	SetFilterMask(p, FilterGroup::eObstacales);
	mObstacles.push_back(p);
	return p;
}

PxRigidDynamic* PxHandler::createArmPos(const PxVec3& p, const PxGeometry& geometry, PxReal mass, Side side, const PxMaterial* material)
{
	PxReal rotate = side == Side::Left ? PxHalfPi : -PxHalfPi;
	PxQuat q = PxQuat(rotate, PxVec3(0, 0, 1));
	return createDynamicRelativePos(PxTransform(p, q), PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))), geometry, mass, material);
}

// Joint
PxD6Joint* PxHandler::createD6Joint(PxRigidDynamic* actor0, const PxTransform& tRelativeToActor0
	, PxRigidDynamic* actor1, const PxTransform& tRelativeToActor1)
{
	PxD6Joint* j = PxD6JointCreate(*mPhysics
		, actor0
		, tRelativeToActor0
		, actor1
		, tRelativeToActor1);
	j->setDrive(PxD6Drive::eSLERP, PxD6JointDrive(0, 1000, FLT_MAX, true));
	//j->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
	return j;
}

PxD6Joint* PxHandler::createD6JointAutomatically(PxRigidDynamic* actor0, const PxTransform& tRelativeToActor0
	, PxRigidActor* actor1)
{
	PxD6Joint* j = PxD6JointCreate(*mPhysics
		, actor0
		, tRelativeToActor0
		, actor1
		, PxTransform::createIdentity());

	PxTransform actorGlobalFrame0 = actor0->getGlobalPose();
	PxTransform actorGlobalFrame1 = actor1->getGlobalPose();

	// 得到actor0的joint节点的在世界空间中的transform
	PxTransform localFrame0 = j->getLocalPose(PxJointActorIndex::eACTOR0);
	PxTransform globalFrame0 = actorGlobalFrame0 * localFrame0;

	// 得到actor0的joint节点在actor1空间中的transform
	PxTransform localFrame1 = actorGlobalFrame1.getInverse() * globalFrame0;
	j->setLocalPose(PxJointActorIndex::eACTOR1, localFrame1);
	return j;
}

PxD6Joint* PxHandler::setDefaultJointDesc(PxD6Joint* j
	, float angularYLimit, float angularZLimit
	, float lowAngularXLimit, float highAngularXLimit)
{
	if (NULL == j) return NULL;

	// swing
	{
		SoftJointLimit m_AngularYLimit, m_AngularZLimit;
		m_AngularYLimit.limit = angularYLimit; m_AngularYLimit.bounciness = 0; m_AngularYLimit.contactDistance = 0;
		m_AngularZLimit.limit = angularZLimit; m_AngularZLimit.bounciness = 0; m_AngularZLimit.contactDistance = 0;
		float angularLimitY = (m_AngularYLimit.limit == 0.f) ? PxHalfPi : Deg2Rad(m_AngularYLimit.limit);
		float angularLimitZ = (m_AngularZLimit.limit == 0.f) ? PxHalfPi : Deg2Rad(m_AngularZLimit.limit);
		float contactDistance = 0.1f; // Deg2Rad(m_AngularYLimit.contactDistance);

		SoftJointLimitSpring m_AngularYZLimitSpring;
		m_AngularYZLimitSpring.spring = 0; m_AngularYZLimitSpring.damper = 0;
		PxJointLimitCone swing(angularLimitY, angularLimitZ, contactDistance);
		swing.restitution = m_AngularYLimit.bounciness;
		swing.stiffness = m_AngularYZLimitSpring.spring;
		swing.contactDistance = FixLimitConeContactDistance(swing.yAngle, swing.zAngle, swing.restitution
			, m_AngularYLimit.contactDistance, contactDistance);

		j->setSwingLimit(swing);
	}

	if (lowAngularXLimit < highAngularXLimit)
		// twist
	{
		SoftJointLimit m_LowAngularXLimit, m_HighAngularXLimit;
		m_LowAngularXLimit.limit = lowAngularXLimit; m_LowAngularXLimit.bounciness = 0; m_LowAngularXLimit.contactDistance = 0;
		m_HighAngularXLimit.limit = highAngularXLimit; m_HighAngularXLimit.bounciness = 0; m_HighAngularXLimit.contactDistance = 0;
		float lowerTwistLimit = Deg2Rad(m_LowAngularXLimit.limit);
		float upperTwistLimit = Deg2Rad(m_HighAngularXLimit.limit);
		float contactDistance = -1;

		SoftJointLimitSpring m_AngularXLimitSpring;
		m_AngularXLimitSpring.spring = 0; m_AngularXLimitSpring.damper = 0;

		PxJointAngularLimitPair twist(lowerTwistLimit, upperTwistLimit, contactDistance);
		twist.restitution = m_LowAngularXLimit.bounciness;
		twist.stiffness = m_AngularXLimitSpring.spring;
		twist.damping = m_AngularXLimitSpring.damper;

		j->setTwistLimit(twist);
	}

	JointDrive m_SlerpDrive;
	m_SlerpDrive.maximumForce = 4.0f; m_SlerpDrive.positionDamper = 1000.0f; m_SlerpDrive.positionSpring = 0.0f;
	j->setDrive(PxD6Drive::eSLERP, m_SlerpDrive.ToPx()); // 'rotationDriveMode': 'Slerp'

	j->setProjectionLinearTolerance(0.1f);
	j->setProjectionAngularTolerance(Deg2Rad(180));
	j->setConstraintFlag(PxConstraintFlag::ePROJECTION, true); // 'projectionMode': 'PositionAndRotation'
	return j;
}


PxD6Joint* PxHandler::setJointMotion(PxD6Joint* j, PxD6Motion::Enum angularMotion, PxD6Motion::Enum motion)
{
	if (NULL == j) return NULL;
	j->setMotion(PxD6Axis::eTWIST, angularMotion);
	j->setMotion(PxD6Axis::eSWING1, angularMotion);
	j->setMotion(PxD6Axis::eSWING2, angularMotion);
	j->setMotion(PxD6Axis::eX, motion);
	j->setMotion(PxD6Axis::eY, motion);
	j->setMotion(PxD6Axis::eZ, motion);
	return j;
}

void PxHandler::createChain(const PxTransform & t, PxU32 length, const PxGeometry & g, PxReal separation, JointCreateFunction createJoint)
{
	PxVec3 offset = PxVec3(separation / 2, 0, 0);

	PxTransform localTm(offset);
	PxRigidDynamic* prev = NULL;


	for (PxU32 i = 0; i < length; i++)
	{
		PxRigidDynamic* current = PxCreateDynamic(*mPhysics, t*localTm, g, *mNormalMaterial, 1.0f);
		(*createJoint)(prev, prev ? PxTransform(offset) : t, current, PxTransform(-offset));
		current->userData = new RdObject(FilterGroup::eObstacales);
		SetFilterMask(current, FilterGroup::eObstacales);
		mObstacles.push_back(current);
		mScene->addActor(*current);
		prev = current;
		localTm.p.x += separation;
	}
}


void PxHandler::SetFilterMask(PxRigidActor* actor, PxU32 fm)
{
	const PxU32 numShapes = actor->getNbShapes();

	PxShape** shapes = (PxShape**)SNIPPET_ALLOC(sizeof(PxShape*)*numShapes);
	actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		PxFilterData simFilterData = shape->getSimulationFilterData();
		simFilterData.word0 = fm;
		shape->setSimulationFilterData(simFilterData);
	}
	SNIPPET_FREE(shapes);
}

void PxHandler::AddFilterData(PxRigidActor* actor0, PxU32 fm, PxU32 fg, bool bIntersect)
{
	const PxU32 numShapes = actor0->getNbShapes();

	PxShape** shapes = (PxShape**)SNIPPET_ALLOC(sizeof(PxShape*)*numShapes);
	actor0->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		PxFilterData simFilterData = shape->getSimulationFilterData();
		if (simFilterData.word0)
		{
			simFilterData.word0 |= fm;
		}
		else
		{
			simFilterData.word0 = fm;
		}

		if (simFilterData.word1)
		{
			if (bIntersect)
			{
				simFilterData.word1 &= fg;
			}
			else
			{
				simFilterData.word1 |= fg;
			}
		}
		else
		{
			simFilterData.word1 = fg;
		}

		shape->setSimulationFilterData(simFilterData);
	}
	SNIPPET_FREE(shapes);
}



PxFilterData PxHandler::GetFilterData(PxRigidActor * actor)
{
	const PxU32 numShapes = actor->getNbShapes();

	PxShape** shapes = (PxShape**)SNIPPET_ALLOC(sizeof(PxShape*)*numShapes);
	actor->getShapes(shapes, numShapes);
	PxFilterData simFilterData;
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		simFilterData = shape->getSimulationFilterData();
	}
	SNIPPET_FREE(shapes);
	return simFilterData;
}

void PxHandler::IgnoreCollision(PxRigidActor* actor0, PxU32 fg0, PxRigidActor* actor1, PxU32 fg1)
{
	AddFilterData(actor0, fg0, ~fg1);
	AddFilterData(actor1, fg1, ~fg0);
}

void PxHandler::IgnoreCollision(PxRigidActor* actor0, PxRigidActor* actor1)
{
	PxU32 fg0 = GetFilterData(actor0).word0;
	PxU32 fg1 = GetFilterData(actor1).word0;
	AddFilterData(actor0, fg0, ~fg1);
	AddFilterData(actor1, fg1, ~fg0);
}

void PxHandler::AddCollision(PxRigidActor* actor0, PxU32 fg0, PxRigidActor* actor1, PxU32 fg1)
{
	AddFilterData(actor0, fg0, fg1, false);
	AddFilterData(actor1, fg1, fg0, false);
}


void PxHandler::AddCollision(PxRigidActor* actor0, PxRigidActor* actor1)
{
	PxU32 fg0 = GetFilterData(actor0).word0;
	PxU32 fg1 = GetFilterData(actor1).word0;
	AddFilterData(actor0, fg0, fg1, false);
	AddFilterData(actor1, fg1, fg0,	false);
}


