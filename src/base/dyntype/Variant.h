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

#ifndef __DVARIANT_H
#define __DVARIANT_H

#include "config.h"

#include "OpdeException.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "integers.h"
#include <stdexcept>

#include <OgreQuaternion.h>

using Ogre::Quaternion;

namespace Opde {
/** a variant class. This is a class that stores a value of a certain type. */
class Variant {
public:
    /** Type specifier */
    typedef enum {
        /** Invalid type */
        DV_INVALID = 0,
        /** Bool type */
        DV_BOOL = 1,
        /** Float type */
        DV_FLOAT = 2,
        /** Int type */
        DV_INT = 3,
        /** Unsigned int type */
        DV_UINT = 4,
        // Shared pointer types start here:
        /** String type. Shared */
        DV_STRING = 5,
        /** Vector type. Shared */
        DV_VECTOR = 6,
        /** Quaternion type. Shared */
        DV_QUATERNION = 7
    } Type;

    /** Construct an invalid, empty Variant instance. */
    Variant();

    /** Construct a Variant instance. Set a value from the pointer target val,
     * if val is not NULL */
    Variant(Type t, void *val = 0);

    /** construct a new variant type using a string value converted to the type
     * t as value
     * @param t Type to use
     * @param txtval Textual value for the new variant to use
     */
    Variant(Type t, const std::string &txtval);

    /** Copy constructor
     * @param b The variant to copy the value from
     * @note If the source variant is shared valued, only pointer to private
     * data is copied, and reference is incremented */
    Variant(const Variant &b);

    /** Bool constructor
     * @param val The value to use */
    Variant(bool val);

    /** float constructor
     * @param val The value to use */
    Variant(float val);

    /** Int constructor
     * @param val The value to use */
    Variant(int val);

    /** Unsigned int constructor
     * @param val The value to use */
    Variant(uint val);

    /** String constructor. Construct a string variant with the length of the
     * input char array up-to 'length' bytes
     * @param text The value to use
     * @param length The limiting length of the source char array */
    Variant(const char *text, int length = -1);

    /** String constructor.
     * @param text the source string to copy value from */
    Variant(const std::string &text);

    /** Vector3 constructor.
     * @param x the X part of the vector
     * @param y the Y part of the vector
     * @param z the Z part of the vector */
    Variant(float x, float y, float z);

    /** Quaternion constructor.
     * @param x the X part of the quaternion
     * @param y the Y part of the quaternion
     * @param z the Z part of the quaternion
     * @param w the W part of the quaternion */
    Variant(float x, float y, float z, float w);

    /** Vector3 constructor.
     * @param vec the source vector to copy value from */
    Variant(const Vector3 &vec);

    /** Quaternion constructor.
     * @param ori the source orientation quaternion to copy value from */
    Variant(const Quaternion &ori);

    /** Destructor */
    virtual ~Variant();

    /** returns a char* containing the type name */
    const char *typeString() const;

    /** returns a char* containing the type name for the specified type */
    static const char *typeToString(Type t);

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
    const Variant &operator=(const Variant &b);

    /** Asignment operator */
    const Variant &operator=(bool b);
    /** Asignment operator */
    const Variant &operator=(int i);
    /** Asignment operator */
    const Variant &operator=(uint u);
    /** Asignment operator */
    const Variant &operator=(float f);
    /** Asignment operator */
    const Variant &operator=(const char *s);
    /** Asignment operator */
    const Variant &operator=(const std::string &s);
    /** Asignment operator */
    const Variant &operator=(const Vector3 &v);
    /** Asignment operator */
    const Variant &operator=(const Quaternion &v);

    /** Type identifier. Returns the Type of the variant's value */
    Type type() const;

    /** Comparison operator. Comapares equality of the values.
     * Tries to convert the values to see if the values match even if those do
     * not have the same type */
    inline bool operator==(const Variant &b) const { return compare(b); }

    /** Comparison operator. Comapares non-equality of the values. @see
     * operator==()*/
    inline bool operator!=(const Variant &b) const { return !compare(b); }

    /** Base shared type for Variant */
    class SharedBase {
    public:
        SharedBase(){};
        virtual ~SharedBase(){};

        virtual SharedBase *clone() = 0;
    };

    /// convertion operation with target type specified as template parameter
    /// (e.g.: dv.as<int>());
    template <typename T> T as() const {
        OPDE_EXCEPT("Invalid Variant as<>() cast");
    }

    /** Templated shared type for Variant. Holds values for the shared types
     * (string,vector) */
    template <typename T> class Shared : public SharedBase {
    public:
        /// Constructor
        Shared(const T &_data) : SharedBase(), data(_data){};

        /// Copy constructor from same-typed shared
        Shared(const Shared<T> &src) : SharedBase(), data(src.data){};

        virtual ~Shared() {}

        virtual SharedBase *clone() {
            SharedBase *copy = new Shared<T>(data);

            return copy;
        };

        T data;
    };

    /** Private data holder. Holds either shared or non-shared value */
    struct Private {
        union Data {
            bool dbool;
            float dfloat;
            int dint;
            uint duint;
            SharedBase *shared;
        } data;

        Type type : 31;
        unsigned int isShared : 1;
    };

    // A const invalid dvariant. Can be used as a shortcut to be able to return
    // const dvariant references
    static const Variant INVALID;

protected:
    /** Helper conversion routine : Vector3 from string
     * Takes comma separated values (first 3, ignoring any further text)
     * @param str The string to be parsed
     * @throw runtime_error if unsupported format is encountered */
    static Vector3 StringToVector(const std::string &str);

    /** Helper conversion routine : Quaternion from string
     * Takes comma separated values (first 4 - w,x,y,z, ignoring any further
     * text)
     * @param str The string to be parsed
     * @throw runtime_error if unsupported format is encountered */
    static Quaternion StringToQuaternion(const std::string &str);

    /** Helper conversion routine : int from String
     * @param str the string to be parsed
     * @note Understands the 0xNUMBER hexadecimal format */
    static int StringToInt(const std::string &str);

    /** Helper conversion routine : uint from String
     * @param str the string to be parsed
     * @note Understands the 0xNUMBER hexadecimal format */
    static uint StringToUInt(const std::string &str);

    /** Helper conversion routine : Bool from String
     * Converts 1,yes,true to true, 0,no,false to false (ignoring case)
     * @throw runtime_error if unsupported text is encountered */
    static bool StringToBool(const std::string &str);

    /** Helper conversion routine : Float from String
     * @throw runtime_error if unsupported text is encountered */
    static float StringToFloat(const std::string &str);

    /** Cast the shared data to a specified type
     * @return reference to the type T if possible. */
    template <typename T> T &shared_cast() const;

    /** The comparison function. Is called by == and != operators */
    bool compare(const Variant &b) const;

    /** The data holder */
    Private mPrivate;
};

template <> bool Variant::as<bool>() const;
template <> float Variant::as<float>() const;
template <> int Variant::as<int>() const;
template <> uint Variant::as<uint>() const;
template <> std::string Variant::as<std::string>() const;
template <> Vector3 Variant::as<Vector3>() const;
template <> Ogre::Quaternion Variant::as<Ogre::Quaternion>() const;

/// Type traits for template to Variant conversions and various interactions
/// (serialization, etc.)
template <typename T> struct VariantTypeTraits {
    static const Variant::Type type = Variant::DV_INVALID;
};

template<>
struct VariantTypeTraits<bool> {
    static const Variant::Type type = Variant::DV_BOOL;
};

template<>
struct VariantTypeTraits<float> {
    static const Variant::Type type = Variant::DV_FLOAT;
};

template<>
struct VariantTypeTraits<int32_t> {
    static const Variant::Type type = Variant::DV_INT;
};

template<>
struct VariantTypeTraits<uint32_t> {
    static const Variant::Type type = Variant::DV_UINT;
};

template<>
struct VariantTypeTraits<std::string> {
    static const Variant::Type type = Variant::DV_STRING;
};

template<>
struct VariantTypeTraits<Vector3> {
    static const Variant::Type type = Variant::DV_VECTOR;
};

template<>
struct VariantTypeTraits<Quaternion> {
    static const Variant::Type type = Variant::DV_QUATERNION;
};

/// Map of string -> Variant values
typedef std::map<std::string, Variant> VariantStringMap;
} // namespace Opde

#endif
