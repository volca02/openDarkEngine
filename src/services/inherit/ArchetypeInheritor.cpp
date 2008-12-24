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

#include "ArchetypeInheritor.h"
#include "logger.h"

using namespace std;

namespace Opde {
    /*--------------------------------------------------------*/
    /*--------------------- NeverInheritor -------------------*/
	/*--------------------------------------------------------*/
    ArchetypeInheritor::ArchetypeInheritor(const InheritorFactory* fac, InheritService* is) : CachedInheritor(fac, is) {
    };

    //------------------------------------------------------
    ArchetypeInheritor::~ArchetypeInheritor() {
    }

    //------------------------------------------------------
    bool ArchetypeInheritor::validate(int srcID, int dstID, unsigned int priority) const {
        // only inherit if target is < 0
        if (dstID < 0)
            return true;

        return false;
    }

    //------------------------------------------------------- Never Inheritor Factory:
    string ArchetypeInheritorFactory::mName = "archetype";

    ArchetypeInheritorFactory::ArchetypeInheritorFactory() {
    }

	string ArchetypeInheritorFactory::getName() const {
	    return mName;
	}

	Inheritor* ArchetypeInheritorFactory::createInstance(InheritService* is) const {
	    return new ArchetypeInheritor(this, is);
	}
}

