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

#include "PxGeometryQuery.h"
#include "GuGeomUtilsInternal.h"
#include "GuOverlapTests.h"
#include "GuSweepTests.h"
#include "GuRaycastTests.h"
#include "GuBoxConversion.h"
#include "GuInternalTriangleMesh.h"
#include "GuMTD.h"
#include "GuGeometryUnion.h"
#include "GuDistancePointSegment.h"
#include "GuConvexMesh.h"
#include "GuDistancePointBox.h"

using namespace physx;
extern Gu::GeomSweepFuncs gGeomSweepFuncs;
extern Gu::GeomOverlapTableEntry7 gGeomOverlapMethodTable[];
extern Gu::RaycastFunc gRaycastMap[7];

bool PxGeometryQuery::sweep(const PxVec3& unitDir, const PxReal distance,
							const PxGeometry& geom0, const PxTransform& pose0,
							const PxGeometry& geom1, const PxTransform& pose1,
							PxSweepHit& sweepHit, PxHitFlags hitFlags,
							const PxReal inflation)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose0.isValid(), "PxGeometryQuery::sweep(): pose0 is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxGeometryQuery::sweep(): pose1 is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(unitDir.isFinite(), "PxGeometryQuery::sweep(): unitDir is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(PxIsFinite(distance), "PxGeometryQuery::sweep(): distance is not valid.", false);
	PX_CHECK_AND_RETURN_VAL((distance >= 0.0f && !(hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP)) || distance > 0.0f,
		"PxGeometryQuery::sweep(): sweep distance must be >=0 or >0 with eASSUME_NO_INITIAL_OVERLAP.", 0);

	const Gu::GeomSweepFuncs &sf = gGeomSweepFuncs;

	switch(geom0.getType())
	{
		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);

			const Gu::Capsule worldCapsule(pose0.p, pose0.p, sphereGeom.radius);

			//pxPrintf("sweep sphere vs %d\n", geom1.getType());
			Gu::SweepCapsuleFunc func = sf.capsuleMap[geom1.getType()];
			return func(geom1, pose1, worldCapsule, unitDir, distance, sweepHit, hitFlags, inflation);
		}

		case PxGeometryType::eCAPSULE:
		{
			const PxCapsuleGeometry& capsGeom = static_cast<const PxCapsuleGeometry&>(geom0);

			Gu::Capsule worldCapsule;
			Gu::getCapsule(worldCapsule, capsGeom, pose0);

			//pxPrintf("sweep capsule vs %d\n", geom1.getType());
			if(!PX_IS_SPU && (hitFlags & PxHitFlag::ePRECISE_SWEEP))
			{
				Gu::SweepCapsuleFunc func = sf.cctCapsuleMap[geom1.getType()];
				return func(geom1, pose1, worldCapsule, unitDir, distance, sweepHit, hitFlags, inflation);
			}
			else
			{
				Gu::SweepCapsuleFunc func = sf.capsuleMap[geom1.getType()];
				return func(geom1, pose1, worldCapsule, unitDir, distance, sweepHit, hitFlags, inflation);
			}
		}

		case PxGeometryType::eBOX:
		{
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom0);

			Gu::Box box;	buildFrom(box, pose0.p, boxGeom.halfExtents, pose0.q);

			//pxPrintf("sweep box vs %d\n", geom1.getType());
			if(!PX_IS_SPU && (hitFlags & PxHitFlag::ePRECISE_SWEEP))
			{
				Gu::SweepBoxFunc func = sf.cctBoxMap[geom1.getType()];
				return func(geom1, pose1, box, unitDir, distance, sweepHit, hitFlags, inflation);
			}
			else
			{
				Gu::SweepBoxFunc func = sf.boxMap[geom1.getType()];
				return func(geom1, pose1, box, unitDir, distance, sweepHit, hitFlags, inflation);
			}
		}

		case PxGeometryType::eCONVEXMESH:
		{
			const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom0);

			Gu::SweepConvexFunc func = sf.convexMap[geom1.getType()];
			return func(geom1, pose1, convexGeom, pose0, unitDir, distance, sweepHit, hitFlags, inflation);
		}
		case PxGeometryType::ePLANE:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default :
			PX_CHECK_MSG(false, "PxGeometryQuery::sweep(): first geometry object parameter must be sphere, capsule, box or convex geometry.");
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool PxGeometryQuery::overlap(	const PxGeometry& geom0, const PxTransform& pose0,
								const PxGeometry& geom1, const PxTransform& pose1)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose0.isValid(), "PxGeometryQuery::overlap(): pose0 is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxGeometryQuery::overlap(): pose1 is not valid.", false);
	
	if(geom0.getType() > geom1.getType())
	{
		//pxPrintf("geomQuery overlap path A %d vs %d\n", geom0.getType(), geom1.getType());
		Gu::GeomOverlapFunc overlapFunc = gGeomOverlapMethodTable[geom1.getType()][geom0.getType()];
		PX_ASSERT(overlapFunc);
		return overlapFunc(geom1, pose1, geom0, pose0, NULL);
	}
	else
	{
		//pxPrintf("geomQuery overlap path B %d vs %d\n", geom0.getType(), geom1.getType());
		Gu::GeomOverlapFunc overlapFunc = gGeomOverlapMethodTable[geom0.getType()][geom1.getType()];
		PX_ASSERT(overlapFunc);
		return overlapFunc(geom0, pose0, geom1, pose1, NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////
PxU32 PxGeometryQuery::raycast(	const PxVec3& rayOrigin, const PxVec3& rayDir,
								const PxGeometry& geom, const PxTransform& pose,
								PxReal maxDist, PxHitFlags hitFlags,
								PxU32 maxHits, PxRaycastHit* PX_RESTRICT rayHits,
								bool anyHit)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(rayDir.isFinite(), "PxGeometryQuery::raycast(): rayDir is not valid.", 0);
	PX_CHECK_AND_RETURN_VAL(rayOrigin.isFinite(), "PxGeometryQuery::raycast(): rayOrigin is not valid.", 0);
	PX_CHECK_AND_RETURN_VAL(pose.isValid(), "PxGeometryQuery::raycast(): pose is not valid.", 0);
	PX_CHECK_AND_RETURN_VAL(maxDist >= 0.0f, "PxGeometryQuery::raycast(): maxDist is negative.", false);
	PX_CHECK_AND_RETURN_VAL(PxIsFinite(maxDist), "PxGeometryQuery::raycast(): maxDist is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(PxAbs(rayDir.magnitudeSquared()-1)<1e-4, "PxGeometryQuery::raycast(): ray direction must be unit vector.", false);

	Gu::RaycastFunc func = gRaycastMap[geom.getType()];
	return func(geom, pose, rayOrigin, rayDir, maxDist, hitFlags, maxHits, rayHits, anyHit, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////////

bool pointConvexDistance(PxVec3& normal_, PxVec3& closestPoint_, PxReal& sqDistance, const PxVec3& pt, const Gu::ConvexMesh* convexMesh, const PxMeshScale& meshScale, const PxTransform& convexPose);

PxReal PxGeometryQuery::pointDistance(const PxVec3& point, const PxGeometry& geom, const PxTransform& pose, PxVec3* closestPoint)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose.isValid(), "PxGeometryQuery::pointDistance(): pose is not valid.", false);

	switch(geom.getType())
	{
		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

			const PxReal r = sphereGeom.radius;

			PxVec3 delta = point - pose.p;
			const PxReal d = delta.magnitude();
			if(d<=r)
				return 0.0f;

			if(closestPoint)
			{
				delta /= d;
				*closestPoint = pose.p + delta * r;
			}

			return (d - r)*(d - r);
		}
		case PxGeometryType::eCAPSULE:
		{
			const PxCapsuleGeometry& capsGeom = static_cast<const PxCapsuleGeometry&>(geom);

			Gu::Capsule capsule;
			getCapsule(capsule, capsGeom, pose);

			const PxReal r = capsGeom.radius;

			PxReal param;
			const PxReal sqDistance = distancePointSegmentSquared(capsule, point, &param);
			if(sqDistance<=r*r)
				return 0.0f;

			const PxReal d = physx::intrinsics::sqrt(sqDistance);

			if(closestPoint)
			{
				const PxVec3 cp = capsule.getPointAt(param);

				PxVec3 delta = point - cp;
				delta.normalize();

				*closestPoint = cp + delta * r;
			}
			return (d - r)*(d - r);
		}
		case PxGeometryType::eBOX:
		{
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

			Gu::Box obb;
			buildFrom(obb, pose.p, boxGeom.halfExtents, pose.q);

			PxVec3 boxParam;
			const PxReal sqDistance = distancePointBoxSquared(point, obb, &boxParam);
			if(closestPoint && sqDistance!=0.0f)
			{
				*closestPoint = obb.transform(boxParam);
			}
			return sqDistance;
		}
		case PxGeometryType::eCONVEXMESH:
		{
			const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

			PxVec3 normal, cp;
			PxReal sqDistance;
			const bool intersect = pointConvexDistance(normal, cp, sqDistance, point, static_cast<Gu::ConvexMesh*>(convexGeom.convexMesh), convexGeom.scale, pose);
			if(!intersect && closestPoint)
				*closestPoint = cp;
			return sqDistance;
		}
		case PxGeometryType::ePLANE:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default :
			PX_CHECK_MSG(false, "PxGeometryQuery::pointDistance(): geometry object parameter must be sphere, capsule box or convex geometry.");
			return -1.0f;
	}
}

///////////////////////////////////////////////////////////////////////////////

PxBounds3 PxGeometryQuery::getWorldBounds(const PxGeometry& geom, const PxTransform& pose, float inflation)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose.isValid(), "Gu::GeometryQuery::getWorldBounds(): pose is not valid.", PxBounds3::empty());
	Gu::GeometryUnion gu;
	gu.set(geom);

	PxBounds3 result;
	gu.computeBounds(result, pose, 0.0f, NULL);
	PX_ASSERT(result.isValid());

	// PT: unfortunately we can't just scale the min/max vectors, we need to go through center/extents.
	// It would be more efficient to return center/extents from 'computeBounds'...
	const PxVec3 center = result.getCenter();
	const PxVec3 inflatedExtents = result.getExtents() * inflation;
	return PxBounds3::centerExtents(center, inflatedExtents);
}

///////////////////////////////////////////////////////////////////////////////

bool PxGeometryQuery::computePenetration(	PxVec3& mtd, PxF32& depth,
							const PxGeometry& geom0, const PxTransform& pose0,
							const PxGeometry& geom1, const PxTransform& pose1)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(pose0.isValid(), "PxGeometryQuery::computePenetration(): pose0 is not valid.", false);
	PX_CHECK_AND_RETURN_VAL(pose1.isValid(), "PxGeometryQuery::computePenetration(): pose1 is not valid.", false);

	if(geom0.getType() > geom1.getType())
	{
		Gu::GeomMTDFunc mtdFunc = Gu::gGeomMTDMethodTable[geom1.getType()][geom0.getType()];
		PX_ASSERT(mtdFunc);
		if(!mtdFunc(mtd, depth, geom1, pose1, geom0, pose0))
			return false;
		mtd = -mtd;
		return true;
	}
	else
	{
		Gu::GeomMTDFunc mtdFunc = Gu::gGeomMTDMethodTable[geom0.getType()][geom1.getType()];
		PX_ASSERT(mtdFunc);
		return mtdFunc(mtd, depth, geom0, pose0, geom1, pose1);
	}
}
