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
#ifndef PX_SIMPLEXMLWRITER_H
#define PX_SIMPLEXMLWRITER_H

#include "PsArray.h"
#include "SnXmlMemoryPoolStreams.h"
#include "CmPhysXCommon.h"

namespace physx { namespace Sn {
	class SimpleXmlWriter
	{
	public:
		
		struct STagWatcher
		{
			typedef SimpleXmlWriter TXmlWriterType;
			TXmlWriterType& mWriter;
			STagWatcher( const STagWatcher& inOther );
			STagWatcher& operator-( const STagWatcher& inOther );
			STagWatcher( TXmlWriterType& inWriter, const char* inTagName )
				: mWriter( inWriter )
			{
				mWriter.beginTag( inTagName );
			}
			~STagWatcher() { mWriter.endTag(); }
		protected:
			STagWatcher& operator=(const STagWatcher&);
		};

		virtual ~SimpleXmlWriter(){}
		virtual void beginTag( const char* inTagname ) = 0;
		virtual void endTag() = 0;
		virtual void addAttribute( const char* inName, const char* inValue ) = 0;
		virtual void writeContentTag( const char* inTag, const char* inContent ) = 0;
		virtual void addContent( const char* inContent ) = 0;
		virtual PxU32 tabCount() = 0;
	private:
		SimpleXmlWriter& operator=(const SimpleXmlWriter&);
	};

	template<typename TStreamType>
	class SimpleXmlWriterImpl : public SimpleXmlWriter
	{
		FoundationWrapper mWrapper;
		TStreamType& mStream;
		SimpleXmlWriterImpl( const SimpleXmlWriterImpl& inOther );
		SimpleXmlWriterImpl& operator=( const SimpleXmlWriterImpl& inOther );
		ProfileArray<const char*>	mTags;
		bool								mTagOpen;
		PxU32							mInitialTagDepth;
	public:

		SimpleXmlWriterImpl( TStreamType& inStream, PxAllocatorCallback& inAllocator, PxU32 inInitialTagDepth = 0 )
			: mWrapper( inAllocator )
			, mStream( inStream )
			, mTags( mWrapper )
			, mTagOpen( false )
			, mInitialTagDepth( inInitialTagDepth )
		{
		}
		virtual ~SimpleXmlWriterImpl()
		{
			while( mTags.size() )
				endTag();
		}
		PxU32 tabCount() { return mTags.size() + mInitialTagDepth; }

		void writeTabs( PxU32 inSize )
		{
			inSize += mInitialTagDepth;
			for ( PxU32 idx =0; idx < inSize; ++idx )
				mStream << "\t";
		}
		void beginTag( const char* inTagname )
		{
			closeTag();
			writeTabs(mTags.size());
			mTags.pushBack( inTagname );
			mStream << "<" << inTagname;
			mTagOpen = true;
		}
		void addAttribute( const char* inName, const char* inValue )
		{
			PX_ASSERT( mTagOpen );
			mStream << " " << inName << "=" << "\"" << inValue << "\"";
		}
		void closeTag(bool useNewline = true)
		{
			if ( mTagOpen )
			{
				mStream << " " << ">";
				if (useNewline )
					mStream << "\n";
			}
			mTagOpen = false;
		}
		void doEndOpenTag()
		{
			mStream << "</" << mTags.back() << ">" << "\n";
		}
		void endTag()
		{
			PX_ASSERT( mTags.size() );
			if ( mTagOpen )
				mStream << " " << "/>" << "\n";
			else
			{
				writeTabs(mTags.size()-1);
				doEndOpenTag();
			}
			mTagOpen = false;
			mTags.popBack();
		}

		static bool IsNormalizableWhitespace(char c) { return c == 0x9 || c == 0xA || c == 0xD; }
		static bool IsValidXmlCharacter(char c) { return IsNormalizableWhitespace(c) || c >= 0x20; }

		void addContent( const char* inContent )
		{
			closeTag(false);
			//escape xml
			for( ; *inContent; inContent++ )
			{
				switch (*inContent) 
				{
				case '<':
					mStream << "&lt;";
					break;
				case '>':
					mStream << "&gt;";
					break;
				case '&':
					mStream << "&amp;";
					break;
				case '\'':
					mStream << "&apos;";
					break;
				case '"':
					mStream << "&quot;";
					break;
				default:
					if (IsValidXmlCharacter(*inContent)) {
						if (IsNormalizableWhitespace(*inContent))
						{
							char s[32];
							sprintf(s, "&#x%02X;", unsigned(*inContent));
							mStream << s;
						}
						else
							mStream << *inContent;
					}
					break;
				}
			}
		}

		void writeContentTag( const char* inTag, const char* inContent )
		{
			beginTag( inTag );
			addContent( inContent );
			doEndOpenTag();
			mTags.popBack();
		}
		void insertXml( const char* inXml )
		{
			closeTag();
			mStream << inXml;
		}
	};

	struct BeginTag
	{
		const char* mTagName;
		BeginTag( const char* inTagName )
			: mTagName( inTagName ) { }
	};

	struct EndTag
	{
		EndTag() {}
	};

	struct Att
	{
		const char* mAttName;
		const char* mAttValue;
		Att( const char* inAttName, const char* inAttValue )
			: mAttName( inAttName )
			, mAttValue( inAttValue ) {	}
	};

	struct Content
	{
		const char* mContent;
		Content( const char* inContent )
			: mContent( inContent ) { }
	};

	
	struct ContentTag
	{
		const char* mTagName;
		const char* mContent;
		ContentTag( const char* inTagName, const char* inContent )
			: mTagName( inTagName )
			, mContent( inContent ) { }
	};

	inline SimpleXmlWriter& operator<<( SimpleXmlWriter& inWriter, const BeginTag& inTag ) { inWriter.beginTag( inTag.mTagName ); return inWriter; }
	inline SimpleXmlWriter& operator<<( SimpleXmlWriter& inWriter, const EndTag& inTag ) { PX_UNUSED(inTag); inWriter.endTag(); return inWriter; }
	inline SimpleXmlWriter& operator<<( SimpleXmlWriter& inWriter, const Att& inTag ) { inWriter.addAttribute(inTag.mAttName, inTag.mAttValue); return inWriter; }
	inline SimpleXmlWriter& operator<<( SimpleXmlWriter& inWriter, const Content& inTag ) { inWriter.addContent(inTag.mContent); return inWriter; }
	inline SimpleXmlWriter& operator<<( SimpleXmlWriter& inWriter, const ContentTag& inTag ) { inWriter.writeContentTag(inTag.mTagName, inTag.mContent); return inWriter; }

	inline void writeProperty( SimpleXmlWriter& inWriter, MemoryBuffer& tempBuffer, const char* inPropName )
	{
		PxU8 data = 0;
		tempBuffer.write( &data, sizeof(PxU8) );
		inWriter.writeContentTag( inPropName, reinterpret_cast<const char*>( tempBuffer.mBuffer ) );
		tempBuffer.clear();
	}

	template<typename TDataType>
	inline void writeProperty( SimpleXmlWriter& inWriter, MemoryBuffer& tempBuffer, const char* inPropName, TDataType inValue )
	{
		tempBuffer << inValue;
		writeProperty( inWriter, tempBuffer, inPropName );
	}
} }
#endif
