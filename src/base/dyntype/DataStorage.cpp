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
 *
 *      $Id$
 *
 *****************************************************************************/

#include "DataStorage.h"
#include "logger.h"

using namespace std;

namespace Opde {

/*------------------------------------------------------*/
/*------------------- Enumeration ----------------------*/
/*------------------------------------------------------*/
Enumeration::Enumeration(const std::string &name, Variant::Type enumType,
                         bool bitfield)
    : mValMap(), mName(name)
{
    mEnumType = enumType;
    mBitField = bitfield;

    if (mBitField && mEnumType != Variant::DV_UINT)
        OPDE_EXCEPT("Only uint is supported for bitfields",
                    "Enumeration::Enumeration");
}

//------------------------------------
void Enumeration::insert(const std::string &key, const Variant &value)
{
    // Type check
    if (mEnumType != value.type())
        throw(runtime_error("Type violation of the enumeration type"));

    mValMap.insert(make_pair(key, value));
}

//------------------------------------
const string &Enumeration::symbol(const Variant &val) const
{
    if (mEnumType != val.type())
        throw(runtime_error("Type violation of the enumeration type"));

    StrValMap::const_iterator it = mValMap.begin();

    for (; it != mValMap.end(); it++) {
        if (it->second == val)
            return it->first;
    }

    throw(out_of_range("Enumeration::symbol"));
}

//------------------------------------
const Variant &Enumeration::value(const std::string &symbol) const
{
    StrValMap::const_iterator it = mValMap.find(symbol);

    if (it == mValMap.end())
        return it->second;

    throw(out_of_range("Enumeration::value"));
}

//------------------------------------
Enumeration::EnumFieldList Enumeration::getFieldList(const Variant &val) const
{
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

// --------------------------------------------------------------------------
// ---------------------------- Data Storage --------------------------------
// --------------------------------------------------------------------------
bool DataStorage::createWithValues(int objID,
                                   const VariantStringMap &dataValues)
{
    if (!create(objID))
        return false;

    VariantStringMap::const_iterator end = dataValues.end();
    VariantStringMap::const_iterator it = dataValues.begin();

    for (; it != end; ++it) {
        setField(objID, it->first, it->second);
    }

    return true;
}

// --------------------------------------------------------------------------
bool DataStorage::createWithValue(int objID, const Variant &value)
{
    if (!create(objID))
        return false;

    return setField(objID, "", value);
}

} // namespace Opde
