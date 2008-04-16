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
 
#ifndef __DVARIANT_H
#define __DVARIANT_H

#include <stdexcept>
#include "vector3.h"
#include "integers.h"
#include "RefCounted.h"

#include <OgreQuaternion.h>

using Ogre::Quaternion;

namespace Opde {
	/** a variant class. This is a class that stores a value of a certain type. */
	class DVariant {
		public:
			/** Type specifier */
			typedef enum {
				/** Invalid type */
				DV_INVALID	= 0,
				/** Bool type */
				DV_BOOL		= 1,
				/** Float type */
				DV_FLOAT	= 2,
				/** Int type */
				DV_INT		= 3,
				/** Unsigned int type */
				DV_UINT		= 4,
				// Shared pointer types start here:
				/** String type. Shared */
				DV_STRING	= 5, 
				/** Vector type. Shared */
				DV_VECTOR	= 6,
				/** Quaternion type. Shared */
				DV_QUATERNION = 7
			} Type;
			
			/** Construct an invalid, empty DVariant instance. */
			DVariant();
			
			/** Construct a DVariant instance. Set a value from the pointer target val, if val is not NULL */
			DVariant(Type t, void *val = 0);
			
			/** construct a new variant type using a string value converted to the type t as value 
			* @param t Type to use
			* @param txtval Textual value for the new variant to use
			*/
			DVariant(Type t, const std::string& txtval);
			
			/** Copy constructor 
			* @param b The variant to copy the value from
			* @note If the source variant is shared valued, only pointer to private data is copied, and reference is incremented */
			DVariant(const DVariant& b);
			
			/** Bool constructor 
			* @param val The value to use */
			DVariant(bool val);
			
			/** float constructor 
			* @param val The value to use */
			DVariant(float val);
			
			/** Int constructor 
			* @param val The value to use */
			DVariant(int val);
			
			/** Unsigned int constructor 
			* @param val The value to use */
			DVariant(uint val);
			
			/** String constructor. Construct a string variant with the length of the input char array up-to 'length' bytes 
			* @param text The value to use 
			* @param length The limiting length of the source char array */
			DVariant(const char* text, int length = -1);
			
			/** String constructor.
			* @param text the source string to copy value from */
			DVariant(const std::string& text);
			
			/** Vector3 constructor.
			* @param x the X part of the vector 
			* @param y the Y part of the vector 
			* @param z the Z part of the vector */
			DVariant(float x, float y, float z);
			
			/** Vector3 constructor.
			* @param vec the source vector to copy value from */
			DVariant(const Vector3& vec);
			
			/** Quaternion constructor.
			* @param ori the source orientation quaternion to copy value from */
			DVariant(const Quaternion& ori);

			/** Destructor */
			virtual ~DVariant();
			
			/** returns a char* containing the type name */
			const char* typeToString() const;
			
			/** converts the inner value to string, if possible.
			* @throw runtime_error if not convertible */
			operator std::string() const;
			
			/** converts the inner value to string, if possible.
			* @throw runtime_error if not convertible */
			std::string toString() const;
			
			/** converts the inner value to Int, if possible.
			* @throw runtime_error if not convertible */
			int toInt() const;
			
			/** converts the inner value to Unsigned Int, if possible.
			* @throw runtime_error if not convertible */
			uint toUInt() const;
			
			/** converts the inner value to float, if possible.
			* @throw runtime_error if not convertible */
			float toFloat() const;
			
			/** converts the inner value to bool, if possible.
			* @throw runtime_error if not convertible */
			bool toBool() const;
			
			/** converts the inner value to Vector3, if possible.
			* @throw runtime_error if not convertible */
			Vector3 toVector() const;
			
			/** converts the inner value to Quaternion, if possible.
			* @throw runtime_error if not convertible */
			Quaternion toQuaternion() const;
			
			/** Asignment operator. Shared pointers are released if needed */
			const DVariant& operator =(const DVariant& b);
			
			/** Asignment operator */
			const DVariant& operator =(bool b);
			/** Asignment operator */
			const DVariant& operator =(int i);
			/** Asignment operator */
			const DVariant& operator =(uint u);
			/** Asignment operator */
			const DVariant& operator =(float f);
			/** Asignment operator */
			const DVariant& operator =(char* s);
			/** Asignment operator */
			const DVariant& operator =(const std::string& s);
			/** Asignment operator */
			const DVariant& operator =(const Vector3& v);
			/** Asignment operator */
			const DVariant& operator =(const Quaternion& v);
			
			
			/** Type identifier. Returns the Type of the variant's value */
			Type type() const;
			
			/** Comparison operator. Comapares equality of the values.
			* Tries to convert the values to see if the values match even if those do not have the same type */
			inline bool operator ==(const DVariant &b) const {
				return compare(b);
			}
			
			/** Comparison operator. Comapares non-equality of the values. @see operator==()*/
			inline bool operator !=(const DVariant &b) const {
				return !compare(b);
			}
			
			/** Base shared type for DVariant */
			class SharedBase {
				public:
					SharedBase() {};
					virtual ~SharedBase() {};
					
					virtual SharedBase * clone() = 0;
			};
			
			/** Templated shared type for DVariant. Holds values for the shared types (string,vector) */
			template <typename T> class Shared : public SharedBase {
				public:
					/// Constructor
					Shared(const T& _data) : SharedBase(), data(_data) {};
					
					/// Copy constructor from same-typed shared
					Shared(const Shared<T>& src) : SharedBase(), data(src.data) {};
					
					virtual ~Shared() {}
					
					virtual SharedBase * clone() { 
						SharedBase* copy = new Shared<T>(data);
						
						return copy;
					};
					
					T data;
			};
			
			/** Private data holder. Holds either shared or non-shared value */
			struct Private {
				union Data {
					bool	dbool;
					float	dfloat;
					int 	dint;
					uint	duint;
					SharedBase* shared;
				} data;
				
				Type type : 31;
				unsigned int isShared : 1;
			};
			
		protected:
			/** Helper conversion routine : Vector3 from string 
			* Takes comma separated values (first 3, ignoring any further text) 
			* @param str The string to be parsed 
			* @throw runtime_error if unsupported format is encountered */
			static Vector3 StringToVector(const std::string& str);
			
			/** Helper conversion routine : Quaternion from string 
			* Takes comma separated values (first 4 - w,x,y,z, ignoring any further text) 
			* @param str The string to be parsed 
			* @throw runtime_error if unsupported format is encountered */
			static Quaternion StringToQuaternion(const std::string& str);
			
			/** Helper conversion routine : int from String 
			* @param str the string to be parsed
			* @note Understands the 0xNUMBER hexadecimal format */
			static int StringToInt(const std::string& str);
			
			/** Helper conversion routine : uint from String 
			* @param str the string to be parsed 
			* @note Understands the 0xNUMBER hexadecimal format */
			static uint StringToUInt(const std::string& str);
			
			/** Helper conversion routine : Bool from String 
			* Converts 1,yes,true to true, 0,no,false to false (ignoring case) 
			* @throw runtime_error if unsupported text is encountered */
			static bool StringToBool(const std::string& str);
			
			/** Helper conversion routine : Float from String 
			* @throw runtime_error if unsupported text is encountered */
			static float StringToFloat(const std::string& str);
			
			/** Cast the shared data to a specified type
			* @return reference to the type T if possible. */
			template <typename T> T& shared_cast() const;
			
			/** The comparison function. Is called by == and != operators */
			bool compare(const DVariant &b) const;
			
			/** The data holder */
			Private mPrivate;
	};
}

#endif
