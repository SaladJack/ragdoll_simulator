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

#ifndef GU_PCM_CONTACT_CONVEX_COMMON_H
#define GU_PCM_CONTACT_CONVEX_COMMON_H

#define	PCM_MAX_CONTACTPATCH_SIZE	32

#include "GuContactBuffer.h"
#include "GuVecCapsule.h"
#include "GuPCMTriangleContactGen.h"
#include "GuContainer.h"

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif

#define PCM_LOCAL_CONTACTS_SIZE		1088

namespace physx
{

namespace Gu
{

class PCMMeshContactGeneration
{
	PX_NOCOPY(PCMMeshContactGeneration)
public:
	PCMContactPatch							mContactPatch[PCM_MAX_CONTACTPATCH_SIZE];
	PCMContactPatch*						mContactPatchPtr[PCM_MAX_CONTACTPATCH_SIZE];
	const Ps::aos::FloatV					mContactDist;
	const Ps::aos::FloatV					mReplaceBreakingThreshold;
	const Ps::aos::PsTransformV&			mConvexTransform;
	const Ps::aos::PsTransformV&			mMeshTransform;
	Gu::MultiplePersistentContactManifold&	mMultiManifold;
	Gu::ContactBuffer&						mContactBuffer;

	Ps::aos::FloatV							mAcceptanceEpsilon;
	Ps::aos::FloatV							mSqReplaceBreakingThreshold;
	Ps::aos::PsMatTransformV				mMeshToConvex;
	Gu::MeshPersistentContact*				mManifoldContacts;
	PxU32									mNumContacts;
	PxU32									mNumContactPatch;
	PxU32									mNumCalls;

	PCMMeshContactGeneration(
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const Ps::aos::PsTransformV& convexTransform,
		const Ps::aos::PsTransformV& meshTransform,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer& contactBuffer

	) :
		mContactDist(contactDist),
		mReplaceBreakingThreshold(replaceBreakingThreshold),
		mConvexTransform(convexTransform),
		mMeshTransform(meshTransform),
		mMultiManifold(multiManifold),
		mContactBuffer(contactBuffer)
		
	{
		using namespace Ps::aos;
		mNumContactPatch = 0;
		mNumContacts = 0;
		mNumCalls = 0;

		mMeshToConvex = mConvexTransform.transformInv(mMeshTransform);

		//Assign the PCMContactPatch to the PCMContactPathPtr
		for(PxU32 i=0; i<PCM_MAX_CONTACTPATCH_SIZE; ++i)
		{
			mContactPatchPtr[i] = &mContactPatch[i];
		}
		mManifoldContacts = PX_CP_TO_MPCP(contactBuffer.contacts);

		mSqReplaceBreakingThreshold = FMul(replaceBreakingThreshold, replaceBreakingThreshold);

		mAcceptanceEpsilon = FLoad(0.996);//5 degree
		//mAcceptanceEpsilon = FloatV_From_F32(0.9999);//5 degree
	}

	template <PxU32 TriangleCount, typename Derived>
	bool processTriangleCache(Gu::TriangleCache<TriangleCount>& cache)
	{
		PxU32 count = cache.mNumTriangles;
		PxVec3* verts = cache.mVertices;
		PxU32* vertInds = cache.mIndices;
		PxU32* triInds = cache.mTriangleIndex;
		PxU8* edgeFlags = cache.mEdgeFlags;
		while(count--)
		{
			((Derived*)(this))->processTriangle(verts, *triInds, *edgeFlags, vertInds);
			verts += 3;
			vertInds += 3;
			triInds++;
			edgeFlags++;
		}
		return true;
	}
	void prioritizeContactPatches();
	void addManifoldPointToPatch(const Ps::aos::Vec3VArg currentPatchNormal, const Ps::aos::FloatVArg maxPen, const PxU32 previousNumContacts);
	void processContacts(const PxU8 maxContactPerManifold, const bool isNotLastPatch = true);
};

/*
	This function is based on the current patch normal to either create a new patch or merge the manifold contacts in this patch with the manifold contacts in the last existing patch
*/
PX_FORCE_INLINE void PCMMeshContactGeneration::addManifoldPointToPatch(const Ps::aos::Vec3VArg currentPatchNormal, const Ps::aos::FloatVArg maxPen, const PxU32 previousNumContacts)
{
	using namespace Ps::aos;

	bool foundPatch = false;
	//we have existing patch
	if(mNumContactPatch > 0)
	{
		//if the direction between the last existing patch normal and the current patch normal are within acceptance epsilon, which means we will be
		//able to merge the last patch's contacts with the current patch's contacts. This is just to avoid to create an extra patch. We have some logic
		//later to refine the patch again
		if(FAllGrtr(V3Dot(mContactPatch[mNumContactPatch-1].mPatchNormal, currentPatchNormal), mAcceptanceEpsilon))
		{
			//get the last patch
			PCMContactPatch& patch = mContactPatch[mNumContactPatch-1];

			//remove duplicate contacts
			for(PxU32 i = patch.mStartIndex; i<patch.mEndIndex; ++i)
			{
				for(PxU32 j = previousNumContacts; j<mNumContacts; ++j)
				{
					Vec3V dif = V3Sub(mManifoldContacts[j].mLocalPointB, mManifoldContacts[i].mLocalPointB);
					FloatV d = V3Dot(dif, dif);
					if(FAllGrtr(mSqReplaceBreakingThreshold, d))
					{
						if(FAllGrtr(V4GetW(mManifoldContacts[i].mLocalNormalPen), V4GetW(mManifoldContacts[j].mLocalNormalPen)))
						{
							//The new contact is deeper than the old contact so we keep the deeper contact
							mManifoldContacts[i] = mManifoldContacts[j];
						}
						mManifoldContacts[j] = mManifoldContacts[mNumContacts-1];
						mNumContacts--;
						j--;
					}
				}
			}
			patch.mEndIndex = mNumContacts;
			patch.mPatchMaxPen = FMin(patch.mPatchMaxPen, maxPen);
			foundPatch = true;
		}
	}

	//If there are no existing patch which match the currentPatchNormal, we will create a new patch
	if(!foundPatch)
	{
		mContactPatch[mNumContactPatch].mStartIndex = previousNumContacts;
		mContactPatch[mNumContactPatch].mEndIndex = mNumContacts;
		mContactPatch[mNumContactPatch].mPatchMaxPen = maxPen;
		mContactPatch[mNumContactPatch++].mPatchNormal = currentPatchNormal;
	}
}

/*
	This function sort the contact patch based on the max penetration so that deepest penetration contact patch will be in front of the less penetration contact
	patch
*/
PX_FORCE_INLINE  void PCMMeshContactGeneration::prioritizeContactPatches()
{
	//we are using insertion sort to prioritize contact patchs
	using namespace Ps::aos;
	//sort the contact patch based on the max penetration
	for(PxU32 i=1; i<mNumContactPatch; ++i)
	{
		const PxU32 indexi = i-1;
		if(FAllGrtr(mContactPatchPtr[indexi]->mPatchMaxPen, mContactPatchPtr[i]->mPatchMaxPen))
		{
			//swap
			PCMContactPatch* tmp = mContactPatchPtr[indexi];
			mContactPatchPtr[indexi] = mContactPatchPtr[i];
			mContactPatchPtr[i] = tmp;

			for(PxI32 j=PxI32(i-2); j>=0; j--)
			{
				const PxU32 indexj = PxU32(j+1);
				if(FAllGrtrOrEq(mContactPatchPtr[indexj]->mPatchMaxPen, mContactPatchPtr[j]->mPatchMaxPen))
					break;
				//swap
				PCMContactPatch* temp = mContactPatchPtr[indexj];
				mContactPatchPtr[indexj] = mContactPatchPtr[j];
				mContactPatchPtr[j] = temp;
			}
		}
	}
}


PX_FORCE_INLINE void PCMMeshContactGeneration::processContacts(const PxU8 maxContactPerManifold, bool isNotLastPatch)
{
	using namespace Ps::aos;
	
	if(mNumContacts != 0)
	{
		//reorder the contact patches based on the max penetration
		prioritizeContactPatches();
		//connect the patches which's angle between patch normals are within 5 degree
		mMultiManifold.refineContactPatchConnective(mContactPatchPtr, mNumContactPatch, mManifoldContacts, mAcceptanceEpsilon);
		//get rid of duplicate manifold contacts in connected contact patches
		mMultiManifold.reduceManifoldContactsInDifferentPatches(mContactPatchPtr, mNumContactPatch, mManifoldContacts, mNumContacts, mSqReplaceBreakingThreshold);
		//add the manifold contact to the corresponding manifold
		mMultiManifold.addManifoldContactPoints(mManifoldContacts, mNumContacts, mContactPatchPtr, mNumContactPatch, mSqReplaceBreakingThreshold, mAcceptanceEpsilon, maxContactPerManifold);
		
		mNumContacts = 0;
		mNumContactPatch = 0;

		if(isNotLastPatch)
		{
			//remap the contact patch pointer to contact patch
			for(PxU32 i=0; i<PCM_MAX_CONTACTPATCH_SIZE; ++i)
			{
				mContactPatchPtr[i] = &mContactPatch[i];
			}
		}
	}
}

struct PCMDeferredPolyData
{
public:
	PxVec3	mVerts[3];
	PxU32	mInds[3];
	PxU32	mTriangleIndex;
	PxU32	mFeatureIndex;
	PxU8	triFlags;
};

#define MAX_CACHE_SIZE	128

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

class PCMConvexVsMeshContactGeneration : public PCMMeshContactGeneration
{
	PCMConvexVsMeshContactGeneration &operator=(PCMConvexVsMeshContactGeneration  &);

public:

	Gu::CacheMap<Gu::CachedEdge, MAX_CACHE_SIZE>	mEdgeCache;
	Ps::aos::Vec3V									mHullCenterMesh;

	Gu::Container&									mDeferredContacts;
	const Gu::PolygonalData&						mPolyData;
	SupportLocal*									mPolyMap;
	const Cm::FastVertex2ShapeScaling&				mConvexScaling;  
	bool											mIdtConvexScale;
	
				
	PCMConvexVsMeshContactGeneration(
		const Ps::aos::FloatVArg				contactDistance,
		const Ps::aos::FloatVArg				replaceBreakingThreshold,
		const Ps::aos::PsTransformV&			convexTransform, 
		const Ps::aos::PsTransformV&			meshTransform,
		Gu::MultiplePersistentContactManifold&	multiManifold,
		Gu::ContactBuffer&						contactBuffer,

		const Gu::PolygonalData&				polyData,
		SupportLocal*							polyMap,
		Gu::Container&							delayedContacts,
		const Cm::FastVertex2ShapeScaling&		convexScaling,
		bool									idtConvexScale
		
	) : PCMMeshContactGeneration(contactDistance, replaceBreakingThreshold, convexTransform, meshTransform, multiManifold, contactBuffer),
		mDeferredContacts(delayedContacts),
		mPolyData(polyData),
		mPolyMap(polyMap),
		mConvexScaling(convexScaling),
		mIdtConvexScale(idtConvexScale)
	{
		using namespace Ps::aos;
	
		// Hull center in local space
		const Vec3V hullCenterLocal = V3LoadU(mPolyData.mCenter);
		// Hull center in mesh space
		mHullCenterMesh = mMeshToConvex.transformInv(hullCenterLocal);

	}

	bool generateTriangleFullContactManifold(Gu::TriangleV& localTriangle, const PxU32 triangleIndex, const PxU32* triIndices, const PxU8 triFlags, const Gu::PolygonalData& polyData,  Gu::SupportLocalImpl<Gu::TriangleV>* localTriMap, Gu::SupportLocal* polyMap, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal);

	bool generatePolyDataContactManifold(Gu::TriangleV& localTriangle, const PxU32 featureIndex, const PxU32 triangleIndex, const PxU8 triFlags, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal);
	void generateLastContacts();
	void addContactsToPatch(const Ps::aos::Vec3VArg patchNormal, const PxU32 previousNumContacts, const PxU32 _numContacts);

	bool processTriangle(const PxVec3* verts, PxU32 triangleIndex, PxU8 triFlags, const PxU32* vertInds); 

	static bool generateTriangleFullContactManifold(Gu::TriangleV& localTriangle, const PxU32 triangleIndex, const PxU8 triFlags, const Gu::PolygonalData& polyData,  Gu::SupportLocalImpl<Gu::TriangleV>* localTriMap, Gu::SupportLocal* polyMap, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal);

	static bool processTriangle(const Gu::PolygonalData& polyData, SupportLocal* polyMap, const PxVec3* verts, const PxU32 triangleIndex, PxU8 triFlags, const Ps::aos::FloatVArg inflation, const bool isDoubleSided, 
		const Ps::aos::PsTransformV& convexTransform, const Ps::aos::PsMatTransformV& meshToConvex, Gu::MeshPersistentContact* manifoldContact, PxU32& numContacts); 
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

class PCMSphereVsMeshContactGeneration : public PCMMeshContactGeneration
{
public:
	Ps::aos::Vec3V	mSphereCenter;
	Ps::aos::FloatV mSphereRadius;
	Ps::aos::FloatV	mSqInflatedSphereRadius;

				
	PCMSphereVsMeshContactGeneration(
		const Ps::aos::Vec3VArg		sphereCenter,
		const Ps::aos::FloatVArg	sphereRadius,
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const Ps::aos::PsTransformV& sphereTransform,
		const Ps::aos::PsTransformV& meshTransform,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer& contactBuffer

	) : PCMMeshContactGeneration(contactDist, replaceBreakingThreshold, sphereTransform, meshTransform, multiManifold, contactBuffer),
		mSphereCenter(sphereCenter),
		mSphereRadius(sphereRadius)
	{
		using namespace Ps::aos;
		const FloatV inflatedSphereRadius = FAdd(sphereRadius, contactDist);
		mSqInflatedSphereRadius = FMul(inflatedSphereRadius, inflatedSphereRadius);
	}


	bool processTriangle(const PxVec3* verts, PxU32 triangleIndex, PxU8 triFlags, const PxU32* vertInds);
};

class PCMCapsuleVsMeshContactGeneration : public PCMMeshContactGeneration
{
	PCMCapsuleVsMeshContactGeneration &operator=(PCMCapsuleVsMeshContactGeneration &);
public:
	Ps::aos::FloatV mInflatedRadius;
	Ps::aos::FloatV	mSqInflatedRadius;
	const CapsuleV&	mCapsule;

				
	PCMCapsuleVsMeshContactGeneration( 
		const CapsuleV&				capsule,
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const Ps::aos::PsTransformV& sphereTransform,
		const Ps::aos::PsTransformV& meshTransform,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer& contactBuffer

	) : PCMMeshContactGeneration(contactDist, replaceBreakingThreshold, sphereTransform, meshTransform, multiManifold, contactBuffer),
		mCapsule(capsule)
	{
		using namespace Ps::aos;
		mInflatedRadius = FAdd(capsule.radius, contactDist);
		mSqInflatedRadius = FMul(mInflatedRadius, mInflatedRadius);
	}

	void generateEEContacts(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b,const Ps::aos::Vec3VArg c, const Ps::aos::Vec3VArg normal, const PxU32 triangleIndex, 
		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q, const Ps::aos::FloatVArg sqInflatedRadius, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);

	void generateEE(const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q,  const Ps::aos::FloatVArg sqInflatedRadius, const Ps::aos::Vec3VArg normal, const PxU32 triangleIndex,
		const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);
	
	static bool generateContacts(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b,const Ps::aos::Vec3VArg c, const Ps::aos::Vec3VArg planeNormal, const Ps::aos::Vec3VArg normal,  
		const PxU32 triangleIndex, const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q, const Ps::aos::FloatVArg inflatedRadius, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);

	static void generateEEContactsMTD(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b,const Ps::aos::Vec3VArg c, const Ps::aos::Vec3VArg normal, const PxU32 triangleIndex,
		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q, const Ps::aos::FloatVArg inflatedRadius, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);

	static void generateEEMTD(const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q,  const Ps::aos::FloatVArg inflatedRadius, const Ps::aos::Vec3VArg normal, const PxU32 trianlgeIndex, 
		const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);

	bool processTriangle(const PxVec3* verts, const PxU32 triangleIndex, PxU8 triFlags, const PxU32* vertInds);

	static bool processTriangle(const TriangleV& triangle, const PxU32 triangleIndex, const CapsuleV& capsule, const Ps::aos::FloatVArg inflatedRadius, const PxU8 triFlag, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts);
};

}
}

#endif
