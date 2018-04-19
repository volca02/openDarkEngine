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

/**
 @file DarkCommon.h
 @brief Common data types used throughout the entire engine. Here, these are
 mainly used for disk access.
 */

#ifndef __DARKCOMMON_H
#define __DARKCOMMON_H

#include <memory>

namespace Opde {

typedef size_t MessageListenerID;

/// Palette type specifier
enum PaletteType {
    ePT_Default = 0, /// Default palette
    ePT_DefaultBook, /// Palette from accompanying BOOK.PCX
    ePT_PCX,         /// PCX file palette
    ePT_External     /// External palette
};

class File;

using FilePtr = std::shared_ptr<File>;

}

#endif
