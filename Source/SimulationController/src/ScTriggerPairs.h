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


#ifndef PX_PHYSICS_SCP_TRIGGER_PAIRS
#define PX_PHYSICS_SCP_TRIGGER_PAIRS

#include "PsArray.h"
#include "CmPhysXCommon.h"
#include "PxFiltering.h"
#include "PxClient.h"
#include "PxSimulationEventCallback.h"

namespace physx
{

class PxShape;

namespace Sc
{
	struct TriggerPairFlag
	{
		enum Enum
		{
			eTEST_FOR_REMOVED_SHAPES = PxTriggerPairFlag::eNEXT_FREE	// for cases where the pair got deleted because one of the shape volumes got removed from broadphase.
																		// This covers scenarios like volume re-insertion into broadphase as well since the shape might get removed
																		// after such an operation. The scenarios to consider are:
																		//
																		// - shape gets removed (this includes raising PxActorFlag::eDISABLE_SIMULATION)
																		// - shape switches to eSCENE_QUERY_SHAPE only
																		// - resetFiltering()
																		// - actor gets removed from an aggregate
		};
	};

	PX_COMPILE_TIME_ASSERT((1 << (8*sizeof(PxTriggerPairFlags::InternalType))) > TriggerPairFlag::eTEST_FOR_REMOVED_SHAPES);


	struct TriggerPairExtraData
	{
		PX_INLINE TriggerPairExtraData() : 
			shape0ID(0xffffffff),
			shape1ID(0xffffffff),
			client0ID(0xff),
			client1ID(0xff),
			actor0ClientBehavior(0),
			actor1ClientBehavior(0)
		{
		}

		PX_INLINE TriggerPairExtraData(PxU32 s0ID, PxU32 s1ID,
										PxClientID cl0ID, PxClientID cl1ID,
										PxU8 a0ClientBehaviorFlag, PxU8 a1ClientBehaviorFlag) : 
			shape0ID(s0ID),
			shape1ID(s1ID),
			client0ID(cl0ID),
			client1ID(cl1ID),
			actor0ClientBehavior(a0ClientBehaviorFlag),
			actor1ClientBehavior(a1ClientBehaviorFlag)
		{
		}

		PxU32						shape0ID;
		PxU32						shape1ID;
		PxClientID					client0ID;
		PxClientID					client1ID;
		PxU8						actor0ClientBehavior;  // for PxActorClientBehaviorFlag
		PxU8						actor1ClientBehavior;
	};


	typedef	Ps::Array<TriggerPairExtraData>	TriggerBufferExtraData;
	typedef	Ps::Array<PxTriggerPair>		TriggerBufferAPI;

} // namespace Sc

}

#endif
