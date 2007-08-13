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


#ifndef __INHERITORCOMMON_H
#define __INHERITORCOMMON_H

#include "SharedPtr.h"
#include "LinkService.h"

namespace Opde {
	// Forward declaration
	class InheritService;

	/** Inheritor interface
	 * Inheritors are used to query effective object id
	 * (the id of an object holding effective value)
	 * given an object id. */
	class Inheritor {
		public:
			/** Manual method for setting that a certain object ID directly implements (or not) the inherited quality */
			virtual void implements(int objID, bool impl) = 0;

			/** Returns the effective object ID for the given object ID
			 * @note The effective object is the object holding the used value for the object ID given
			 * @return The effective object ID */
			virtual int getEffectiveID(int srcID) = 0;

			/** The core inheritance describing method.
			 * This method is used to validate the given inheritance situation to be acceptible or not.
			 * An example:
			 * @code
			 * // Only inherit on abstract objects
			 * bool MyInheritor::validate(int srcID, int dstID, unsigned int priority) {
			 *    if (dstID > 0)
			 *         return false;
			 *
			 *      return true;
			 * }
			 *
			 * // Don't inherit on metaproperty assignments
			 * bool My2Inheritor::validate(int srcID, int dstID, unsigned int priority) {
			 *    if (priority > 0)
			 *         return false;
			 *
			 *      return true;
			 * }
			 * @endcode
			 * @note For realy special kind of inheritors, overriding the refresh method is a good alternative
			 * @param srcID The source object ID
			 * @param dstID The destination object ID
			 * @return true if the source is valid, false if the source is not valid inheritance source */
			virtual bool validate(int srcID, int dstID, unsigned int priority) = 0;

			/// Used upon total cleanout of the database
			virtual void clear();
	};

	/// Shared pointer to Inheritor
	typedef shared_ptr< Inheritor > InheritorPtr;

	/// Inheritance change types
	typedef enum {
		/// Inherit. source was added (Sent after the addition)
		INH_ADDED = 1,
		/// Inherit. was removed (Sent before the removal)
		INH_REMOVED,
		/// Inherit. source changed priority
		INH_CHANGED,
		/// The inheritance data were erased
		INH_CLEARED_ALL
	} InheritChangeType;

	/// Inheritance change message
	typedef struct InheritChangeMsg {
		/// A change that happened
		InheritChangeType change;
		/// An ID of src object of the inheritance
		int srcID;
		/// An ID of the dst object of the inheritance
		int dstID;
		/// Priority of the inheritance
		unsigned int priority;
	};

	/** Inheritance change listener abstract class.
	* This class is to be inherited by classes wanting to listen to Inheritor messages */
	class InheritChangeListener {};

	/// Callback method declaration
	typedef void (InheritChangeListener::*InheritChangeMethodPtr)(const InheritChangeMsg& msg);

	/// Listener pair. InheritChangeListener
	typedef struct InheritChangeListenerPtr {
		InheritChangeListener* listener;
		InheritChangeMethodPtr method;
	};

	class InheritorFactory {
		public:
			InheritorFactory() {};

			virtual void getName() = 0;

			virtual InheritorPtr createInstance() = 0;
	};

	typedef shared_ptr< InheritorFactory > InheritorFactoryPtr;

}

#endif
