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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __DTHELPERS_H
#define __DTHELPERS_H

/*
This file contains various dynamic datatype helpers, such as those for byte
swapping
*/

namespace Opde {
void swap_any(void *src, void *tgt, size_t size) {
    char *csrc = reinterpret_cast<char *>(src);
    char *cdest = reinterpret_cast<char *>(tgt);

    size_t pos;

    for (pos = 0; pos < size; ++pos) {
        cdest[size - pos - 1] = csrc[pos];
    }
}

// Following are the actual data manipulation routines to be used:
#ifdef __OPDE_BIG_ENDIAN
template <typename T> void writeLE(void *tgt, T src) {
    swap_any(&src, tgt, sizeof(T));
}

template <typename T> T readLE(void *src) {
    T res;

    swap_any(&src, &res, sizeof(T));

    return res;
}
#else
template <typename T> void writeLE(void *tgt, T src) {
    // no swapping
    *(reinterpret_cast<T *>(tgt)) = src;
}

template <typename T> T readLE(void *src) {
    // no swapping
    return *(reinterpret_cast<T *>(src));
}
#endif
} // namespace Opde

#endif // __DTHELPERS_H
