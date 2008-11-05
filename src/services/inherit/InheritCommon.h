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

/*! 
  \file InheritCommon.h
  \brief Common declarations for the inherit service
  \ingroup InheritService
 */

#ifndef __INHERITORCOMMON_H
#define __INHERITORCOMMON_H

#include "SharedPtr.h"
#include "LinkService.h"

namespace Opde {
	// Forward declaration
	class InheritService;

    /** Inheritance value change message types.
    * This is not a converted version of the link message as InheritChangeType,
    * but rather the description of what that change caused to the implemented values on object ID's
    */
	typedef enum {
		/// Object gained an inherited value, did not implement before
		INH_VAL_ADDED = 1,
		/// Inheritance of the value was canceled (object stopped implementing a value)
		INH_VAL_REMOVED,
		/// Object changed the inheritance source, did implement before
		INH_VAL_CHANGED,
		/// Object changed a certain field's value (effectively - not masked by metaprops), inheritance did not change
		INH_VAL_FIELD_CHANGED
	} InheritValueChangeType;

	/// Inheritance change message
	typedef struct InheritValueChangeMsg {
		/// A change that happened
		InheritValueChangeType change;
		/// An ID of the affected object
		int objectID;
		/// An ID of the new source, or zero if the inherited value has been removed (ignore for INH_VAL_FIELD_CHANGED)
		int srcID;
		/// A field name of the change (only valid for INH_VAL_FIELD_CHANGED)
		std::string field;
		/// A new value of the field (only valid for INH_VAL_FIELD_CHANGED)
		DVariant value;
	};

	/** Inheritor interface
	 * Inheritors are used to query effective object id
	 * (the id of an object holding effective value)
	 * given an object id.
	 * The inheritor is a source of the implementation messages.
	 * This means that all the objects that are changed by the inheritance modification can be notified through the listener */
	class Inheritor : public MessageSource<InheritValueChangeMsg> {
		public:
            /// default destructor
			virtual ~Inheritor() {};
		
			/** Returns true if the objID has true 'implements' record */
			virtual bool getImplements(int objID) const = 0;

			/** Manual method for setting that a certain object ID directly implements (or not) the inherited quality */
            virtual void setImplements(int objID, bool impl) = 0;

			/** Returns the effective object ID for the given object ID
			 * @note The effective object is the object holding the used value for the object ID given
			 * @return The effective object ID, or 0, if no effective is found (the object itself is effective) */
			virtual int getEffectiveID(int srcID) const = 0;

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
			virtual bool validate(int srcID, int dstID, unsigned int priority) const = 0;

			/// Used upon total cleanout of the database
			virtual void clear() = 0;
			
			/// grows the internal tables of the inheritor to allow the storage of data about defined object range
			virtual void grow(int minID, int maxID) {};

			/// on a value change, the inheritor propagates the field and the new value to all affected objects
			virtual void valueChanged(int objID, const std::string& field, const DVariant& value) = 0;
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

	class InheritorFactory {
		public:
			InheritorFactory() {};
			
			virtual ~InheritorFactory() {};

			virtual std::string getName() const = 0;

			virtual InheritorPtr createInstance(InheritService* is) const = 0;
	};

	typedef shared_ptr< InheritorFactory > InheritorFactoryPtr;

}

#endif
