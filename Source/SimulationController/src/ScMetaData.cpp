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


#include "ScActorCore.h"
#include "ScActorSim.h"
#include "ScBodyCore.h"
#include "ScStaticCore.h"
#include "ScConstraintCore.h"
#include "ScMaterialCore.h"
#include "ScShapeCore.h"
#include "ScArticulationCore.h"
#include "ScArticulationJointCore.h"
#include "ScClothFabricCore.h"
#include "ScClothCore.h"
#include "ScParticleSystemCore.h"
#include "PxsParticleData.h"
#include "PxIO.h"

using namespace physx;
using namespace Ps;
using namespace Cm;
using namespace Sc;

///////////////////////////////////////////////////////////////////////////////

template <typename T> class PxMetaDataArray : public physx::shdfnd::Array<T>
{
public:
    static PX_FORCE_INLINE physx::PxU32 getDataOffset()           { return PX_OFFSET_OF(PxMetaDataArray<T>, mData); }
    static PX_FORCE_INLINE physx::PxU32 getDataSize()             { return PX_SIZE_OF(PxMetaDataArray<T>, mData); }
    static PX_FORCE_INLINE physx::PxU32 getSizeOffset()           { return PX_OFFSET_OF(PxMetaDataArray<T>, mSize); }
    static PX_FORCE_INLINE physx::PxU32 getSizeSize()             { return PX_SIZE_OF(PxMetaDataArray<T>, mSize); }
    static PX_FORCE_INLINE physx::PxU32 getCapacityOffset()       { return PX_OFFSET_OF(PxMetaDataArray<T>, mCapacity); }
    static PX_FORCE_INLINE physx::PxU32 getCapacitySize()         { return PX_SIZE_OF(PxMetaDataArray<T>, mCapacity); }
};

void Sc::ActorCore::getBinaryMetaData(PxOutputStream& stream)
{
	// 16 bytes
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxActorFlags,     PxU16)
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxDominanceGroup, PxU8)
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxClientID,       PxU8)

	PX_DEF_BIN_METADATA_CLASS(stream,	Sc::ActorCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxU32,			    mAggregateID,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, ActorSim,			mSim,					PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxActorFlags,		mActorFlags,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxU8,				mActorType,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxU8,				mClientBehaviorFlags,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxDominanceGroup,	mDominanceGroup,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxClientID,			mOwnerClient,			0)

#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ActorCore, PxU16,			    mPadding,				PxMetaDataFlag::ePADDING)
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_PxsRigidCore(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	PxsRigidCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	PxsBodyCore, PxTransform,		body2World,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsBodyCore, PxRigidBodyFlags,	mFlags,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsBodyCore, PxU8,				mInternalFlags,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsBodyCore, PxU16,				solverIterationCounts,	0)
}


static void getBinaryMetaData_PxsBodyCore(PxOutputStream& stream)
{
	getBinaryMetaData_PxsRigidCore(stream);

	PX_DEF_BIN_METADATA_CLASS(stream,		PxsBodyCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	PxsBodyCore, PxsRigidCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxVec3,		linearVelocity,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		maxPenBias,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxVec3,		angularVelocity,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		contactReportThreshold,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxTransform,	body2Actor,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		ccdAdvanceCoefficient,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		maxAngularVelocitySq,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		maxLinearVelocitySq,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		linearDamping,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		angularDamping,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxVec3,		inverseInertia,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsBodyCore, PxReal,		inverseMass,			0)
}

/*
We need to fix the header deps by moving the API out of PhysXCore and into its own dir where other code can get to it.
[25.08.2010 18:34:57] Dilip Sequeira: In the meantime, I think it's Ok to include PxSDK.h, but you're right, we need to be very careful about include deps in that direction.
[25.08.2010 18:38:15] Dilip Sequeira: On the memory thing... PxsBodyCore has 28 bytes of padding at the end, for no reason. In addition, it has two words of padding after the velocity fields, to facilitate SIMD loads. But in fact, Vec3FromVec4 is fast enough such that unless you were using it in an inner loop (which we never are with PxsBodyCore) that padding isn't worth it.
[25.08.2010 18:38:58] Dilip Sequeira: So, we should drop the end-padding, and move the damping values to replace the velocity padding. This probably requires a bit of fixup in the places where we do SIMD writes to the velocity.
[25.08.2010 18:39:18] Dilip Sequeira: Then we're down to 92 bytes of data, and 4 bytes of padding I think.
[25.08.2010 18:50:41] Dilip Sequeira: The reason we don't want to put the sleep data there explicitly is that it isn't LL data so I'd rather not have it in an LL interface struct.
[25.08.2010 19:04:53] Gordon Yeoman nvidia: simd loads are faster when they are 16-byte aligned.  I think the padding might be to ensure the second vector is also 16-byte aligned.  We could drop the second 4-byte pad but dropping the 1st 4-byte pad will likely  have performance implications.
[25.08.2010 19:06:22] Dilip Sequeira: We should still align the vec3s, as now - but we shouldn't use padding to do it, since there are a boatload of scalar data fields floating around in that struct too.
*/
void Sc::BodyCore::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_PxsBodyCore(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxRigidBodyFlags, PxU8)

// 176 => 144 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::BodyCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Sc::BodyCore, Sc::RigidCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::BodyCore, PxsBodyCore,	mCore,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::BodyCore, PxReal,		mSleepThreshold,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::BodyCore, PxReal,		mFreezeThreshold,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::BodyCore, PxReal,		mWakeCounter,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::BodyCore, SimStateData,	mSimStateData,			PxMetaDataFlag::ePTR)
}

///////////////////////////////////////////////////////////////////////////////

void Sc::ConstraintCore::getBinaryMetaData(PxOutputStream& stream)
{   
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxConstraintFlags, PxU16)

	PX_DEF_BIN_METADATA_CLASS(stream,	ConstraintCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxConstraintFlags,		mFlags,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxU16,					mPaddingFromFlags,		PxMetaDataFlag::ePADDING)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxVec3,					mAppliedForce,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxVec3,					mAppliedTorque,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxConstraintConnector,	mConnector,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxConstraintProject,	mProject,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxConstraintSolverPrep,	mSolverPrep,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, void,					mSolverPrepSpu,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxU32,					mSolverPrepSpuByteSize,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxConstraintVisualize,	mVisualize,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxU32,					mDataSize,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxReal,					mLinearBreakForce,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, PxReal,					mAngularBreakForce,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConstraintCore, ConstraintSim,			mSim,					PxMetaDataFlag::ePTR)
}

///////////////////////////////////////////////////////////////////////////////

void Sc::MaterialCore::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxCombineMode::Enum, PxU32)
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxMaterialFlags, PxU16)

	PX_DEF_BIN_METADATA_CLASS(stream,	MaterialCore)

	// MaterialData
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxReal,			dynamicFriction,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxReal,			staticFriction,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxReal,			restitution,			0)

	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxMaterialFlags,	flags,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxU8,				fricRestCombineMode,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxU8,				padding,				PxMetaDataFlag::ePADDING)

	// MaterialCore
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxMaterial,		mNxMaterial,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialCore, PxU32,			mMaterialIndex,			0)
}

///////////////////////////////////////////////////////////////////////////////

void Sc::RigidCore::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::RigidCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Sc::RigidCore, Sc::ActorCore)
}


///////////////////////////////////////////////////////////////////////////////

void Sc::StaticCore::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::StaticCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Sc::StaticCore, Sc::RigidCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::StaticCore, PxsRigidCore,		mCore,					0)

}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_PxFilterData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	PxFilterData)

	PX_DEF_BIN_METADATA_ITEM(stream,	PxFilterData, PxU32,		word0,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxFilterData, PxU32,		word1,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxFilterData, PxU32,		word2,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxFilterData, PxU32,		word3,		0)
}

static void getBinaryMetaData_PxsShapeCore(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		PxsShapeCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		PxsShapeCore,	PxTransform,		transform,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsShapeCore,	Gu::GeometryUnion,	geometry,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsShapeCore,	PxReal,				contactOffset,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsShapeCore,	PxShapeFlags,		mShapeFlags,	0)
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	PxsShapeCore,	PxU8,	            padding,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsShapeCore,	PxU16,				materialIndex,	0)
}

void Sc::ShapeCore::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_PxFilterData(stream);
	getBinaryMetaData_PxsShapeCore(stream);

	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxShapeFlags, PxU8)

// 144 => 128 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	ShapeCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	ShapeCore, PxFilterData,	mQueryFilterData,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ShapeCore, PxFilterData,	mSimulationFilterData,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ShapeCore, PxsShapeCore,	mCore,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ShapeCore, PxReal,		    mRestOffset,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ShapeCore, bool,			mOwnsMaterialIdxMemory,	0)
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_PxsArticulationCore(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	PxsArticulationCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxU32,		internalDriveIterations,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxU32,		externalDriveIterations,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxU32,		maxProjectionIterations,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxU16,		solverIterationCounts,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxReal,	separationTolerance,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxReal,	sleepThreshold,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxReal,	freezeThreshold,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsArticulationCore, PxReal,	wakeCounter,				0)
}

void Sc::ArticulationCore::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_PxsArticulationCore(stream);

	PX_DEF_BIN_METADATA_CLASS(stream,	ArticulationCore)

	PX_DEF_BIN_METADATA_ITEM(stream,	ArticulationCore, ArticulationSim,		mSim,		PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ArticulationCore, PxsArticulationCore,	mCore,		0)
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_PxsArticulationJointCore(PxOutputStream& stream)
{
	// 172 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		PxsArticulationJointCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxTransform,	parentPose,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxTransform,	childPose,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxQuat,		targetPosition,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxVec3,		targetVelocity,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		spring,						0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		damping,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		solverSpring,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		solverDamping,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		internalCompliance,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		externalCompliance,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		swingYLimit,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		swingZLimit,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		swingLimitContactDistance,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, bool,			swingLimited,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxU8,			driveType,					0)
#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	PxsArticulationJointCore, PxU8,			paddingFromDriveType,	PxMetaDataFlag::ePADDING)
#endif
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tangentialStiffness,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tangentialDamping,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		twistLimitHigh,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		twistLimitLow,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		twistLimitContactDistance,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, bool,			twistLimited,				0)
#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	PxsArticulationJointCore, PxU8,			paddingFromTwistLimited,	PxMetaDataFlag::ePADDING)
#endif
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQSwingY,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQSwingZ,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQSwingPad,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQTwistHigh,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQTwistLow,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsArticulationJointCore, PxReal,		tanQTwistPad,				0)
}

void Sc::ArticulationJointCore::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_PxsArticulationJointCore(stream);
	PX_DEF_BIN_METADATA_CLASS(stream,	ArticulationJointCore)
	PX_DEF_BIN_METADATA_ITEM(stream,	ArticulationJointCore, ArticulationJointSim,		mSim,		PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ArticulationJointCore, PxsArticulationJointCore,	mCore,		0)
}

///////////////////////////////////////////////////////////////////////////////

#define PX_DEF_BIN_METADATA_ARRAY(stream, Class, type, array)	\
{ PxMetaDataEntry tmp = {"void", #array".mData", (PxU32)PX_OFFSET_OF(Class, array) + PxMetaDataArray<type>::getDataOffset(), PxMetaDataArray<type>::getDataSize(), 1, 0, PxMetaDataFlag::ePTR, 0};							PX_STORE_METADATA(stream, tmp); } \
{ PxMetaDataEntry tmp = {"PxU32", #array".mSize", (PxU32)PX_OFFSET_OF(Class, array) + PxMetaDataArray<type>::getSizeOffset(), PxMetaDataArray<type>::getSizeSize(), 1, 0, 0, 0};											PX_STORE_METADATA(stream, tmp);	} \
{ PxMetaDataEntry tmp = {"PxU32", #array".mCapacity", (PxU32)PX_OFFSET_OF(Class, array) + PxMetaDataArray<type>::getCapacityOffset(), PxMetaDataArray<type>::getCapacitySize(), 1, 0, PxMetaDataFlag::eCOUNT_MASK_MSB, 0};	PX_STORE_METADATA(stream, tmp);	} \
{ PxMetaDataEntry tmp = {#type, 0, (PxU32)PX_OFFSET_OF(Class, array) + PxMetaDataArray<type>::getSizeOffset(), PxMetaDataArray<type>::getSizeSize(), 0, 0, PxMetaDataFlag::eEXTRA_DATA, 0};									PX_STORE_METADATA(stream, tmp); }


#if PX_USE_CLOTH_API
void Sc::ClothFabricCore::getBinaryMetaData(PxOutputStream& stream)
{
	// register bulk data class
	Sc::ClothFabricBulkData::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::ClothFabricCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothFabricCore, void, mLowLevelFabric, PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothFabricCore, void, mLowLevelGpuFabric, PxMetaDataFlag::ePTR)

	PX_DEF_BIN_METADATA_ARRAY(stream,		Sc::ClothFabricCore, PxU32, mPhaseTypes)

	//------ Extra-data ------

	// use the mLowLevelFabric pointer as the control
	PX_DEF_BIN_METADATA_EXTRA_ITEM(stream,	Sc::ClothFabricCore, Sc::ClothFabricBulkData, mLowLevelFabric, 0)
}

void Sc::ClothFabricBulkData::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	Sc::ClothFabricBulkData)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothFabricBulkData,  PxU32,  mNbParticles, 0)	
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxU32,  mPhases)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxU32,  mSets)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxReal, mRestvalues)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxU32,  mIndices)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxU32,  mTetherAnchors)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothFabricBulkData, PxReal, mTetherLengths)
}

void Sc::ClothBulkData::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	Sc::ClothBulkData)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mTetherConstraintScale, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mTetherConstraintStiffness, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mMotionConstraintScale, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mMotionConstraintBias, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mMotionConstraintStiffness, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mAcceleration, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mDamping, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mFriction, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mCollisionMassScale, 0)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mLinearDrag, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mAngularDrag, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mLinearInertia, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mAngularInertia, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxVec3,      mCentrifugalInertia, 0)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mSolverFrequency, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mStiffnessFrequency, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mSelfCollisionDistance, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mSelfCollisionStiffness, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxTransform, mGlobalPose, 0)

	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mSleepThreshold, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Sc::ClothBulkData, PxReal,      mWakeCounter, 0)

	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mParticles)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxU32,      mVpData)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec3,     mVpWeightData)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mCollisionSpheres)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxU32,      mCollisionPairs)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mCollisionPlanes)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxU32,      mConvexMasks)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxMat33,    mCollisionTriangles)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mConstraints)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mSeparationConstraints)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mParticleAccelerations)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxU32,      mSelfCollisionIndices)
	PX_DEF_BIN_METADATA_ARRAY(stream,	Sc::ClothBulkData, PxVec4,     mRestPositions)
}

void Sc::ClothCore::getBinaryMetaData(PxOutputStream& stream)
{
	// register bulk data class
	Sc::ClothBulkData::getBinaryMetaData(stream);

	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::ClothCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Sc::ClothCore, Sc::ActorCore)

	PX_DEF_BIN_METADATA_TYPEDEF(stream,		PxClothFlags, PxU16)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxVec3,              mExternalAcceleration, 0)	
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, Sc::ClothFabricCore, mFabric, PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxFilterData,        mFilterData, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxClothFlags,        mClothFlags, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxReal,              mContactOffset, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxReal,              mRestOffset, 0)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxU32, mNumUserSpheres, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxU32, mNumUserCapsules, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxU32, mNumUserPlanes, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxU32, mNumUserConvexes, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, PxU32, mNumUserTriangles, 0)

	// items not serialized (void)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, void, mLowLevelCloth, PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, void, mBulkData,      PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ClothCore, void, mPhaseConfigs,  PxMetaDataFlag::ePTR)	

	//------ Extra-data ------

	// use the mFabric pointer as the control because mBulkData is not 
	// set at export time just need any value that will evaluate to true
	PX_DEF_BIN_METADATA_EXTRA_ITEM(stream,	Sc::ClothCore, Sc::ClothBulkData, mFabric, 0)
}

#endif // PX_USE_CLOTH_API

#if PX_USE_PARTICLE_SYSTEM_API
void Sc::ParticleSystemCore::getBinaryMetaData(PxOutputStream& stream)
{
	PxsParticleData::getBinaryMetaData(stream);
	
	PX_DEF_BIN_METADATA_CLASS(stream,		Sc::ParticleSystemCore)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Sc::ParticleSystemCore, Sc::ActorCore)

	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ParticleSystemCore, PxsParticleData,            mStandaloneData,       PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ParticleSystemCore, PxFilterData,               mSimulationFilterData, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ParticleSystemCore, PxVec3,                     mExternalAcceleration, 0)	
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ParticleSystemCore, PxReal,                     mParticleMass,         0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Sc::ParticleSystemCore, PxvParticleSystemParameter, mLLParameter,          0)
	
	//------ Extra-data ------
	PX_DEF_BIN_METADATA_EXTRA_ITEM(stream,	Sc::ParticleSystemCore, PxsParticleData,			mParticleMass,			16)
}
#endif // PX_USE_PARTICLE_SYSTEM_API




