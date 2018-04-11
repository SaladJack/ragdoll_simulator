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


#ifndef PX_PHYSICS_NP_MATERIAL
#define PX_PHYSICS_NP_MATERIAL

#include "PxMaterial.h"
#include "ScMaterialCore.h"
#include "PsUserAllocated.h"
#include "CmRefCountable.h"


// PX_SERIALIZATION
#include "PxMetaData.h"
#include "PxSerialFramework.h"
//~PX_SERIALIZATION

namespace physx
{

// Compared to other objects, materials are special since they belong to the SDK and not to scenes
// (similar to meshes). That's why the NpMaterial does have direct access to the core material instead
// of having a buffered interface for it. Scenes will have copies of the SDK material table and there
// the materials will be buffered.


class NpMaterial : public PxMaterial,  public Ps::UserAllocated, public Cm::RefCountable
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION            
									NpMaterial(PxBaseFlags baseFlags) : PxMaterial(baseFlags), Cm::RefCountable(PxEmpty), mMaterial(PxEmpty) {}								
	virtual		void				onRefCountZero();
	virtual		void				resolveReferences(PxDeserializationContext& context);
	static		NpMaterial*			createObject(PxU8*& address, PxDeserializationContext& context);
	static		void				getBinaryMetaData(PxOutputStream& stream);
				void				exportExtraData(PxSerializationContext&)	{}
				void				importExtraData(PxDeserializationContext&) {}
	virtual		void				requires(PxProcessPxBaseCallback&){}
//~PX_SERIALIZATION
									NpMaterial(const Sc::MaterialCore& desc);
									~NpMaterial();

	virtual		void				release();
	virtual		PxU32				getReferenceCount() const;

	virtual		void				setDynamicFriction(PxReal);
	virtual		PxReal				getDynamicFriction() const;
	virtual		void				setStaticFriction(PxReal);
	virtual		PxReal				getStaticFriction() const;
	virtual		void				setRestitution(PxReal);
	virtual		PxReal				getRestitution() const;  
	virtual		void				setFlag(PxMaterialFlag::Enum flag, bool value);
	virtual		void				setFlags(PxMaterialFlags inFlags);
	virtual		PxMaterialFlags		getFlags() const;
	virtual		void				setFrictionCombineMode(PxCombineMode::Enum);
	virtual		PxCombineMode::Enum	getFrictionCombineMode() const;
	virtual		void				setRestitutionCombineMode(PxCombineMode::Enum);
	virtual		PxCombineMode::Enum	getRestitutionCombineMode() const;

	PX_INLINE	Sc::MaterialCore&	getScMaterial()				{ return mMaterial;			}
	//PX_INLINE	PxU32				getTableIndex()		const	{ return mTableHook.index;	}
	PX_INLINE	PxU32				getHandle()			const	{ return mMaterial.getMaterialIndex();}
	PX_INLINE	void				setHandle(PxU32 handle)		{ return mMaterial.setMaterialIndex(handle);}

	PX_FORCE_INLINE static void		getMaterialIndices(PxMaterial*const* materials, PxU16* materialIndices, PxU32 materialCount);

private:
	PX_INLINE	void				updateMaterial();

// PX_SERIALIZATION
public:
//~PX_SERIALIZATION
		Sc::MaterialCore			mMaterial;
		//Cm::InvasiveSetHook			mTableHook;

		//friend struct NpMaterialGetTableHook;
};


PX_FORCE_INLINE void NpMaterial::getMaterialIndices(PxMaterial*const* materials, PxU16* materialIndices, PxU32 materialCount)
{
	for(PxU32 i=0; i < materialCount; i++)
	{
		materialIndices[i] = Ps::to16((static_cast<NpMaterial*>(materials[i]))->getHandle());
	}
}


}

#endif
