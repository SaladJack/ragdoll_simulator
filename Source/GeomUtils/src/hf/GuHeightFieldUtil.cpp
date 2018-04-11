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


#include "PsIntrinsics.h"
#include "GuHeightFieldUtil.h"
#include "GuSweepSharedTests.h"

#include "PsFoundation.h"
#include "GuHeightField.h"
#include "GuEntityReport.h"
#include "PxMeshScale.h"

#ifdef __SPU__
#include "CmPS3CellUtil.h"
#endif

using namespace physx;

//#define USE_SIMD_NORMALIZE
#ifdef USE_SIMD_NORMALIZE
#include "PsVecMath.h"
using namespace Ps::aos::;
PX_FORCE_INLINE void SIMD_Normalize(PxVec3& v)
{
	const Vec3V v4 = V3NormaliseFast(V3LoadU(v));
	V3StoreU(v4, v);
}
#endif

void Gu::HeightFieldUtil::computeLocalBounds(PxBounds3& bounds) const
{
	PxMeshScale scale(PxVec3(mHfGeom->rowScale, mHfGeom->heightScale, mHfGeom->columnScale), PxQuat(PxIdentity));
	PxMat33 mat33=scale.toMat33();
	bounds.minimum=mat33.transform(mHeightField->getData().mAABB.minimum);
	bounds.maximum=mat33.transform(mHeightField->getData().mAABB.maximum);
	if(mHeightField->getData().thickness < 0)
		bounds.minimum.y += mHeightField->getData().thickness;
	else
		bounds.maximum.y += mHeightField->getData().thickness;
}

PxU32 Gu::HeightFieldUtil::getFaceIndexAtShapePoint(PxReal x, PxReal z) const
{
	if (isShapePointOnHeightField(x, z)) 
	{
		const PxU32 triangleIndex = mHeightField->getTriangleIndex(x * mOneOverRowScale, z * mOneOverColumnScale);
		return (mHeightField->getTriangleMaterial(triangleIndex) != PxHeightFieldMaterial::eHOLE) ? triangleIndex : 0xffffffff;
	}
	return 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getFaceIndexAtTriangleIndex(PxU32 triangleIndex) const
{
	return (mHeightField->getTriangleMaterial(triangleIndex) != PxHeightFieldMaterial::eHOLE) ? triangleIndex : 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getFaceIndexAtShapePointNoTest(PxReal x, PxReal z) const
{
	PX_ASSERT(isShapePointOnHeightField(x, z));

	const PxU32 triangleIndex = mHeightField->getTriangleIndex(x * mOneOverRowScale, z * mOneOverColumnScale);
	return (mHeightField->getTriangleMaterial(triangleIndex) != PxHeightFieldMaterial::eHOLE) ? triangleIndex : 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getFaceIndexAtShapePointNoTest2(PxU32 cell, PxReal fracX, PxReal fracZ) const
{
	const PxU32 triangleIndex = mHeightField->getTriangleIndex2(cell, fracX, fracZ);
	return (mHeightField->getTriangleMaterial(triangleIndex) != PxHeightFieldMaterial::eHOLE) ? triangleIndex : 0xffffffff;
}


PxVec3 Gu::HeightFieldUtil::getSmoothNormalAtShapePoint(PxReal x, PxReal z) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(getFaceIndexAtShapePoint(x, z) != 0xffffffff);
#endif
	x *= mOneOverRowScale;
	z *= mOneOverColumnScale;

	PxReal fracX, fracZ;
	const PxU32 cell = mHeightField->computeCellCoordinates(x, z, fracX, fracZ);

	if (mHeightField->isZerothVertexShared(cell))
	{
		//    <----Z---+
		//      +----+ | 
		//      |   /| |
		//      |  / | X
		//      | /  | |
		//      |/   | |
		//      +----+ |
		//             V
		if (fracZ > fracX)
		{
			//    <----Z---+
			//      1----0 | 
			//      |   /  |
			//      |  /   X
			//      | /    |
			//      |/     |
			//      2      |
			//             V
			const PxVec3 n0 = getVertexNormal(cell);
			const PxVec3 n1 = getVertexNormal(cell + 1);
			const PxVec3 n2 = getVertexNormal(cell + mHeightField->getNbColumnsFast() + 1);
			return n0 + fracZ*(n1-n0) + fracX*(n2-n1);
		}
		else
		{
			//    <----Z---+
			//           0 | 
			//          /| |
			//         / | X
			//        /  | |
			//       /   | |
			//      2----1 |
			//             V
			const PxVec3 n0 = getVertexNormal(cell);
			const PxVec3 n1 = getVertexNormal(cell + mHeightField->getNbColumnsFast());
			const PxVec3 n2 = getVertexNormal(cell + mHeightField->getNbColumnsFast() + 1);
			return n0 + fracX*(n1-n0) + fracZ*(n2-n1);
		}
	}
	else
	{
		//    <----Z---+
		//      +----+ | 
		//      |\   | |
		//      | \  | X
		//      |  \ | |
		//      |   \| |
		//      +----+ |
		//             V
		if (fracX + fracZ < 1)
		{
			//    <----Z---+
			//      1----0 | 
			//       \   | |
			//        \  | X
			//         \ | |
			//          \| |
			//           2 |
			//             V
			const PxVec3 n0 = getVertexNormal(cell);
			const PxVec3 n1 = getVertexNormal(cell + 1);
			const PxVec3 n2 = getVertexNormal(cell + mHeightField->getNbColumnsFast());
			return n0 + fracZ*(n1-n0) + fracX*(n2-n0);
		}
		else
		{
			//    <----Z---+
			//      2      | 
			//      |\     |
			//      | \    X
			//      |  \   |
			//      |   \  |
			//      0----1 |
			//             V
			//
			// Note that we need to flip fracX and fracZ since we are moving the origin
			const PxVec3 n0 = getVertexNormal(cell + mHeightField->getNbColumnsFast() + 1);
			const PxVec3 n1 = getVertexNormal(cell + mHeightField->getNbColumnsFast());
			const PxVec3 n2 = getVertexNormal(cell + 1);
			return n0 + (1-fracZ)*(n1-n0) + (1-fracX)*(n2 - n0);
		}
	}		
}

/*
PxVec3 Gu::HeightFieldUtil::getVertexNormal(PxU32 vertexIndex) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField.isValidVertex(vertexIndex));
#endif
//	PxU32 edges[8];
//	const PxU32 edgeCount = mHeightField.getVertexEdgeIndices(vertexIndex, edges);

const PxU32 nbColumns = mHeightField.getData().columns;
const PxU32 row = vertexIndex / nbColumns;
const PxU32 column = vertexIndex % nbColumns;
EdgeData edgeIndices[8];
const PxU32 edgeCount = ::getVertexEdgeIndices(mHeightField, vertexIndex, row, column, edgeIndices);


	PxVec3 n(0,0,0);
	PxU32 c = 0;
	PxVec3 tn;
	for (PxU32 i=0; i<edgeCount; i++)
	{
		PxU32 faces[2];
//		const PxU32 faceCount = mHeightField.getEdgeTriangleIndices(edges[i], faces);
const PxU32 faceCount = ::getEdgeTriangleIndices(mHeightField, edgeIndices[i], faces);

		switch(faceCount)
		{
		case 2:
			if (mHeightField.getTriangleMaterial(faces[1]) != PxHeightFieldMaterial::eHOLE)
			{
				tn = mHeightField.getTriangleNormalInternal(faces[1]);
#ifdef USE_SIMD_NORMALIZE
				SIMD_Normalize(tn);
#else
				tn.normalize();
#endif
				n+=tn;
				c++;
			}
			// yes, the fall-through is intentional :)
		case 1:
			if (mHeightField.getTriangleMaterial(faces[0]) != PxHeightFieldMaterial::eHOLE)
			{
				tn = mHeightField.getTriangleNormalInternal(faces[0]);
#ifdef USE_SIMD_NORMALIZE
				SIMD_Normalize(tn);
#else
				tn.normalize();
#endif
				n+=tn;
				c++;
			}
		}
	}
	// this should only be called for solid vertices
	PX_ASSERT(c > 0);
	return hf2shapen(n / PxReal(c));
}
*/
PxVec3 Gu::HeightFieldUtil::getVertexNormal(PxU32 vertexIndex, PxU32 row, PxU32 column) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidVertex(vertexIndex));
#endif
	//	PxU32 edges[8];
	//	const PxU32 edgeCount = mHeightField.getVertexEdgeIndices(vertexIndex, edges);

	//const PxU32 nbColumns = mHeightField.getData().columns;
	//const PxU32 row = vertexIndex / nbColumns;
	//const PxU32 column = vertexIndex % nbColumns;
	PX_ASSERT(row == vertexIndex / mHeightField->getData().columns);
	PX_ASSERT(column == vertexIndex % mHeightField->getData().columns);
	EdgeData edgeIndices[8];
	const PxU32 edgeCount = ::getVertexEdgeIndices(*mHeightField, vertexIndex, row, column, edgeIndices);


	PxVec3 n(0.0f);
	PxVec3 tn;
	for (PxU32 i=0; i<edgeCount; i++)
	{
		PxU32 faces[2];
		//		const PxU32 faceCount = mHeightField.getEdgeTriangleIndices(edges[i], faces);
		const PxU32 faceCount = ::getEdgeTriangleIndices(*mHeightField, edgeIndices[i], faces);

		switch(faceCount)
		{
		case 2:
			if (mHeightField->getTriangleMaterial(faces[1]) != PxHeightFieldMaterial::eHOLE)
			{
				tn = hf2shapen(mHeightField->getTriangleNormalInternal(faces[1])).getNormalized();
				n+=tn;
			}
			// yes, the fall-through is intentional :)
		case 1:
			if (mHeightField->getTriangleMaterial(faces[0]) != PxHeightFieldMaterial::eHOLE)
			{
				tn = hf2shapen(mHeightField->getTriangleNormalInternal(faces[0])).getNormalized();
				n+=tn;
			}
		}
	}

	return n.getNormalized();
}

PxU32 Gu::HeightFieldUtil::findClosestPointsOnCell(
	PxU32 row, PxU32 column, PxVec3 point,
	PxVec3* PX_RESTRICT closestPoints, PxU32* PX_RESTRICT featureCodes,
	bool testFaces, bool testEdges, bool skipEdgesIfFaceHits) const
{
	PxU32 count = 0;

	const PxU32 offset = row * mHeightField->getNbColumnsFast() + column;
	const PxU32 firstEdgeIndex = 3 * offset;

	// ptchernev TODO:
	// move the material assignments to an else in the ifs on triangle material
	// instead of doing it all the time

	PX_ASSERT(row < (mHeightField->getNbRowsFast() - 1));
	PX_ASSERT(column < (mHeightField->getNbColumnsFast() - 1));
	const bool lastRow		= (row == (mHeightField->getNbRowsFast() - 2));
	const bool lastColumn	= (column == (mHeightField->getNbColumnsFast() - 2));

	bool testVertex0		= testEdges;
	bool testColumnEdge0	= testEdges;
	bool testRowEdge0		= testEdges;
	bool testDiagonal		= testEdges;
	bool testVertex1		= lastColumn && testEdges;
	bool testVertex2		= lastRow && testEdges;

	bool testRowEdge1		= lastColumn && testEdges;
	bool testColumnEdge1	= lastRow && testEdges;
	bool testVertex3		= lastRow && lastColumn && testEdges;

	const PxU32 triangleIndex0 = offset << 1;
	const PxMaterialTableIndex materialIndex0 = mHeightField->getTriangleMaterial(triangleIndex0);
	const PxU32 triangleIndex1 = triangleIndex0 + 1;
	const PxMaterialTableIndex materialIndex1 = mHeightField->getTriangleMaterial(triangleIndex1);

	if (testFaces)
	{
		if (materialIndex0 != PxHeightFieldMaterial::eHOLE)
		{
			// face 0
			PxVec3 closestPoint;
			if (findProjectionOnTriangle(triangleIndex0, row, column, point, closestPoint))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(triangleIndex0, eFACE);
				count++;
				testRowEdge0 = false;
				testVertex0 = false;
				testVertex2 = false;
				testDiagonal = false;
			}
		}

		if (materialIndex1 != PxHeightFieldMaterial::eHOLE)
		{
			// face 1			
			PxVec3 closestPoint;
			if (findProjectionOnTriangle(triangleIndex1, row, column, point, closestPoint))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(triangleIndex1, eFACE);
				count++;
				testRowEdge1 = false;
				testVertex1 = false;
				testVertex3 = false;
				testDiagonal = false;
			}
		}
		if (!testEdges)
			return count;
	}

	// if there were any face contacts and we asked to skip edges if face contacts, return current count here
	if (count && skipEdgesIfFaceHits)
		return count;

	const PxU32 nbColumns = mHeightField->getNbColumnsFast();
	if (testVertex0 || testColumnEdge0 || testVertex1)
	{
		PxVec3 closestPoint;
		PxReal t = findClosestPointOnEdge(firstEdgeIndex, offset, row, column, point, closestPoint);
		if (t <= 0)
		{
			if (testVertex0 && 0xffffffff != (getVertexFaceIndex(offset, row, column)))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(row*nbColumns+column, eVERTEX);
				count++;
			}
			testVertex0 = false;
		}
		else if (t < 1)
		{
			if (testColumnEdge0 && 0xffffffff != (getEdgeFaceIndex(firstEdgeIndex)))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(firstEdgeIndex, eEDGE);
				count++;
			}
		}
		else
		{
			if (testVertex1 && 0xffffffff != (getVertexFaceIndex(offset + 1, row, column + 1)))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(row*nbColumns+column+1, eVERTEX);
				count++;
			}
		}
	}

	if (testVertex0 || testRowEdge0 || testVertex2)
	{
		PxVec3 closestPoint;
		PxReal t = findClosestPointOnEdge(firstEdgeIndex + 2, offset, row, column, point, closestPoint);
		if (t <= 0) 
		{
			if (testVertex0 && 0xffffffff != (getVertexFaceIndex(offset, row, column))) 
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(row*nbColumns+column, eVERTEX);
				count++;
			}
		}
		else if(t < 1)
		{
			if (testRowEdge0 && 0xffffffff != (getEdgeFaceIndex(firstEdgeIndex + 2))) 
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(firstEdgeIndex+2, eEDGE);
				count++;
			}
		}
		else 
		{
			if (testVertex2 && 0xffffffff != (getVertexFaceIndex(offset + nbColumns, row + 1, column))) 
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode((row+1)*nbColumns+column, eVERTEX);
				count++;
			}
		}
	}

	if (testColumnEdge1)
	{
		PxVec3 closestPoint;
		PxReal t = findClosestPointOnEdge(firstEdgeIndex + 3 * nbColumns, offset + nbColumns, row + 1, column, point, closestPoint);
		if (t <= 0)
			; // do nothing
		else if (t < 1)
		{
			const PxU32 edgeIndex3 = firstEdgeIndex + 3 * nbColumns;
			if (0xffffffff != (getEdgeFaceIndex(edgeIndex3)))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(edgeIndex3, eEDGE);
				count++;
			}
		}
	}

	if (testRowEdge1)
	{
		PxVec3 closestPoint;
		PxReal t = findClosestPointOnEdge(firstEdgeIndex + 5, offset + 1, row, column + 1, point, closestPoint);
		if (t <= 0)
			; // do nothing
		else if (t < 1)
		{
			if (0xffffffff != (getEdgeFaceIndex(firstEdgeIndex + 5)))
			{
				closestPoints[count] = closestPoint;
				if (featureCodes) featureCodes[count] = makeFeatureCode(firstEdgeIndex+5, eEDGE);
				count++;
			}
		}
	}

	if (testVertex3 && 0xffffffff != (getVertexFaceIndex(offset + nbColumns + 1, row + 1, column + 1)))
	{
		closestPoints[count] = PxVec3((row + 1) * mHfGeom->rowScale, mHfGeom->heightScale * mHeightField->getHeight(offset + mHeightField->getNbColumnsFast() + 1), (column + 1) * mHfGeom->columnScale);
		if (featureCodes) featureCodes[count] = makeFeatureCode((row+1)*nbColumns+column+1, eVERTEX);
		count++;
	}

	if (testDiagonal && 0xffffffff != (getEdgeFaceIndex(firstEdgeIndex + 1)))
	{
		PxVec3 closestPoint;
		PxReal t = findClosestPointOnEdge(firstEdgeIndex + 1, offset, row, column, point, closestPoint);
		if (t <= 0) 
			; // do nothing
		else if (t < 1) 
		{
			closestPoints[count] = closestPoint;
			if (featureCodes) featureCodes[count] = makeFeatureCode(firstEdgeIndex+1, eEDGE);
			count++;
		}
	}

	return count;
}

//PxReal Gu::HeightFieldUtil::findClosestPointOnEdge(PxU32 edgeIndex, const PxVec3& point, PxVec3& closestPoint) const
PxReal Gu::HeightFieldUtil::findClosestPointOnEdge(
	PxU32 edgeIndex, PxU32 cell, PxU32 row, PxU32 column, const PxVec3& point, PxVec3& closestPoint) const
{
//	const PxU32 cell = edgeIndex / 3;
	PX_ASSERT(cell == edgeIndex / 3);
//	const PxU32 row = cell / mHeightField->getNbColumnsFast();
	PX_ASSERT(row == cell / mHeightField->getNbColumnsFast());
//	const PxU32 column = cell % mHeightField->getNbColumnsFast();
	PX_ASSERT(column == cell % mHeightField->getNbColumnsFast());

	PxVec3 origin, direction;
	PxReal lengthSquared;
//	switch (edgeIndex % 3)
	switch (edgeIndex - cell*3)
	{
	case 0:
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y1 = mHfGeom->heightScale * mHeightField->getHeight(cell + 1);
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			const PxReal dy = y1 - y0;
			direction = PxVec3(0, dy, mHfGeom->columnScale);
			lengthSquared = mHfGeom->columnScale * mHfGeom->columnScale + dy * dy;
		}
		break;
	case 1:
		if (mHeightField->isZerothVertexShared(cell))
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y3 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast() + 1);
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			const PxReal dy = y3 - y0;
			direction = PxVec3(mHfGeom->rowScale, dy, mHfGeom->columnScale);
			lengthSquared = mHfGeom->rowScale * mHfGeom->rowScale + mHfGeom->columnScale * mHfGeom->columnScale + dy * dy;
		}
		else
		{
			const PxReal y1 = mHfGeom->heightScale * mHeightField->getHeight(cell + 1);
			const PxReal y2 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast());
			origin = PxVec3(row * mHfGeom->rowScale, y1, (column + 1) * mHfGeom->columnScale);
			const PxReal dy = y2 - y1;
			direction = PxVec3(mHfGeom->rowScale, dy, -mHfGeom->columnScale);
			lengthSquared = mHfGeom->rowScale * mHfGeom->rowScale + mHfGeom->columnScale * mHfGeom->columnScale + dy * dy;
		}
		break;
	case 2:
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y2 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast());
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			const PxReal dy = y2 - y0;
			direction = PxVec3(mHfGeom->rowScale, dy, 0);
			lengthSquared = mHfGeom->rowScale * mHfGeom->rowScale + dy * dy;
		}
		break;
	default:
		origin = direction = PxVec3(PxReal(0));
		lengthSquared = 0.0f;
		PX_ALWAYS_ASSERT_MESSAGE("Invalid edge index in findClosestPointOnEdge");
	} //	switch (edgeIndex % 3)

	const PxVec3 relative = point - origin;
	const PxReal t = relative.dot(direction) / lengthSquared;
	if (t < 0)
		closestPoint = origin;
	else if (t > 1)
		closestPoint = origin + direction;
	else
		closestPoint = origin + direction * t;

	return t;
}

PxU32 Gu::HeightFieldUtil::getVertexFaceIndex(PxU32 vertexIndex, PxU32 row, PxU32 column) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidVertex(vertexIndex));
#endif

//	PxU32 edgeIndices[8];
//	const PxU32 count = mHeightField->getVertexEdgeIndices(vertexIndex, edgeIndices);

//const PxU32 nbColumns = mHeightField->getData().columns;
//const PxU32 row = vertexIndex / nbColumns;
//const PxU32 column = vertexIndex % nbColumns;
PX_ASSERT(row == vertexIndex / mHeightField->getData().columns);
PX_ASSERT(column == vertexIndex % mHeightField->getData().columns);
EdgeData edgeIndices[8];
const PxU32 count = ::getVertexEdgeIndices(*mHeightField, vertexIndex, row, column, edgeIndices);

	for (PxU32 i = 0; i<count; i+= 2)
	{
		const PxU32 index = getEdgeFaceIndex(edgeIndices[i].edgeIndex, edgeIndices[i].cell, edgeIndices[i].row, edgeIndices[i].column);
		if (index != 0xffffffff) return index;
	}
	return 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getEdgeFaceIndex(PxU32 edgeIndex) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif
	PxU32 faceIndices[2];
	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices);
	if (count > 1) 
	{
		// ptchernev TODO: this is a bit arbitrary
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
		if (mHeightField->getTriangleMaterial(faceIndices[1]) != PxHeightFieldMaterial::eHOLE) return faceIndices[1];
	} 
	else 
	{
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
	}
	return 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getEdgeFaceIndex(PxU32 edgeIndex, PxU32 cell, PxU32 row, PxU32 column) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif
	PxU32 faceIndices[2];
	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices, cell, row, column);
	if (count > 1) 
	{
		// ptchernev TODO: this is a bit arbitrary
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
		if (mHeightField->getTriangleMaterial(faceIndices[1]) != PxHeightFieldMaterial::eHOLE) return faceIndices[1];
	} 
	else 
	{
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
	}
	return 0xffffffff;
}

PxU32 Gu::HeightFieldUtil::getEdgeFaceIndex(PxU32 edgeIndex, PxU32 count, const PxU32* PX_RESTRICT faceIndices) const
{
	PX_UNUSED(edgeIndex);

#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif
//	PxU32 faceIndices[2];
//	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices);
	if (count > 1) 
	{
		// ptchernev TODO: this is a bit arbitrary
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
		if (mHeightField->getTriangleMaterial(faceIndices[1]) != PxHeightFieldMaterial::eHOLE) return faceIndices[1];
	} 
	else 
	{
		if (mHeightField->getTriangleMaterial(faceIndices[0]) != PxHeightFieldMaterial::eHOLE) return faceIndices[0];
	}
	return 0xffffffff;
}

bool Gu::HeightFieldUtil::findProjectionOnTriangle(PxU32 triangleIndex, PxU32 row, PxU32 column, const PxVec3& point, PxVec3& projection) const
{
	const PxU32 cell = (triangleIndex >> 1);
	PX_ASSERT(row == cell / mHeightField->getNbColumnsFast());
	PX_ASSERT(column == cell % mHeightField->getNbColumnsFast());
	const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
	const PxReal y1 = mHfGeom->heightScale * mHeightField->getHeight(cell + 1);
	const PxReal y2 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast());
	const PxReal y3 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast() + 1);
	PxVec3 origin;
	PxReal h0, h1, h2, uInvScale, vInvScale;

	// specify a triangle according to current triangle index as origin, 3 heights and (uInvScale, vInvScale) vector for (z,x)
	// set uInvScale in h1-h0; vScale in h2-h0 direction
	if (mHeightField->isZerothVertexShared(cell)) 
	{
		//    COLUMN -->
		//
		// R  0---1
		// O  |\ 1|
		// W  | \ |
		// |  |0 \|
		// |  2---3
		// V 
		if ((triangleIndex & 1) == 0)
		{
			// case 0
			// face 0
			origin = PxVec3((row + 1) * mHfGeom->rowScale, y2, column * mHfGeom->columnScale);
			// origin -> 2
			// verts -> 2,3,0
			h0 = y2;
			h1 = y3;
			h2 = y0;
			uInvScale = mOneOverColumnScale;
			vInvScale = -mOneOverRowScale;
		}
		else // if (testFace1)
		{
			// case 1
			// face 1			
			origin = PxVec3(row * mHfGeom->rowScale, y1, (column + 1) * mHfGeom->columnScale);
			// origin -> 1
			// verts -> 1,0,3
			h0 = y1;
			h1 = y0;
			h2 = y3;
			uInvScale = -mOneOverColumnScale;
			vInvScale = mOneOverRowScale;
		}
	}
	else
	{
		//    COLUMN -->
		//
		// R  0---1
		// O  |0 /|
		// W  | / |
		// |  |/ 1|
		// |  2---3
		// V 
		if ((triangleIndex & 1) == 0)
		{
			// case 2
			// face 0			
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			// origin -> 0
			// verts -> 0,1,2
			h0 = y0;
			h1 = y1;
			h2 = y2;
			uInvScale = mOneOverColumnScale;
			vInvScale = mOneOverRowScale;
		}
		else
		{
			// case 3
			// face 1			
			origin = PxVec3((row + 1) * mHfGeom->rowScale, y3, (column + 1) * mHfGeom->columnScale);
			// origin -> 3
			// verts -> 3,2,1
			h0 = y3;
			h1 = y2;
			h2 = y1;
			uInvScale = -mOneOverColumnScale;
			vInvScale = -mOneOverRowScale;
		}
	}

	// vector from triangle origin to point we want to project
	const PxVec3 relative = point - origin;

	// Looking at the triangle diagram for case 2
	// The normal computation should be
	// n = (p1-p0) x (p2-p0)
	// For a right handed cross product that's pointing into the screen (negative h), so -n is in the direction of increasing h
	// cs = column scale, rs = row scale, h10 = h1-h0; u=column, v=row
	//		   (i   j    k);
	// p1-p0 = (cs, h10, 0); this is column, u and z
	// p2-p0 = (0,  h20, rs); this is row, v and x
	// n = (h10*rs, -cs*rs, +cs*h20)
	// n/(cs*rs) = (h10/cs, -1, h20/rs)
	// -n = (-h10/cs, 1, -h20/rs)
	PxReal h10 = h1-h0, h20 = h2-h0;
	PxReal nu = -h10 * uInvScale;
	PxReal nv = -h20 * vInvScale;

	PxVec3 n(nv, 1.0f, nu); // for whatever reason.. x is v, z is u.
	//n *= 1.0f / PxSqrt(nu*nu + nv*nv + 1.0f); // technically we need to do this but since later

	// project relative onto the n plane, it gives us unclipped projection onto the triangle
	// the computation without sqrt shortcut is relPrj = relative - n.dot(relative)*n, but because we divide by sqrt^2 we skip sqrt
	PxVec3 relPrj = relative - n.dot(relative)* (1.0f/(nu*nu+nv*nv+1.0f)) * n;

	// project relPrj onto 2d UV plane with h = 0 oriented according to vInvScale, uInvScale (for x and z)
	// to convert to HF cell coords we'd multiply by inv scale, after that the coords should be >0 and the sum within 1 to be
	// inside of a 2d triangle
	PxReal scaledX = relPrj.x * vInvScale, scaledZ = relPrj.z * uInvScale;
	//PxVec3 testProjection = relPrj + origin;
	//PxVec3 testN = (point - testProjection).getNormalized();
	if (scaledX > 0.0f && scaledZ > 0.0f && scaledX + scaledZ < 1.0f)
	{
		projection = relPrj + origin;
		return true;
	}

	return false;
}

bool Gu::HeightFieldUtil::isCollisionEdge(PxU32 edgeIndex) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif

	// This code was simple, readable but slow
	// return isBoundaryEdge(edgeIndex) || (mHeightField->isConvexEdge(edgeIndex) && isSolidEdge(edgeIndex));

	PxU32 faceIndices[2];
	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices);
	if (count > 1) 
	{
		PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		PxMaterialTableIndex mat1 = mHeightField->getTriangleMaterial(faceIndices[1]);
		if (mat0 == PxHeightFieldMaterial::eHOLE) return (mat1 != PxHeightFieldMaterial::eHOLE);
		if (mat1 == PxHeightFieldMaterial::eHOLE) return (mat0 != PxHeightFieldMaterial::eHOLE);
	} 
	else 
	{
		if (mHeightField->getFlagsFast() & PxHeightFieldFlag::eNO_BOUNDARY_EDGES) return false;
		PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		return (mat0 != PxHeightFieldMaterial::eHOLE);
	}

	return mHeightField->isConvexEdge(edgeIndex);
}

bool Gu::HeightFieldUtil::isCollisionEdge(PxU32 edgeIndex, PxU32 count, const PxU32* PX_RESTRICT faceIndices, PxU32 cell, PxU32 row, PxU32 column) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif

	// This code was simple, readable but slow
	// return isBoundaryEdge(edgeIndex) || (mHeightField->isConvexEdge(edgeIndex) && isSolidEdge(edgeIndex));

//	PxU32 faceIndices[2];
//	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices);
	if (count > 1) 
	{
		PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		PxMaterialTableIndex mat1 = mHeightField->getTriangleMaterial(faceIndices[1]);
		if (mat0 == PxHeightFieldMaterial::eHOLE) return (mat1 != PxHeightFieldMaterial::eHOLE);
		if (mat1 == PxHeightFieldMaterial::eHOLE) return (mat0 != PxHeightFieldMaterial::eHOLE);
	} 
	else 
	{
		if (mHeightField->getFlagsFast() & PxHeightFieldFlag::eNO_BOUNDARY_EDGES) return false;
		PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		return (mat0 != PxHeightFieldMaterial::eHOLE);
	}

//	return mHeightField->isConvexEdge(edgeIndex);
	return mHeightField->isConvexEdge(edgeIndex, cell, row, column);
}

void Gu::HeightFieldUtil::getEdge(PxU32 edgeIndex, PxU32 cell, PxU32 row, PxU32 column, PxVec3& origin, PxVec3& extent) const
{
#ifdef PX_HEIGHTFIELD_DEBUG		
	PX_ASSERT(mHeightField->isValidEdge(edgeIndex));
#endif
//	const PxU32 cell = edgeIndex / 3;
	PX_ASSERT(cell == edgeIndex / 3);
//	const PxU32 row = cell / mHeightField->getNbColumnsFast();
	PX_ASSERT(row == cell / mHeightField->getNbColumnsFast());
//	const PxU32 column = cell % mHeightField->getNbColumnsFast();
	PX_ASSERT(column == cell % mHeightField->getNbColumnsFast());

//	switch (edgeIndex % 3)
	switch (edgeIndex - cell*3)
	{
	case 0:
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y1 = mHfGeom->heightScale * mHeightField->getHeight(cell + 1);
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			extent = PxVec3(0, y1 - y0, mHfGeom->columnScale);
		}
		break;
	case 1:
		if (mHeightField->isZerothVertexShared(cell))
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y3 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast() + 1);
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			extent = PxVec3(mHfGeom->rowScale, y3 - y0, mHfGeom->columnScale);
		}
		else
		{
			const PxReal y1 = mHfGeom->heightScale * mHeightField->getHeight(cell + 1);
			const PxReal y2 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast());
			origin = PxVec3(row * mHfGeom->rowScale, y1, (column + 1) * mHfGeom->columnScale);
			extent = PxVec3(mHfGeom->rowScale, y2 - y1, -mHfGeom->columnScale);
		}
		break;
	case 2:
		{
			const PxReal y0 = mHfGeom->heightScale * mHeightField->getHeight(cell);
			const PxReal y2 = mHfGeom->heightScale * mHeightField->getHeight(cell + mHeightField->getNbColumnsFast());
			origin = PxVec3(row * mHfGeom->rowScale, y0, column * mHfGeom->columnScale);
			extent = PxVec3(mHfGeom->rowScale, y2 - y0, 0);
		}
		break;
	}
}

bool Gu::HeightFieldUtil::overlapAABBTriangles(
	const PxTransform& pose, const PxBounds3& bounds, PxU32 flags, EntityReport<PxU32>* callback) const
{
	PxBounds3 localBounds = bounds;

	if(flags & PxHfQueryFlags::eWORLD_SPACE)
	{
		PX_ASSERT(!localBounds.isEmpty());
		localBounds = PxBounds3::transformFast(pose.getInverse(), localBounds);
	}

	#if MIDPHASE_TRACE
	PxBounds3 scaledBounds = localBounds;
	#endif

	localBounds.minimum.x *= mOneOverRowScale;
	localBounds.minimum.y *= mOneOverHeightScale;
	localBounds.minimum.z *= mOneOverColumnScale;

	localBounds.maximum.x *= mOneOverRowScale;
	localBounds.maximum.y *= mOneOverHeightScale;
	localBounds.maximum.z *= mOneOverColumnScale;

	if (mHfGeom->rowScale < 0) 
	{
		PxReal swap = localBounds.minimum.x;
		localBounds.minimum.x = localBounds.maximum.x;
		localBounds.maximum.x = swap;
	}

	if (mHfGeom->columnScale < 0) 
	{
		PxReal swap = localBounds.minimum.z;
		localBounds.minimum.z = localBounds.maximum.z;
		localBounds.maximum.z = swap;
	}

	// early exit for aabb does not overlap in XZ plane
	// DO NOT MOVE: since rowScale / columnScale may be negative this has to be done after scaling localBounds
	if (localBounds.minimum.x > mHeightField->getNbRowsFast() - 1) 
		return false;
	if (localBounds.minimum.z > mHeightField->getNbColumnsFast() - 1) 
		return false;
	if (localBounds.maximum.x < 0) 
		return false;
	if (localBounds.maximum.z < 0) 
		return false;

	PxU32 minRow = mHeightField->getMinRow(localBounds.minimum.x);
	PxU32 maxRow = mHeightField->getMaxRow(localBounds.maximum.x);
	PxU32 minColumn = mHeightField->getMinColumn(localBounds.minimum.z);
	PxU32 maxColumn = mHeightField->getMaxColumn(localBounds.maximum.z);

	PxU32 maxNbTriangles = 2 * (maxColumn - minColumn) * (maxRow - minRow);

	if (maxNbTriangles == 0) 
		return false;

	if (flags & PxHfQueryFlags::eFIRST_CONTACT) maxNbTriangles = 1;

	static const PxU32 bufferSize = HF_SWEEP_REPORT_BUFFER_SIZE;
	PxU32 indexBuffer[bufferSize];
	PxU32 indexBufferUsed = 0;
	PxU32 nb = 0;

	PxU32 offset = minRow * mHeightField->getNbColumnsFast() + minColumn;

	const PxReal& miny = localBounds.minimum.y;
	const PxReal& maxy = localBounds.maximum.y;

	#if MIDPHASE_TRACE
	Ps::Array<PxU32> touchedFaces;
	#endif
	for (PxU32 row = minRow; row < maxRow; row++)
	{
		for (PxU32 column = minColumn; column < maxColumn; column++)
		{
			PxReal h0 = mHeightField->getHeight(offset);
			PxReal h1 = mHeightField->getHeight(offset + 1);
			PxReal h2 = mHeightField->getHeight(offset + mHeightField->getNbColumnsFast());
			PxReal h3 = mHeightField->getHeight(offset + mHeightField->getNbColumnsFast() + 1);
			if (!((maxy < h0 && maxy < h1 && maxy < h2 && maxy < h3) || (miny > h0 && miny > h1 && miny > h2 && miny > h3)))
			{
				PxU32 material0 = mHeightField->getMaterialIndex0(offset);
				if (material0 != PxHeightFieldMaterial::eHOLE) 
				{
					if(indexBufferUsed >= bufferSize)
					{
						callback->onEvent(indexBufferUsed, indexBuffer);
						indexBufferUsed = 0;
					}

					indexBuffer[indexBufferUsed++] = offset << 1;
					#if MIDPHASE_TRACE
					touchedFaces.pushBack(offset << 1);
					#endif
					nb++;

					if (flags & PxHfQueryFlags::eFIRST_CONTACT) goto search_done;
				}

				PxU32 material1 = mHeightField->getMaterialIndex1(offset);
				if (material1 != PxHeightFieldMaterial::eHOLE)
				{
					if(indexBufferUsed >= bufferSize)
					{
						callback->onEvent(indexBufferUsed, indexBuffer);
						indexBufferUsed = 0;
					}

					indexBuffer[indexBufferUsed++] = (offset << 1) + 1;
					#if MIDPHASE_TRACE
					touchedFaces.pushBack((offset << 1)+1);
					#endif
					nb++;

					if (flags & PxHfQueryFlags::eFIRST_CONTACT) goto search_done;
				}
			}
			offset++;
		}
		offset += (mHeightField->getNbColumnsFast() - (maxColumn - minColumn));
	}

search_done:

	if(indexBufferUsed > 0)
		callback->onEvent(indexBufferUsed, indexBuffer);

		#if MIDPHASE_TRACE
			PxVec3 inflate = (scaledBounds.maximum-scaledBounds.minimum)*0.5f;
			PxVec3 center = (scaledBounds.minimum+scaledBounds.maximum)*0.5f;
			PxVec3 dir(0);
			PxReal maxT(0);
			bool newHf = mHeightField->mSampleStride <= 4;
			MPTContext* ctx = MPT_GET_CONTEXT;
			if (newHf)
			{
				PxU32 meshId = getMeshId();
				((Gu::HeightField*)mHeightField)->mSampleStride = (meshId+5);

				// generate hf mesh verts if this is the first time we see it
				Ps::Array<PxVec3> verts;
				Ps::Array<PxU32> inds;
				const PxI32 nbCols = (PxI32)mHeightField->getNbColumnsFast(), nbRows = (PxI32)mHeightField->getNbRowsFast();
				PxU32 numVerts, numInds;
				generateMesh(PxU32(nbCols), PxU32(nbRows), verts, inds, numVerts, numInds);
				writeTrace(ctx->tag, pose, PxMeshScale(), touchedFaces, true, meshId, verts.begin(), numVerts, inds.begin(), numInds, false, &center, &dir, &maxT, true, &inflate, NULL, mHfGeom);
			} else
			{
				PxU32 meshId = mHeightField->mSampleStride-5;
				writeTrace(ctx->tag, pose, PxMeshScale(), touchedFaces, false, meshId, NULL, 0, NULL, 0, false, &center, &dir, &maxT, true, &inflate, NULL, mHfGeom);
			}
		#endif


	return nb > 0;
}

PxU32 Gu::HeightFieldUtil::getTriangle(const PxTransform& pose, PxTriangle& worldTri,
									   PxU32* _vertexIndices, PxU32* adjacencyIndices, PxTriangleID triangleIndex, bool worldSpaceTranslation, bool worldSpaceRotation) const
{
#ifdef PX_CHECKED
	if (!mHeightField->isValidTriangle(triangleIndex)) 
	{
	#ifndef __SPU__
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "HeightFieldShape::getTriangle: Invalid triangle index!");
	#endif
		return 0;
	}
#endif

	PxVec3 handedness(1.0f);	// Vector to invert normal coordinates according to the heightfield scales
	bool wrongHanded = false;
	if (mHfGeom->columnScale < 0)
	{
		wrongHanded = !wrongHanded;
		handedness.z = -1.0f;
	}
	if (mHfGeom->rowScale < 0)
	{
		wrongHanded = !wrongHanded;
		handedness.x = -1.0f;
	}
	if (mHeightField->getThicknessFast() > 0)
	{
		wrongHanded = !wrongHanded;
		handedness.y = -1.0f;
	}

/*	if (0) // ptchernev: Iterating over triangles becomes a pain.
	{
		if (mHeightField.getTriangleMaterial(triangleIndex) == mHfGeom.holeMaterialIndex)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "HeightFieldShape::getTriangle: Non-existing triangle (triangle has hole material)!");
			return 0;
		}
	}*/

	PxU32 vertexIndices[3];
	if (wrongHanded)
		mHeightField->getTriangleVertexIndices(triangleIndex, vertexIndices[0], vertexIndices[2], vertexIndices[1]);
	else
		mHeightField->getTriangleVertexIndices(triangleIndex, vertexIndices[0], vertexIndices[1], vertexIndices[2]);

	if(adjacencyIndices)
	{
		if (wrongHanded)
			mHeightField->getTriangleAdjacencyIndices(triangleIndex, vertexIndices[0], vertexIndices[2], vertexIndices[1],adjacencyIndices[2],adjacencyIndices[1],adjacencyIndices[0]);
		else
			mHeightField->getTriangleAdjacencyIndices(triangleIndex, vertexIndices[0], vertexIndices[1], vertexIndices[2],adjacencyIndices[0],adjacencyIndices[1],adjacencyIndices[2]);
	}

	if(_vertexIndices)
	{
		_vertexIndices[0] = vertexIndices[0];
		_vertexIndices[1] = vertexIndices[1];
		_vertexIndices[2] = vertexIndices[2];
	}

	if (worldSpaceRotation)
	{
		if (worldSpaceTranslation)
		{
			for (PxU32 vi = 0; vi < 3; vi++)
				worldTri.verts[vi] = hf2worldp(pose, mHeightField->getVertex(vertexIndices[vi]));
		}
		else
		{
			for (PxU32 vi = 0; vi < 3; vi++)
			{
				// TTP 2390 
				// local space here is rotated (but not translated) world space
				worldTri.verts[vi] = pose.q.rotate(hf2shapep(mHeightField->getVertex(vertexIndices[vi])));
			}
		}
	}
	else
	{
		if (worldSpaceTranslation)
		{
			for (PxU32 vi = 0; vi < 3; vi++)
				worldTri.verts[vi] = hf2shapep(mHeightField->getVertex(vertexIndices[vi])) + pose.p;		
		}
		else
		{
			for (PxU32 vi = 0; vi < 3; vi++)
				worldTri.verts[vi] = hf2shapep(mHeightField->getVertex(vertexIndices[vi]));		
		}
	}
	return PxU32(mHeightField->getTriangleMaterial(triangleIndex) != PxHeightFieldMaterial::eHOLE);
}


bool Gu::HeightFieldUtil::isBoundaryEdge(PxU32 edgeIndex) const
{
#ifdef PX_HEIGHTFIELD_DEBUG
	PX_ASSERT(mHeightField.isValidEdge(edgeIndex));
#endif
	PxU32 faceIndices[2];
	const PxU32 count = mHeightField->getEdgeTriangleIndices(edgeIndex, faceIndices);
	if (count > 1) 
	{
		const PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		const PxMaterialTableIndex mat1 = mHeightField->getTriangleMaterial(faceIndices[1]);
		if (mat0 == PxHeightFieldMaterial::eHOLE) return (mat1 != PxHeightFieldMaterial::eHOLE);
		if (mat1 == PxHeightFieldMaterial::eHOLE) return (mat0 != PxHeightFieldMaterial::eHOLE);
	}
	else 
	{
		const PxMaterialTableIndex mat0 = mHeightField->getTriangleMaterial(faceIndices[0]);
		return (mat0 != PxHeightFieldMaterial::eHOLE);
	}
	return false;
}


/*PxReal Gu::HeightFieldUtil::getHeightAtShapePoint(PxReal x, PxReal z) const
{
	return mHfGeom.heightScale * mHeightField->getHeightInternal(x * mOneOverRowScale, z * mOneOverColumnScale);
}*/


/*
PxVec3 Gu::HeightFieldUtil::getNormalAtShapePoint(PxReal x, PxReal z) const
{
	return hf2shapen(mHeightField->getNormal_(x * mOneOverRowScale, z * mOneOverColumnScale));
}
*/

#if MIDPHASE_TRACE
void Gu::HeightFieldUtil::generateMesh(PxU32 nbCols, PxU32 nbRows, Ps::Array<PxVec3>& verts, Ps::Array<PxU32>& inds, PxU32& numVerts, PxU32& numInds) const
{
	numVerts = nbCols*nbRows;
	verts.resize(numVerts);
	PxU32 numTris = nbCols*nbRows*2; // 2 tris per hf quad
	inds.resize(numTris*3); // resize with some padding
	for (PxU32 iRow = 0; iRow < nbRows; iRow++)
		for (PxU32 iCol = 0; iCol < nbCols; iCol++)
		{
			// row = x|u, column = z|v
			const PxU32 vertexIndex = iRow * nbCols + iCol;
			// heightScale is factored in by getHeightAtShapePoint2
			PxVec3 hfGridPoint(iRow*mHfGeom->rowScale, mHeightField->getHeight(vertexIndex)*mHfGeom->heightScale, iCol*mHfGeom->columnScale);
			verts[vertexIndex] = hfGridPoint;
		}
	PxU32* inds1 = inds.begin();
	numInds = 0;
	for (PxU32 iTri = 0; iTri < numTris; iTri++)
	{
		PxTriangle tri;
		if ( ((Gu::HeightField*)mHfGeom->heightField)->isValidTriangle(iTri) )
		{
			PxMeshQuery::getTriangle(*mHfGeom, PxTransform(PxIdentity), iTri, tri, inds1);
			PX_CHECK(verts[inds1[0]] == tri.verts[0]);
			PX_CHECK(verts[inds1[1]] == tri.verts[1]);
			PX_CHECK(verts[inds1[2]] == tri.verts[2]);
			inds1 += 3;
			numInds += 3;
		}
	}
}
#endif
