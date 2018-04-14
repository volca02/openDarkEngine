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

#ifndef __SYMNAMEPROPERTYSTORAGE_H
#define __SYMNAMEPROPERTYSTORAGE_H

#include "SingleFieldDataStorage.h"

namespace Opde {

/** A Bi-Directional unique string Symbolic name storage for symbolic names.
 * Overrides the StringPropertyStorage. */
class SymNamePropertyStorage : public StringDataStorage {
protected:
    typedef std::map<std::string, int> ReverseNameMap;

    ReverseNameMap mReverseMap;

public:
    SymNamePropertyStorage();
    virtual ~SymNamePropertyStorage();

    virtual bool destroy(int objID);

    virtual bool setField(int objID, const std::string &field,
                          const DVariant &value);

    virtual bool _create(int objID, const std::string &text);

    virtual void clear();

    /// Reverse mapper. Get's id of object named 'name'
    int objectNamed(const std::string &name);

    /// Tester for name usage
    bool nameUsed(const std::string &name);
};

typedef shared_ptr<SymNamePropertyStorage> SymNamePropertyStoragePtr;
}; // namespace Opde

#endif
