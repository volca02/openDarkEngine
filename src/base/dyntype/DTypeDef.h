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
 *****************************************************************************/
 
#ifndef __DTYPEDEF_H
#define __DTYPEDEF_H

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

namespace Opde {
	/** Type aware variant based enumeration definition. Used for enumeration and bitfields. 
	@note For bitfields, the base type must be DV_UINT
	@note No check on multiple keys with the same values is made */
	class DEnum {
		private:
			typedef std::map<std::string, DVariant> StrValMap;
			
			StrValMap mValMap;
			
			DVariant::Type mEnumType;
			
			bool mBitField;
		public:
			DEnum(DVariant::Type enumType, bool bitfield);
			
			/** Insert a field definition inside the Enum 
			* @note It is not adviced to modify enumerations on other times than creation as this could confuse other pieces of code */
			void insert(const std::string& key, const DVariant& value);
			
			/** get the symbol text by the enumeration value */
			const std::string& symbol(const DVariant& val) const;
			
			/** get the value by the symbol text */
			const DVariant& value(const std::string& symbol) const;
			
			/** Enumeration field. A single field that is either checked or not, and has a specific value */
			typedef struct EnumField {
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
	};
	
	/// Shared pointer to DEnum
	typedef shared_ptr<DEnum> DEnumPtr;
	
	// Forward declaration 
	class DTypeDef;
	
	typedef shared_ptr<DTypeDef> DTypeDefPtr;
	
	/** vector of type definitions. Used to fill in a struct type. */
	typedef std::vector<DTypeDefPtr> DTypeDefVector;
	
	/// Base definition of the DTypes dat storage
	class DTPrivateBase {
		public:
			/** DTypeDef private node type */
			typedef enum NodeType {
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
	class DTypeDef : public NonCopyable {
		public:
			/// Construct as a copy of other DTypeDef, with different name
			DTypeDef(const std::string& name, const DTypeDef& src);
					
			/// Constucts a possibly enumerated (if _enum member is not NULL) type with default value and type taken from templ
			DTypeDef(const std::string& name, const DVariant& templ, int size, DEnumPtr _enum = NULL);
			
			/// Constucts a possibly enumerated (if _enum is not NULL) type with type taken from templ
			DTypeDef(const std::string& name, DVariant::Type type, int size, DEnumPtr _enum = NULL);
			
			/** Construct an array type, with the type defined by member, and default value of the fields defined by member, sized size
			* @note The base name is taken from the member
			*/
			DTypeDef(DTypeDefPtr member, int size);
			
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
			
			/** Create a new data instance based on the default values. Deallocation of the returned data is to be done by caller */
			char* create();
			
			/** Returns the size of this type definition, or -1 if variable sized
			* @note use size(char* data) for dynamically sized typedefs */
			int size();
			
			int size(char *dataptr) {
				int sz = size();
				if (sz < 0) {
					return static_cast<uint32_t>(*dataptr) + sizeof(uint32_t);
				} else {
					return sz;
				}
			}
			
			/** Get the value of the field specified by name, reading the data from the dataptr mem. location 
			* @note The field name has to exist, and has to be a simple field (Not array/union/struct field) 
			* @note No check on the dataptr allocated size is done. This means the allocated size has to be at least the same as the size of this dtypedef */
			DVariant get(char* dataptr, const std::string& field);
			
			/** Sets the data of a certain field.
			* @return New pointer if the data must have been reallocated (var-length string), otherwise NULL 
			* @note Even if reallocation took place, the old data are NOT deleted by this method */
			char* set(char* dataptr, const std::string& field, const DVariant& nval);
			
			/** Get the definition of a specific field from the built cache
			* @note The field name has to exist, and has to be a simple field (Not array/union/struct field) */
			DTypeDef getFieldDef(const std::string& field);
			
			/// Convert to a readable name from the internal field name @throws Exception if not found
			std::string toLabel(const std::string& fname);
			
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
			typedef struct FieldDef {
				unsigned int	offset;
				std::string	name; // Absolute name
				DTypeDefPtr 	type;
				
				inline char* set(char* dataptr, const DVariant& val) const {
					return type->_set(reinterpret_cast<char*>(dataptr) + offset, val);
				}
				
				inline char* setDefault(char* dataptr) const {
					return type->_set(reinterpret_cast<char*>(dataptr) + offset, type->getDefault());
				}
				
				inline DVariant get(char* dataptr) const {
					return type->_get(reinterpret_cast<char*>(dataptr) + offset);
				}
			};
		
			/// Field list. Order is sequential (increasing offset), with one exception - unions
			typedef std::vector<FieldDef> Fields;
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
			
		protected:
			/** Internal field getter */
			DVariant _get(char* ptr);
			
			/** Internal field setter 
			* @param ptr Direct pointer to the start of the data to be set 
			* @return ptr if there was no reallocation needed, new pointer if reallocation was done (var length string)
			*/
			char* _set(char* ptr, const DVariant& val);
			
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
		
		private:
			// Hidden copy constructor
			DTypeDef(const DTypeDef &b) : mDefVal(0) {};
			
			// Hidden default constructor
			DTypeDef()  : mDefVal(0) {};
			
			DTypeDef& operator =(const DTypeDef &b) {};
	};
	
	/** DTypeDef 'variable'. */
	class DType {
		public:
			DType(const DType& b) {
				char* odata = mData;
				
				int sz = b.size();
				mData = new char[sz];
				
				// copy the data
				memcpy(mData, b.mData, sz);
				
				mType = b.mType;
				
				delete[] odata;
			}
			
			DType(DTypeDefPtr type) : mType(type) {
				assert(!mType.isNull()); // no type def, no meaning!
				
				mData = mType->create();
			}
			
			/** Constructor, using FilePtr to read the DType values 
			* @param file The file used as data source 
			* @param size The size of the data expected
			* @note for dynamic size types (variable length strings), the size should be the overall length of the data (32bits size + the data itself) */
			DType(DTypeDefPtr type, FilePtr file, int _size) : mType(type) {
				if (mType->size() < 0) { // dyn. size. we have to use the size
					mData = new char[_size];
					
					file->read(mData, _size);
					
					// inconsistency smells badly
					assert(static_cast<uint32_t>(*mData) == (_size - sizeof(uint32_t)));
				} else {
					assert(size() == _size);
					mData = mType->create();
					file->read(mData, size());
				}
			}
			
			/** Value setter. 
			* @see DTypeDef.set */
			void set(const std::string& field, const DVariant& value) {
				char* ndata = mType->set(mData, field, value);
				
				if (ndata != mData) {
					char* odata = mData;
					mData = ndata;
					delete odata;
				}
			}
			
			/** Value getter. 
			* @see DTypeDef.get */
			DVariant get(const std::string& field) const {
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
				
				delete[] odata;
				
				return *this;
			}
		
			/** Real size of the data. 
			@note In case of a dynamic field, returns the size of the uint32 + bytes the field is consisted of */
			int size() const {
				return mType->size(mData);
			}
			
			/** Serializer. Writes the type data into a FilePtr
			* @param file The file pointer to write into */
			void serialize(FilePtr file) const {
				file->write(mData, size());
			}
			
			/** Type getter. 
			*@return Type pointer DTypeDefPtr. */
			DTypeDefPtr type() {
				return mType;
			}
			
		
		protected:
			DTypeDefPtr mType;
			char* mData; 
	};
	
	typedef shared_ptr< DType > DTypePtr;
} // namespace Opde

#endif
