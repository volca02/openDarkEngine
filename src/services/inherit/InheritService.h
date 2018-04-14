/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __INHERITSERVICE_H
#define __INHERITSERVICE_H

#include "config.h"

#include "InheritCommon.h"
#include "Iterator.h"
#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "SharedPtr.h"

namespace Opde {

struct InheritLink {
    /// Source object ID
    int srcID;
    /// Destination object ID
    int dstID;
    /// The priority of this inheritance definition (It's int to help comparison
    /// with negative values)
    int priority;
};

typedef std::vector<InheritLink> InheritLinkList;

typedef shared_ptr<InheritLink> InheritLinkPtr;

/** Inheritance query result. Put like this for effectiveness */
typedef ConstIterator<InheritLinkPtr> InheritQueryResult;

/// Shared pointer to InheritQuery
typedef shared_ptr<InheritQueryResult> InheritQueryResultPtr;

/** Inherit Service - service managing object inheritance and metaproperties.
 * This service is responsible for Inheritor management. Inheritors are classes
 * that are used to track inheritance of certain qualities of an object (be it
 * abstract or concrete). An example usage of Inheritor is property inheritance.
 * Properties internally use the inheritors to transparently return values not
 * directly assigned to the object ID, but rather inherited from the Effective
 * object. Effective object is the carrier of the effective value - the value
 * that has the maximal priority.
 * @note This class creates and uses the Metaproperty link, which is built-in
 * */
class OPDELIB_EXPORT InheritService : public ServiceImpl<InheritService>,
                                      public MessageSource<InheritChangeMsg> {
public:
    /// Constructor
    InheritService(ServiceManager *manager, const std::string &name);

    /// Destructor
    virtual ~InheritService();

    /** Registers an inheritor factory */
    void addInheritorFactory(const InheritorFactoryPtr &factory);

    /** Creates an inheritor instance, given the inheritor name
     * @return Inheritor pointer on success
     * @throw BasicException if the inheritor name was not found in factories */
    Inheritor *createInheritor(const std::string &name);

    /** Destroys the given inheritor (The inheritor has to be constructed via
     * createInheritor!)
     */
    void destroyInheritor(Inheritor *inh);

    /** Requests all sources for inheritance for the given object ID
     * @param objID the object id to get the Sources for */
    InheritQueryResultPtr getSources(int objID) const;

    /** Requests all inheritance targets for given object ID
     * @param objID the id of the object to get inheritance targets for
     * */
    InheritQueryResultPtr getTargets(int objID) const;

    /** Simple detector of inheritance target existance. Returns true if there
     * are objects inheriting from the given one
     */
    bool hasTargets(int objID) const;

    /** Sets an archetype for given object (MP link with priority 0)
     * @param objID the target object to set archetype for
     * @param archetypeID the id of the archetype object to link to
     * @note This should be only done on object creation. The method checks if
     * the object has archetype already set and excepts if so
     */
    void setArchetype(int objID, int archetypeID);

    /** Returns the archetype ID for the given object, or 0 if the object has no
     * archetype */
    int getArchetype(int objID) const;

    /** Adds a new mp link that causes the object to inherit from a metaproperty
     * object. Will use the first free priority possible.
     * @param objID the object id to add MP to
     * @param mpID the ID of the metaproperty to add */
    void addMetaProperty(int objID, int mpID);

    /** Removes a metaproperty from an object.
     * @param objID the object id to remove the MP from
     * @param mpID the ID of the metaproperty to remove */
    void removeMetaProperty(int objID, int mpID);

    /** Tester for object's inclusion of certain MP.
     * @param objID the object id to remove the MP from
     * @param mpID the ID of the metaproperty to remove
     * @return true if the object inherits from the given mpID with priority > 0
     * (==0 is Archetype inheritance!), false otherwise
     */
    bool hasMetaProperty(int objID, int mpID) const;

    /** A tester for object inheritance. Returns true if the given object
     * inherits, in any way, from the given source.
     * @param objID the object ID to look for
     * @param srcID the source object ID to test for inheritance on the objID
     * @return true if the objID inherits from srcID, false otherwise
     */
    bool inheritsFrom(int objID, int srcID) const;

    /// Clears out the inheritance map (leaves the other things intact)
    void clear();

    /// grows all the inheritors to be able to contain given range of object IDs
    void grow(int minID, int maxID);

    /// Map of object (src/dst) to inherit link
    typedef std::map<int, InheritLinkPtr> InheritLinkMap;

    /// Map of the effective object ID's
    typedef std::map<int, InheritLinkMap> InheritMap;

private:
    /** Service initialization
     * @see Service::init()
     */
    virtual bool init();

    /** Service bootstraping ended
     * @see Service::bootstrapFinished()
     */
    virtual void bootstrapFinished();

    /** Adds an inheritance link.
     * @param link The link to be added as InheritLink
     * @param priority The priority of the link to be added */
    void _addLink(const LinkPtr &link, unsigned int priority);

    /** Modifies the inheritance link priority
     * @param link The link to be modified
     * @param priority The priority of the link to be modified to */
    void _changeLink(const LinkPtr &link, unsigned int priority);

    /** Removes the given inheritance link */
    void _removeLink(const LinkPtr &link);

    /// Listener for the metaprop
    void onMetaPropMsg(const LinkChangeMsg &msg);

    /// Creates a new metaproperty link with the specified priority
    void _createMPLink(int objID, int srcID, int priority);

    /// Inheritance sources
    InheritMap mInheritSources;

    /// Inheritance destinations
    InheritMap mInheritTargets;

    ///  Map of named inheritor factories
    typedef std::map<std::string, InheritorFactoryPtr> InheritorFactoryMap;

    /// Map of inheritor factories
    InheritorFactoryMap mInheritorFactoryMap;

    /// Link (Relation metaproperty) listener registration ID
    Relation::ListenerID mMetaPropListenerID;

    /// Handle to the link service
    LinkServicePtr mLinkService;

    /// Direct link to the metaprop relation
    RelationPtr mMetaPropRelation;

    /// List of instanced inheritors
    typedef std::vector<Inheritor *> InheritorList;

    /// All instanced inheritors are here
    InheritorList mInheritors;
};

/// Shared pointer to Inherit service
typedef shared_ptr<InheritService> InheritServicePtr;

/// Factory for the InheritService objects
class OPDELIB_EXPORT InheritServiceFactory : public ServiceFactory {
public:
    InheritServiceFactory();
    ~InheritServiceFactory(){};

    /** Creates a InheritService instance */
    Service *createInstance(ServiceManager *manager);

    virtual const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
