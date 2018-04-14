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

#include "Serializer.h"

namespace Opde {

template <>
void TypeSerializer<bool>::serialize(FilePtr &dest, const void *valuePtr) {
    // bool always 4 byte long
    uint32_t conv = *static_cast<const bool *>(valuePtr) ? 1 : 0;
    dest->writeElem(&conv, sizeof(uint32_t));
};

template <>
void TypeSerializer<bool>::deserialize(FilePtr &src, void *valuePtr) {
    // bool always 4 byte long
    uint32_t conv;
    src->readElem(&conv, sizeof(uint32_t));

    *static_cast<bool *>(valuePtr) = conv ? true : false;
};

template <> size_t TypeSerializer<bool>::getStoredSize(const void *valuePtr) {
    return sizeof(uint32_t);
}

template <>
void TypeSerializer<Vector3>::serialize(FilePtr &dest, const void *valuePtr) {
    float x, y, z;

    x = static_cast<const Vector3 *>(valuePtr)->x;
    y = static_cast<const Vector3 *>(valuePtr)->y;
    z = static_cast<const Vector3 *>(valuePtr)->z;

    dest->writeElem(&x, sizeof(float));
    dest->writeElem(&y, sizeof(float));
    dest->writeElem(&z, sizeof(float));
};

template <>
void TypeSerializer<Vector3>::deserialize(FilePtr &src, void *valuePtr) {
    float x, y, z;

    src->readElem(&x, sizeof(float));
    src->readElem(&y, sizeof(float));
    src->readElem(&z, sizeof(float));

    Vector3 *result = static_cast<Vector3 *>(valuePtr);

    result->x = x;
    result->y = y;
    result->z = z;
};

template <>
size_t TypeSerializer<Vector3>::getStoredSize(const void *valuePtr) {
    return sizeof(float) * 3;
}

template <>
void TypeSerializer<Ogre::Quaternion>::serialize(FilePtr &dest,
                                                 const void *valuePtr) {
    int16_t h, p, b;

    Ogre::Matrix3 m;

    static_cast<const Ogre::Quaternion *>(valuePtr)->ToRotationMatrix(m);

    Ogre::Radian x, y, z;

    m.ToEulerAnglesZYX(z, y, x);

    h = static_cast<int16_t>(x.valueRadians() * 32768 / Ogre::Math::PI);
    p = static_cast<int16_t>(y.valueRadians() * 32768 / Ogre::Math::PI);
    b = static_cast<int16_t>(z.valueRadians() * 32768 / Ogre::Math::PI);

    dest->writeElem(&h, sizeof(int16_t));
    dest->writeElem(&p, sizeof(int16_t));
    dest->writeElem(&b, sizeof(int16_t));
};

template <>
void TypeSerializer<Ogre::Quaternion>::deserialize(FilePtr &src,
                                                   void *valuePtr) {
    int16_t h, p, b;

    src->readElem(&h, sizeof(int16_t));
    src->readElem(&p, sizeof(int16_t));
    src->readElem(&b, sizeof(int16_t));

    Ogre::Real x, y, z;

    /* Order of HPB application is crucial. It was detected that Dark orders HPB
    in this order:
    1. Bank is applied
    2. Pitch is applied
    3. Heading is applied
    */
    x = ((float)(h) / 32768) * Ogre::Math::PI; // heading - y
    y = ((float)(p) / 32768) * Ogre::Math::PI; // pitch - x
    z = ((float)(b) / 32768) * Ogre::Math::PI; // bank - z

    Ogre::Matrix3 m;

    // Still not there, but close
    m.FromEulerAnglesZYX(Ogre::Radian(z), Ogre::Radian(y), Ogre::Radian(x));

    Ogre::Quaternion *q = static_cast<Ogre::Quaternion *>(valuePtr);

    q->FromRotationMatrix(m);
};

template <>
size_t TypeSerializer<Ogre::Quaternion>::getStoredSize(const void *valuePtr) {
    return sizeof(int16_t) * 3;
}

template <>
void TypeSerializer<std::string>::serialize(FilePtr &dest,
                                            const void *valuePtr) {
    const std::string *str = static_cast<const std::string *>(valuePtr);

    uint32_t size = str->size();

    dest->writeElem(&size, sizeof(uint32_t));

    // write the data itself
    dest->write(str->c_str(), size);
};

template <>
void TypeSerializer<std::string>::deserialize(FilePtr &src, void *valuePtr) {
    uint32_t size;

    src->readElem(&size, sizeof(uint32_t));

    // prepare the string temp buffer
    char *str = new char[size + 1];

    str[size] = 0; // terminate to be sure

    src->read(str, size);

    std::string sobj(str);

    *static_cast<std::string *>(valuePtr) = sobj;

    delete[] str;
};

template <>
size_t TypeSerializer<std::string>::getStoredSize(const void *valuePtr) {
    return static_cast<const std::string *>(valuePtr)->size();
}
} // namespace Opde
