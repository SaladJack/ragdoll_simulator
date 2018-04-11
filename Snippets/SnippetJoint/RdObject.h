#pragma once
#include "PxPhysicsAPI.h"
#include "Actor.h"
class RdObject
{
public:
	RdObject(PxU32 f) :fm(f) {}
	RdObject(void* pActor, PxU32 f) :fm(f) 
	{
		a = reinterpret_cast<Actor*>(pActor);
	}
	~RdObject()
	{
		delete a;
		a = NULL;
	}
public:
	physx::PxU32 fm;
	Actor* a = NULL;
};