/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSICS_SCB_AGGREGATE
#define PX_PHYSICS_SCB_AGGREGATE

#include "CmPhysXCommon.h"
#include "PxAggregate.h"
#include "ScbActor.h"
#include "ScbAggregate.h"
#include "ScbBase.h"

// PX_SERIALIZATION
#include "PxSerialFramework.h"
//~PX_SERIALIZATION

namespace physx
{
namespace Scb
{

class Actor;

struct AggregateBuffer
{
	AggregateBuffer() : addBufferIdx(0xffffffff), addCount(0), removeBufferIdx(0xffffffff), removeCount(0) {}

	PxU32			addBufferIdx;
	PxU32			addCount;
	PxU32			removeBufferIdx;
	PxU32			removeCount;
};


class Aggregate : public Base
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
private:
	enum BufferFlag
	{
		BF_ADD_ACTOR			= (1 << 0),
		BF_REMOVE_ACTOR			= (1 << 1)
	};

public:

// PX_SERIALIZATION
										Aggregate(const PxEMPTY&) : Base(PxEmpty) {}
	static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
	PX_INLINE							Aggregate(PxAggregate* px, PxU32 maxActors, bool selfCollision);
	PX_INLINE							~Aggregate();

				void					addActor(Scb::Actor&);
				void					removeActor(Scb::Actor& actor, bool reinsert);


	//---------------------------------------------------------------------------------
	// Data synchronization
	//---------------------------------------------------------------------------------
	PX_INLINE void						syncState(Scb::Scene& scene);

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
	PX_FORCE_INLINE	PxU32				getMaxActorCount() const { return mMaxNbActors; }
	PX_FORCE_INLINE	bool				getSelfCollide() const { return mSelfCollide; }
	PX_FORCE_INLINE PxU32				getAggregateID() const { return mAggregateID; }
	PX_FORCE_INLINE void				setAggregateID(PxU32 cid) { mAggregateID = cid; }

	PX_FORCE_INLINE bool				isBufferingSpecial(ControlState::Enum state) const;

					PxAggregate*		mPxAggregate;	// Back pointer
private:
					PxU32				mAggregateID;
					PxU32				mMaxNbActors;
					bool				mSelfCollide;

	PX_FORCE_INLINE	const Scb::AggregateBuffer*	getBufferedData()	const	{ return (const Scb::AggregateBuffer*)getStream();	}
	PX_FORCE_INLINE	Scb::AggregateBuffer*		getBufferedData()			{ return (Scb::AggregateBuffer*)getStream();		}
};


PX_INLINE Aggregate::Aggregate(PxAggregate* px, PxU32 maxActors, bool selfCollision) :
	mPxAggregate	(px),
	mAggregateID		(PX_INVALID_U32),
	mMaxNbActors	(maxActors),
	mSelfCollide	(selfCollision)
{
	setScbType(ScbType::AGGREGATE);
}


PX_INLINE Aggregate::~Aggregate()
{
}

PX_FORCE_INLINE bool Aggregate::isBufferingSpecial(ControlState::Enum state) const
{
	// A special version of the buffer check is needed for aggregates because it is not fine for adding/removing 
	// an actor to be not double buffered if the aggregate is pending for insertion.
	// For example: Adding an actor can not be processed if the aggregate is pending for insertion because the aggregateID
	//              is not yet available (an there is no Sc::Aggregate object to store the actors)

	Scb::Scene* scbScene = getScbSceneForAPI();
	return state == ControlState::eREMOVE_PENDING || // pending remove not possible if not buffered
		(scbScene && scbScene->isPhysicsBuffering());
}

//--------------------------------------------------------------
//
// PVD Events
//
//--------------------------------------------------------------

namespace
{
#if PX_SUPPORT_VISUAL_DEBUGGER
	PX_FORCE_INLINE void PvdAttachActorToAggregate(Scb::Aggregate* pAggregate, Scb::Actor* pScbActor)
	{
		Scb::Scene* scbScene = pAggregate->getScbSceneForAPI();
		if( scbScene && scbScene->getSceneVisualDebugger().isConnected(true)/* && scbScene->getSceneVisualDebugger().isInstanceValid(pAggregate)*/)
		{
			scbScene->getSceneVisualDebugger().attachAggregateActor( pAggregate, pScbActor );
		}
	}

	PX_FORCE_INLINE void PvdDetachActorFromAggregate(Scb::Aggregate* pAggregate, Scb::Actor* pScbActor)
	{
		Scb::Scene* scbScene = pAggregate->getScbSceneForAPI();
		if( scbScene && scbScene->getSceneVisualDebugger().isConnected(true) /*&& scbScene->getSceneVisualDebugger().isInstanceValid(pAggregate)*/)
		{
			scbScene->getSceneVisualDebugger().detachAggregateActor( pAggregate, pScbActor );
		}
	}

	PX_FORCE_INLINE void PvdUpdateProperties(Scb::Aggregate* pAggregate)
	{
		Scb::Scene* scbScene = pAggregate->getScbSceneForAPI();
		if( scbScene && scbScene->getSceneVisualDebugger().isConnected(true) /*&& scbScene->getSceneVisualDebugger().isInstanceValid(pAggregate)*/)
		{
			scbScene->getSceneVisualDebugger().updatePvdProperties( pAggregate );
		}
	}
#else
#define PvdAttachActorToAggregate(aggregate, scbActor) {}
#define PvdDetachActorFromAggregate(aggregate, scbActor) {}
#define PvdUpdateProperties(aggregate)	{}
#endif
}

//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------

PX_INLINE void Aggregate::syncState(Scb::Scene& scene)
{
	PxU32 flags = getBufferFlags();
	
	enum AggregateSyncDirtyType
	{
		DIRTY_NONE		= 0,
		DIRTY_ADD_ACTOR		= 1<<0,
		DIRTY_REMOVE_ACTOR	= 1<<1
	};
	
	PxU32 dirtyType = DIRTY_NONE;

	if (flags)
	{
		const Scb::AggregateBuffer* PX_RESTRICT bufferedData = getBufferedData();
		
		if (flags & BF_ADD_ACTOR)
		{
			dirtyType |= DIRTY_ADD_ACTOR;

			Scb::Actor* const* actorBuffer = scene.getActorBuffer(bufferedData->addBufferIdx);

			PX_ASSERT(mAggregateID != PX_INVALID_U32);
			for(PxU32 i=0; i < bufferedData->addCount; i++)
			{
				actorBuffer[i]->getActorCore().mAggregateID = mAggregateID;
				PvdAttachActorToAggregate( this, actorBuffer[i] );				
			}
		}

		if (flags & BF_REMOVE_ACTOR)
		{
			dirtyType |= DIRTY_REMOVE_ACTOR;
			Scb::Actor* const* actorBuffer = scene.getActorBuffer(bufferedData->removeBufferIdx);

			for(PxU32 i=0; i < bufferedData->removeCount; i++)
			{
				const ControlState::Enum state = actorBuffer[i]->getControlState();

				Sc::ActorCore& ac = actorBuffer[i]->getActorCore();
				ac.mAggregateID = PX_INVALID_U32;
				if(state != ControlState::eREMOVE_PENDING)
					PvdDetachActorFromAggregate( this, actorBuffer[i] );
				if (state == ControlState::eINSERT_PENDING || state == ControlState::eIN_SCENE)
					ac.reinsertShapes();
			}
		}

		if(dirtyType != DIRTY_NONE)
			PvdUpdateProperties( this );
	}

	postSyncState();
}  

}// namespace Scb
}// namespace physx

#endif
