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

#ifndef __SINGLEDATASTORAGE_H
#define __SINGLEDATASTORAGE_H

#include "config.h"

#include "DataStorage.h"

#include "Variant.h"
#include "File.h"
#include "Iterator.h"
#include "Serializer.h"

namespace Opde {

/** Common ancestor template for all the single value data storages (with fixed
 * length data) */
template <typename T, typename SerT = TypeSerializer<T>>
class SingleFieldDataStorage : public DataStorage {
public:
    explicit SingleFieldDataStorage(Enumeration *enm) : mFieldDesc(), mSerializer() {
        DataFieldDesc fieldDesc;
        fieldDesc.enumerator = enm;
        fieldDesc.name = "";
        fieldDesc.size = sizeof(T);
        fieldDesc.type = VariantTypeTraits<T>::type;
        mFieldDesc.push_back(fieldDesc);
    };

    virtual ~SingleFieldDataStorage() { }

    /** @see DataStorage::create */
    virtual bool create(int objID) {
        // call _create to handle the creation, with the default value
        return _create(objID, T());
    }

    /** @see DataStorage::destroy */
    virtual bool destroy(int objID) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            mDataMap.erase(it);

            // true, erase went ok
            return true;
        }

        return false;
    }

    /** @see DataStorage::has */
    virtual bool has(int objID) {
        typename DataMap::iterator it = mDataMap.find(objID);

        return (it != mDataMap.end());
    }

    /** @see DataStorage::clone */
    virtual bool clone(int srcID, int dstID) {
        typename DataMap::iterator it = mDataMap.find(srcID);

        if (it != mDataMap.end()) {
            return _create(dstID, it->second);
        }

        return false;
    }

    /** @see DataStorage::getField */
    virtual bool getField(int objID, const std::string &field,
                          Variant &target) {
        assert(field == "");

        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            target = it->second;

            return true;
        }

        return false;
    }

    /** @see DataStorage::setField */
    virtual bool setField(int objID, const std::string &field,
                          const Variant &value) {
        assert(field == "");

        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            it->second = fromVariant(value);

            return true;
        }

        return false;
    }

    /** @see DataStorage::writeToFile */
    virtual bool writeToFile(FilePtr &file, int objID, bool sizeStored) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            const T &dta = it->second;

            uint32_t size = mSerializer.getStoredSize(&dta);

            // Write the size if requested
            if (sizeStored)
                file->writeElem(&size, sizeof(uint32_t));

            // write the data itself
            mSerializer.serialize(file, &dta);

            return true;
        }

        return false;
    }

    /** @see DataStorage::readFromFile */
    virtual bool readFromFile(FilePtr &file, int objID, bool sizeStored) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it == mDataMap.end()) {
            uint32_t size = sizeof(T);

            if (sizeStored)
                file->readElem(&size, sizeof(uint32_t));

            // assert(size == sizeof(T));

            T dta;

            mSerializer.deserialize(file, &dta);

            _create(objID, dta);

            return true;
        } else {
            // skip the data...
            uint32_t size;

            file->readElem(&size, sizeof(uint32_t));
            file->seek(size, File::FSEEK_CUR);
        }

        return false;
    }

    /** @see DataStorage::clear */
    virtual void clear() { mDataMap.clear(); }

    /** @see DataStorage::isEmpty */
    virtual bool isEmpty() { return mDataMap.empty(); }

    /** @see DataStorage::getAllStoredObjects */
    virtual IntIteratorPtr getAllStoredObjects() {
        return IntIteratorPtr(new DataMapKeyIterator(mDataMap));
    }

    /** Core Data creation routine */
    virtual bool _create(int objID, const T &val) {
        std::pair<typename DataMap::iterator, bool> res =
            mDataMap.insert(std::make_pair(objID, val));

        return res.second;
    };

    /** @see DataStorage::getFieldDescIterator */
    const DataFields &getFieldDesc(void) override {
        return mFieldDesc;
    }

    /** @see DataStorage::getDataSize */
    virtual size_t getDataSize(void) { return sizeof(T); }

protected:
    explicit SingleFieldDataStorage(const DataFieldDesc &fieldDesc)
        : mFieldDesc(1, fieldDesc), mSerializer()
    {}

    /// Converts from variant to the internal storage type. Should be overrided,
    /// or reimplemented by explicit specialization
    virtual T fromVariant(const Variant &v) const { return v.as<T>(); }

    /// Data map
    typedef typename std::map<int, T> DataMap;

    /// Holder of data values
    DataMap mDataMap;

    /// Bool prop. storage is a single-field construct. This is the field desc
    /// for the field
    DataFields mFieldDesc;

    typedef MapKeyIterator<DataMap, int> DataMapKeyIterator;

    SerT mSerializer;
};

// Various simple single-fielded data storages (it could be done more easily
// through some data definition structures):

/// Boolean (4 byte) data storage
typedef SingleFieldDataStorage<bool> BoolDataStorage;

/// Float (4 byte) data storage
typedef SingleFieldDataStorage<float> FloatDataStorage;

/// int (4 byte) data storage
typedef SingleFieldDataStorage<int32_t> IntDataStorage;

/// unsigned int (4 byte) data storage
typedef SingleFieldDataStorage<uint32_t> UIntDataStorage;

/// Vector3 data storage
typedef SingleFieldDataStorage<Vector3> Vector3DataStorage;

/// Variable length string data storage
class StringDataStorage : public SingleFieldDataStorage<std::string> {
public:
    StringDataStorage(Enumeration *enm = NULL)
        : SingleFieldDataStorage<std::string>(enm) {
        mFieldDesc[0].size = -1; // override needed
    }

    /** @see DataStorage::getDataSize */
    virtual size_t getDataSize(void) {
        OPDE_EXCEPT("Invalid call - string length is variable");
    }
};

/// Fixed size string data storage template.
template <int Len>
class FixedStringDataStorage
    : public SingleFieldDataStorage<std::string, FixedStringSerializer<Len>> {
    using Parent =
        SingleFieldDataStorage<std::string, FixedStringSerializer<Len>>;

public:
    FixedStringDataStorage(Enumeration *enm = NULL)
        : Parent(DataFieldDesc{"", "", Len,
                               VariantTypeTraits<std::string>::type}) {}
};

} // namespace Opde

#endif // __PROPERTYSTORAGE_H
