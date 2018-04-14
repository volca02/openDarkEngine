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

#include "CachedInheritor.h"
#include "logger.h"

using namespace std;

namespace Opde {
/*---------------------------------------------------------*/
/*--------------------- CachedInheritor -------------------*/
/*---------------------------------------------------------*/
CachedInheritor::CachedInheritor(const InheritorFactory *fac,
                                 InheritService *is)
    : Inheritor(fac), mInheritService(is) {
    InheritService::ListenerPtr callback(
        new ClassCallback<InheritChangeMsg, CachedInheritor>(
            this, &CachedInheritor::onInheritMsg));

    mListenerID = mInheritService->registerListener(callback);
};

//------------------------------------------------------
CachedInheritor::~CachedInheritor() {
    mInheritService->unregisterListener(mListenerID);
}

//------------------------------------------------------
void CachedInheritor::setImplements(int objID, bool impl) {
    bool oldval = mImplements.setBit(objID, impl);

    // Did we change something?
    if (oldval != impl) {
        refresh(objID); // refresh the object ID and all the inheritance targets
    }
}

//------------------------------------------------------
bool CachedInheritor::getImplements(int objID) const {
    return mImplements[objID];
}

//------------------------------------------------------
int CachedInheritor::getEffectiveID(int srcID) const {
    return mEffObjMap[srcID];
}

//------------------------------------------------------
bool CachedInheritor::setEffectiveID(int srcID, int effID) {
    int prev = mEffObjMap[srcID];
    mEffObjMap[srcID] = effID;

    return prev != effID;
}

//------------------------------------------------------
bool CachedInheritor::unsetEffectiveID(int srcID) {
    return setEffectiveID(srcID, 0);
}

//------------------------------------------------------
bool CachedInheritor::refresh(int objID) {
    // First, get the old effective ID
    int oldEffID = getEffectiveID(objID);

    // Let's vote for a new effective object ID
    InheritQueryResultPtr sources = mInheritService->getSources(objID);

    int maxPrio = -1; // no inheritance indicator itself
    int newEffID = 0; // Detected new effective ID

    int niter = 0;
    InheritLinkPtr effective(NULL);

    // If self-implements
    if (getImplements(objID)) {
        // self, because prop on object masks any inherited prop, be it mp or
        // archetype
        newEffID = objID;
    } else {
        // does not self-implement...
        // now for each of the sources, find the one with the max. priority that
        // still implements. Some logic to accept the self assigned is also
        // present
        while (!sources->end()) {
            InheritLinkPtr il = sources->next();
            ++niter;

            int effID = getEffectiveID(
                il->srcID); // look for the effective ID of the source

            /*
            validate and compare to the maximal. If priority is greater,
            we have a new winner (but only if it validates and the source has
            some effective ID)
            */
            // Comparing the priority to signed int...
            if (validate(il->srcID, il->dstID, il->priority) &&
                (il->priority > maxPrio) && (effID != 0)) {
                effective = il;
                maxPrio = il->priority;
                newEffID = effID;
            }
        }
    }

    if (newEffID != 0) {
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

            LOG_VERBOSE(
                "Inheritance change happened on %d (new src %d, old src %d)",
                objID, newEffID, oldEffID);
            broadcastMessage(msg);
        }
    } else if (unsetEffectiveID(objID)) {
        InheritValueChangeMsg msg;

        msg.change = INH_VAL_REMOVED;

        msg.objectID = objID;
        msg.srcID = 0;

        LOG_VERBOSE("Inheritance removal happened on %d (old src %d)", objID,
                    oldEffID);
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
bool CachedInheritor::validate(int srcID, int dstID,
                               unsigned int priority) const {
    return true;
}

//------------------------------------------------------
void CachedInheritor::onInheritMsg(const InheritChangeMsg &msg) {
    if (msg.change == INH_CLEARED_ALL) {
        clear();
        return;
    }

    // Received an even about inheritance change. Must refresh target object of
    // such change
    refresh(msg.dstID);
}

//------------------------------------------------------
void CachedInheritor::grow(int minID, int maxID) {
    mEffObjMap.grow(minID, maxID);
    mImplements.grow(minID, maxID);
}

//------------------------------------------------------
void CachedInheritor::clear() {
    mEffObjMap.clear();
    mImplements.clear();
}

//------------------------------------------------------
void CachedInheritor::valueChanged(int objID, const std::string &field,
                                   const DVariant &value) {
    // TODO: search for all inheriting objects, broadcast for each
    InheritValueChangeMsg msg;

    msg.change = INH_VAL_FIELD_CHANGED;

    msg.objectID = objID;
    msg.srcID = 0;
    msg.field = field;
    msg.value = value;

    broadcastMessage(msg);
}

//------------------------------------------------------- Cached Inheritor
//Factory:
string CachedInheritorFactory::mName = "always";

CachedInheritorFactory::CachedInheritorFactory() {}

string CachedInheritorFactory::getName() const { return mName; }

Inheritor *CachedInheritorFactory::createInstance(InheritService *is) const {
    return new CachedInheritor(this, is);
}
} // namespace Opde
