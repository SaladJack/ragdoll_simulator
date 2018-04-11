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


#include "PxVisualizationParameter.h"
#include "PsIntrinsics.h"
#include "CmPhysXCommon.h"
#include "CmRenderOutput.h"
#include "PsMathUtils.h"
#include "GuConvexMesh.h"
#include "GuTriangle32.h"
#include "GuBigConvexData2.h"
#include "GuSerialize.h"
#include "GuMeshFactory.h"
#include "CmUtils.h"

using namespace physx;
using namespace Gu;

// PX_SERIALIZATION
#include "PsIntrinsics.h"
//~PX_SERIALIZATION

bool Gu::ConvexMesh::getPolygonData(PxU32 i, PxHullPolygon& data) const
{
	if(i>=mHullData.mNbPolygons)
		return false;

	const HullPolygonData& poly = mHullData.mPolygons[i];
	data.mPlane[0]	= poly.mPlane.n.x;
	data.mPlane[1]	= poly.mPlane.n.y;
	data.mPlane[2]	= poly.mPlane.n.z;
	data.mPlane[3]	= poly.mPlane.d;
	data.mNbVerts	= poly.mNbVerts;
	data.mIndexBase	= poly.mVRef8;
	return true;
}

/// ======================================

static void initConvexHullData(Gu::ConvexHullData& data)
{
	data.mAABB.setEmpty();
	data.mCenterOfMass = PxVec3(0);
	data.mNbEdges = 0;
	data.mNbHullVertices = 0;
	data.mNbPolygons = 0;
	data.mPolygons = NULL;
	data.mBigConvexRawData = NULL;
	data.mInternal.mRadius = 0.0f;
	data.mInternal.mExtents[0] = data.mInternal.mExtents[1] = data.mInternal.mExtents[2] = 0.0f;
}

Gu::ConvexMesh::ConvexMesh()
: PxConvexMesh(PxConcreteType::eCONVEX_MESH, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
, mNb				(0)
, mBigConvexData	(NULL)
, mMass				(0)
, mInertia			(PxMat33(PxIdentity))
{
	initConvexHullData(mHullData);
}


Gu::ConvexMesh::~ConvexMesh()
{
// PX_SERIALIZATION
	if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
//~PX_SERIALIZATION
	{
		PX_DELETE_POD(mHullData.mPolygons);
		PX_DELETE_AND_RESET(mBigConvexData);
	}
}

// PX_SERIALIZATION
void Gu::ConvexMesh::exportExtraData(PxSerializationContext& stream)
{
	stream.alignData(PX_SERIAL_ALIGN);
	const PxU32 bufferSize = computeBufferSize(mHullData, getNb());
	stream.writeData(mHullData.mPolygons, bufferSize);

	if(mBigConvexData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mBigConvexData, sizeof(BigConvexData));

		mBigConvexData->exportExtraData(stream);
	}
}

void Gu::ConvexMesh::importExtraData(PxDeserializationContext& context)
{
	const PxU32 bufferSize = computeBufferSize(mHullData, getNb());
	mHullData.mPolygons = (Gu::HullPolygonData*)context.readExtraData<PxU8, PX_SERIAL_ALIGN>(bufferSize);

	if(mBigConvexData)
	{
		mBigConvexData = context.readExtraData<BigConvexData, PX_SERIAL_ALIGN>();
		new(mBigConvexData)BigConvexData(PxEmpty);
		mBigConvexData->importExtraData(context);
		mHullData.mBigConvexRawData = &mBigConvexData->mData;
	}
}

Gu::ConvexMesh* Gu::ConvexMesh::createObject(PxU8*& address, PxDeserializationContext& context)
{
	ConvexMesh* obj = new (address) ConvexMesh(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(ConvexMesh);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

static bool convexHullLoad(Gu::ConvexHullData& data, PxInputStream& stream, PxBitAndDword& bufferSize)
{
	PxU32 version;
	bool Mismatch;
	if(!ReadHeader('C', 'L', 'H', 'L', version, Mismatch, stream))
		return false;

	if(!ReadHeader('C', 'V', 'H', 'L', version, Mismatch, stream))
		return false;

	PxU32 Nb;

	// Import figures
	{
		PxU32 tmp[4];
		ReadDwordBuffer(tmp, 4, Mismatch, stream);
		data.mNbHullVertices	= Ps::to8(tmp[0]);
		data.mNbEdges			= Ps::to16(tmp[1]);
		data.mNbPolygons		= Ps::to8(tmp[2]);
		Nb						= tmp[3];
	}

	//AM: In practice the old aligner approach wastes 20 bytes and there is no reason to 20 byte align this data.
	//I changed the code to just 4 align for the time being.  
	//On consoles if anything we will need to make this stuff 16 byte align vectors to have any sense, which will have to be done by padding data structures.
	PX_ASSERT(sizeof(Gu::HullPolygonData) % sizeof(PxReal) == 0);	//otherwise please pad it.
	PX_ASSERT(sizeof(PxVec3) % sizeof(PxReal) == 0);

	PxU32 bytesNeeded = computeBufferSize(data, Nb);

	PX_FREE(data.mPolygons);	// Load() can be called for an existing convex mesh. In that case we need to free
	// the memory first.

	bufferSize = (PxU32)Nb;
	void* mDataMemory = PX_ALLOC(bytesNeeded, PX_DEBUG_EXP("ConvexHullData data"));

	PxU8* address = (PxU8*)mDataMemory;

	data.mPolygons				= (Gu::HullPolygonData*)	address;	address += sizeof(Gu::HullPolygonData) * data.mNbPolygons;
	PxVec3* mDataHullVertices	= (PxVec3*)					address;	address += sizeof(PxVec3) * data.mNbHullVertices;
	PxU8* mDataFacesByEdges8	= (PxU8*)					address;	address += sizeof(PxU8) * data.mNbEdges * 2;
	PxU8* mDataFacesByVertices8 = (PxU8*)					address;	address += sizeof(PxU8) * data.mNbHullVertices * 3;
	PxU8* mDataVertexData8		= (PxU8*)					address;	address += sizeof(PxU8) * Nb;	// PT: leave that one last, so that we don't need to serialize "Nb"


	PX_ASSERT(!(size_t(mDataHullVertices) % sizeof(PxReal)));
	PX_ASSERT(!(size_t(data.mPolygons) % sizeof(PxReal)));
	PX_ASSERT(size_t(address)<=size_t(mDataMemory)+bytesNeeded);

	// Import vertices
	readFloatBuffer(&mDataHullVertices->x, PxU32(3*data.mNbHullVertices), Mismatch, stream);

	if(version<=6)
	{
		PxU16 useUnquantizedNormals = readWord(Mismatch, stream);
		PX_UNUSED(useUnquantizedNormals);
	}

	// Import polygons
	stream.read(data.mPolygons, data.mNbPolygons*sizeof(Gu::HullPolygonData));

	if(Mismatch)
	{
		for(PxU32 i=0;i<data.mNbPolygons;i++)
			flipData(data.mPolygons[i]);
	}

	stream.read(mDataVertexData8, Nb);
	stream.read(mDataFacesByEdges8, PxU32(data.mNbEdges*2));
	if(version <= 5)
	{
		for(PxU32 a = 0; a < data.mNbHullVertices; ++a)
		{
			mDataFacesByVertices8[a*3] = 0xFF;
			mDataFacesByVertices8[a*3+1] = 0xFF;
			mDataFacesByVertices8[a*3+2] = 0xFF;
		}
	}
	else
		stream.read(mDataFacesByVertices8, PxU32(data.mNbHullVertices * 3));

	return true;
}

bool Gu::ConvexMesh::load(PxInputStream& stream)
{
	// Import header
	PxU32 version;
	bool mismatch;
	if(!readHeader('C', 'V', 'X', 'M', version, mismatch, stream))
		return false;

	// Check if old (incompatible) mesh format is loaded
	if (version < PX_CONVEX_VERSION)
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Loading convex mesh failed: "
			"Deprecated mesh cooking format. Please install and run the mesh converter tool to convert your mesh to the new cooking format.");
		return false;
	}

	// Import serialization flags
	PxU32 serialFlags	= readDword(mismatch, stream);
	PX_UNUSED(serialFlags);

	if(!convexHullLoad(mHullData, stream, mNb))
		return false;

	// Import local bounds
	float tmp[8];
	readFloatBuffer(tmp, 8, mismatch, stream);
//	geomEpsilon				= tmp[0];
	mHullData.mAABB = PxBounds3(PxVec3(tmp[1], tmp[2], tmp[3]), PxVec3(tmp[4],tmp[5],tmp[6]));

	// Import mass info
	mMass = tmp[7];
	if(mMass!=-1.0f)
	{
		readFloatBuffer(&mInertia(0,0), 9, mismatch, stream);
		readFloatBuffer(&mHullData.mCenterOfMass.x, 3, mismatch, stream);
	}

	// Import gaussmaps
	PxF32 gaussMapFlag = readFloat(mismatch, stream);
	if(gaussMapFlag != -1.0f)
	{
		PX_ASSERT(gaussMapFlag == 1.0f);	//otherwise file is corrupt

		PX_DELETE_AND_RESET(mBigConvexData);
		PX_NEW_SERIALIZED(mBigConvexData,BigConvexData);	
      
		if(mBigConvexData)	
		{
			mBigConvexData->Load(stream);
			mHullData.mBigConvexRawData = &mBigConvexData->mData;
		}
	}
	
/*
	printf("\n\n");
	printf("COM: %f %f %f\n", massInfo.centerOfMass.x, massInfo.centerOfMass.y, massInfo.centerOfMass.z);
	printf("BND: %f %f %f\n", mHullData.aabb.getCenter().x, mHullData.aabb.getCenter().y, mHullData.aabb.getCenter().z);
	printf("CNT: %f %f %f\n", mHullData.mCenterxx.x, mHullData.mCenterxx.y, mHullData.mCenterxx.z);
	printf("COM-BND: %f BND-CNT: %f, CNT-COM: %f\n", (massInfo.centerOfMass - mHullData.aabb.getCenter()).magnitude(), (mHullData.aabb.getCenter() - mHullData.mCenterxx).magnitude(), (mHullData.mCenterxx - massInfo.centerOfMass).magnitude());
*/

// TEST_INTERNAL_OBJECTS
	mHullData.mInternal.mRadius		= readFloat(mismatch, stream);
	mHullData.mInternal.mExtents[0]	= readFloat(mismatch, stream);
	mHullData.mInternal.mExtents[1]	= readFloat(mismatch, stream);
	mHullData.mInternal.mExtents[2]	= readFloat(mismatch, stream);
//~TEST_INTERNAL_OBJECTS
	return true;
}



void Gu::ConvexMesh::release()
{
	mMeshFactory->notifyFactoryListener(this, PxConcreteType::eCONVEX_MESH, false);

	mBaseFlags &= ~PxBaseFlag::eIS_RELEASABLE;
	decRefCount();
}

void Gu::ConvexMesh::onRefCountZero()
{
	if ((!getBufferSize()) || mMeshFactory->removeConvexMesh(*this))  // when the mesh failed to load properly, it will not have been added to the convex array
	{
		GuMeshFactory* mf = mMeshFactory;
		Cm::deletePxBase(this);
		mf->notifyFactoryListener(this, PxConcreteType::eCONVEX_MESH, true);
		return;
	}

	// PT: if we reach this point, we didn't find the mesh in the Physics object => don't delete!
	// This prevents deleting the object twice.
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Gu::ConvexMesh::release: double deletion detected!");
}

PxU32 Gu::ConvexMesh::getReferenceCount() const
{
	return getRefCount();
}


void Gu::ConvexMesh::getMassInformation(PxReal& mass, PxMat33& localInertia, PxVec3& localCenterOfMass) const
{
	mass = Gu::ConvexMesh::getMass();
	localInertia = Gu::ConvexMesh::getInertia();
	localCenterOfMass = Gu::ConvexMesh::getHull().mCenterOfMass;
}

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "CmMatrix34.h"
#include "GuDebug.h"
void Gu::ConvexMesh::debugVisualize(
	Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
	const PxU64 mask, const PxReal /*fscale*/, const PxU32 /*numMaterials*/)	const
{
	// Skip all polygons if at least one vertice is not within culling box.
	if (mask & ((PxU64)1 << PxVisualizationParameter::eCULL_BOX))
	{
		const PxVec3* vertices = mHullData.getHullVertices();
		const PxU8* indexBuffer = mHullData.getVertexData8();
		const PxU32 nbPolygons = getNbPolygonsFast();

		for (PxU32 i = 0; i < nbPolygons; i++)
		{
			PxU32 pnbVertices = mHullData.mPolygons[i].mNbVerts;

			for (PxU32 j = 1; j < pnbVertices; j++)
			{
				const PxVec3& vertex = vertices[indexBuffer[j]];
				if (!cullbox.contains(absPose.base3 + vertex))
					return;
			}

			indexBuffer += pnbVertices;
		}
	}

	if (mask & ((PxU64)1 << PxVisualizationParameter::eCOLLISION_SHAPES))
	{
		const PxU32 scolor = PxU32(PxDebugColor::eARGB_MAGENTA);

		const PxVec3* vertices = mHullData.getHullVertices();
		const PxU8* indexBuffer = mHullData.getVertexData8();
		const PxU32 nbPolygons = getNbPolygonsFast();

		const PxMat44 m44 = Gu::Debug::convertToPxMat44(absPose);

		out << m44 << scolor;	// PT: no need to output this for each segment!

		for (PxU32 i = 0; i < nbPolygons; i++)
		{
			const PxU32 pnbVertices = mHullData.mPolygons[i].mNbVerts;

			PxVec3 begin = m44.transform(vertices[indexBuffer[0]]);	// PT: transform it only once before the loop starts
			for (PxU32 j = 1; j < pnbVertices; j++)
			{
				PxVec3 end = m44.transform(vertices[indexBuffer[j]]);
				out.outputSegment(begin, end);
				begin = end;
			}
			out.outputSegment(begin, m44.transform(vertices[indexBuffer[0]]));

			indexBuffer += pnbVertices;
		}
	}
}

#endif
