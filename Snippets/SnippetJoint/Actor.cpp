#include "Actor.h"
#include "RagDollMath.h"
#include "RdObject.h"
void Actor::Init()
{
	mBodyHandler->InitMaterials();
	mBodyHandler->SetupBodyParts();
	mBodyHandler->SetupBodyPartsFiltering();
	mBodyHandler->SetupBodyPartsJoints();
}

void Actor::Update()
{
	mControlHandler->GetInput();
}

void Actor::AddObstacle(PxRigidDynamic * p)
{
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, mBodyHandler->leftHand, FilterGroup::eLeftHand);
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, mBodyHandler->rightHand, FilterGroup::eRightHand);
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, mBodyHandler->leftFoot, FilterGroup::eLeftFoot);
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, mBodyHandler->rightFoot, FilterGroup::eRightFoot);
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, mBodyHandler->ball, FilterGroup::eBall);
	gPxHandler->AddCollision(p, FilterGroup::eObstacales, gPxHandler->mGroundPlane, FilterGroup::eGround);
}

void Actor::OnCollisionEnter(const PxContactPairHeader & pairHeader, const PxContactPair & cp)
{
	OnCollision(mBodyHandler->leftFoot, pairHeader, cp, NULL, [this](PxVec3 normal)-> bool
	{
		if (SurfaceWithinAngle(normal, PxUp, 50.f / 180.f * PxPi))
		{
			mStateHandler->bLeftFootOnGround = true;
			return true;
		}
		return false;
	});
	OnCollision(mBodyHandler->rightFoot, pairHeader, cp, NULL, [this](PxVec3 normal)-> bool
	{
		if (SurfaceWithinAngle(normal, PxUp, 50.f / 180.f * PxPi))
		{
			mStateHandler->bRightFootOnGround = true;
			return true;
		}
		return false;
	});
	OnCollision(mBodyHandler->ball, pairHeader, cp, NULL, [this](PxVec3 normal)-> bool
	{
		if (SurfaceWithinAngle(normal, PxUp, 50.f / 180.f * PxPi))
		{
			mStateHandler->bBallOnGround = true;
			return true;
		}
		return false;
	});

	OnCollision(mBodyHandler->leftHand, pairHeader, cp, [this](PxActor* otherActor)->void {mControlHandler->GrabCheck(Side::Left, ((PxRigidActor*)otherActor)->getGlobalPose(), (PxRigidBody*)otherActor); });
	OnCollision(mBodyHandler->rightHand, pairHeader, cp, [this](PxActor* otherActor)->void {mControlHandler->GrabCheck(Side::Right, ((PxRigidActor*)otherActor)->getGlobalPose(), (PxRigidBody*)otherActor); });
}

void Actor::OnCollisionStay(const PxContactPairHeader & pairHeader, const PxContactPair & cp)
{
	//OnCollision(mBodyHandler->leftHand, pairHeader, cp, [this](PxActor* otherActor)->void {mControlHandler->GrabCheck(Side::Left, ((PxRigidActor*)otherActor)->getGlobalPose(), (PxRigidBody*)otherActor); });
	OnCollision(mBodyHandler->leftHand, pairHeader, cp, [this](PxActor* otherActor)->void 
	{
		mControlHandler->GrabCheck(Side::Left, ((PxRigidActor*)otherActor)->getGlobalPose(), (PxRigidBody*)otherActor);
		if (otherActor->userData)
		{
			RdObject* o = reinterpret_cast<RdObject*>(otherActor->userData);
			if (o && o->a)
			{
				if (o->a->mIsPC)
				{
					o->a->mStateHandler->actorState = ActorState::Unconscious;
				}
			}
		}
	});
	OnCollision(mBodyHandler->rightHand, pairHeader, cp, [this](PxActor* otherActor)->void {mControlHandler->GrabCheck(Side::Right, ((PxRigidActor*)otherActor)->getGlobalPose(), (PxRigidBody*)otherActor); });
}

void Actor::OnCollisionExit(const PxContactPairHeader & pairHeader, const PxContactPair & cp)
{
	OnCollision(mBodyHandler->leftFoot, pairHeader, cp, [this](PxActor*)->void
	{
		mStateHandler->bLeftFootOnGround = false;
	});

	OnCollision(mBodyHandler->rightFoot, pairHeader, cp, [this](PxActor*)->void
	{
		mStateHandler->bRightFootOnGround = false;
	});

	OnCollision(mBodyHandler->ball, pairHeader, cp, [this](PxActor*)->void
	{
		mStateHandler->bBallOnGround = false;
	});
}

// Ragdoll��ײ����
// f0: �Ӵ����ϰ������������ʱ����
// f1���Ӵ����ϰ������������ʱ����Ҫ�Ӵ���ķ��߲���ʱ����
void Actor::OnCollision(const PxActor * actor, const PxContactPairHeader & pairHeader, const PxContactPair & cp, std::function<void(PxActor*)> f0, std::function<bool(PxVec3)> f1)
{
	if (pairHeader.actors[0] != actor && pairHeader.actors[1] != actor)
		return;

	bool bFirst = actor == pairHeader.actors[0];
	PxActor* otherActor = (bFirst) ? pairHeader.actors[1] : pairHeader.actors[0];

	PxU32 selfRagDollActorFm;
	PxU32 otherRagDollActorFm;
	
	
	selfRagDollActorFm = actor->userData == NULL ?  0 : reinterpret_cast<RdObject*>(actor->userData)->fm;
	otherRagDollActorFm = otherActor->userData == NULL ?  0 : reinterpret_cast<RdObject*>(otherActor->userData)->fm;


	// ȥ��ActorID�ж��Ƿ���eObstacales
	bool bIsObstacle = (otherRagDollActorFm & ~ACTOR_FILTER_SHIFT) == FilterGroup::eObstacales;
	// ɸ��ActorID���ж��Ƿ���ͬ
	bool bIsOtherRagDollActor = selfRagDollActorFm & ACTOR_FILTER_SHIFT && otherRagDollActorFm & ACTOR_FILTER_SHIFT ? (selfRagDollActorFm & ACTOR_FILTER_SHIFT) != (otherRagDollActorFm & ACTOR_FILTER_SHIFT) : false;

	if (bIsObstacle || bIsOtherRagDollActor)
	{
		if (f0)
		{
			f0(otherActor);
		}

		if (f1)
		{
			const PxU32 numContact = cp.contactCount;
			if (numContact > 0)
			{
				PxContactPairPoint* contactBuffer = new PxContactPairPoint[numContact];
				const PxU32 nb = cp.extractContacts(contactBuffer, numContact);
				for (PxU32 n = 0; n < nb; ++n)
				{
					// �нӴ������Ҫ��ͽ���
					// The normal direction points from the second shape to the first shape. 
					if (bFirst)
					{
						if (f1(contactBuffer[n].normal))
							break;
					}
					else
					{
						if (f1(-contactBuffer[n].normal))
							break;
					}
				}
				delete[] contactBuffer;
				contactBuffer = NULL;
			}
		}
	}
}
