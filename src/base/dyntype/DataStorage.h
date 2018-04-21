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

#ifndef __DATASTORAGE_H
#define __DATASTORAGE_H

#include "config.h"

#include "Variant.h"
#include "File.h"
#include "Iterator.h"

namespace Opde {

/** Type aware variant based enumeration definition. Used for enumeration and
   bitfields.
        @note For bitfields, the base type must be DV_UINT
        @note No check on multiple keys with the same values is made */
class Enumeration {
private:
    typedef std::map<std::string, Variant> StrValMap;

    StrValMap mValMap;

    Variant::Type mEnumType;

    bool mBitField;

    std::string mName;

public:
    Enumeration(const std::string &name, Variant::Type enumType, bool bitfield);
    ~Enumeration() = default;

    /** Insert a field definition inside the Enum
     * @note It is not adviced to modify enumerations on other times than
     * creation as this could confuse other pieces of code */
    void insert(const std::string &key, const Variant &value);

    /** get the symbol text by the enumeration value */
    const std::string &symbol(const Variant &val) const;

    /** get the value by the symbol text */
    const Variant &value(const std::string &symbol) const;

    /** Enumeration field. A single field that is either checked or not, and has
     * a specific value */
    struct EnumField {
        EnumField(std::string _k, const Variant &_v, bool _c)
            : checked(_c), key(_k), value(_v){};

        /** True if the field is enabled or the value is selected in case of
         * non-bitfield enumerations */
        bool checked;
        /** The field name (key) */
        std::string key;
        /** value of the field */
        Variant value;
    };

    /** Enumeration of the fields. */
    typedef std::vector<EnumField> EnumFieldList;

    /** Gets the field list for the enumeration given the field value
     * @note For bitfields, multiple can be checked==true, for Enumeration, one
     * will (if there is only a single key for any value) */
    EnumFieldList getFieldList(const Variant &val) const;

    /** Returns true if the Enumeration is a bitfield */
    inline bool isBitfield() const { return mBitField; };

    /** Returns the name of this enumeration */
    const std::string &getName(void) { return mName; };
};

/// A unified unstructured field list description structure
struct DataFieldDesc {
    DataFieldDesc() {};
    DataFieldDesc(const std::string &name, const std::string &label, int size,
                  Variant::Type type, Enumeration *enumerator = nullptr)
        : name(name), label(label), size(size), type(type),
          enumerator(enumerator) {}

    /// Name of the field
    std::string name;
    /// Label of the field - readable
    std::string label;
    /// Size of the field. -1 for variable-sized fields (strings)
    int size;
    /// Type of the field
    Variant::Type type;
    /// Enumerator of the allowed values (either enum or bitfield type). NULL
    /// means this field is not enumerated
    Enumeration *enumerator;
};

typedef std::vector<DataFieldDesc> DataFields;

/** @brief Storage for data (Interface). This class is used as a backend for
 * either property or link storage, and provides data these classes. This
 * storage can be overriden to suit better for particular data handling to be
 * managed in effective/different manner for example.
 * @note This class does no inheritance resolving itself (in case of
 * properties). Only stores data.
 * @note The objID, object id referenced here has the meaning of a ID of any
 * kind appropriate - game object id for Properties, Link id for links.
 */
class DataStorage {
public:
    virtual ~DataStorage(){};

    /** Creates a default-value data for object numbered objID
     * @param objID The object ID
     * @return true if creation wen't ok, false if something went wrong (object
     * already has the data attached)
     */
    virtual bool create(int objID) = 0;

    /** Creates a default-value data with value overrides
     * @param objID the object id
     * @param dataValues map of key name -> value to set upon creation
     * @see create
     * @note for simplicity, invalid key names are ignored
     * @return true if creation wen't ok
     *
     */
    virtual bool createWithValues(int objID,
                                  const VariantStringMap &dataValues);

    /** Creates a single-value data with specified value
     * @param objID the object id
     * @param values Variant value of the created record
     * @see create
     * @return true if creation wen't ok
     *
     */
    virtual bool createWithValue(int objID, const Variant &values);

    /** Destroys data on a defined object
     * @param objID the object to destroy the data for
     * @return true if data was destroyed, false if something went wrong (data
     * was not assigned to the object)
     */
    virtual bool destroy(int objID) = 0;

    /** Data ownership detector. Returns true if this data storage holds data
     * for the given object ID
     */
    virtual bool has(int objID) = 0;

    /** Clones the data to another object ID (copies all values)
     * @note The target data has to not exit (this routine does not overwrite)
     * @param srcID the source object ID
     * @param dstID the destination ID
     * @return true if all went ok, false otherwise (srcID invalid, dstID
     * invalid...)
     */
    virtual bool clone(int srcID, int dstID) = 0;

    /** Field value getter
     * @param objID The object id to get data value for
     * @param field The field to get value for
     * @param target The target variant to fill with value
     * @return true if value was set to target, false if the field was invalid
     */
    virtual bool getField(int objID, const std::string &field,
                          Variant &target) = 0;

    /** Field value setter
     * @param objID The object id to set data value for
     * @param field The field to set value for
     * @param value The new value to use
     * @return true if value was set to target, false if the field was invalid
     */
    virtual bool setField(int objID, const std::string &field,
                          const Variant &value) = 0;

    /** Serialization core routine
     * This routine is used to serialize the data for given object ID into a
     *File handle. Normally, this routine should write data for the stored data
     *in this format:
     * @code
     *	32-bit unnsigned size (N) - only expected/written if sizeStored is true
     *	N bytes of data data for the given property
     * @endcode
     *
     * @param file The file to store the data into
     * @param objID The id of the object data to write
     * @param sizeStored if true, the size is stored with the data
     */
    virtual bool writeToFile(FilePtr &file, int objID, bool sizeStored) = 0;

    /** Deserialization core routine
     * This routine is used to read property data for a given object id from a
     *given File handle. Normally, this format format is used:
     * @code
     *	32-bit unnsigned size (N) - only expected/written if sizeStored is true
     *	N bytes of Property data for the given property
     * @endcode
     * @note This routine automatically creates property data slot for the
     *objID, and does no load over any previous data (so clear before load is
     *encouraged)
     *
     * @param file The file to read the data from
     * @param objID The id of the object data to read
     * @param sizeStored if true, the size is stored with the data, and is read
     *before the data. Otherwise the size of the date this storage uses is used
     */
    virtual bool readFromFile(FilePtr &file, int objID, bool sizeStored) = 0;

    /** Called to clear all data */
    virtual void clear() = 0;

    /** Emptyness of data storage detector
     * @return true If the data storage does not hold any data, false if does */
    virtual bool isEmpty() = 0;

    /** Getter for all stored object ID's
     */
    virtual IntIteratorPtr getAllStoredObjects() = 0;

    /** Optional handler for object ID range re-sets (f.e. when growing the
     * concretes)
     */
    virtual void grow(int minID, int MaxID){/**/};

    /** Data description retrieval routine. Can be used to generate GUI for data
     * editor, etc.
     * @return The data fields description iterator, preferably in the order
     * sored
     */
    virtual const DataFields &getFieldDesc(void) = 0;

    /** Data size getter
     * @return Data size, if available. 0 otherwise */
    virtual size_t getDataSize(void) { return 0; };
};

/// Shared pointer to data storage
typedef shared_ptr<DataStorage> DataStoragePtr;

} // namespace Opde

#endif // __PROPERTYSTORAGE_H
