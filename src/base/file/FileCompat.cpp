/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

/** @file FileCompat.cpp
 * @brief A various utility methods for File class usage - implementation
 */

#include "config.h"

#include "FileCompat.h"
#include <OgreMath.h>
#include <OgreMatrix3.h>

namespace Opde {
// Vector2
File &operator<<(File &st, const Vector2 &val) {
    st << val.x << val.y;
    return st;
}

File &operator>>(File &st, Vector2 &val) {
    st >> val.x >> val.y;
    return st;
}

// Vector3
File &operator<<(File &st, const Vector3 &val) {
    st << val.x << val.y << val.z;
    return st;
}

File &operator>>(File &st, Vector3 &val) {
    st >> val.x >> val.y >> val.z;
    return st;
}

// Plane
File &operator<<(File &st, const Plane &val) {
    st << val.normal << val.d;
    return st;
}

File &operator>>(File &st, Plane &val) {
    st >> val.normal >> val.d;
    return st;
}

File &operator<<(File &st, const Quaternion &val) {
    int16_t xi, yi, zi;

    Ogre::Matrix3 m;

    val.ToRotationMatrix(m);

    Ogre::Radian x, y, z;

    m.ToEulerAnglesZYX(z, y, x);

    xi = static_cast<int16_t>(x.valueRadians() * 32768 / Ogre::Math::PI);
    yi = static_cast<int16_t>(y.valueRadians() * 32768 / Ogre::Math::PI);
    zi = static_cast<int16_t>(z.valueRadians() * 32768 / Ogre::Math::PI);

    st << xi << yi << zi;

    return st;
}

File &operator>>(File &st, Quaternion &val) {
    int16_t xi, yi, zi;
    float x, y, z;

    st >> xi >> yi >> zi;

    x = ((float)(xi) / 32768) * Ogre::Math::PI;
    y = ((float)(yi) / 32768) * Ogre::Math::PI;
    z = ((float)(zi) / 32768) * Ogre::Math::PI;

    Ogre::Matrix3 m;

    // Still not there, but close
    m.FromEulerAnglesZYX(Ogre::Radian(z), Ogre::Radian(y), Ogre::Radian(x));
    Quaternion q;
    val.FromRotationMatrix(m);

    return st;
}

} // namespace Opde
