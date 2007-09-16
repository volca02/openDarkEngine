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


#ifndef __INHERITSERVICE_H
#define __INHERITSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "InheritCommon.h"
#include "Iterator.h"

namespace Opde {

	typedef struct InheritLink {
		/// Source object ID
		int 	srcID;
		/// Source object ID
		int 	dstID;
		/// The priority of this inheritance definition (It's int to help comparison with negative values)
		int	priority;
	};

	typedef std::vector< InheritLink > InheritLinkList;

    typedef shared_ptr< InheritLink > InheritLinkPtr;

    /** Inheritance query result. Put like this for effectiveness */
    typedef ConstIterator< InheritLinkPtr > InheritQueryResult;

    /// Shared pointer to InheritQuery
    typedef shared_ptr< InheritQueryResult > InheritQueryResultPtr;

	/** Inherit Service - service managing object inheritance and metaproperties.
	* This service is responsible for Inheritor management. Inheritors are classes that are used to track inheritance of certain qualities of an object (be it abstract or concrete).
	*  An example usage of Inheritor is property inheritance. Properties internally use the inheritors to transparently return values not directly assigned to the object ID, but rather inherited from the Effective object.
	* Effective object is the carrier of the effective value - the value that has the maximal priority.
	* */
	class InheritService : public Service, public MessageSource<InheritChangeMsg> {
		public:
			/// Constructor
			InheritService(ServiceManager *manager);

			/// Destructor
			virtual ~InheritService();

            /// Service initialization - @see Service::init()
			virtual void init();

			/** Registers an inheritor factory */
			void addInheritorFactory(InheritorFactoryPtr factory);

			/** Creates an inheritor instance, given the inheritor name
			 * @return Inheritor pointer on success
			 * @throw BasicException if the inheritor name was not found in factories */
			InheritorPtr createInheritor(const std::string& name);

			/** Requests all sources for inheritance for the given object ID
			 * @param objID the object id to get the Sources for */
			InheritQueryResultPtr getSources(int objID) const;

			/** Requests all inheritance targets for given object ID
			 * @param objID the id of the object to get inheritance targets for
			 * */
			InheritQueryResultPtr getTargets(int objID) const;

			/// Todo: AddMetaProp, RemoveMetaProp, ...

            /// Clears out the inheritance map (leaves the other things intact)
            void clear();

            /// Map of object (src/dst) to inherit link
            typedef std::map< int, InheritLinkPtr > InheritLinkMap;

			/// Map of the effective object ID's
			typedef std::map< int, InheritLinkMap > InheritMap;

		private:
            /** Adds an inheritance link.
            * @param link The link to be added as InheritLink
            * @param priority The priority of the link to be added */
            void _addLink(LinkPtr link, unsigned int priority);

            /** Modifies the inheritance link priority
            * @param link The link to be modified
            * @param priority The priority of the link to be modified to */
            void _changeLink(LinkPtr link, unsigned int priority);

            /** Removes the given inheritance link */
            void _removeLink(LinkPtr link);

			/// Listener for the metaprop
			void onMetaPropMsg(const LinkChangeMsg& msg);

      		/// Inheritance sources
			InheritMap mInheritSources;

			/// Inheritance destinations
			InheritMap mInheritTargets;

			///  Map of named inheritor factories
			typedef std::map< std::string, InheritorFactoryPtr > InheritorFactoryMap;

			/// Map of inheritor factories
			InheritorFactoryMap mInheritorFactoryMap;

			/// Link (Relation metaproperty) listener registration ID
			Relation::ListenerID mMetaPropListenerID;

			/// Handle to the link service
			LinkServicePtr mLinkService;

			/// Direct link to the metaprop relation
			RelationPtr mMetaPropRelation;

            /// List of instanced inheritors
			typedef std::vector< InheritorPtr > InheritorList;

            /// All instanced inheritors are here
			InheritorList mInheritors;
	};

	/// Shared pointer to Link service
	typedef shared_ptr< InheritService > InheritServicePtr;

	/// Factory for the LinkService objects
	class InheritServiceFactory : public ServiceFactory {
		public:
			InheritServiceFactory();
			~InheritServiceFactory() {};

			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

		private:
			static std::string mName;
	};
}


#endif
