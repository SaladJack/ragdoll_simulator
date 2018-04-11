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


#include "ExtSphericalJoint.h"
#include "ExtConstraintHelper.h"
#include "CmConeLimitHelper.h"
#include "CmRenderOutput.h"
#include "CmVisualization.h"

#ifdef PX_PS3
#include "PS3/ExtSphericalJointSpu.h"
#endif
#include "common/PxSerialFramework.h"

using namespace physx;
using namespace Ext;

namespace physx
{
	PxSphericalJoint* PxSphericalJointCreate(PxPhysics& physics,
		PxRigidActor* actor0, const PxTransform& localFrame0,
		PxRigidActor* actor1, const PxTransform& localFrame1);
}

PxSphericalJoint* physx::PxSphericalJointCreate(PxPhysics& physics,
												PxRigidActor* actor0, const PxTransform& localFrame0,
												PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxSphericalJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxSphericalJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxSphericalJointCreate: actors must be different");
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxSphericalJointCreate: at least one actor must be dynamic");

	SphericalJoint* j;
	PX_NEW_SERIALIZED(j,SphericalJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);

	if(j->attach(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

void SphericalJoint::setProjectionLinearTolerance(PxReal tolerance)
{	
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance) && tolerance >=0, "PxSphericalJoint::setProjectionLinearTolerance: invalid parameter");
	data().projectionLinearTolerance = tolerance;
	markDirty(); 
}

PxReal SphericalJoint::getProjectionLinearTolerance() const	
{	
	return data().projectionLinearTolerance;		
}

void SphericalJoint::setLimitCone(const PxJointLimitCone &limit)
{	
	PX_CHECK_AND_RETURN(limit.isValid(), "PxSphericalJoint::setLimit: invalid parameter");
	data().limit = limit; 
	markDirty();
}

PxJointLimitCone SphericalJoint::getLimitCone() const
{	
	return data().limit; 
}


PxSphericalJointFlags SphericalJoint::getSphericalJointFlags(void) const
{ 
	return data().jointFlags; 
}

void SphericalJoint::setSphericalJointFlags(PxSphericalJointFlags flags)
{ 
	data().jointFlags = flags; 
}
void SphericalJoint::setSphericalJointFlag(PxSphericalJointFlag::Enum flag, bool value)
{
	if(value)
		data().jointFlags |= flag;
	else
		data().jointFlags &= ~flag;
	markDirty();
}


void* Ext::SphericalJoint::prepareData()
{
	SphericalJointData& d = data();
	if(d.jointFlags & PxSphericalJointFlag::eLIMIT_ENABLED)
	{
		d.tanQYLimit = PxTan(d.limit.yAngle/4);
		d.tanQZLimit = PxTan(d.limit.zAngle/4);
		d.tanQPad	 = PxTan(d.limit.contactDistance/4);
	}
	return SphericalJointT::prepareData();
}

namespace
{

void SphericalJointVisualize(PxConstraintVisualizer& viz,
							 const void* constantBlock,
							 const PxTransform& body0Transform,
							 const PxTransform& body1Transform,
							 PxU32 /*flags*/)
{
	using namespace joint;
	const SphericalJointData& data = *reinterpret_cast<const SphericalJointData*>(constantBlock);

	PxTransform cA2w = body0Transform * data.c2b[0];
	PxTransform cB2w = body1Transform * data.c2b[1];

	viz.visualizeJointFrames(cA2w, cB2w);


	if(data.jointFlags & PxSphericalJointFlag::eLIMIT_ENABLED)
	{
		if(cA2w.q.dot(cB2w.q)<0)
			cB2w.q = -cB2w.q;

		PxTransform cB2cA = cA2w.transformInv(cB2w);	
		PxQuat swing, twist;
		Ps::separateSwingTwist(cB2cA.q,swing,twist);

		PxVec3 tanQSwing = PxVec3(0, Ps::tanHalf(swing.z,swing.w), -Ps::tanHalf(swing.y,swing.w));
		Cm::ConeLimitHelper coneHelper(data.tanQZLimit, data.tanQYLimit, data.tanQPad);
		viz.visualizeLimitCone(cB2w, data.tanQZLimit, data.tanQYLimit, 
			!coneHelper.contains(tanQSwing));
	}
}



void SphericalJointProject(const void* constantBlock,
						   PxTransform& bodyAToWorld,
						   PxTransform& bodyBToWorld,
						   bool projectToA)
{
	using namespace joint;

	const SphericalJointData& data = *reinterpret_cast<const SphericalJointData*>(constantBlock);

	PxTransform cA2w, cB2w, cB2cA, projected;
	joint::computeDerived(data, bodyAToWorld, bodyBToWorld, cA2w, cB2w, cB2cA);

	bool linearTrunc;
	projected.p = truncateLinear(cB2cA.p, data.projectionLinearTolerance, linearTrunc);

	if(linearTrunc)
	{
		projected.q = cB2cA.q;
		projectTransforms(bodyAToWorld, bodyBToWorld, cA2w, cB2w, projected, data, projectToA);
	}
}
}


bool Ext::SphericalJoint::attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(SphericalJointData));
	return mPxConstraint!=NULL;
}

void SphericalJoint::exportExtraData(PxSerializationContext& stream)
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(SphericalJointData));
	}
	stream.writeName(mName);
}

void SphericalJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<SphericalJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void SphericalJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));	
}

SphericalJoint* SphericalJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	SphericalJoint* obj = new (address) SphericalJoint(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(SphericalJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

#ifdef PX_PS3
PxConstraintShaderTable Ext::SphericalJoint::sShaders = { Ext::SphericalJointSolverPrep, ExtSphericalJointSpu, EXTSPHERICALJOINTSPU_SIZE, SphericalJointProject, SphericalJointVisualize };
#else
PxConstraintShaderTable Ext::SphericalJoint::sShaders = { Ext::SphericalJointSolverPrep, 0, 0, SphericalJointProject, SphericalJointVisualize };
#endif
