/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "SnConvX.h"
#include "PxIO.h"
#include "PxSerializer.h"
#include "serialization/SnSerialUtils.h"

using namespace physx;
using namespace physx::Sn;

//#define REMOVE_EXPLICIT_PADDING

static const char gVTablePtr[] = "v-table ptr";
static const char gAutoPadding[] = "auto-generated padding";
static const char gByte[] = "paddingByte";

///////////////////////////////////////////////////////////////////////////////

bool PxMetaDataEntry::isVTablePtr() const
{
	return mType==gVTablePtr;
}

///////////////////////////////////////////////////////////////////////////////

bool MetaClass::getFieldByType(const char* type, PxMetaDataEntry& entry) const
{
	assert(type);
	PxU32 nbFields = mFields.size();
	for(PxU32 i=0;i<nbFields;i++)
	{
		if(strcmp(mFields[i].mType, type)==0)
		{
			entry = mFields[i];
			return true;
		}
	}
	return false;
}

bool MetaClass::getFieldByName(const char* name, PxMetaDataEntry& entry) const
{
	assert(name);
	PxU32 nbFields = mFields.size();
	for(PxU32 i=0;i<nbFields;i++)
	{
		if(strcmp(mFields[i].mName, name)==0)
		{
			entry = mFields[i];
			return true;
		}
	}
	return false;
}

void MetaClass::checkAndCompleteClass(const MetaData& owner, int& startOffset, int& nbBytes)
{
	if(startOffset!=-1)
	{
		owner.mConvX.displayMessage(PxErrorCode::eDEBUG_INFO,
			"\n Adding %d padding bytes at offset %d in class %s.\n", nbBytes, startOffset, mClassName);

		// Leap of faith: add padding bytes there
		PxMetaDataEntry padding;
		padding.mType	= gByte;
		padding.mName	= gAutoPadding;
		padding.mOffset	= startOffset;
		padding.mSize	= nbBytes;
		padding.mCount	= nbBytes;
		padding.mFlags	= PxMetaDataFlag::ePADDING;
		mFields.pushBack(padding);

		startOffset = -1;
	}
}

bool MetaClass::check(const MetaData& owner)
{
	owner.mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, "Checking class: %s\n", mClassName);

	if(mCallback)
		return true;	// Skip atomic types
	if(mMaster)
		return true;	// Skip typedefs

	bool* map = (bool*)PX_ALLOC(sizeof(bool)*mSize, PX_DEBUG_EXP("bool"));
	memset(map, 0, (size_t)mSize);

	const PxU32 nbFields = mFields.size();
	for(PxU32 i=0;i<nbFields;i++)
	{
		const PxMetaDataEntry& field = mFields[i];
		if(field.mFlags & PxMetaDataFlag::eEXTRA_DATA)
			continue;
//		if((field.mFlags & PxMetaDataFlag::eUNION) && !field.mSize)
//			continue;	// Union type
		assert(field.mSize);
		const int byteStart = field.mOffset;
		const int byteEnd = field.mOffset + field.mSize;
		assert(byteStart>=0 && byteStart<mSize);
		assert(byteEnd>=0 && byteEnd<=mSize);

		int startOffset = -1;
		int nbBytes = 0;
		for(int j=byteStart;j<byteEnd;j++)
		{
			if(map[j])
			{
				if(startOffset==-1)
				{
					startOffset = (int)i;
					nbBytes = 0;
				}
				nbBytes++;
//				displayErrorMessage(" %s: found overlapping bytes!\n", mClassName);
			}
			else
			{
				if(startOffset!=-1)
				{
					owner.mConvX.displayMessage(PxErrorCode::eINTERNAL_ERROR, 
						"PxBinaryConverter: %s: %d overlapping bytes at offset %d!\n", mClassName, nbBytes, startOffset);
					startOffset = -1;
					PX_ALWAYS_ASSERT_MESSAGE("Overlapping bytes!");
				}
			}
			map[j] = true;
		}
		if(startOffset!=-1)
		{
			owner.mConvX.displayMessage(PxErrorCode::eINTERNAL_ERROR,
				"PxBinaryConverter: %s: %d overlapping bytes at offset %d!\n", mClassName, nbBytes, startOffset);
			startOffset = -1;
			PX_ALWAYS_ASSERT_MESSAGE("Overlapping bytes!");
		}
	}

	{
		int startOffset = -1;
		int nbBytes = 0;
		for(int i=0;i<mSize;i++)
		{
			if(!map[i])
			{
				if(startOffset==-1)
				{
					startOffset = i;
					nbBytes = 0;
				}
				nbBytes++;
			}
			else
			{
				checkAndCompleteClass(owner, startOffset, nbBytes);
			}
		}
		checkAndCompleteClass(owner, startOffset, nbBytes);
	}
	PX_FREE(map);


	//
	for(PxU32 i=0;i<nbFields;i++)
	{
		const PxMetaDataEntry& current = mFields[i];
		if(current.mFlags & PxMetaDataFlag::ePTR)
			continue;

		MetaClass* fieldMetaClass = owner.mConvX.getMetaClass(current.mType, owner.getType());
		if(!fieldMetaClass)
		{
			owner.mConvX.displayMessage(PxErrorCode::eINTERNAL_ERROR,
				"PxBinaryConverter: Missing meta-data for: %s\n", current.mType);
			return false;
		}
		else
		{
			if(current.mFlags & PxMetaDataFlag::eEXTRA_DATA)
			{       
				owner.mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, "Extra data: %s\n", current.mType);
			}
			else
			{
				assert(fieldMetaClass->mSize*current.mCount==current.mSize);
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

MetaData::MetaData(ConvX& convx) :
	mConvX					(convx),
	mType					(META_DATA_NONE),
	mNbEntries				(0),
	mEntries				(NULL),
	mStringTable			(NULL),
	mVersion				(0),
	mBuildNumber			(0),
	mSizeOfPtr				(0),
	mPlatformTag			(0),
	mGaussMapLimit			(0),
	mTiledHeightFieldSamples(0),
	mFlip					(false)
{
}

MetaData::~MetaData()
{
	PxU32 nbMetaClasses = mMetaClasses.size();
	for(PxU32 i=0;i<nbMetaClasses;i++)
	{
		MetaClass* current = mMetaClasses[i];
		PX_DELETE(current);
	}

	PX_FREE(mStringTable);
	PX_DELETE_ARRAY(mEntries);
}

MetaClass* MetaData::getMetaClass(const char* name)	const
{
	PxU32 nbMetaClasses = mMetaClasses.size();
	for(PxU32 i=0;i<nbMetaClasses;i++)
	{
		MetaClass* current = mMetaClasses[i];
		if(strcmp(current->mClassName, name)==0)
		{
			while(current->mMaster)
				current = current->mMaster;
			return current;
		}
	}
	return NULL;
}

MetaClass* MetaData::getMetaClass(PxConcreteType::Enum concreteType) const
{
	for(PxU32 i=0; i< mConcreteTypeTable.size(); i++)
	{
		if(mConcreteTypeTable[i].first == concreteType)
		{
			const char* className = offsetToText((const char*)size_t(mConcreteTypeTable[i].second));
			return getMetaClass(className);
		}
	}
	return NULL;
}

MetaClass* MetaData::addNewClass(const char* name, int size, MetaClass* master, ConvertCallback callback)
{
	// PT: if you reach this assert, you used PX_DEF_BIN_METADATA_TYPEDEF twice on the same type
	assert(!getMetaClass(name));
	MetaClass* mc = PX_NEW(MetaClass);
	mc->mCallback	= callback;
	mc->mMaster		= master;
	mc->mClassName	= name;
	mc->mSize		= size;
	mc->mDepth		= 0;
	mc->mProcessed	= false;
//	mc->mNbEntries	= -1;

	mMetaClasses.pushBack(mc);

	return mc;
}

bool MetaData::load(PxInputStream& inputStream, MetaDataType type)
{
	assert(type!=META_DATA_NONE);

	mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, "Loading %s meta-data...\n", type==META_DATA_SRC ? "source" : "target");

	mType = type;

	bool platformIsWiiU;
	mFlip = false;
	{
		int header;
		inputStream.read(&header, 4);
		if(header==PX_MAKE_FOURCC('M','E','T','A'))
		{
			mFlip = false;
		}
		else if(header==PX_MAKE_FOURCC('A','T','E','M'))
		{
			mFlip = true;
		}
		else
		{
			mConvX.displayMessage(PxErrorCode::eINVALID_PARAMETER, "PxBinaryConverter: invalid meta-data file!\n");
			return false;
		}

		if (type == META_DATA_SRC && mFlip)
		{
			mConvX.displayMessage(PxErrorCode::eINVALID_PARAMETER,
				"PxBinaryConverter: source meta data needs to match endianness with current system!");
			return false;
		}

		inputStream.read(&mVersion, 4);
		inputStream.read(&mBinaryVersion, 4);
		if(mFlip)
		{
			flip(mVersion);
			flip(mBinaryVersion);
		}

		if (!checkCompatibility((PxU32)mVersion, (PxU32)mBinaryVersion))
		{
			char buffer[512];
		    getCompatibilityVersionsStr(buffer, 512);

			mConvX.displayMessage(PxErrorCode::eINVALID_PARAMETER,
				"PxBinaryConverter: data version (%x-%d) is incompatible with this PhysX sdk.\n These versions would be compatible: %s",
				mVersion, mBinaryVersion, buffer);
			
			return false;
		}
		inputStream.read(&mBuildNumber, 4);
		if(mFlip)
			flip(mBuildNumber);

		
		inputStream.read(&mSizeOfPtr, 4);
		if(mFlip)
			flip(mSizeOfPtr);

		inputStream.read(&mPlatformTag, 4);
		if(mFlip)
			flip(mPlatformTag);

		platformIsWiiU = (mPlatformTag == PX_MAKE_FOURCC('W','I','I','U'));

		if (!Sn::isBinaryPlatformTagValid((PxU32)mPlatformTag))
		{
			mConvX.displayMessage(PxErrorCode::eINVALID_PARAMETER, "PxBinaryConverter: Unknown meta data platform tag");
			return false;
		}

		inputStream.read(&mGaussMapLimit, 4);
		if(mFlip)
			flip(mGaussMapLimit);

		inputStream.read(&mTiledHeightFieldSamples, 4);
		if(mFlip)
			flip(mTiledHeightFieldSamples);

		inputStream.read(&mNbEntries, 4);
		if(mFlip)
			flip(mNbEntries);

		mEntries = PX_NEW(PxMetaDataEntry)[(PxU32)mNbEntries];
		if(mSizeOfPtr==8)
		{
			for(int i=0;i<mNbEntries;i++)
			{
				MetaDataEntry64 tmp;
				inputStream.read(&tmp, sizeof(MetaDataEntry64));
				if (mFlip)  // important to flip them first, else the cast below might destroy information
				{
					flip(tmp.mType);
					flip(tmp.mName);
				}
				// We can safely cast to 32bits here since we transformed the pointers to offsets in the string table on export
				mEntries[i].mType		= (const char*)(size_t)tmp.mType;
				mEntries[i].mName		= (const char*)(size_t)tmp.mName;
				mEntries[i].mOffset		= tmp.mOffset;
				mEntries[i].mSize		= tmp.mSize;
				mEntries[i].mCount		= tmp.mCount;
				mEntries[i].mOffsetSize	= tmp.mOffsetSize;
				mEntries[i].mFlags		= tmp.mFlags;
				mEntries[i].mAlignment	= tmp.mAlignment;
			}
		}
		else
		{
			assert(mSizeOfPtr==4);
//			inputStream.read(mEntries, mNbEntries*sizeof(PxMetaDataEntry));
			for(int i=0;i<mNbEntries;i++)
			{
				MetaDataEntry32 tmp;
				inputStream.read(&tmp, sizeof(MetaDataEntry32));
				if (mFlip)
				{
					flip(tmp.mType);
					flip(tmp.mName);
				}
				mEntries[i].mType		= (const char*)size_t(tmp.mType);
				mEntries[i].mName		= (const char*)size_t(tmp.mName);
				mEntries[i].mOffset		= tmp.mOffset;
				mEntries[i].mSize		= tmp.mSize;
				mEntries[i].mCount		= tmp.mCount;
				mEntries[i].mOffsetSize	= tmp.mOffsetSize;
				mEntries[i].mFlags		= tmp.mFlags;
				mEntries[i].mAlignment	= tmp.mAlignment;
			}
		}

		if(mFlip)
		{
			for(int i=0;i<mNbEntries;i++)
			{
				// mEntries[i].mType and mEntries[i].mName have been flipped already because they need special treatment
				// on 64bit to 32bit platform conversions
				flip(mEntries[i].mOffset);
				flip(mEntries[i].mSize);
				flip(mEntries[i].mCount);
				flip(mEntries[i].mOffsetSize);
				flip(mEntries[i].mFlags);
				flip(mEntries[i].mAlignment);
			}
		}

		int nbConcreteType;
		inputStream.read(&nbConcreteType, 4);
		if(mFlip)
			flip(nbConcreteType);

		for(int i=0; i<nbConcreteType; i++)
		{
			PxU16 concreteType;
			PxU32 nameOffset;
			inputStream.read(&concreteType, 2);
			inputStream.read(&nameOffset, 4);
			if(mFlip)
			{
			    flip(concreteType);
				flip(nameOffset);
			}
			
			mConcreteTypeTable.pushBack( Ps::Pair<PxConcreteType::Enum, PxU32>(PxConcreteType::Enum(concreteType), nameOffset) );
		}

		int tableSize;
		inputStream.read(&tableSize, 4);
		if(mFlip)
			flip(tableSize);

		mStringTable = (char*)PX_ALLOC(sizeof(char)*tableSize, PX_DEBUG_EXP("MetaData StringTable"));
		inputStream.read(mStringTable, (PxU32)tableSize);
	}

	// Register atomic types
	{
		addNewClass("bool",			1, NULL, &ConvX::convert8);
		addNewClass("char",			1, NULL, &ConvX::convert8);
		addNewClass("short",		2, NULL, &ConvX::convert16);
		addNewClass("int",			4, NULL, &ConvX::convert32);
		addNewClass("PxU64",		8, NULL, &ConvX::convert64);
		addNewClass("float",		4, NULL, &ConvX::convertFloat);

		addNewClass("paddingByte",	1, NULL, &ConvX::convertPad8);
	}

	{
		MetaClass* currentClass = NULL;
		for(int i=0;i<mNbEntries;i++)
		{
			mEntries[i].mType = offsetToText(mEntries[i].mType);
			mEntries[i].mName = offsetToText(mEntries[i].mName);

			if(mEntries[i].mFlags & PxMetaDataFlag::eTYPEDEF)
			{
				mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, "Found typedef: %s => %s\n", mEntries[i].mName, mEntries[i].mType);
				MetaClass* mc = getMetaClass(mEntries[i].mName);
				if(mc)
					addNewClass(mEntries[i].mType, mc->mSize, mc, mc->mCallback);
				else
					mConvX.displayMessage(PxErrorCode::eINTERNAL_ERROR,
					  "PxBinaryConverter: Invalid typedef - Missing metadata for: %s, please check the source metadata.\n"
					  , mEntries[i].mName);
			}
			else if(mEntries[i].mFlags & PxMetaDataFlag::eCLASS)
			{
				if(!mEntries[i].mName)
				{
							mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, "Found class: %s\n", mEntries[i].mType);
					currentClass = addNewClass(mEntries[i].mType, mEntries[i].mSize);

					if(mEntries[i].mFlags & PxMetaDataFlag::eVIRTUAL)
					{
						PxMetaDataEntry vtable;
						vtable.mType	= gVTablePtr;
						vtable.mName	= gVTablePtr;

						if (!platformIsWiiU)
							vtable.mOffset	= 0;
						else
							vtable.mOffset	= -1; 
						   // on Wii U the vtable ptr is at the end of a class. The offsets will get adjusted further below.

						vtable.mSize	= mSizeOfPtr;
						vtable.mCount	= 1;
						vtable.mFlags	= PxMetaDataFlag::ePTR;
						currentClass->mFields.pushBack(vtable);
					}
				}
				else
				{
					assert(currentClass);
					mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, " - inherits from: %s\n", mEntries[i].mName);
					currentClass->mBaseClasses.pushBack(mEntries[i]);
				}
			}
			else
			{
				const int isUnion = mEntries[i].mFlags & PxMetaDataFlag::eUNION;

				if(isUnion && !mEntries[i].mSize)
				{
					mConvX.registerUnionType(mEntries[i].mType, mEntries[i].mName, mEntries[i].mOffset);
				}
				else
				{
					if(isUnion)
					{
						mConvX.registerUnion(mEntries[i].mType);
					}

					const int isPadding = mEntries[i].mFlags & PxMetaDataFlag::ePADDING;

					assert(currentClass);
#ifdef REMOVE_EXPLICIT_PADDING
					if(!isPadding)
#endif
						currentClass->mFields.pushBack(mEntries[i]);
					
					if(isPadding)
						mConvX.displayMessage(PxErrorCode::eDEBUG_INFO, 
						" - contains padding: %s - %s\n", mEntries[i].mType, mEntries[i].mName);
					else if(mEntries[i].mFlags & PxMetaDataFlag::eEXTRA_DATA)
						mConvX.displayMessage(PxErrorCode::eDEBUG_INFO,
						" - contains extra data: %s%s\n", mEntries[i].mType, mEntries[i].mFlags & PxMetaDataFlag::ePTR ? "*" : "");
					else
						mConvX.displayMessage(PxErrorCode::eDEBUG_INFO,
						" - contains field: %s%s\n", mEntries[i].mType, mEntries[i].mFlags & PxMetaDataFlag::ePTR ? "*" : "");
					
				}
			}
		}
	}

	if (platformIsWiiU)
	{
		// Adjust vtable pointer entries of base classes because they are at the end of a class instance on Wii U
		// (the derived classes are handled further below)
		{
			const PxU32 nbMetaClasses = mMetaClasses.size();
			for(PxU32 i=0;i<nbMetaClasses;i++)
			{
				MetaClass* current = mMetaClasses[i];
				
				if (!current->mBaseClasses.size())
				{
					PxU32 nbBaseFields = current->mFields.size();
					if (nbBaseFields)
					{
#ifdef PX_DEBUG
						for(PxU32 j=0;j<nbBaseFields;j++)
						{
							PxMetaDataEntry f = current->mFields[j];

							if(f.mType==gVTablePtr)
							{
								PX_ASSERT(j == 0);  // there should only be one vtable ptr in a base class and it should be the first field
							}
						}
#endif

						PxMetaDataEntry& f = current->mFields[0];
						if (f.mType == gVTablePtr)
						{
							f.mOffset = 0;
							if (nbBaseFields > 1)
							{
								//
								//we need to cover a special case:
								//the meta data system does not describe all base classes, especially it sometimes ignores abstract classes 
								//and only describes the derived ones. However, the abstract classes define the offset of the vtable, so we
								//need to make sure not to adjust the offset in those cases.
								//

								// find field with smallest offset
								// should enforce that fields are described in the order in which they are declared?
								const int initValue = 0x6fffffff;
								int minOffset = initValue;
								for(PxU32 j=1; j < nbBaseFields; j++)
								{
									PxMetaDataEntry& ft = current->mFields[j];
									if (ft.mName && (ft.mOffset < minOffset))
									{
										minOffset = ft.mOffset;
									}
								}
								PX_ASSERT(minOffset != initValue);

								if (minOffset == 0)  
								{	// else the class inherits from a not-described, abstract class with no members, in which case the
									//vtable ptr is at 0 (size=4 - 4)
								
									f.mOffset = current->mSize - 4;	
									// this might be a bit iffy in cases where the class inherits from another one but does not describe
									//the latter in the meta data. The size to choose would then be the one of the base class.
									//If we get into this, the system should complain about overlapping fields though.
								}
							}
						}
					}
				}
			}
		}
	}

	// Sort classes by depth
	struct Local
	{
		static bool _computeDepth(const MetaData& md, MetaClass* current, int currentDepth, int& maxDepth)
		{
			if(currentDepth>maxDepth)
				maxDepth = currentDepth;

			PxU32 nbBases = current->mBaseClasses.size();
			for(PxU32 i=0;i<nbBases;i++)
			{
				const PxMetaDataEntry& baseClassEntry = current->mBaseClasses[i];
				MetaClass* baseClass = md.getMetaClass(baseClassEntry.mName);
				if(!baseClass)
				{
					md.mConvX.displayMessage(PxErrorCode::eINTERNAL_ERROR,
					"PxBinaryConverter: Can't find class %s metadata, please check the source metadata.\n", baseClassEntry.mName);	
					return false;
				}
				if (!_computeDepth(md, baseClass, currentDepth+1, maxDepth))
					return false;
			}
			return true;
		}

		static int compareClasses(const void* c0, const void* c1)
		{
			MetaClass** mc0 = (MetaClass**)c0;
			MetaClass** mc1 = (MetaClass**)c1;
//			return (*mc0)->mSize - (*mc1)->mSize;
			return (*mc0)->mDepth - (*mc1)->mDepth;
		}

		static int compareEntries(const void* c0, const void* c1)
		{
			PxMetaDataEntry* mc0 = (PxMetaDataEntry*)c0;
			PxMetaDataEntry* mc1 = (PxMetaDataEntry*)c1;
			//mOffset is used to access control information for extra data, and not for offsets of the data itself.
			assert(!(mc0->mFlags & PxMetaDataFlag::eEXTRA_DATA));
			assert(!(mc1->mFlags & PxMetaDataFlag::eEXTRA_DATA));
			return mc0->mOffset - mc1->mOffset;
		}
	};
	{
		// Compute depths
		const PxU32 nbMetaClasses = mMetaClasses.size();
		for(PxU32 i=0;i<nbMetaClasses;i++)
		{
			MetaClass* current = mMetaClasses[i];
			int maxDepth = 0;
			if(!Local::_computeDepth(*this, current, 0, maxDepth))
				return false;
			current->mDepth = maxDepth;
		}

		// Sort by depth
		MetaClass** metaClasses = &mMetaClasses[0];
		qsort(metaClasses, (size_t)nbMetaClasses, sizeof(MetaClass*), Local::compareClasses);
	}

	// Replicate fields from base classes
	{
		PxU32 nbMetaClasses = mMetaClasses.size();
		for(PxU32 k=0;k<nbMetaClasses;k++)
		{
			MetaClass* current = mMetaClasses[k];
			PxU32 nbBases = current->mBaseClasses.size();

			// merge entries of base classes and current class in the right order
			// this is needed for extra data ordering, which is not covered by the mOffset sort 
			// in the next stage below
			PsArray<PxMetaDataEntry> mergedEntries;

			for(PxU32 i=0;i<nbBases;i++)
			{
				const PxMetaDataEntry& baseClassEntry = current->mBaseClasses[i];
				MetaClass* baseClass = getMetaClass(baseClassEntry.mName);
				assert(baseClass);
				assert(baseClass->mBaseClasses.size()==0 || baseClass->mProcessed);

				PxU32 nbBaseFields = baseClass->mFields.size();
				for(PxU32 j=0;j<nbBaseFields;j++)
				{
					PxMetaDataEntry f = baseClass->mFields[j];
					// Don't merge primary v-tables to avoid redundant v-table entries.
					// It means the base v-table won't be inherited & needs to be explicitly defined in the metadata. Seems reasonable.
					// Could be done better though.

					if (!platformIsWiiU)
					{
						if(f.mType==gVTablePtr && !f.mOffset && !baseClassEntry.mOffset)
							continue;
					}
					else
					{
						// WiiU compiler puts vtable pointer at the end
						if((f.mType==gVTablePtr) && (!baseClassEntry.mOffset))
						{
							PX_ASSERT(current->mFields.size());
							PxMetaDataEntry& fcur = current->mFields[0];
							PX_ASSERT(fcur.mType == gVTablePtr);

							// Potentially dangerous: what if the vtables in the base class are not sorted?
							// Would have to find the one with smallest offset
							if (fcur.mOffset == -1)
							{
								fcur.mOffset = f.mOffset;
								continue;
							}
						}
					}
					
					f.mOffset += baseClassEntry.mOffset;
					mergedEntries.pushBack(f);
				}
				current->mProcessed = true;
			}

			//append current fields to base class fields 
			for (PxU32 i = 0; i < current->mFields.size(); i++)
			{
				mergedEntries.pushBack(current->mFields[i]);
			}
			current->mFields.clear();
			current->mFields.assign(mergedEntries.begin(), mergedEntries.end());

			if (platformIsWiiU)
				PX_ASSERT((!current->mFields.size()) || (current->mFields[0].mType != gVTablePtr) || (current->mFields[0].mOffset >= 0));
				// if this assert fails it is likely that a class has a vtable but was described as PX_DEF_BIN_METADATA_CLASS
			    // instead of PX_DEF_BIN_METADATA_VCLASS
		}
	}

	// Check classes
	{
		PxU32 nbMetaClasses = mMetaClasses.size();
		for(PxU32 i=0;i<nbMetaClasses;i++)
		{
			MetaClass* current = mMetaClasses[i];
			if(!current->check(*this))
				return false;
		}
	}

	// Sort meta-data by offset
	{
		PxU32 nbMetaClasses = mMetaClasses.size();
		for(PxU32 i=0;i<nbMetaClasses;i++)
		{
			MetaClass* current = mMetaClasses[i];
			PxU32 nbFields = current->mFields.size();
			if(nbFields<2)
				continue;
			PxMetaDataEntry* entries = &current->mFields[0];

			PxMetaDataEntry* newEntries = PX_NEW(PxMetaDataEntry)[nbFields];
			PxU32 nb = 0;
			for(PxU32 j=0;j<nbFields;j++)
				if(!(entries[j].mFlags & PxMetaDataFlag::eEXTRA_DATA))
					newEntries[nb++] = entries[j];
			PxU32 nbToSort = nb;
			for(PxU32 j=0;j<nbFields;j++)
				if(entries[j].mFlags & PxMetaDataFlag::eEXTRA_DATA)
					newEntries[nb++] = entries[j];
			assert(nb==nbFields);
			memcpy(entries, newEntries, nb*sizeof(PxMetaDataEntry));
			PX_DELETE_ARRAY(newEntries);
			qsort(entries, (size_t)nbToSort, sizeof(PxMetaDataEntry), Local::compareEntries);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void ConvX::releaseMetaData()
{
	DELETESINGLE(mMetaData_Dst);
	DELETESINGLE(mMetaData_Src);
}

const MetaData* ConvX::loadMetaData(PxInputStream& inputStream, MetaDataType type)
{
	if (type != META_DATA_SRC && type != META_DATA_DST)
	{
		displayMessage(PxErrorCode::eINTERNAL_ERROR,
			           "PxBinaryConverter: Wrong meta data type, please check the source metadata.\n");
		return NULL;
	}

	PX_ASSERT(type == META_DATA_SRC || type == META_DATA_DST);

	MetaData*& metaDataPtr = (type == META_DATA_SRC) ? mMetaData_Src : mMetaData_Dst;
	metaDataPtr = PX_NEW(MetaData)(*this);
	if(!(metaDataPtr)->load(inputStream, type))
		DELETESINGLE(metaDataPtr);
	return metaDataPtr;
}

const MetaData* ConvX::getBinaryMetaData(MetaDataType type)
{
	if(type==META_DATA_SRC)
		return mMetaData_Src;
	if(type==META_DATA_DST)
		return mMetaData_Dst;
	assert(0);
	return NULL;
}

int ConvX::getNbMetaClasses(MetaDataType type)
{
	if(type==META_DATA_SRC)
		return mMetaData_Src->getNbMetaClasses();
	if(type==META_DATA_DST)
		return mMetaData_Dst->getNbMetaClasses();
	assert(0);
	return 0;
}

MetaClass* ConvX::getMetaClass(int i, MetaDataType type) const
{
	if(type==META_DATA_SRC)
		return mMetaData_Src->getMetaClass((unsigned int)i);
	if(type==META_DATA_DST)
		return mMetaData_Dst->getMetaClass((unsigned int)i);
	assert(0);
	return NULL;
}

MetaClass* ConvX::getMetaClass(const char* name, MetaDataType type) const
{
	if(type==META_DATA_SRC)
		return mMetaData_Src->getMetaClass(name);
	if(type==META_DATA_DST)
		return mMetaData_Dst->getMetaClass(name);
	assert(0);
	return NULL;
}

MetaClass* ConvX::getMetaClass(PxConcreteType::Enum concreteType, MetaDataType type)
{
	MetaClass* metaClass = NULL;
	if(type==META_DATA_SRC)
		metaClass = mMetaData_Src->getMetaClass(concreteType);
	if(type==META_DATA_DST)
		metaClass = mMetaData_Dst->getMetaClass(concreteType);

	if(!metaClass)
	{		
		displayMessage(PxErrorCode::eINTERNAL_ERROR,
			"PxBinaryConverter: Missing concreteType %d metadata! serialized a class without dumping metadata. Please check the metadata.",
			concreteType);
		return NULL;
	}	

	return metaClass;
}

///////////////////////////////////////////////////////////////////////////////

// Peek & poke, yes sir.
PxU64 physx::Sn::peek(int size, const char* buffer, int flags)
{
	const int maskMSB = flags & PxMetaDataFlag::eCOUNT_MASK_MSB;
	const int skipIfOne = flags & PxMetaDataFlag::eCOUNT_SKIP_IF_ONE;
	switch(size)
	{
		case 1:
		{
			unsigned char value = *(unsigned char*)buffer;
			if(maskMSB)
				value &= 0x7f;
			if(skipIfOne && value==1)
				return 0;
			return PxU64(value);
		}
		case 2:
		{
			unsigned short value = *(unsigned short*)buffer;
			if(maskMSB)
				value &= 0x7fff;
			if(skipIfOne && value==1)
				return 0;
			return PxU64(value);
		}
		case 4:
		{
			unsigned int value = *(unsigned int*)buffer;
			if(maskMSB)
				value &= 0x7fffffff;
			if(skipIfOne && value==1)
				return 0;
			return PxU64(value);
		}
		case 8:
		{
			PxU64 value = *(PxU64*)buffer;
			if(maskMSB)
				value &= 0x7fffffffffffffffll;
			if(skipIfOne && value==1)
				return 0;
			return value;
		}
	};
	assert(0);
	return 0xffffffffffffffffull;
}


