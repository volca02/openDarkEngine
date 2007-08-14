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

#include "NeverInheritor.h"
#include "logger.h"

using namespace std;

namespace Opde {
    /*--------------------------------------------------------*/
    /*--------------------- NeverInheritor -------------------*/
	/*--------------------------------------------------------*/
    NeverInheritor::NeverInheritor(InheritService* is) : mInheritService(is) {
    };

    //------------------------------------------------------
    NeverInheritor::~NeverInheritor() {
    }

    //------------------------------------------------------
	void NeverInheritor::setImplements(int objID, bool impl) {
	    ImplementsMap::iterator it = mImplements.find(objID);
        bool modified = false;

        if (impl) {

            if (it != mImplements.end()) {
                if (!it->second) {
                    it->second = true; // just set the new value
                    modified = true;
                }
            } else {
               mImplements.insert(make_pair(objID, impl)); // insert if non-present
               modified = true;
            }

        } else { // false - only remove if present

            if (it != mImplements.end()) {
                mImplements.erase(it);
                modified = true;
            }

        }
	}

	//------------------------------------------------------
	bool NeverInheritor::getImplements(int objID) const {
	    ImplementsMap::const_iterator it = mImplements.find(objID);

        if (it != mImplements.end()) {
            return it->second;
        } else {
            // does not have a record, so does not implement
            return false;
        }
	}

    //------------------------------------------------------
    int NeverInheritor::getEffectiveID(int srcID) const {
        // look for the implements. return 0 if not found, srcID otherwise
        ImplementsMap::const_iterator it = mImplements.find(srcID);

        if (it != mImplements.end()) {
            return srcID;
        } else {
            // does not have a record, so does not implement ever
            return 0;
        }

    }

    //------------------------------------------------------
    void NeverInheritor::clear() {
        mImplements.clear();
    }

    //------------------------------------------------------
    bool NeverInheritor::validate(int srcID, int dstID, unsigned int priority) const {
        return false;
    }

    //------------------------------------------------------- Never Inheritor Factory:
    string NeverInheritorFactory::mName = "never";

    NeverInheritorFactory::NeverInheritorFactory() {
    }

	string NeverInheritorFactory::getName() const {
	    return mName;
	}

	InheritorPtr NeverInheritorFactory::createInstance(InheritService* is) const {
	    return new NeverInheritor(is);
	}
}

