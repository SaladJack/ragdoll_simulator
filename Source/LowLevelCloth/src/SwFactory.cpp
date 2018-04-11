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

#include "SwFactory.h"
#include "SwFabric.h"
#include "SwCloth.h"
#include "SwSolver.h"
#include "ClothImpl.h"
#include <string.h> // for memcpy

using namespace physx;

namespace physx
{
	namespace cloth
	{
		// defined in Factory.cpp
		uint32_t getNextFabricId();	
	}
}

cloth::SwFactory::SwFactory() 
: Factory(CPU)
{
}

cloth::SwFactory::~SwFactory()
{
}

cloth::Fabric* cloth::SwFactory::createFabric( uint32_t numParticles, 
	Range<const uint32_t> phases, Range<const uint32_t> sets, 
	Range<const float> restvalues, Range<const uint32_t> indices,
	Range<const uint32_t> anchors, Range<const float> tetherLengths)
{
	return new SwFabric( *this, numParticles, phases, sets, 
		restvalues, indices, anchors, tetherLengths, getNextFabricId());
}

cloth::Cloth* cloth::SwFactory::createCloth( Range<const PxVec4> particles, Fabric& fabric )
{
	return new SwClothImpl( *this, fabric, particles );
}

cloth::Solver* cloth::SwFactory::createSolver(physx::PxProfileZone* profiler, physx::PxTaskManager* taskMgr)
{
#ifdef PX_PHYSX_GPU_EXPORTS 
	// SwSolver not defined in PhysXGpu project
	PX_UNUSED(profiler);
	PX_UNUSED(taskMgr);
	return 0;
#else
	return new SwSolver(profiler, taskMgr);
#endif
}

cloth::Cloth* cloth::SwFactory::clone( const Cloth& cloth )
{
	if (cloth.getFactory().getPlatform() != Factory::CPU)
		return cloth.clone(*this); // forward to CuCloth

	// copy construct
	return new SwClothImpl(*this, static_cast<const SwClothImpl&>(cloth));
}

void cloth::SwFactory::copyToHost( const void* srcIt, const void* srcEnd, void* dstIt ) const
{
	memcpy(dstIt, srcIt, size_t(intptr_t(srcEnd) - intptr_t(srcIt)));
}

void cloth::SwFactory::copyFromHost( const void* srcIt, const void* srcEnd, void* dstIt ) const
{
	copyToHost(srcIt, srcEnd, dstIt);
}

void cloth::SwFactory::copyToHost( const void* srcIt, const void* srcEnd, void* dstIt, CUstream ) const
{
	copyToHost(srcIt, srcEnd, dstIt);
}

void cloth::SwFactory::copyFromHost( const void* srcIt, const void* srcEnd, void* dstIt, CUstream ) const
{
	copyToHost(srcIt, srcEnd, dstIt);
}

void cloth::SwFactory::extractFabricData(const Fabric& fabric, 
	Range<uint32_t> phases, Range<uint32_t> sets, 
	Range<float> restvalues, Range<uint32_t> indices, 
	Range<uint32_t> anchors, Range<float> tetherLengths) const
{
	const SwFabric& swFabric = static_cast<const SwFabric&>(fabric);

	PX_ASSERT(phases.empty() || phases.size() == swFabric.getNumPhases());
	PX_ASSERT(restvalues.empty() || restvalues.size() == swFabric.getNumRestvalues());
	PX_ASSERT(sets.empty() || sets.size() == swFabric.getNumSets());
	PX_ASSERT(indices.empty() || indices.size() == swFabric.getNumIndices());
	PX_ASSERT(anchors.empty() || anchors.size() == swFabric.getNumTethers());
	PX_ASSERT(tetherLengths.empty() || tetherLengths.size() == swFabric.getNumTethers());

	for(uint32_t i=0; !phases.empty(); ++i, phases.popFront())
		phases.front() = swFabric.mPhases[i];

	const uint32_t* sEnd = swFabric.mSets.end(), *sIt;
	const float* rBegin = swFabric.mRestvalues.begin(), *rIt = rBegin;
	const uint16_t* iIt = swFabric.mIndices.begin();

	uint32_t* sDst = sets.begin();
	float* rDst = restvalues.begin();
	uint32_t* iDst = indices.begin();

	uint32_t numConstraints = 0;
	for(sIt = swFabric.mSets.begin(); ++sIt != sEnd; )
	{
		const float* rEnd = rBegin + *sIt;
		for(; rIt != rEnd; ++rIt)
		{
			uint16_t i0 = *iIt++;
			uint16_t i1 = *iIt++;

			if(PxMax(i0, i1) >= swFabric.mNumParticles)
				continue;

			if(!restvalues.empty())
				*rDst++ = *rIt;

			if(!indices.empty())
			{
				*iDst++ = i0;
				*iDst++ = i1;
			}

			++numConstraints;
		}

		if(!sets.empty())
			*sDst++ = numConstraints;
	}
	
	for(uint32_t i=0; !anchors.empty(); ++i, anchors.popFront())
		anchors.front() = swFabric.mTethers[i].mAnchor;

	for(uint32_t i=0; !tetherLengths.empty(); ++i, tetherLengths.popFront())
		tetherLengths.front() = swFabric.mTethers[i].mLength * swFabric.mTetherLengthScale;
}

void cloth::SwFactory::extractCollisionData(const Cloth& cloth, Range<PxVec4> spheres, 
	Range<uint32_t> capsules, Range<PxVec4> planes, 
	Range<uint32_t> convexes, Range<PxVec3> triangles) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;

	PX_ASSERT(spheres.empty() || spheres.size() == swCloth.mStartCollisionSpheres.size());
	PX_ASSERT(capsules.empty() || capsules.size() == swCloth.mCapsuleIndices.size()*2);
	PX_ASSERT(planes.empty() || planes.size() == swCloth.mStartCollisionPlanes.size());
	PX_ASSERT(convexes.empty() || convexes.size() == swCloth.mConvexMasks.size());
	PX_ASSERT(triangles.empty() || triangles.size() == swCloth.mStartCollisionTriangles.size());

	if (!swCloth.mStartCollisionSpheres.empty() && !spheres.empty())
		memcpy(spheres.begin(), &swCloth.mStartCollisionSpheres.front(), 
			swCloth.mStartCollisionSpheres.size()*sizeof(PxVec4));

	if (!swCloth.mCapsuleIndices.empty() && !capsules.empty())
		memcpy(capsules.begin(), &swCloth.mCapsuleIndices.front(),
			swCloth.mCapsuleIndices.size()*sizeof(IndexPair));

	if (!swCloth.mStartCollisionPlanes.empty() && !planes.empty())
		memcpy(planes.begin(), &swCloth.mStartCollisionPlanes.front(), 
			swCloth.mStartCollisionPlanes.size()*sizeof(PxVec4));

	if (!swCloth.mConvexMasks.empty() && !convexes.empty())
		memcpy(convexes.begin(), &swCloth.mConvexMasks.front(),
			swCloth.mConvexMasks.size()*sizeof(uint32_t));

	if (!swCloth.mStartCollisionTriangles.empty() && !triangles.empty())
		memcpy(triangles.begin(), &swCloth.mStartCollisionTriangles.front(), 
			swCloth.mStartCollisionTriangles.size()*sizeof(PxVec3));
}

void cloth::SwFactory::extractMotionConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;

	Vec4fAlignedVector const& srcConstraints = !swCloth.mMotionConstraints.mTarget.empty() 
		? swCloth.mMotionConstraints.mTarget : swCloth.mMotionConstraints.mStart;

	if (!srcConstraints.empty())
	{
		// make sure dest array is big enough
		PX_ASSERT(destConstraints.size() == srcConstraints.size());

		memcpy(destConstraints.begin(), &srcConstraints.front(), 
			srcConstraints.size()*sizeof(PxVec4));
	}
}

void cloth::SwFactory::extractSeparationConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;

	Vec4fAlignedVector const& srcConstraints = !swCloth.mSeparationConstraints.mTarget.empty() 
		? swCloth.mSeparationConstraints.mTarget : swCloth.mSeparationConstraints.mStart;

	if (!srcConstraints.empty())
	{
		// make sure dest array is big enough
		PX_ASSERT(destConstraints.size() == srcConstraints.size());

		memcpy(destConstraints.begin(), &srcConstraints.front(), 
			srcConstraints.size()*sizeof(PxVec4));
	}
}

void cloth::SwFactory::extractParticleAccelerations(const Cloth& cloth, Range<PxVec4> destAccelerations) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;

	if (!swCloth.mParticleAccelerations.empty())
	{
		// make sure dest array is big enough
		PX_ASSERT(destAccelerations.size() == swCloth.mParticleAccelerations.size());

		memcpy(destAccelerations.begin(), &swCloth.mParticleAccelerations.front(), 
			swCloth.mParticleAccelerations.size() * sizeof(PxVec4));
	}
}

void cloth::SwFactory::extractVirtualParticles(const Cloth& cloth, 
	Range<uint32_t[4]> indices, Range<PxVec3> weights) const
{
	PX_ASSERT(this == &cloth.getFactory());

	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;

	uint32_t numIndices = cloth.getNumVirtualParticles();
	uint32_t numWeights = cloth.getNumVirtualParticleWeights();

	PX_ASSERT(indices.size() == numIndices || indices.empty());	
	PX_ASSERT(weights.size() == numWeights || weights.empty());

	if ( weights.size() == numWeights )
	{
		PxVec3* wDestIt = reinterpret_cast<PxVec3*>(weights.begin());

		// convert weights from vec4 to vec3
		cloth::Vec4fAlignedVector::ConstIterator wIt = swCloth.mVirtualParticleWeights.begin();
		cloth::Vec4fAlignedVector::ConstIterator wEnd= wIt + numWeights;

		for (; wIt != wEnd; ++wIt, ++wDestIt)
			*wDestIt = PxVec3(wIt->x, wIt->y, wIt->z);

		PX_ASSERT(wDestIt == weights.end());
	}
	if ( indices.size() == numIndices )
	{
		// convert indices
		Vec4u* iDestIt = reinterpret_cast<Vec4u*>(indices.begin());	
		Vector<Vec4us>::Type::ConstIterator iIt = swCloth.mVirtualParticleIndices.begin();
		Vector<Vec4us>::Type::ConstIterator iEnd = swCloth.mVirtualParticleIndices.end();

		uint32_t numParticles = uint32_t(swCloth.mCurParticles.size());

		for (; iIt != iEnd; ++iIt)
		{
			// skip dummy indices
			if (iIt->x < numParticles)
				// byte offset to element index
				*iDestIt++ = Vec4u(*iIt);
		}

		PX_ASSERT(&array(*iDestIt) == indices.end());
	}
}

void cloth::SwFactory::extractSelfCollisionIndices( 
	const Cloth& cloth, Range<uint32_t> destIndices ) const
{
	const SwCloth& swCloth = static_cast<const SwClothImpl&>(cloth).mCloth;
	PX_ASSERT(destIndices.size() == swCloth.mSelfCollisionIndices.size());
	copyToHost(swCloth.mSelfCollisionIndices.begin(), 
		swCloth.mSelfCollisionIndices.end(), destIndices.begin());
}

void cloth::SwFactory::extractRestPositions( 
	const Cloth& cloth, Range<PxVec4> destRestPositions ) const
{
	const SwCloth& cuCloth = static_cast<const SwClothImpl&>(cloth).mCloth;
	PX_ASSERT(destRestPositions.size() == cuCloth.mRestPositions.size());
	copyToHost(cuCloth.mRestPositions.begin(), 
		cuCloth.mRestPositions.end(), destRestPositions.begin());
}
