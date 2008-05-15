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

#include "CachedInheritor.h"
#include "logger.h"

using namespace std;

namespace Opde {
    /*---------------------------------------------------------*/
    /*--------------------- CachedInheritor -------------------*/
	/*---------------------------------------------------------*/
    CachedInheritor::CachedInheritor(InheritService* is) : mInheritService(is) {
		InheritService::ListenerPtr callback = new
			ClassCallback<InheritChangeMsg, CachedInheritor>(this, &CachedInheritor::onInheritMsg);

		mListenerID = mInheritService->registerListener(callback);
    };

    //------------------------------------------------------
    CachedInheritor::~CachedInheritor() {
        mInheritService->unregisterListener(mListenerID);
    }

    //------------------------------------------------------
	void CachedInheritor::setImplements(int objID, bool impl) {
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

        // Did we change something?
        if (modified) {
            refresh(objID); // refresh the object ID and all the inheritance targets
        }
	}

	//------------------------------------------------------
	bool CachedInheritor::getImplements(int objID) const {
	    ImplementsMap::const_iterator it = mImplements.find(objID);

        if (it != mImplements.end()) {
            return it->second;
        } else {
            // does not have a record, so does not implement
            return false;
        }
	}

    //------------------------------------------------------
    int CachedInheritor::getEffectiveID(int srcID) const {
        EffectiveObjectMap::const_iterator it = mEffObjMap.find(srcID);

        if (it != mEffObjMap.end()) {
            return it->second;
        } else
            return 0; // Can't be self, as that is also recorded
    }

    //------------------------------------------------------
    bool CachedInheritor::setEffectiveID(int srcID, int effID) {
		std::pair<EffectiveObjectMap::iterator, bool> result = mEffObjMap.insert(make_pair(srcID, effID));    	
		
		if (result.second) {
			// did insert, was not already there
			return true;
		} else {
			// did not insert, already was there.
			if (result.first->second != effID) {
				result.first->second = effID;
				return true;
			}
			
			return false;
		}
    }

    //------------------------------------------------------
    bool CachedInheritor::unsetEffectiveID(int srcID) {
        EffectiveObjectMap::iterator it = mEffObjMap.find(srcID);
        bool changed = false;

        if (it != mEffObjMap.end()) {
            changed = true;
            mEffObjMap.erase(it);
        }

        return changed;
    }

    //------------------------------------------------------
    bool CachedInheritor::refresh(int objID) {
        // First, get the old effective ID
        int oldEffID = getEffectiveID(objID);

        // Let' vote for a new effective object ID
        InheritQueryResultPtr sources = mInheritService->getSources(objID);

        int maxPrio = -1; // no inheritance indicator itself
        int newEffID = 0; // Detected new effective ID

        int niter = 0;
        InheritLinkPtr effective(NULL);

        // now for each of the sources, find the one with the max. priority that still implements.
        // Some logic to accept the self assigned is also present
        while (!sources->end()) {
            InheritLinkPtr il = sources->next();
            ++niter;

            int effID = getEffectiveID(il->srcID); // look for the effective ID of the source

            /*
            validate and compare to the maximal. If priority is greater,
            we have a new winner (but only if it validates and the source has some effective ID)
            */
            // Comparing the priority to signed int...
            if (validate(il->srcID, il->dstID, il->priority) && (il->priority > maxPrio) && (effID != 0)) {
                effective = il;
                maxPrio = il->priority;
                newEffID = effID;
            }
        }


        // If self-implements
        if (getImplements(objID)) {

            // If there is no source supply, set self. Respect priority>0 as metaprop
            if (effective.isNull() || maxPrio <= 0) {
                newEffID = objID; // self, because we mask 0 prio inh. or no source was found
            }

        }

        if (newEffID != 0)  {
            if (setEffectiveID(objID, newEffID)) {
                // Broadcast the change to a new ID
                InheritValueChangeMsg msg;

                if (oldEffID != 0) {
                    msg.change = INH_VAL_CHANGED;
                } else {
                    msg.change = INH_VAL_ADDED;
                }


                msg.objectID = objID;
                msg.srcID = newEffID;

				LOG_VERBOSE("Inheritance change happened on %d (new src %d, old src %d)", objID, newEffID, oldEffID);
                broadcastMessage(msg);
            }
        }
        else
            if (unsetEffectiveID(objID)) {
                InheritValueChangeMsg msg;

                msg.change = INH_VAL_REMOVED;

                msg.objectID = objID;
                msg.srcID = 0;


				LOG_VERBOSE("Inheritance removal happened on %d (old src %d)", objID, oldEffID);
                broadcastMessage(msg);
            }

        // If there was a change, propagate
        if (newEffID != oldEffID) {
            InheritQueryResultPtr targets = mInheritService->getTargets(objID);

            while (!targets->end()) {
                InheritLinkPtr il = targets->next();

                refresh(il->dstID); // refresh the target object
            }
        }
		return false;
    }

    //------------------------------------------------------
    bool CachedInheritor::validate(int srcID, int dstID, unsigned int priority) const {
        return true;
    }

    //------------------------------------------------------
    void CachedInheritor::onInheritMsg(const InheritChangeMsg& msg) {
        // Received an even about inheritance change. Must refresh target object of such change
        refresh(msg.dstID);
    }

    //------------------------------------------------------
    void CachedInheritor::clear() {
        mEffObjMap.clear();
        mImplements.clear();
    }

    //------------------------------------------------------- Cached Inheritor Factory:
    string CachedInheritorFactory::mName = "always";

    CachedInheritorFactory::CachedInheritorFactory() {
    }

	string CachedInheritorFactory::getName() const {
	    return mName;
	}

	InheritorPtr CachedInheritorFactory::createInstance(InheritService* is) const {
	    return new CachedInheritor(is);
	}
}

