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
 
#include "DTypeDef.h"

#include <stdexcept>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <string>
#include "integers.h"
#include "OpdeException.h"

using namespace std;

namespace Opde {
	class DTypeDef;
	
	/*------------------------------------------------------*/
	/*------------------- DEnum ----------------------------*/
	/*------------------------------------------------------*/
	DEnum::DEnum(DVariant::Type enumType, bool bitfield) : mValMap() {
		mEnumType = enumType;
		mBitField = bitfield;
		
		if (mBitField && mEnumType != DVariant::DV_UINT) 
			OPDE_EXCEPT("Only uint is supported for bitfields", "DEnum::DEnum");	
		
	}
	
	//------------------------------------
	void DEnum::insert(const std::string& key, const DVariant& value) {
		// Type check
		if (mEnumType != value.type())
			throw(runtime_error("Type violation of the enumeration type"));
		
		mValMap.insert(make_pair<string, DVariant>(key, value));
	}
	
	//------------------------------------
	const string& DEnum::symbol(const DVariant& val) const {
		if (mEnumType != val.type())
			throw(runtime_error("Type violation of the enumeration type"));
		
		StrValMap::const_iterator it = mValMap.begin();
		
		for (;it != mValMap.end(); it++) {
			if (it->second == val)
				return it->first;
		}
		
		throw(out_of_range("DEnum::symbol"));
	}
	
	//------------------------------------
	const DVariant& DEnum::value(const std::string& symbol) const {
		StrValMap::const_iterator it = mValMap.find(symbol);
		
		if (it == mValMap.end())
			return it->second;
		
		throw(out_of_range("DEnum::value"));
	}
	
	//------------------------------------
	DEnum::EnumFieldList DEnum::getFieldList(const DVariant& val) const {
		uint uval;
		
		if (mBitField) {
			// is it convertible to the uint? If not, this will throw an exception, so it's alright
			uval = val.toUInt();
		}
		
		// insert all the fields, comparing the variants to check the selected
		StrValMap::const_iterator it = mValMap.begin();
		
		EnumFieldList l;
		
		for (;it != mValMap.end(); it++) {
			
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
	class DTPrivateBase : public RefCounted {
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
			NodeType pNodeType;
			
			/// Destructor
			virtual ~DTPrivateBase() {
			}
			
			/// Size getter
			virtual size_t size() = 0;
			
		protected:
			/// Constructor
			DTPrivateBase(NodeType type) : pNodeType(type), RefCounted() {};
	};
	
	/** Simple value holder. */
	class DTPrivateSimple : public DTPrivateBase {
		public:
			/// Type of the field
			DVariant::Type mType;
		
			/// Specified field size
			int mSize;
			
			/// Enum holder. for enumerated types, NULL otherwise
			DEnum* mEnum;
			
			/// Constructor
			DTPrivateSimple(const DVariant::Type& type, size_t size, DEnum *_enum) : DTPrivateBase(NT_SIMPLE), 
				mType(type), mEnum(_enum), mSize(size) {
				
				if (mEnum != NULL)
					mEnum->addRef();
			};
		
			/// Destructor
			virtual ~DTPrivateSimple() {
				if (mEnum != NULL)
					mEnum->release();
			}
			
			/// true if the simple type uses enumeration/bitfield
			bool isEnumerated() {
				return (mEnum != NULL);
			}
			
			/// Size of the simple type (of the data holder)
			virtual size_t size() {
				return mSize;
			}
	};
	
	/// Structured Type definition. can be Array or Struct
	class DTPrivateStructured : public DTPrivateBase {
		public:	
			/// The types of the submembers
			DTypeDefVector mSubTypes; 
			
			/// the size of the array. 0 otherwise.
			size_t	mArraySize;
			
			/// Specifies whether the struct type is 'union' or 'struct'
			bool mUnioned;
			
			/// Array type constructor. Constructs an array type
			DTPrivateStructured(DTypeDef* type, size_t size) : DTPrivateBase(NT_ARRAY), mArraySize(size), mSubTypes(), mUnioned(false) {
				mSubTypes.push_back(type);
				type->addRef();
			}
			
			/// Struct/Union constructor
			DTPrivateStructured(const DTypeDefVector& types, bool unioned) : DTPrivateBase(NT_STRUCT), mArraySize(0), 
					mSubTypes(types), mUnioned(unioned) {
				// Reference the contained type defs
				DTypeDefVector::const_iterator it = mSubTypes.begin();
				for(; it != mSubTypes.end(); it++) {
					(*it)->addRef();
				}
			}
			
			/// Destructor
			virtual ~DTPrivateStructured() {
				DTypeDefVector::const_iterator it = mSubTypes.begin();
				
				for(; it != mSubTypes.end(); it++) 
					(*it)->release();
					
				mSubTypes.clear();
			}
			
			/// Calculates the total size of this structured private type
			virtual size_t size() {
				if (mArraySize == 0) { // Struct/union
					// For struct, sum all the members. For Union, compute maximum.
					size_t maxsz = 0, sumsz = 0;
					
					DTypeDefVector::const_iterator it = mSubTypes.begin();
					
					for (; it != mSubTypes.end(); it++) {
						size_t elemsz = (*it)->size();
						
						if (maxsz < elemsz)
							maxsz = elemsz;
						
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
	DTypeDef::DTypeDef(const std::string& name, const DTypeDef& src) : mDefVal(0), RefCounted() {
		mPriv = src.mPriv;
		mPriv->addRef();
		
		mDefVal = src.mDefVal;
		mTypeName = name;
	}
	
	//------------------------------------
	// Field constructor
	DTypeDef::DTypeDef(const std::string& name, const DVariant& templ, int size, DEnum* _enum) : mTypeName(name), mDefVal(templ), RefCounted() {
		// Check for constraints
		
		// zero size is meaningles
		if (size == 0)
			OPDE_EXCEPT("Zero sized field cannot be constructed", "DTypeDef::DTypeDef");
		
		// Float with non-4 byte length
		if (templ.type() == DVariant::DV_FLOAT && size != 4)
			OPDE_EXCEPT("Only 4-byte floats are supported", "DTypeDef::DTypeDef");
		
		// Vector with size != 12
		if (templ.type() == DVariant::DV_VECTOR && size != 12)
			OPDE_EXCEPT("Only 12-byte float vectors are supported", "DTypeDef::DTypeDef");
		
		// Enumeration which is not uint
		if (_enum !=NULL && templ.type() == DVariant::DV_UINT)
			OPDE_EXCEPT("Only uint enumerations are supported", "DTypeDef::DTypeDef");
		
		// Size <=0 and not string
		if (templ.type() != DVariant::DV_STRING && size < 0)
			OPDE_EXCEPT("Dynamic size is only supported for strings", "DTypeDef::DTypeDef");
		
		mTypeName = name;
		mDefVal = templ;
		
		mPriv = new DTPrivateSimple(templ.type(), size, _enum);
	}
	
	//------------------------------------
	// Array constructor
	DTypeDef::DTypeDef(const std::string& name, DTypeDef* member, int size) : mTypeName(name), mDefVal(0), RefCounted() {
		// Check the member for being a dynamic length string (prohibited)
		if (member->size() <= 0)
			OPDE_EXCEPT("Dynamic sized type is not supported as a field", "DTypeDef::DTypeDef");
		
		mTypeName = name;
		
		size_t ms = member->size(); // member size
		string basen = name + '['; // base name
		
		// Now build the field cache.
		// Not as simple, as we have to insert all fields of the member inside
		for (int i = 0; i < size; i++) {
			if (!member->isField()) {
				const Fields& f = member->getFields();
				Fields::const_iterator it = f.begin();
				
				for (;it != f.end(); it++) {
					// add the field to the list
					const FieldDef& orig = *it;
					
					FieldDef field;
					field.offset = i * ms + orig.offset;
					
					std::stringstream sst;
					
					sst << basen << i << "]." << orig.name;
					sst >> field.name;
					
					field.type = orig.type;
			
					mFields.push_back(field);
				}
			} else { 
				// in reality, this creates a sortof alias. that is - the name of the original field is masked by array name
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
		
		mPriv = new DTPrivateStructured(member, size);
	}
	
	//------------------------------------
	// Struct/Union constructor
	DTypeDef::DTypeDef(const std::string& name, DTypeDefVector members, bool unioned) :  mDefVal(0), RefCounted() {
		// Check all the members, if any of those is variable length, throw an exception
		DTypeDefVector::const_iterator mit = members.begin();
		
		for (;mit != members.end(); mit++) {
			if ((*mit)->isField() && (*mit)->size() <= 0) // Check only field types. Non-Field are checked anyway
				OPDE_EXCEPT("Dynamic sized type is not supported as a field", "DTypeDef::DTypeDef");
		}
		
		mTypeName = name;
		
		// Refresh the field cache
		size_t offs = 0; // cummulative offset
		string basen = name + '.'; // base name
		
		// Back again
		mit = members.begin();
		
		// Now build the field cache.
		for (; mit != members.end(); mit++) {
			
			DTypeDef* member = *mit;
			
			// if the member is a container
			if (!member->isField()) { // unfold the fields
				const Fields& f = member->getFields();
				Fields::const_iterator it = f.begin();
				
				for (;it != f.end(); it++) {
					// add the field to the list
					const FieldDef& orig = *it;
					
					FieldDef field;
					field.offset = offs + orig.offset;
					
					std::stringstream sst;
					
					if (member->isArray()) {
						sst << orig.name; // array does not get the dotted notation
					} else {
						sst << member->mTypeName << '.' << orig.name;
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
		
		mPriv = new DTPrivateStructured(members, unioned);
	}
	
	//------------------------------------
	DTypeDef::~DTypeDef() {
		mPriv->release();
	}
	
	//------------------------------------
	bool DTypeDef::isField() const {
		return (mPriv->pNodeType == DTPrivateBase::NT_SIMPLE);
	}
	
	//------------------------------------
	bool DTypeDef::isArray() const {
		return (mPriv->pNodeType == DTPrivateBase::NT_ARRAY);
	}
	
	//------------------------------------
	bool DTypeDef::isStruct() const {
		return (mPriv->pNodeType == DTPrivateBase::NT_STRUCT);
	}
	
	//------------------------------------
	bool DTypeDef::isEnumerated() const {
		if (isField()) {
			return dynamic_cast<DTPrivateSimple*>(mPriv)->isEnumerated();
		} else 
			return false;
	}
	
	//------------------------------------
	int DTypeDef::size() {
		return mPriv->size();
	}
	
	//------------------------------------
	const DEnum* DTypeDef::getEnum() {
		if (isField()) {
			return dynamic_cast<DTPrivateSimple*>(mPriv)->mEnum;
		} else 
			return false;
	}
	
	//------------------------------------
	void DTypeDef::setDefault(const DVariant& val) {
		if (!isField())
			OPDE_EXCEPT("Default value can only be set on field types", "DTypeDef::setDefault");
		
		mDefVal = val;
	}
	
	//------------------------------------
	DVariant DTypeDef::get(void* dataptr, const std::string& field) {
		if (isField()) //  && field=="" - I do not waste time to check, I choose to believe!
			return _get(dataptr);
		
		FieldDef& fld = getByName(field);
		
		if (!fld.type->isField())
			OPDE_EXCEPT("Field get resulted in non-field _get","DTypeDef::_get");
		
		// offset the data and call the getter
		return fld.type->_get(((char *)dataptr) + fld.offset);
	}
	
	//------------------------------------
	void* DTypeDef::set(void* dataptr, const std::string& field, DVariant& nval) {
		if (isField()) //  && field=="" - I do not waste time to check, I choose to believe!
			_set(dataptr, nval);
		
		FieldDef& fld = getByName(field);
		
		if (!fld.type->isField())
			OPDE_EXCEPT("Field set resulted in non-field _set","DTypeDef::_set");
		
		// offset the data and call the getter
		return fld.type->_set(((char *)dataptr) + fld.offset, nval);
	}
	
	//------------------------------------
	DVariant DTypeDef::_get(void *ptr) {
		// Based on the type of the data and size, construct the Variant and return
		const DTPrivateSimple* ps = dynamic_cast<DTPrivateSimple*>(mPriv);
		
		if (ps->mType == DVariant::DV_BOOL) {
			bool bl;
			
			switch (ps->mSize) {
				case 1: bl = *reinterpret_cast<uint8_t*>(ptr) != 0; break;
				case 2: bl = *reinterpret_cast<uint16_t*>(ptr) != 0; break;
				case 4: bl = *reinterpret_cast<uint32_t*>(ptr) != 0; break;
				default:
					OPDE_EXCEPT("Unsupported bool size", "DTypeDef::_get");
			}
			return DVariant(bl);
			
		} else if(ps->mType == DVariant::DV_FLOAT) {
			switch (ps->mSize) {
				case 4: return DVariant(*reinterpret_cast<float*>(ptr));
				case 8: return DVariant((float)*reinterpret_cast<double*>(ptr));
				default:
					OPDE_EXCEPT("Unsupported float size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_INT) {
			switch (ps->mSize) {
				case 1: return DVariant(*reinterpret_cast<int8_t*>(ptr));
				case 2: return DVariant(*reinterpret_cast<int16_t*>(ptr));
				case 4: return DVariant(*reinterpret_cast<int32_t*>(ptr));
				default:
					OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_UINT) {
			switch (ps->mSize) {
				case 1: return DVariant(*reinterpret_cast<uint8_t*>(ptr));
				case 2: return DVariant(*reinterpret_cast<uint16_t*>(ptr));
				case 4: return DVariant(*reinterpret_cast<uint32_t*>(ptr));
				default:
					OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_STRING) {
				// special - either zero terminated, fixed size, or length specified as a first byte.
				// I can only support the variable length string alone, which is ok
				if (ps->mSize == -1) {// Variable length string
					// First 4 bytes size uint, then data
					uint32_t* uptr = reinterpret_cast<uint32_t*>(ptr);
					
					return DVariant(reinterpret_cast<char*>(uptr + 1), *uptr);
				} else {
					// compose a string out of the buffer (max len is mSize, can be less)
					int len = strlen(reinterpret_cast<char*>(ptr));
					return DVariant(reinterpret_cast<char*>(ptr), std::min(ps->mSize, len));
				}
				
				OPDE_EXCEPT("String not supported now", "DTypeDef::_get");
				
		} else if(ps->mType == DVariant::DV_VECTOR) {			
				float x, y, z;
				float* fptr = (float*)ptr;
				
				x = *fptr; // TODO: Compile-time float size check (has to be 4 to let this work)
				y = *(fptr+1);
				z = *(fptr+2);
				
				return DVariant(Vector3(x, y, z));
		} else {
			OPDE_EXCEPT("Unsupported type", "DTypeDef::_get");
		}
	}
	
	//------------------------------------
	void* DTypeDef::_set(void* ptr, const DVariant& val) {
		const DTPrivateSimple* ps = dynamic_cast<DTPrivateSimple*>(mPriv);
		
		if (ps->mType == DVariant::DV_BOOL) {
			switch (ps->mSize) {
				case 1: 
					*reinterpret_cast<uint8_t*>(ptr) = val.toBool(); 
					return ptr;
					
				case 2: 
					*reinterpret_cast<uint16_t*>(ptr) = val.toBool(); 
					return ptr;
					
				case 4: 
					*reinterpret_cast<uint32_t*>(ptr) = val.toBool(); 
					return ptr;
					
				default:
					OPDE_EXCEPT("Unsupported bool size", "DTypeDef::_get");
			}
		
		} else if(ps->mType == DVariant::DV_FLOAT) {
			switch (ps->mSize) {
				case 4: 
					*reinterpret_cast<float*>(ptr) = val.toFloat(); 
					return ptr;
					
				case 8: 
					*reinterpret_cast<double*>(ptr) = val.toFloat();
					return ptr;
					
				default:
					OPDE_EXCEPT("Unsupported float size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_INT) {
			switch (ps->mSize) {
				case 1: 
					*reinterpret_cast<int8_t*>(ptr)  = val.toInt();
					return ptr;
				case 2: 
					*reinterpret_cast<int16_t*>(ptr) = val.toInt();
					return ptr;
					
				case 4: 
					*reinterpret_cast<int32_t*>(ptr) = val.toInt();
					return ptr;
					
				default:
					OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_UINT) {
			switch (ps->mSize) {
				case 1: 
					*reinterpret_cast<uint8_t*>(ptr)  = val.toUInt();
					return ptr;
					
				case 2: 
					*reinterpret_cast<uint16_t*>(ptr) = val.toUInt();
					return ptr;
					
				case 4: 
					*reinterpret_cast<uint32_t*>(ptr) = val.toUInt();
					return ptr;
					
				default:
					OPDE_EXCEPT("Unsupported int size", "DTypeDef::_get");
			}
			
		} else if(ps->mType == DVariant::DV_STRING) {
				// special - either zero terminated, fixed size, or length specified as a first byte.
				// I can only support the variable length string alone, which is ok
				if (ps->mSize == -1) {// Variable length string
					/* I would have to look at the previous size of the void, which I do not have.
					 I simply alloc a new one and let the caller do the deallocation and stuff
					*/
					const std::string& s = val.toString();
					
					int size = s.length() + 1;
					
					char* newptr = new char[size + sizeof(uint32_t)];
					
					// First 4 bytes size uint, then data
					uint32_t* uptr = reinterpret_cast<uint32_t*>(newptr);
					
					*uptr = size;
					
					// now copy the string
					s.copy(&newptr[sizeof(uint32_t)], size - 1);
					
					// terminate. the string
					newptr[size + sizeof(uint32_t)] = '\0';
					
					return newptr;
				} else {
					char* cptr = reinterpret_cast<char*>(ptr);
					
					const std::string& s = val.toString();
					
					int len = s.length();
					
					// Copy the string to the buffer. Limit by 1 to have a trailing zero at end
					s.copy(reinterpret_cast<char*>(ptr), ps->mSize - 1);
					
					// Terminating zero.
					cptr[min(ps->mSize - 1, len)] = '\0';
					
					return ptr;
				}
				
				OPDE_EXCEPT("String not supported now", "DTypeDef::_get");
				
		} else if(ps->mType == DVariant::DV_VECTOR) {			
				float* fptr = reinterpret_cast<float*>(ptr);
				
				Vector3 v = val.toVector();
				
				*fptr = v.x; 
				*(fptr+1) = v.y;
				*(fptr+2) = v.z;
				
				return ptr;
		} else {
			OPDE_EXCEPT("Unsupported _set type", "DTypeDef::_set");
		}
	}
	
	//------------------------------------
	DTypeDef::FieldDef& DTypeDef::getByName(const std::string& name) {
		// Search for field in mFieldMap
		FieldMap::iterator f = mFieldMap.find(name);
		
		if (f==mFieldMap.end())
			OPDE_EXCEPT(string("Field not found : ") + name,"DTypeDef::get");
		
		return f->second;
	}
	
	//------------------------------------
	void DTypeDef::_makeFieldMap() {
		Fields::const_iterator it = mFields.begin();
		
		for (; it != mFields.end(); it++) {
			const FieldDef& fd = *it;
			
			std::pair<FieldMap::iterator, bool> result = mFieldMap.insert(make_pair(fd.name, fd));
			
			if (!result.second)
				OPDE_EXCEPT(string("Attempt to insert a non-unique member name (already present) : ") + fd.name, "DTypeDef::_makeFieldMap");
		}
	}
}
