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
#include "PsUserAllocated.h"
#include "GuSerialize.h"
#include "GuBigConvexData2.h"
#include "GuCubeIndex.h"
#include "PsIntrinsics.h"
#include "CmUtils.h"

using namespace physx;
using namespace Gu;

BigConvexData::BigConvexData() : mVBuffer(NULL)
{
	mData.mSubdiv			= 0;
	mData.mNbSamples		= 0;
	mData.mSamples			= NULL;

	//////

	mData.mNbVerts			= 0;
	mData.mNbAdjVerts		= 0;
	mData.mValencies		= NULL;
	mData.mAdjacentVerts	= NULL;
}

BigConvexData::~BigConvexData()
{
	PX_FREE(mData.mSamples);

	///////////

	if(mVBuffer)
	{
		PX_FREE(mVBuffer);
	}
	else
	{
		// Allocated from somewhere else!!
		PX_FREE(mData.mValencies);
		PX_FREE(mData.mAdjacentVerts);
	}
}

void BigConvexData::CreateOffsets()
{
	// Create offsets (radix style)
	mData.mValencies[0].mOffset = 0;
	for(PxU32 i=1;i<mData.mNbVerts;i++)
		mData.mValencies[i].mOffset = PxU16(mData.mValencies[i-1].mOffset + mData.mValencies[i-1].mCount);
}

bool BigConvexData::VLoad(PxInputStream& stream)
{
	// Import header
	PxU32 Version;
	bool Mismatch;
	if(!ReadHeader('V', 'A', 'L', 'E', Version, Mismatch, stream))
		return false;

	mData.mNbVerts		= readDword(Mismatch, stream);
	mData.mNbAdjVerts	= readDword(Mismatch, stream);

	PX_FREE(mVBuffer);

	// PT: align Gu::Valency?
	const PxU32 numVerts = (mData.mNbVerts+3)&~3;
	const PxU32 TotalSize = sizeof(Gu::Valency)*numVerts + sizeof(PxU8)*mData.mNbAdjVerts;
	mVBuffer = PX_ALLOC(TotalSize, PX_DEBUG_EXP("BigConvexData data"));
	mData.mValencies		= (Gu::Valency*)mVBuffer;
	mData.mAdjacentVerts	= ((PxU8*)mVBuffer) + sizeof(Gu::Valency)*numVerts;

	PX_ASSERT(0 == (size_t(mData.mAdjacentVerts) & 0xf));
	PX_ASSERT(Version==2);

	{
		PxU16* temp = (PxU16*)mData.mValencies;

		PxU32 MaxIndex = readDword(Mismatch, stream);
		ReadIndices(Ps::to16(MaxIndex), mData.mNbVerts, temp, stream, Mismatch);

		// We transform from:
		//
		// |5555|4444|3333|2222|1111|----|----|----|----|----|
		//
		// to:
		//
		// |5555|4444|4444|2222|3333|----|2222|----|1111|----|
		//
		for(PxU32 i=0;i<mData.mNbVerts;i++)
			mData.mValencies[mData.mNbVerts-i-1].mCount = temp[mData.mNbVerts-i-1];
	}
	stream.read(mData.mAdjacentVerts, mData.mNbAdjVerts);

	// Recreate offsets
	CreateOffsets();

	return true;
}

PxU32 BigConvexData::ComputeOffset(const PxVec3& dir) const
{
	return ComputeCubemapOffset(dir, mData.mSubdiv);
}

PxU32 BigConvexData::ComputeNearestOffset(const PxVec3& dir) const
{
	return ComputeCubemapNearestOffset(dir, mData.mSubdiv);
}

bool BigConvexData::Load(PxInputStream& stream)
{
	// Import header
	PxU32 Version;
	bool Mismatch;
	if(!ReadHeader('S', 'U', 'P', 'M', Version, Mismatch, stream))
		return false;

	// Load base gaussmap
//	if(!GaussMap::Load(stream))	return false;

		// Import header
		if(!ReadHeader('G', 'A', 'U', 'S', Version, Mismatch, stream))
			return false;

		// Import basic info
		mData.mSubdiv		= Ps::to16(readDword(Mismatch, stream));
		mData.mNbSamples	= Ps::to16(readDword(Mismatch, stream));

	// Load map data
	mData.mSamples = (PxU8*)PX_ALLOC(sizeof(PxU8)*mData.mNbSamples*2, PX_DEBUG_EXP("BigConvex Samples Data "));

	// These byte buffers shouldn't need converting
	stream.read(mData.mSamples, sizeof(PxU8)*mData.mNbSamples*2);

	//load the valencies
	return VLoad(stream);
}

// PX_SERIALIZATION
void BigConvexData::exportExtraData(PxSerializationContext& stream)
{
	if(mData.mSamples)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData.mSamples, sizeof(PxU8)*mData.mNbSamples*2);
	}

	if(mData.mValencies)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		PxU32 numVerts = (mData.mNbVerts+3)&~3;
		const PxU32 TotalSize = sizeof(Gu::Valency)*numVerts + sizeof(PxU8)*mData.mNbAdjVerts;
		stream.writeData(mData.mValencies, TotalSize);
	}
}

void BigConvexData::importExtraData(PxDeserializationContext& context)
{
	if(mData.mSamples)
		mData.mSamples = context.readExtraData<PxU8, PX_SERIAL_ALIGN>(PxU32(mData.mNbSamples*2));

	if(mData.mValencies)
	{
		context.alignExtraData();
		PxU32 numVerts = (mData.mNbVerts+3)&~3;
		mData.mValencies		= context.readExtraData<Gu::Valency>(numVerts);
		mData.mAdjacentVerts	= context.readExtraData<PxU8>(mData.mNbAdjVerts);

	}
}
//~PX_SERIALIZATION

