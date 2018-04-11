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
#ifndef PX_XML_READER_H
#define PX_XML_READER_H

#include "PxSimpleTypes.h"
#include "extensions/PxRepXSimpleType.h"

namespace physx {
	namespace Sn { struct XmlNode; }
	
	/**
	 *	Reader used to read data out of the repx format.
	 */
	class XmlReader
	{
	protected:
		virtual ~XmlReader(){}
	public:
		/** Read a key-value pair out of the database */
		virtual bool read( const char* inName, const char*& outData ) = 0;
		/** Read an object id out of the database */
		virtual bool read( const char* inName, PxSerialObjectId& outId ) = 0;
		/** Goto a child element by name.  That child becomes this reader's context */
		virtual bool gotoChild( const char* inName ) = 0;
		/** Goto the first child regardless of name */
		virtual bool gotoFirstChild() = 0;
		/** Goto the next sibling regardless of name */
		virtual bool gotoNextSibling() = 0;
		/** Count all children of the current object */
		virtual PxU32 countChildren() = 0;
		/** Get the name of the current item */
		virtual const char* getCurrentItemName() = 0;
		/** Get the value of the current item */
		virtual const char* getCurrentItemValue() = 0;
		/** Leave the current child */
		virtual bool leaveChild() = 0;

		/**
		 *	Ensures we don't leave the reader in an odd state
		 *	due to not leaving a given child
		 */
		virtual void pushCurrentContext() = 0;
		/** Pop the current context back to where it during push*/
		virtual void popCurrentContext() = 0; 
	};

	//Used when upgrading a repx collection
	class XmlReaderWriter : public XmlReader
	{
	public:
		//Clears the stack of nodes (push/pop current node reset)
		//and sets the current node to inNode.
		virtual void setNode( Sn::XmlNode& node ) = 0;
		//If the child exists, add it.
		//the either way goto that child.
		virtual void addOrGotoChild( const char* name ) = 0;
		//Value is copied into the collection, inValue has no further references
		//to it.
		virtual void setCurrentItemValue( const char* value ) = 0;
		//Removes the child but does not release the char* name or char* data ptrs.
		//Those pointers are never released and are shared among collections.
		//Thus copying nodes is cheap and safe.
		virtual bool removeChild( const char* name ) = 0;
		virtual void release() = 0;

		bool renameProperty( const char* oldName, const char* newName )
		{
			if ( gotoChild( oldName ) )
			{
				const char* value = getCurrentItemValue();
				leaveChild();
				removeChild( oldName );
				addOrGotoChild( newName );
				setCurrentItemValue( value );
				leaveChild();
				return true;
			}
			return false;
		}
		bool readAndRemoveProperty( const char* name, const char*& outValue )
		{
			bool retval = read( name, outValue ); 
			if ( retval ) removeChild( name );
			return retval;
		}

		bool writePropertyIfNotEmpty( const char* name, const char* value )
		{
			if ( value && *value )
			{ 
				addOrGotoChild( name ); 
				setCurrentItemValue( value ); 
				leaveChild(); 
				return true;
			}
			return false;
		}
	};

} 
#endif
