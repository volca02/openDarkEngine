/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *		$Id$
 *
 *****************************************************************************/

#ifndef __DTYPEDEF_H
#define __DTYPEDEF_H

#include "config.h"

#include <stdexcept>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <sstream>

#include "vector3.h"
#include "integers.h"
#include "SharedPtr.h"
#include "DVariant.h"
#include "File.h"
#include "NonCopyable.h"
#include "Iterator.h"

namespace Opde {
	/** Type aware variant based enumeration definition. Used for enumeration and bitfields.
	@note For bitfields, the base type must be DV_UINT
	@note No check on multiple keys with the same values is made */
	class OPDELIB_EXPORT DEnum {
		private:
			typedef std::map<std::string, DVariant> StrValMap;

			StrValMap mValMap;

			DVariant::Type mEnumType;

			bool mBitField;

			std::string mName;

		public:
			DEnum(const std::string& name, DVariant::Type enumType, bool bitfield);

			~DEnum();

			/** Insert a field definition inside the Enum
			* @note It is not adviced to modify enumerations on other times than creation as this could confuse other pieces of code */
			void insert(const std::string& key, const DVariant& value);

			/** get the symbol text by the enumeration value */
			const std::string& symbol(const DVariant& val) const;

			/** get the value by the symbol text */
			const DVariant& value(const std::string& symbol) const;

			/** Enumeration field. A single field that is either checked or not, and has a specific value */
			struct EnumField {
				EnumField(std::string _k, const DVariant& _v, bool _c) : checked(_c), key(_k), value(_v) {};

				/** True if the field is enabled or the value is selected in case of non-bitfield enumerations */
				bool checked;
				/** The field name (key) */
				std::string key;
				/** value of the field */
				const DVariant& value;
			};

			/** Enumeration of the fields. */
			typedef std::list< EnumField > EnumFieldList;

			/** Gets the field list for the enumeration given the field value
			* @note For bitfields, multiple can be checked==true, for Enumeration, one will (if there is only a single key for any value) */
			EnumFieldList getFieldList(const DVariant& val) const;

			/** Returns true if the Enumeration is a bitfield */
			inline bool isBitfield() const { return mBitField; };

			/** Returns the name of this enumeration */
			const std::string& getName(void) { return mName; };
	};

	/// Shared pointer to DEnum
	typedef shared_ptr<DEnum> DEnumPtr;

	// Forward declaration
	class DTypeDef;

	typedef shared_ptr<DTypeDef> DTypeDefPtr;

	/** vector of type definitions. Used to fill in a struct type. */
	typedef std::vector<DTypeDefPtr> DTypeDefVector;

	/// Base definition of the DTypes dat storage
	class OPDELIB_EXPORT DTPrivateBase {
		public:
			/** DTypeDef private node type */
			enum NodeType {
				/// Simple value holder
				NT_SIMPLE,
				/// Array of elements
				NT_ARRAY,
				/// Struct/Union
				NT_STRUCT
			};

			/// Type of this node
			NodeType mNodeType;

			/// Destructor
			virtual ~DTPrivateBase() {
			}

			virtual bool isEnumerated() {
				return false;
			}

			virtual DEnumPtr getEnum() {
				return DEnumPtr(NULL);
			}

			virtual DVariant::Type type() {
				return DVariant::DV_INVALID;
			}

			/// Size getter
			virtual int size() = 0;

		protected:
			/// Constructor
			DTPrivateBase(NodeType type) : mNodeType(type) {};
	};

	typedef shared_ptr<DTPrivateBase> DTPrivateBasePtr;

	/** Reference counted dynamic type definition. Used to create a descriptive object, which can be used to access structures/arrays/unions in
	* memory.
	* The structure of DTypeDef can only be modified at creation time, that means that no structural changes are possible once the dyntype is constructed.
	*
	* The DTypeDef can be either:
	* 	- field type - e.g. a value holder
	* 	- a container - array/structure/union
	*
	*
	*/
	class OPDELIB_EXPORT DTypeDef : public NonCopyable {
		public:
			/// Construct as a copy of other DTypeDef, with different name
			DTypeDef(const std::string& name, const DTypeDef& src);

			/// Constucts a possibly enumerated (if _enum member is not NULL) type with default value and type taken from templ
			DTypeDef(const std::string& name, const DVariant& templ, size_t size, const DEnumPtr& _enum);

			/// Constucts a possibly enumerated (if _enum is not NULL) type with type taken from templ
			DTypeDef(const std::string& name, DVariant::Type type, size_t size, const DEnumPtr& _enum);

			/** Construct an array type, with the type defined by member, and default value of the fields defined by member, sized size
			* @note The base name is taken from the member
			*/
			DTypeDef(const DTypeDefPtr& member, size_t size);

			/// Construct a Structure with members defined by the list. If unioned is true, all the fields share the same start offset (0)
			DTypeDef(const std::string& name, DTypeDefVector members, bool unioned = false);

			/// Destructor. Releases all references
			~DTypeDef();

			/** Field type detection. Field type is that which would hold a single value
			* if not a field, the type is a container - array/structure/union */
			bool isField() const;

			/** Returns true if the instance is an Array */
			bool isArray() const;

			/** Returns true if the instance is a struct - multiple childs, each can have a different type */
			bool isStruct() const;

			/** Returns true if the type is enumerated */
			bool isEnumerated() const;

			/** Create an alias to this DTypeDef, assigning a new name. Functionally equal to DTypeDef(name, type_to_clone);
			* The purpose of this is to support struct field names and aliased structs
			* @param name New name for this typedef
			* @note Setting the default value must be done after aliasing, as this also aliases container types
			*/
			inline DTypeDefPtr alias(std::string name) {
				return DTypeDefPtr(new DTypeDef(name, *this));
			}

			/** Returns the size of this type definition */
			size_t size();

			/** Initializes the given data pointer (has to be allocated to the length of at least size()) */
			void toDefaults(char* data);

			/** Get the value of the field specified by name, reading the data from the dataptr mem. location
			* @note The field name has to exist, and has to be a simple field (Not array/union/struct field)
			* @note No check on the dataptr allocated size is done. This means the allocated size has to be at least the same as the size of this dtypedef */
			DVariant get(char* dataptr, const std::string& field);

			/** Sets the data of a certain field.
			* @param dataptr The pointer to the data to fill
			* @param field the field to set
			* @param nval the new value to use */
			void set(char* dataptr, const std::string& field, const DVariant& nval);

			/** Get the definition of a specific field from the built cache
			* @note The field name has to exist, and has to be a simple field (Not array/union/struct field) */
			DTypeDef getFieldDef(const std::string& field);

			/** Field only - get the enumeration for the field */
			const DEnumPtr getEnum();

			/// Gets the default value for this typedef
			inline const DVariant& getDefault() {
				return mDefVal;
			}

			/// Returns true if this field has any default value
			inline bool hasDefault() {
				return mDefaultUsed;
			}

			/// Sets the default value for this typedef
			void setDefault(const DVariant& val);

			/** Field definition. Used in the built version of the type definition */
			struct FieldDef {
				unsigned int	offset;
				std::string	name; // Absolute name
				DTypeDefPtr 	type;

				inline void set(char* dataptr, const DVariant& val) const {
					type->_set(reinterpret_cast<char*>(dataptr) + offset, val);
				}

				inline void setDefault(char* dataptr) const {
					type->_set(reinterpret_cast<char*>(dataptr) + offset, type->getDefault());
				}

				inline DVariant get(char* dataptr) const {
					return type->_get(reinterpret_cast<char*>(dataptr) + offset);
				}
			};

			/// Field list. Order is sequential (increasing offset), with one exception - unions
			typedef std::vector<FieldDef> Fields;

			/// const iterator for fields of this type definition
			typedef Fields::const_iterator const_iterator;

			/// The direct field access cache. This has to be refreshed manually after definition is complete
			typedef std::map<std::string, FieldDef> FieldMap;


			inline const_iterator begin() {
				return mFields.begin();
			}

			inline const_iterator end() {
				return mFields.end();
			}

			inline const std::string& name() {
				return mTypeName;
			}

			DVariant::Type getDataType();

		protected:
			/** Internal field getter */
			DVariant _get(char* ptr);

			/** Internal field setter
			* @param ptr Direct pointer to the start of the data to be set
			* @param val the value to set the field to
			*/
			void _set(char* ptr, const DVariant& val);

			/** find field def by it's name (or throw an error if not found) */
			FieldDef& getByName(const std::string& name);

			/// Field vector getter
			inline const Fields& getFields() const {
				return mFields;
			}

			/** Create the map of field name -> FieldDef */
			void _makeFieldMap();

			/// Name of this type
			std::string mTypeName;

			/// The real definition of the field(s)
			DTPrivateBasePtr mPriv;

			/// Default value. Only valid if the dtypedef is simple. This fills the fields of a newly created type
			DVariant mDefVal;

			/// Default value is used at all
			bool mDefaultUsed;

			Fields mFields;
			FieldMap mFieldMap;
	};

	/** DTypeDef 'variable'. Ugly and a bit outdated concept that should be got rid off soon.
	* @deprecated */
	class OPDELIB_EXPORT DType {
		public:
			/** Copy constructor. Copies the data and the type definition from another DType instance */
			DType(const DType& b, bool useCache = false) : mUseCache(useCache), mCache() {
				char* odata = mData;

				size_t sz = b.size();
				mData = new char[sz];

				// copy the data
				memcpy(mData, b.mData, sz);

				mType = b.mType;
				mCache = b.mCache;

				delete[] odata;
			}

			/** Empty constructor. Constructs a new type instance using the default values. */
			DType(const DTypeDefPtr& type, bool useCache = false) : mType(type), mUseCache(useCache) {
				assert(!mType.isNull()); // no type def, no meaning!

				mData = new char[mType->size()];
				mType->toDefaults(mData);
			}

			/** Constructor, using FilePtr to read the DType values
			* @param file The file used as data source
			* @param size The size of the data expected
			* @note for dynamic size types (variable length strings), the size should be the overall length of the data (32bits size + the data itself) */
			DType(const DTypeDefPtr& type, FilePtr& file, size_t _size, bool useCache = false) : mType(type), mUseCache(useCache) {
				mData = 0;
				read(file, _size);
			}

			~DType() {
				delete[] mData;
				mData = NULL;
			};

			/** Value setter.
			* @see DTypeDef.set */
			void set(const std::string& field, const DVariant& value) {
				mType->set(mData, field, value);

				// cache value if so requested
				if (mUseCache)
					mCache[field] = value;
			}

			/** Value getter.
			* @see DTypeDef.get */
			DVariant get(const std::string& field) {
				if (mUseCache) {
					ValueCache::const_iterator it = mCache.find(field);

					if (it != mCache.end())
						return it->second;
					else {
						// No value cached, cache now
						DVariant val = mType->get(mData, field);
						mCache[field] = val;
					}
				}

				return mType->get(mData, field);
			}

			/** Copy operator */
			const DType& operator =(const DType& b) {
				char* odata = mData;

				int sz = b.size();
				mData = new char[sz];

				// copy the data
				memcpy(mData, b.mData, sz);

				mType = b.mType;

				mUseCache = b.mUseCache;
				mCache = b.mCache;

				delete[] odata;

				return *this;
			}

			/** Real size of the data. */
			size_t size() const {
				return mType->size();
			}

			/** Serializer. Writes the type data into a FilePtr
			* @param file The file pointer to write into */
			void serialize(FilePtr& file) const {
				file->write(mData, size());
			}

			/** Deserializer. Reads the type data into this instance
			* @param file The file pointer to read from */
			void read(FilePtr& file, size_t _size) {
				delete[] mData;

				assert(size() == _size);

				mData = new char[mType->size()];
				file->read(mData, _size);
			}

			/** Type getter.
			*@return Type pointer DTypeDefPtr. */
			DTypeDefPtr type() {
				return mType;
			}


		protected:
			DTypeDefPtr mType;
			char* mData;
			bool mUseCache; // cache for values is used

			typedef std::map<std::string, DVariant> ValueCache;

			ValueCache mCache;
	};

	typedef shared_ptr< DType > DTypePtr;

	/// A unified unstructured field list description structure
	typedef struct {
		/// Name of the field
		std::string name;
		/// Label of the field - readable
		std::string label;
		/// Size of the field. -1 for variable-sized fields (strings)
		int size;
		/// Type of the field
		DVariant::Type type;
		/// Enumerator of the allowed values (either enum or bitfield type). NULL means this field is not enumerated
		DEnum* enumerator;
	} DataFieldDesc;

	/// Data field const iterator. The const is ommited to leave the type's name in reasonable length
	typedef ConstIterator< DataFieldDesc > DataFieldDescIterator;

	typedef shared_ptr< DataFieldDescIterator > DataFieldDescIteratorPtr;

	/// std::list of DataFieldDesc records
	typedef std::list<DataFieldDesc> DataFieldDescList;

	/// Iterator wrapper over field description list.
	class OPDELIB_EXPORT DataFieldDescListIterator : public DataFieldDescIterator {
		public:
			DataFieldDescListIterator(const DataFieldDescList& src);

			virtual const DataFieldDesc& next();
			virtual bool end() const;

		protected:
			const DataFieldDescList& mList;
			DataFieldDescList::const_iterator mIt;
	};

	// Empty iterator over data fields (for data-less links)
	class OPDELIB_EXPORT EmptyDataFieldDescListIterator : public DataFieldDescListIterator {
		public:
			EmptyDataFieldDescListIterator();

			virtual const DataFieldDesc& next();
			virtual bool end() const;

		protected:
			DataFieldDescList mEmptyList;
	};


} // namespace Opde

#endif
