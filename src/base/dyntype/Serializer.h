/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __SERIALIZER_H
#define __SERIALIZER_H

#include "compat.h"

#include <OgreMatrix3.h>
#include <OgreQuaternion.h>
#include <OgreMath.h>

#include "File.h"
#include "NonCopyable.h"
#include "vector3.h"

namespace Opde {
	/// Data serializer - used to fill the values of data based on File contents, and the other way round
	class Serializer : public NonCopyable {
		public:
			// destructor
			virtual ~Serializer() {};
			
			/// serializes the data into the specified fileptr
			virtual void serialize(FilePtr& dest, const void* valuePtr) = 0;

			/// deserializes the data from the specified fileptr
			virtual void deserialize(FilePtr& src, void* valuePtr) = 0;

			/// Returns the stored size of the type
			virtual size_t getStoredSize(const void* valuePtr) = 0;
	};

	/// Default template implementation of the serializer
	template<typename T> class TypeSerializer : public Serializer {
		public:
			virtual void serialize(FilePtr& dest, const void* valuePtr) {
				dest->writeElem(valuePtr, sizeof(T));
			};

			virtual void deserialize(FilePtr& src, void* valuePtr) {
				src->readElem(valuePtr, sizeof(T));
			};

			virtual size_t getStoredSize(const void* valuePtr) {
				return sizeof(T);
			};
	};

	// specializations for various special types

	/// Bool specialization of the TypeSerializer::serialize
	template<> void TypeSerializer<bool>::serialize(FilePtr& dest, const void* valuePtr);
	/// Bool specialization of the TypeSerializer::deserialize
	template<> void TypeSerializer<bool>::deserialize(FilePtr& src, void* valuePtr);
	/// Bool stored size getter
	template<> size_t TypeSerializer<bool>::getStoredSize(const void* valuePtr);

	// Vector3 specialization of the TypeSerializer::serialize
	template<> void TypeSerializer<Vector3>::serialize(FilePtr& dest, const void* valuePtr);
	/// Vector3 specialization of the TypeSerializer::deserialize
	template<> void TypeSerializer<Vector3>::deserialize(FilePtr& src, void* valuePtr);
	/// Vector3 stored size getter
	template<> size_t TypeSerializer<Vector3>::getStoredSize(const void* valuePtr);

	// Quaternion spec. Stored as HPB on disk
	/// Quaternion specialization of the TypeSerializer::serialize
	template<> void TypeSerializer<Ogre::Quaternion>::serialize(FilePtr& dest, const void* valuePtr);
	/// Quaternion specialization of the TypeSerializer::deserialize
	template<> void TypeSerializer<Ogre::Quaternion>::deserialize(FilePtr& src, void* valuePtr);
	/// Quaternion stored size getter
	template<> size_t TypeSerializer<Ogre::Quaternion>::getStoredSize(const void* valuePtr);

	/// Variable length string specialization of the TypeSerializer::serialize
	template<> void TypeSerializer<std::string>::serialize(FilePtr& dest, const void* valuePtr);
	/// Variable length string specialization of the Serializer::deserialize
	template<> void TypeSerializer<std::string>::deserialize(FilePtr& src, void* valuePtr);
	/// Variable length string stored size getter
	template<> size_t TypeSerializer<std::string>::getStoredSize(const void* valuePtr);

}

#endif

