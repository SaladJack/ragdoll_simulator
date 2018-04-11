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


#include "GuVecCapsule.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif

using namespace physx;
using namespace Gu;


namespace physx
{
namespace Gu
{


bool pcmContactPlaneCapsule(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(shape0);
	using namespace Ps::aos;

	Gu::PersistentContactManifold& manifold = cache.getManifold();
	Ps::prefetchLine(&manifold, 256);

	// Get actual shape data
	const PxCapsuleGeometry& shapeCapsule = shape1.get<const PxCapsuleGeometry>();

	const PsTransformV transf0 = loadTransformA(transform1);//capsule transform
	const PsTransformV transf1 = loadTransformA(transform0);//plane transform
	//capsule to plane
	const PsTransformV aToB(transf1.transformInv(transf0));

	//in world space
	const Vec3V negPlaneNormal = V3Normalize(V3Neg(QuatGetBasisVector0(transf1.q)));
	//ML:localNormal is the local space of plane normal, however, because shape1 is capulse and shape0 is plane, we need to use the reverse of contact normal(which will be the plane normal) to make the refreshContactPoints
	//work out the correct pentration for points
	const Vec3V localNormal = V3UnitX();

	const FloatV cDist = FLoad(contactDistance);

	const FloatV radius = FLoad(shapeCapsule.radius);
	const FloatV halfHeight = FLoad(shapeCapsule.halfHeight);

	//capsule is in the local space of plane(n = (1.f, 0.f, 0.f), d=0.f)
	const Vec3V basisVector = QuatGetBasisVector0(aToB.q);
	const Vec3V tmp = V3Scale(basisVector, halfHeight);
	const Vec3V s = V3Add(aToB.p, tmp);
	const Vec3V e = V3Sub(aToB.p, tmp);

	const FloatV inflatedRadius = FAdd(radius, cDist);
	const FloatV replaceBreakingThreshold = FMul(radius, FLoad(0.001f));
	const FloatV projectBreakingThreshold = FMul(radius, FLoad(0.05f));
	const PxU32 initialContacts = manifold.mNumContacts;

	//manifold.refreshContactPoints(curRTrans, projectBreakingThreshold, contactDist);
	const FloatV refreshDist = FAdd(cDist, radius);
	manifold.refreshContactPoints(aToB, projectBreakingThreshold, refreshDist);

	const PxU32 newContacts = manifold.mNumContacts;
	const bool bLostContacts = (newContacts != initialContacts);//((initialContacts == 0) || (newContacts != initialContacts));

	//PX_UNUSED(bLostContacts);
	if(bLostContacts || manifold.invalidate_PrimitivesPlane(aToB, radius, FLoad(0.02f)))  
	{
		manifold.mNumContacts = 0;
		manifold.setRelativeTransform(aToB);
		//calculate the distance from s to the plane
		const FloatV signDist0  = V3GetX(s);//V3Dot(localNormal, s);
		if(FAllGrtr(inflatedRadius, signDist0))
		{
			const Vec3V localPointA = aToB.transformInv(s);
			const Vec3V localPointB = V3NegScaleSub(localNormal, signDist0, s);
			const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(localNormal), signDist0);
			//add to manifold
			manifold.addManifoldPoint2(localPointA, localPointB, localNormalPen, replaceBreakingThreshold); 
		}

		const FloatV signDist1 = V3GetX(e);//V3Dot(localNormal, e);
		if(FAllGrtr(inflatedRadius, signDist1))
		{
			const Vec3V localPointA = aToB.transformInv(e);
			const Vec3V localPointB = V3NegScaleSub(localNormal, signDist1, e);
			const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(localNormal), signDist1);
			//add to manifold
			manifold.addManifoldPoint2(localPointA, localPointB, localNormalPen, replaceBreakingThreshold); 
		}

		manifold.addManifoldContactsToContactBuffer(contactBuffer, negPlaneNormal, transf0, radius);


		return manifold.getNumContacts() > 0;
	}
	else
	{
		manifold.addManifoldContactsToContactBuffer(contactBuffer, negPlaneNormal, transf0, radius);
		return manifold.getNumContacts() > 0;
	}
	
}

}//Gu
}//physx
