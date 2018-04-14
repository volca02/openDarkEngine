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
 *    $Id$
 *
 *****************************************************************************/

#include "DTypeDef.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include "DTHelpers.h"
#include "OpdeException.h"
#include "integers.h"

#include <OgreMath.h>
#include <OgreMatrix3.h>

using namespace std;

using Ogre::Math;
using Ogre::Matrix3;
using Ogre::Radian;

namespace Opde {
class DTypeDef;

/*------------------------------------------------------*/
/*------------------- DEnum ----------------------------*/
/*------------------------------------------------------*/
DEnum::DEnum(const std::string &name, DVariant::Type enumType, bool bitfield)
    : mValMap(), mName(name) {
    mEnumType = enumType;
    mBitField = bitfield;

    if (mBitField && mEnumType != DVariant::DV_UINT)
        OPDE_EXCEPT("Only uint is supported for bitfields", "DEnum::DEnum");
}

//------------------------------------
DEnum::~DEnum() {}

//------------------------------------
void DEnum::insert(const std::string &key, const DVariant &value) {
    // Type check
    if (mEnumType != value.type())
        throw(runtime_error("Type violation of the enumeration type"));

    mValMap.insert(make_pair(key, value));
}

//------------------------------------
const string &DEnum::symbol(const DVariant &val) const {
    if (mEnumType != val.type())
        throw(runtime_error("Type violation of the enumeration type"));

    StrValMap::const_iterator it = mValMap.begin();

    for (; it != mValMap.end(); it++) {
        if (it->second == val)
            return it->first;
    }

    throw(out_of_range("DEnum::symbol"));
}

//------------------------------------
const DVariant &DEnum::value(const std::string &symbol) const {
    StrValMap::const_iterator it = mValMap.find(symbol);

    if (it == mValMap.end())
        return it->second;

    throw(out_of_range("DEnum::value"));
}

//------------------------------------
DEnum::EnumFieldList DEnum::getFieldList(const DVariant &val) const {
    uint uval = 0;

    if (mBitField) {
        // is it convertible to the uint? If not, this will throw an exception,
        // so it's alright
        uval = val.toUInt();
    }

    // insert all the fields, comparing the variants to check the selected
    StrValMap::const_iterator it = mValMap.begin();

    EnumFieldList l;

    for (; it != mValMap.end(); it++) {

        EnumField f(it->first, it->second, false);

        if (!mBitField) {
            if (it->second == val)
                f.checked = true;
        } else {
            if (it->second.toUInt() & uval)
                f.checked = true;
        }

        l.push_back(f);
    }

    return l;
}

/*------------------------------------------------------*/
/*------------------- DTypeDef -------------------------*/
/*------------------------------------------------------*/
/** Reference counted private type definition base. */

/** Simple value holder. */
class DTPrivateSimple : public DTPrivateBase {
protected:
    /// Type of the field
    DVariant::Type mType;

    /// Specified field size
    int mSize;

    /// Enum holder. for enumerated types, NULL otherwise
    DEnumPtr mEnum;

public:
    /// Constructor
    DTPrivateSimple(const DVariant::Type &type, size_t size,
                    const DEnumPtr &_enum)
        : DTPrivateBase(NT_SIMPLE), mType(type), mSize(size), mEnum(_enum){};

    /// Destructor
    virtual ~DTPrivateSimple() {}

    /// Enum getter
    virtual DEnumPtr getEnum() { return mEnum; }

    /// true if the simple type uses enumeration/bitfield
    bool isEnumerated() { return mEnum.get(); }

    /// Size of the simple type (of the data holder)
    virtual int size() { return mSize; }

    virtual DVariant::Type type() { return mType; }
};

/// Structured Type definition. can be Array or Struct
class DTPrivateStructured : public DTPrivateBase {
public:
    /// The types of the submembers
    DTypeDefVector mSubTypes;

    /// the size of the array. 0 otherwise.
    size_t mArraySize;

    /// Specifies whether the struct type is 'union' or 'struct'
    bool mUnioned;

    /// Array type constructor. Constructs an array type
    DTPrivateStructured(const DTypeDefPtr &type, size_t size)
        : DTPrivateBase(NT_ARRAY), mSubTypes(), mArraySize(size),
          mUnioned(false) {
        mSubTypes.push_back(type);
    }

    /// Struct/Union constructor
    DTPrivateStructured(const DTypeDefVector &types, bool unioned)
        : DTPrivateBase(NT_STRUCT), mSubTypes(types), mArraySize(0),
          mUnioned(unioned) {}

    /// Destructor
    virtual ~DTPrivateStructured() { mSubTypes.clear(); }

    /// Calculates the total size of this structured private type
    virtual int size() {
        if (mArraySize == 0) { // Struct/union
            // For struct, sum all the members. For Union, compute maximum.
            size_t maxsz = 0, sumsz = 0;

            unsigned int Size = mSubTypes.size();
            for (unsigned int I = 0; I < Size; I++) {
                size_t elemsz = mSubTypes[I]->size();

                if (mUnioned) {
                    if (maxsz < elemsz)
                        maxsz = elemsz;
                } else
                    sumsz += elemsz;
            }

            if (mUnioned)
                return maxsz;

            return sumsz;

        } else { // Array
            assert(mSubTypes.size() > 0);

            return mSubTypes[0]->size() * mArraySize;
        }
    }
};

//------------------------------------
DTypeDef::DTypeDef(const std::string &name, const DTypeDef &src) : mDefVal(0) {
    mPriv = src.mPriv;

    mDefVal = src.mDefVal;
    mTypeName = name;
    mDefaultUsed = src.mDefaultUsed;
    mPriv = src.mPriv;

    // Build the field map from the original
    if (!src.isField()) {
        mFields = src.getFields();
        _makeFieldMap();
    }
}

//------------------------------------
// Field constructor
DTypeDef::DTypeDef(const std::string &name, const DVariant &templ, size_t size,
                   const DEnumPtr &_enum)
    : mTypeName(name), mDefVal(templ) {
    // Check for constraints

    // zero size is meaningles
    if (size == 0)
        OPDE_EXCEPT("Zero sized field cannot be constructed",
                    "DTypeDef::DTypeDef");

    // Float with non-4 byte length
    if (templ.type() == DVariant::DV_FLOAT && size != 4)
        OPDE_EXCEPT("Only 4-byte floats are supported", "DTypeDef::DTypeDef");

    // Vector with size != 12
    if (templ.type() == DVariant::DV_VECTOR && size != 12)
        OPDE_EXCEPT("Only 12-byte float vectors are supported",
                    "DTypeDef::DTypeDef");

    // Short Vector with size != 6 (converted to quaternion on usage)
    if (templ.type() == DVariant::DV_QUATERNION && size != 6)
        OPDE_EXCEPT("Only 6-byte short vectors are supported",
                    "DTypeDef::DTypeDef");

    // Bitfield controled field which is not uint

    if (_enum && _enum->isBitfield() && templ.type() != DVariant::DV_UINT)
        OPDE_EXCEPT("Only uint can be used for bitfield controled fields",
                    "DTypeDef::DTypeDef");

    // Size <=0 and not string
    if (templ.type() != DVariant::DV_STRING && size < 0)
        OPDE_EXCEPT("Dynamic size is only supported for strings",
                    "DTypeDef::DTypeDef");

    mTypeName = name;
    mDefVal = templ;
    mDefaultUsed = true;

    mPriv = DTPrivateBasePtr(new DTPrivateSimple(templ.type(), size, _enum));
}

//------------------------------------
// Field constructor, no default value
DTypeDef::DTypeDef(const std::string &name, const DVariant::Type type,
                   size_t size, const DEnumPtr &_enum)
    : mTypeName(name), mDefVal() {
    // Check for constraints

    // zero size is meaningles
    if (size == 0)
        OPDE_EXCEPT("Zero sized field cannot be constructed",
                    "DTypeDef::DTypeDef");

    // Float with non-4 byte length
    if (type == DVariant::DV_FLOAT && size != 4)
        OPDE_EXCEPT("Only 4-byte floats are supported", "DTypeDef::DTypeDef");

    // Vector with size != 12
    if (type == DVariant::DV_VECTOR && size != 12)
        OPDE_EXCEPT("Only 12-byte float vectors are supported",
                    "DTypeDef::DTypeDef");

    // Short Vector with size != 6 (converted to quaternion on usage)
    if (type == DVariant::DV_QUATERNION && size != 6)
        OPDE_EXCEPT("Only 6-byte short vectors are supported",
                    "DTypeDef::DTypeDef");

    // Bitfield controled field which is not uint

    if (_enum && _enum->isBitfield() && type != DVariant::DV_UINT)
        OPDE_EXCEPT("Only uint can be used for bitfield controled fields",
                    "DTypeDef::DTypeDef");

    // Size <=0 and not string
    if (type != DVariant::DV_STRING && size < 0)
        OPDE_EXCEPT("Dynamic size is only supported for strings",
                    "DTypeDef::DTypeDef");

    mTypeName = name;

    mDefaultUsed = false;

    mPriv = DTPrivateBasePtr(new DTPrivateSimple(type, size, _enum));
}

//------------------------------------
// Array constructor
DTypeDef::DTypeDef(const DTypeDefPtr &member, size_t size)
    : mTypeName(member->name()), mDefVal() {
    // Check the member for being a dynamic length string (prohibited)
    if (member->size() <= 0)
        OPDE_EXCEPT("Dynamic sized type is not supported as a field",
                    "DTypeDef::DTypeDef");

    if (size <= 0)
        OPDE_EXCEPT("Array of size less than 1 element", "DTypeDef::DTypeDef");

    size_t ms = member->size();     // member size
    string basen = mTypeName + '['; // base name

    // Now build the field cache.
    // Not as simple, as we have to insert all fields of the member inside
    for (unsigned int i = 0; i < size; i++) {
        if (!member->isField()) {
            const Fields &f = member->getFields();
            Fields::const_iterator it = f.begin();

            for (; it != f.end(); it++) {
                // add the field to the list
                const FieldDef &orig = *it;

                FieldDef field;
                field.offset = i * ms + orig.offset;

                std::stringstream sst;

                sst << basen << i << "]." << orig.name;
                sst >> field.name;

                field.type = orig.type;

                mFields.push_back(field);
            }
        } else {
            FieldDef field;
            field.offset = i * ms;

            std::stringstream sst;

            sst << basen << i << ']';
            sst >> field.name;

            field.type = member;

            mFields.push_back(field);
        }
    }

    _makeFieldMap();

    mDefaultUsed = false;

    mPriv = DTPrivateBasePtr(new DTPrivateStructured(member, size));
}

//------------------------------------
// Struct/Union constructor
DTypeDef::DTypeDef(const std::string &name, DTypeDefVector members,
                   bool unioned)
    : mDefVal() {
    // Check all the members, if any of those is variable length, throw an
    // exception
    DTypeDefVector::const_iterator mit = members.begin();

    mTypeName = name;

    // Refresh the field cache
    size_t offs = 0;           // cummulative offset
    string basen = name + '.'; // base name

    // Back again
    mit = members.begin();

    // Now build the field cache.
    // What is done here is that all the members of the struct are unfolded -
    // theire fields are inserted into the field vector of this struct
    for (; mit != members.end(); mit++) {

        DTypeDefPtr member = *mit;

        // if the member is a container
        if (!member->isField()) {
            const Fields &f = member->getFields();
            Fields::const_iterator it = f.begin();

            // Not a field, a container. Insert all the original fields
            for (; it != f.end(); it++) {
                // add the field to the list
                const FieldDef &orig = *it;

                FieldDef field;
                field.offset = offs + orig.offset;

                std::stringstream sst;

                if (member->isArray()) {
                    sst << orig.name; // array does not get the dotted notation
                                      // (would cause member )
                } else {
                    sst << member->mTypeName << '.'
                        << orig.name; // To let aliases work. User the mTypeName
                }
                sst >> field.name;

                field.type = orig.type;

                mFields.push_back(field);
            }
        } else {
            // create a standard field info
            FieldDef field;
            field.offset = offs;

            field.name = member->mTypeName;

            field.type = member;

            mFields.push_back(field);
        }

        // if not unioned, increase the offset
        if (!unioned)
            offs += member->size(); // increase the offset
    }

    _makeFieldMap();

    mDefaultUsed = false;

    mPriv = DTPrivateBasePtr(new DTPrivateStructured(members, unioned));
}

//------------------------------------
DTypeDef::~DTypeDef() {
    mFields.clear();
    mPriv = DTPrivateBasePtr();
}

//------------------------------------
bool DTypeDef::isField() const {
    return (mPriv->mNodeType == DTPrivateBase::NT_SIMPLE);
}

//------------------------------------
bool DTypeDef::isArray() const {
    return (mPriv->mNodeType == DTPrivateBase::NT_ARRAY);
}

//------------------------------------
bool DTypeDef::isStruct() const {
    return (mPriv->mNodeType == DTPrivateBase::NT_STRUCT);
}

//------------------------------------
bool DTypeDef::isEnumerated() const {
    if (isField()) {
        return mPriv->isEnumerated();
    } else
        return false;
}

//------------------------------------
void DTypeDef::toDefaults(char *data) {
    // could've used memset but some say C++ concepts should not be mixed with C
    // ones...
    for (unsigned int x = 0; x < size(); ++x)
        data[x] = '\0';

    if (isField()) {
        if (mDefaultUsed)
            _set(data, mDefVal);
    } else {
        // iterate the fields. If the field has a default value, fill
        Fields::iterator it = mFields.begin();

        for (; it != mFields.end(); ++it) {
            if (it->type->hasDefault())
                it->set(data, it->type->getDefault());
        }
    }
}

//------------------------------------
size_t DTypeDef::size() { return mPriv->size(); }

//------------------------------------
const DEnumPtr DTypeDef::getEnum() { return mPriv->getEnum(); }

//------------------------------------
void DTypeDef::setDefault(const DVariant &val) {
    if (!isField())
        OPDE_EXCEPT("Default value can only be set on field types",
                    "DTypeDef::setDefault");

    mDefVal = val;
    mDefaultUsed = true;
}

//------------------------------------
DVariant::Type DTypeDef::getDataType() {
    if (!isField())
        return DVariant::DV_INVALID;

    return mPriv->type();
}

//------------------------------------
DVariant DTypeDef::get(char *dataptr, const std::string &field) {
    if (isField()) //  && field=="" - I do not waste time to check, I choose to
                   //  believe!
        return _get(dataptr);

    FieldDef &fld = getByName(field);

    if (!fld.type->isField())
        OPDE_EXCEPT("Field get resulted in non-field _get", "DTypeDef::get");

    // offset the data and call the getter
    return fld.type->_get(((char *)dataptr) + fld.offset);
}

//------------------------------------
void DTypeDef::set(char *dataptr, const std::string &field,
                   const DVariant &nval) {
    if (isField()) { //  && field=="" - I do not waste time to check, I choose
                     //  to believe!
        _set(dataptr, nval);
        return;
    }

    FieldDef &fld = getByName(field);

    if (!fld.type->isField())
        OPDE_EXCEPT("Field set resulted in non-field _set", "DTypeDef::set");

    // offset the data and call the getter
    fld.type->_set(((char *)dataptr) + fld.offset, nval);
}

//------------------------------------
DVariant DTypeDef::_get(char *ptr) {
    // Based on the type of the data and size, construct the Variant and return
    // TODO: Big/Little endian fixes

    if (mPriv->type() == DVariant::DV_BOOL) {
        bool bl;

        switch (mPriv->size()) {
        case 1:
            bl = readLE<uint8_t>(ptr) != 0;
            break;
        case 2:
            bl = readLE<uint16_t>(ptr) != 0;
            break;
        case 4:
            bl = readLE<uint32_t>(ptr) != 0;
            break;
        default:
            OPDE_EXCEPT("Unsupported bool size", "DTypeDef::_get");
        }
        return DVariant(bl);

    } else if (mPriv->type() == DVariant::DV_FLOAT) {
        switch (mPriv->size()) {
        case 4:
            return DVariant(readLE<float>(ptr));
        case 8:
            return DVariant((float)readLE<double>(ptr));
        default:
            OPDE_EXCEPT("Unsupported float size", "DTypeDef::_get");
        }

    } else if (mPriv->type() == DVariant::DV_INT) {
        switch (mPriv->size()) {
        case 1:
            return DVariant(readLE<int8_t>(ptr));
        case 2:
            return DVariant(readLE<int16_t>(ptr));
        case 4:
            return DVariant(readLE<int32_t>(ptr));
        default:
            OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
        }

    } else if (mPriv->type() == DVariant::DV_UINT) {
        switch (mPriv->size()) {
        case 1:
            return DVariant(readLE<uint8_t>(ptr));
        case 2:
            return DVariant(readLE<uint16_t>(ptr));
        case 4:
            return DVariant(readLE<uint32_t>(ptr));
        default:
            OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
        }

    } else if (mPriv->type() == DVariant::DV_STRING) {
        // compose a string out of the buffer (max len is mSize, can be less)
        int len = strlen(reinterpret_cast<char *>(ptr));
        return DVariant(reinterpret_cast<char *>(ptr),
                        std::min(mPriv->size(), len));
    } else if (mPriv->type() == DVariant::DV_VECTOR) {
        float x, y, z;
        float *fptr = (float *)ptr;

        x = readLE<float>(fptr); // TODO: Compile-time float size check (has to
                                 // be 4 to let this work)
        y = readLE<float>(fptr + 1);
        z = readLE<float>(fptr + 2);

        return DVariant(Vector3(x, y, z));
    } else if (mPriv->type() == DVariant::DV_QUATERNION) {
        // 3 short ints, converted to quaternion
        // This seems kinda time consuming, so it should be probably cached
        // Also, this seems to still have some conversion problems
        // And lastly: One loses precision when converting to and from 16 bit
        // short vector! Maybe we should support some kind of additional storage
        Ogre::Real x, y, z;

        /* Order of HPB application is crucial. It was detected that Dark orders
           HPB in this order:
           1. Bank is applied
           2. Pitch is applied
           3. Heading is applied
        */
        int16_t *rot = reinterpret_cast<int16_t *>(ptr);

        x = ((float)(readLE<int16_t>(rot)) / 32768) * Math::PI; // heading - y
        y = ((float)(readLE<int16_t>(rot + 1)) / 32768) * Math::PI; // pitch - x
        z = ((float)(readLE<int16_t>(rot + 2)) / 32768) * Math::PI; // bank - z

        Matrix3 m;

        // Still not there, but close
        m.FromEulerAnglesZYX(Radian(z), Radian(y), Radian(x));
        Quaternion q;
        q.FromRotationMatrix(m);

        return DVariant(q);
    } else {
        OPDE_EXCEPT("Unsupported type", "DTypeDef::_get");
    }
}

//------------------------------------
void DTypeDef::_set(char *ptr, const DVariant &val) {
    // TODO: Big/Little endian fixes

    if (mPriv->type() == DVariant::DV_BOOL) {
        switch (mPriv->size()) {
        case 1:
            // *reinterpret_cast<uint8_t*>(ptr) = val.toBool();
            writeLE<uint8_t>(ptr, val.toBool());
            return;

        case 2:
            writeLE<uint16_t>(ptr, val.toBool());
            return;

        case 4:
            writeLE<uint32_t>(ptr, val.toBool());
            return;

        default:
            OPDE_EXCEPT("Unsupported bool size", "DTypeDef::_set");
        }

    } else if (mPriv->type() == DVariant::DV_FLOAT) {
        switch (mPriv->size()) {
        case 4:
            writeLE<float>(ptr, val.toFloat());
            return;

        case 8:
            writeLE<double>(ptr, val.toFloat());
            return;

        default:
            OPDE_EXCEPT("Unsupported float size", "DTypeDef::_set");
        }

    } else if (mPriv->type() == DVariant::DV_INT) {
        switch (mPriv->size()) {
        case 1:
            writeLE<int8_t>(ptr, val.toInt());
            return;
        case 2:
            writeLE<int16_t>(ptr, val.toInt());
            return;

        case 4:
            writeLE<int32_t>(ptr, val.toInt());
            return;

        default:
            OPDE_EXCEPT("Unsupported int size", "DTypeDef::_set");
        }

    } else if (mPriv->type() == DVariant::DV_UINT) {
        switch (mPriv->size()) {
        case 1:
            writeLE<uint8_t>(ptr, val.toUInt());
            return;

        case 2:
            writeLE<uint16_t>(ptr, val.toUInt());
            return;

        case 4:
            writeLE<uint32_t>(ptr, val.toUInt());
            return;

        default:
            OPDE_EXCEPT("Unsupported int size", "DTypeDef::_set");
        }

    } else if (mPriv->type() == DVariant::DV_STRING) {
        char *cptr = reinterpret_cast<char *>(ptr);

        const std::string &s = val.toString();

        int len = s.length();

        // Copy the string to the buffer. Limit by 1 to have a trailing zero at
        // end
        s.copy(reinterpret_cast<char *>(ptr), mPriv->size() - 1);

        // Terminating zero.
        cptr[min(mPriv->size() - 1, len)] = '\0';

        return;

    } else if (mPriv->type() == DVariant::DV_VECTOR) {
        float *fptr = reinterpret_cast<float *>(ptr);

        Vector3 v = val.toVector();

        writeLE<float>(fptr, v.x);
        writeLE<float>(fptr + 1, v.y);
        writeLE<float>(fptr + 2, v.z);

        return;
    } else if (mPriv->type() == DVariant::DV_QUATERNION) {
        int16_t *vptr = reinterpret_cast<int16_t *>(ptr);

        Quaternion q = val.toQuaternion();

        Matrix3 m;

        q.ToRotationMatrix(m);

        Radian x, y, z;

        // Still not there, but close
        m.ToEulerAnglesZYX(z, y, x);

        writeLE<int16_t>(
            vptr, static_cast<int16_t>(x.valueRadians() * 32768 / Math::PI));
        writeLE<int16_t>(vptr + 1, static_cast<int16_t>(y.valueRadians() *
                                                        32768 / Math::PI));
        writeLE<int16_t>(vptr + 2, static_cast<int16_t>(z.valueRadians() *
                                                        32768 / Math::PI));

        return;
    } else {
        OPDE_EXCEPT("Unsupported _set type", "DTypeDef::_set");
    }
}

//------------------------------------
DTypeDef::FieldDef &DTypeDef::getByName(const std::string &name) {
    // Search for field in mFieldMap
    FieldMap::iterator f = mFieldMap.find(name);

    if (f == mFieldMap.end())
        OPDE_EXCEPT(string("Field not found : ") + name, "DTypeDef::getByName");

    return f->second;
}

//------------------------------------
void DTypeDef::_makeFieldMap() {
    Fields::const_iterator it = mFields.begin();

    for (; it != mFields.end(); it++) {
        const FieldDef &fd = *it;

        std::pair<FieldMap::iterator, bool> result =
            mFieldMap.insert(make_pair(fd.name, fd));

        if (!result.second)
            OPDE_EXCEPT(string("Attempt to insert a non-unique member name "
                               "(already present) : ") +
                            fd.name,
                        "DTypeDef::_makeFieldMap");
    }
}

//------------------------------------
DataFieldDescListIterator::DataFieldDescListIterator(
    const DataFieldDescList &src)
    : mList(src) {
    mIt = mList.begin();
}

//------------------------------------
const DataFieldDesc &DataFieldDescListIterator::next() { return *(mIt++); }

//------------------------------------
bool DataFieldDescListIterator::end() const { return mIt == mList.end(); }

//------------------------------------
EmptyDataFieldDescListIterator::EmptyDataFieldDescListIterator()
    : DataFieldDescListIterator(mEmptyList) {}

//------------------------------------
const DataFieldDesc &EmptyDataFieldDescListIterator::next() {
    OPDE_EXCEPT("Tried to get an item out of an empty list!",
                "DataFieldDescListIterator::next");
}

//------------------------------------
bool EmptyDataFieldDescListIterator::end() const { return true; }

} // namespace Opde
