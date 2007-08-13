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

#ifndef __CACHEDINHERITOR_H
#define __CACHEDINHERITOR_H

#include "InheritService.h"

namespace Opde {
		/** Base class for cached inheritor implementations.
		 * This class implements some basic methods common to usual cached Inheritor implementations (refreshed every change, quick reads).
		 * To accomplish this, the inheritor registers as a listener to the InheritService, and upon a change, it refreshes the cache. */
		class CachedInheritor : public Inheritor, public InheritChangeListener {
				public:
					/// Constructor
					CachedInheritor(InheritServicePtr is) : mInheritService(is) {};

					/// Destructor
					~CachedInheritor() {};

					/// @see Inheritor::implements
					virtual void implements(int objID, bool impl);

					/// @see Inheritor::getEffectiveID
					virtual int getEffectiveID(int srcID) {
					}

					virtual bool refresh(int srcID) {
						// Look at the source objects, sorted by priority desc, assign the first valid
						// If our source changes, broadcast to all objects inheriting from this
					}

					/** Propagates new Effective source, recursively, on all inheritor targets.
					*
					* This method changes the records in Effective object cache. For each object, a list of the sources is queried. Then:
					* @li If the propagated effective ID is zero, a vote for new effective object is done (highest priority), querying parent objects for thei're effective ID's.
					* That ID is then recursively propagated.
					* @li If the propagated new effective ID is non-zero and if the source object ID matches to the effective parent,
					* the propagated new ID is taken as effective, and propagated to the child objects
					*
					* @param objID the object the change is propagated to
					* @param srcID the source of the propagation (a parent object ID)
					* @param newEffID the new effective object ID (or 0, if the propagation removes the inheritance source)
					*
					* @note The propagation stops (won't recurse) when:
					* @li The effective object is encountered as a target object (objID) - a cycle
					* @li The propagated effective ID encounters an object where the srcID propagation is masked ineffective
					* @li No targets are found to propagate to
					*/
					virtual void propagate(int objID, int srcID, int newEffID);

				protected:
					/// InheritService reference
					InheritServicePtr mInheritService;

					/// Map of effective object ID's
					typedef std::map < int, int > EffectiveObjectMap;

					/// Map of effective object ID's - instance
					EffectiveObjectMap mEffObjMap;
			};


		class CachedInheritorFactory : public InheritorFactory {
				public:
					CachedInheritorFactory();

					virtual void getName();

					InheritorPtr createInstance();
			};
	}

#endif
