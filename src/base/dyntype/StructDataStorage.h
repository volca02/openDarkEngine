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

#ifndef __STRUCTDATASTORAGE_H
#define __STRUCTDATASTORAGE_H

#include <map>

#include "config.h"

#include "DataStorage.h"

#include "DVariant.h"
#include "File.h"
#include "Iterator.h"
#include "Serializer.h"

namespace Opde {

/** Common ancestor template for all the struct based data storages. Template
 * parameter is the struct name. To use this, one must expose all the fields of
 * the struct using the field method (which registers the field in all internal
 * structures)
 */
template <typename T>
class OPDELIB_EXPORT StructDataStorage : public DataStorage {
protected:
    // forward decl
    struct TypeHelperBase;

    typedef shared_ptr<TypeHelperBase> TypeHelperBasePtr;

    /** Field setter member pointer - called when setting a value on certain
     * field
     * @param field The field name
     * @param data The struct instance to set value for
     * @param value the new desired value
     * @return true if setting was successful, false otherwise
     */

    typedef bool (StructDataStorage::*FieldSetter)(
        const TypeHelperBasePtr &helper, T &data, const DVariant &value);
    /** Field getter member pointer - called when setting a value on certain
     * field
     * @param field The field name
     * @param data The struct instance to set value for
     * @param value the target DVariant instance ref to fill with value
     * @return true if getting was successful, false otherwise
     */
    typedef bool (StructDataStorage::*FieldGetter)(
        const TypeHelperBasePtr &helper, const T &data, DVariant &value);

    /// Type helper (for easy struct field manipulation)
    class TypeHelperBase : public NonCopyable {
    public:
        TypeHelperBase(FieldGetter _getter, FieldSetter _setter)
            : mGetter(_getter), mSetter(_setter){};

        virtual ~TypeHelperBase(){};

        /// sets the struct's field with the given value
        virtual void toField(T &data, const DVariant &val) = 0;

        /// sets the variant with the value from the struct's field
        virtual void fromField(const T &data, DVariant &val) = 0;

        /// serializer getter
        virtual Serializer *getSerializer() = 0;

        /// getter access method
        FieldGetter getter() { return mGetter; };

        /// setter access method
        FieldSetter setter() { return mSetter; };

        /// field of given struct mem. pointer getter
        virtual void *getFieldPtr(T &data) = 0;

        /// field of given struct const mem. pointer getter
        virtual const void *getFieldPtr(const T &data) const = 0;

    protected:
        FieldGetter mGetter;
        FieldSetter mSetter;
    };

    typedef std::map<std::string, TypeHelperBasePtr> TypeHelperMap;

    TypeHelperMap mTypeHelpers;

public:
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
                          DVariant &target) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            typename TypeHelperMap::iterator tit = mTypeHelpers.find(field);

            if (tit != mTypeHelpers.end()) {
                return (*this.*(tit->second->getter()))(tit->second, it->second,
                                                        target);
            }
        }

        return false;
    }

    /** @see DataStorage::setField */
    virtual bool setField(int objID, const std::string &field,
                          const DVariant &value) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {

            typename TypeHelperMap::iterator tit = mTypeHelpers.find(field);

            if (tit != mTypeHelpers.end()) {
                return (*this.*(tit->second->setter()))(tit->second, it->second,
                                                        value);
            }
        }

        return false;
    }

    /** @see DataStorage::writeToFile */
    virtual bool writeToFile(FilePtr &file, int objID, bool sizeStored) {
        typename DataMap::iterator it = mDataMap.find(objID);

        if (it != mDataMap.end()) {
            const T &dta = it->second;

            uint32_t size = mStoredSize;

            // Write the size if requested
            if (sizeStored)
                file->writeElem(&size, sizeof(uint32_t));

            const auto &fields = getFieldDesc();

            for(const auto &field : fields) {
                TypeHelperBasePtr thb = mTypeHelpers[field.name];
                thb->getSerializer()->serialize(file, thb->getFieldPtr(dta));
            }

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

            assert(size == mStoredSize);

            T dta;

            // iterate over the fields
            const auto &fields = getFieldDesc();
            for(const auto &field : fields) {
                TypeHelperBasePtr thb = mTypeHelpers[field.name];
                thb->getSerializer()->deserialize(file, thb->getFieldPtr(dta));
            }

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
    virtual const DataFields &getFieldDesc(void) {
        return mFieldDesc;
    }

    /** @see DataStorage::getDataSize */
    virtual size_t getDataSize(void) { return sizeof(T); }

protected:
    template <typename FT> class TypeHelper : public TypeHelperBase {
    public:
        typedef FT T::*StructField;

        TypeHelper(StructField fieldPtr, FieldGetter getter, FieldSetter setter)
            : TypeHelperBase(getter, setter), mField(fieldPtr) {
            mSerializer = new TypeSerializer<FT>();
        };

        virtual ~TypeHelper() { delete mSerializer; };

        virtual void toField(T &data, const DVariant &val) {
            data.*mField = val.as<FT>();
        }

        virtual void fromField(const T &data, DVariant &val) {
            val = DVariant(data.*mField);
        };

        virtual Serializer *getSerializer() { return mSerializer; }

        virtual void *getFieldPtr(T &data) { return &(data.*mField); };

        virtual const void *getFieldPtr(const T &data) const {
            return &(data.*mField);
        };

    protected:
        Serializer *mSerializer;

        StructField mField;
    };

    // --- default field handlers ---

    /// Default field setter. Used when no other specified
    bool defaultFieldSetter(const TypeHelperBasePtr &helper, T &data,
                            const DVariant &value) {
        helper->toField(data, value);
        return true;
    }

    /// Default field getter. Used when no other specified
    bool defaultFieldGetter(const TypeHelperBasePtr &helper, const T &data,
                            DVariant &value) {
        helper->fromField(data, value);
        return true;
    }

    template <typename FT>
    void field(const std::string &name, FT T::*fieldPtr, DEnum *enumer = NULL,
               FieldGetter getter = NULL, FieldSetter setter = NULL) {
        // insert into the field def array.
        DataFieldDesc fd;

        fd.name = name;
        fd.label = name;
        fd.size = sizeof(FT);
        fd.enumerator = enumer;
        fd.type = DVariantTypeTraits<FT>::type;

        mFieldDesc.push_back(fd);

        if (getter == NULL)
            getter = &StructDataStorage::defaultFieldGetter;

        if (setter == NULL)
            setter = &StructDataStorage::defaultFieldSetter;

        // data manipulation helper
        TypeHelperBasePtr thb(new TypeHelper<FT>(fieldPtr, getter, setter));
        mTypeHelpers[name] = thb;

        mStoredSize += thb->getSerializer()->getStoredSize(0);
    };

    explicit StructDataStorage() {
        mStoredSize = 0; // incremented in field method.
    };

    virtual ~StructDataStorage() {}

    /// Data map
    typedef typename std::map<int, T> DataMap;

    /// Holder of data values
    DataMap mDataMap;

    DataFields mFieldDesc;

    typedef MapKeyIterator<DataMap, int> DataMapKeyIterator;

    size_t mStoredSize;
};
} // namespace Opde

#endif
