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

#include "Quaternion.h"
#include <OgreMath.h>
#include <OgreMatrix3.h>

#include "File.h"
#include "NonCopyable.h"
#include "Vector3.h"

namespace Opde {

/// Data serializer - used to fill the values of data based on File contents,
/// and the other way round
class Serializer : public NonCopyable {
public:
    /// destructor
    virtual ~Serializer(){};

    /// serializes the data into the specified fileptr
    virtual void serialize(FilePtr &dest, const void *valuePtr) = 0;

    /// deserializes the data from the specified fileptr
    virtual void deserialize(FilePtr &src, void *valuePtr) = 0;

    /// Returns the stored size of the type
    virtual size_t getStoredSize(const void *valuePtr) = 0;
};

/// Default template implementation of the serializer
template <typename T> class TypeSerializer : public Serializer {
public:
    virtual void serialize(FilePtr &dest, const void *valuePtr) {
        dest->writeElem(valuePtr, sizeof(T));
    };

    virtual void deserialize(FilePtr &src, void *valuePtr) {
        src->readElem(valuePtr, sizeof(T));
    };

    virtual size_t getStoredSize(const void *valuePtr) { return sizeof(T); };
};

/// Fixed size string serializer - serializes first N characters of given string
/// pointer
template<size_t LenT>
class FixedStringSerializer : public Serializer {
public:
    static_assert(LenT > 0, "Fixed string has to have nonzero positive length");

    // contructor
    FixedStringSerializer() {};

    /// destructor
    virtual ~FixedStringSerializer() {};

    /// serializes the data into the specified fileptr
    virtual void serialize(FilePtr &dest, const void *valuePtr) {
        // prepare a fixed char array for the write
        char copyStr[LenT];

        const std::string *str = static_cast<const std::string *>(valuePtr);
        size_t strsz = str->length();

        if (strsz > LenT)
            strsz = LenT;

        str->copy(copyStr, strsz);
        copyStr[std::min(strsz, LenT - 1)] = '\0';

        dest->write(copyStr, LenT);
    };

    /// deserializes the data from the specified fileptr
    virtual void deserialize(FilePtr &src, void *valuePtr) {
        // read the buf, fill the dest str with it
        char copyStr[LenT + 1];
        src->read(copyStr, LenT);
        copyStr[LenT] = '\0';
        static_cast<std::string *>(valuePtr)->assign(copyStr);
    }

    /// Returns the stored size of the type
    virtual size_t getStoredSize(const void *valuePtr) { return LenT; }
};

// specializations for various special types

/// Bool specialization of the TypeSerializer::serialize
template <>
void TypeSerializer<bool>::serialize(FilePtr &dest, const void *valuePtr);
/// Bool specialization of the TypeSerializer::deserialize
template <>
void TypeSerializer<bool>::deserialize(FilePtr &src, void *valuePtr);
/// Bool stored size getter
template <> size_t TypeSerializer<bool>::getStoredSize(const void *valuePtr);

// Vector3 specialization of the TypeSerializer::serialize
template <>
void TypeSerializer<Vector3>::serialize(FilePtr &dest, const void *valuePtr);
/// Vector3 specialization of the TypeSerializer::deserialize
template <>
void TypeSerializer<Vector3>::deserialize(FilePtr &src, void *valuePtr);
/// Vector3 stored size getter
template <> size_t TypeSerializer<Vector3>::getStoredSize(const void *valuePtr);

// Quaternion spec. Stored as HPB on disk
/// Quaternion specialization of the TypeSerializer::serialize
template <>
void TypeSerializer<Ogre::Quaternion>::serialize(FilePtr &dest,
                                                 const void *valuePtr);
/// Quaternion specialization of the TypeSerializer::deserialize
template <>
void TypeSerializer<Ogre::Quaternion>::deserialize(FilePtr &src,
                                                   void *valuePtr);
/// Quaternion stored size getter
template <>
size_t TypeSerializer<Ogre::Quaternion>::getStoredSize(const void *valuePtr);

/// Variable length string specialization of the TypeSerializer::serialize
template <>
void TypeSerializer<std::string>::serialize(FilePtr &dest,
                                            const void *valuePtr);
/// Variable length string specialization of the Serializer::deserialize
template <>
void TypeSerializer<std::string>::deserialize(FilePtr &src, void *valuePtr);
/// Variable length string stored size getter
template <>
size_t TypeSerializer<std::string>::getStoredSize(const void *valuePtr);

} // namespace Opde

#endif
