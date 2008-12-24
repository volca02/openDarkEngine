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

#ifndef __CACHEDINHERITOR_H
#define __CACHEDINHERITOR_H

#include "InheritService.h"
#include "Array.h"
#include "BitArray.h"

namespace Opde {
		/** Base class for cached inheritor implementations. Works self as "always" inheritor, inheriting in every situation.
		 * This class implements all methods common to usual cached Inheritor implementations (refreshed every change, quick reads).
		 * To accomplish this, the inheritor registers as a listener to the InheritService, and upon a change, it refreshes the cache. */
		class CachedInheritor : public Inheritor {
				public:
					/// Constructor
					CachedInheritor(const InheritorFactory* fac, InheritService* is);

					/// Destructor
					~CachedInheritor();

					/// @see Inheritor::setImplements
					virtual void setImplements(int objID, bool impl);

					/// @see Inheritor::getImplements
					virtual bool getImplements(int objID) const;

					/// @see Inheritor::getEffectiveID
					virtual int getEffectiveID(int srcID) const;

                    /** Dummy CachedInheritor validator. Always returns true. To be overriden to customize the inheritance schemas.
                   * @see Inheritor::validate */
                    virtual bool validate(int srcID, int dstID, unsigned int priority) const;

                    virtual void clear();

                    void onInheritMsg(const InheritChangeMsg& msg);
                    
                    void grow(int minID, int maxID);
				protected:
                    /** Sets new cached effective ID for the given object
                  * @param srcID the object to set effective ID for
                  * @param the new effective ID
                  * @return True if a change happened */
					virtual bool setEffectiveID(int srcID, int effID);

					/** Erases the effective ID record altogether, meaning that object has no effective value
                    * @param srcID the object to set effective ID for
                    * @param the new effective ID
                    * @return True if a change happened */
					virtual bool unsetEffectiveID(int srcID);

                    /** Propagates new Effective source, recursively, on all inheritor targets.
					*
					* This method changes the records in Effective object cache. For each object, a list of the sources is queried. Then:
					* @li A vote for new effective object is done (highest priority), querying parent objects for thei're effective ID's.
					* That ID is then recursively propagated, but only if a change happened to the effective ID of the Object.
					*
					* @param objID the source of the propagation (a parent object ID)
					*
					* @note The propagation stops (won't recurse) when:
					* @li The propagated effective ID encounters an object where the srcID propagation is masked ineffective
					* @li No targets are found to propagate to
					* @li No change hapened to the object's effective ID
					*/
					virtual bool refresh(int objID);

					/// @see Inheritor::valueChanged
					void valueChanged(int objID, const std::string& field, const DVariant& value);

					/// InheritService reference
					InheritService* mInheritService;

                    /// Inheritance change listener ID
					InheritService::ListenerID mListenerID;

					/// Map of effective object ID's
					typedef Array< int > EffectiveObjectMap;

					/// Map of effective object ID's - instance
					EffectiveObjectMap mEffObjMap;

					/// Map of implementing object IDs - id->true means object implements the inherited property
					typedef BitArray ImplementsMap;

					ImplementsMap mImplements;
			};


        /** Cached inheritor factory. The inheritor produced is named "always" and will, as the name suggests,
         * inherit in all situations. */
		class CachedInheritorFactory : public InheritorFactory {
				public:
					CachedInheritorFactory();

					virtual std::string getName() const;

					Inheritor* createInstance(InheritService* is) const;

                protected:
                    static std::string mName;
			};
	}

#endif
