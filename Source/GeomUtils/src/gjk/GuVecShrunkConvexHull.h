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

#ifndef GU_VEC_SHRUNK_CONVEX_HULL_H
#define GU_VEC_SHRUNK_CONVEX_HULL_H

#include "PxPhysXCommonConfig.h"
#include "GuVecConvexHull.h"
#include "GuConvexMeshData.h"
#include "GuBigConvexData.h"
#include "GuConvexSupportTable.h"
#include "GuCubeIndex.h"
#include "PsFPU.h"


namespace physx
{
namespace Gu
{

	PX_SUPPORT_FORCE_INLINE Ps::aos::Vec3V intersectPlanes(const Ps::aos::Vec3VArg n1, const Ps::aos::FloatVArg d1,
															const Ps::aos::Vec3VArg n2, const Ps::aos::FloatVArg d2,
															const Ps::aos::Vec3VArg n3, const Ps::aos::FloatVArg d3)
	{
		using namespace Ps::aos;
		const Vec3V u = V3Cross(n2, n3);
		const FloatV denom = V3Dot(n1, u);
		const Vec3V temp = V3NegScaleSub(n2, d3, V3Scale(n3, d2));
		const Vec3V p = V3NegScaleSub(u, d1, V3Cross(n1, temp));
		return V3ScaleInv(p, denom);
	}

	

	/*
		ML:
		ShrinkedConvexHull is used in GJK code but not EPA code
	*/
	class ShrunkConvexHullV : public ConvexHullV
	{


		public:
		/**
		\brief Constructor
		*/
		PX_SUPPORT_INLINE ShrunkConvexHullV(): ConvexHullV()
		{
		}

		PX_SUPPORT_INLINE ShrunkConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot):
													ConvexHullV(_hullData, _center, scale, scaleRot)
		{
		}


		PX_SUPPORT_INLINE ShrunkConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot) : 
													ConvexHullV(_hullData, _center, _margin, _minMargin, scale, scaleRot)
		{
		}



		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			return planeShift((PxU32)index, margin);
		}

		//get the support point in vertex space
		PX_SUPPORT_INLINE Ps::aos::Vec3V planeShift(const PxU32 index, const Ps::aos::FloatVArg margin_)const 
		{
			using namespace Ps::aos;

			//calculate the support point for the core(shrunk) shape
			const PxU8* __restrict polyInds = hullData->getFacesByVertices8();

			//transfrom the vertex from vertex space to shape space
			const Vec3V p = M33MulV3(vertex2Shape, V3LoadU(verts[index]));

			const PxU32 ind = index*3;

			const PxPlane& data1 = hullData->mPolygons[polyInds[ind]].mPlane;
			const PxPlane& data2 = hullData->mPolygons[polyInds[ind+1]].mPlane;
			const PxPlane& data3 = hullData->mPolygons[polyInds[ind+2]].mPlane;

			//transform the normal from vertex space to shape space
			//This is only required if the scale is not uniform
			const Vec3V n1 = V3Normalize(M33TrnspsMulV3(shape2Vertex, V3LoadU(data1.n)));
			const Vec3V n2 = V3Normalize(M33TrnspsMulV3(shape2Vertex, V3LoadU(data2.n)));
			const Vec3V n3 = V3Normalize(M33TrnspsMulV3(shape2Vertex, V3LoadU(data3.n)));

			//This is only required if the scale is not 1
			const FloatV d1 = FSub(margin_, V3Dot(p, n1));
			const FloatV d2 = FSub(margin_, V3Dot(p, n2));
			const FloatV d3 = FSub(margin_, V3Dot(p, n3));


			//This is unavoidable unless we pre-calc the core shape. The intersect point is in shape space
			const Vec3V intersectPoints = intersectPlanes(n1, d1, n2, d2, n3, d3);
			const Vec3V v =V3Sub(p, intersectPoints); 
			marginDif = FSub(V3Length(v), margin_);
			return intersectPoints;
		}



		//This function is used in gjk
		//dir in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space, for non-uniform scale, we don't want the scale in the dir, therefore, we are using
			//the transpose of the inverse of shape2Vertex(which is vertex2shape). This will allow us igore the scale and keep the rotation
			const Vec3V _dir = M33TrnspsMulV3(vertex2Shape, dir);
			//get the extreme point index
			const PxU32 maxIndex = supportVertexIndex(_dir);
			index = (PxI32)maxIndex;
			//p is in the shape space
			const Vec3V p = planeShift(maxIndex, margin);
			support = p;
			return p;
		}

	

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;

			//transform dir from b space to the shape space of a space
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//M33MulV3(skewInvRot, dir);
			const Vec3V p = supportLocal(_dir, support, index);
			//transfrom from a to b space
			const Vec3V ret = aTob.transform(p);
			support = ret;
			return ret;
		}

	};

}

}

#endif	// 
