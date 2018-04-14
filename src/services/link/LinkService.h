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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __LINKSERVICE_H
#define __LINKSERVICE_H

#include "config.h"

#include "BitArray.h"
#include "FileGroup.h"
#include "LinkCommon.h"
#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "Relation.h"
#include "SharedPtr.h"
#include "database/DatabaseService.h"

namespace Opde {

/** @brief Link service - service managing in-game object links
 */
class OPDELIB_EXPORT LinkService : public ServiceImpl<LinkService> {
public:
    LinkService(ServiceManager *manager, const std::string &name);

    virtual ~LinkService();

    /** Sets the Relation chunk version */
    void setChunkVersion(uint major, uint minor) {
        mRelVMaj = major;
        mRelVMin = minor;
    }

    /** Convert the relation name to a flavor
     * @param name the relation name
     * @return int The relation id (Flavor), or zero if not found */
    int nameToFlavor(const std::string &name);

    /** Convert the relation name to a flavor
     * @param name the relation name
     * @return int The relation id (Flavor), or empty string if not found */
    std::string flavorToName(int flavor);

    /** Creates a relation type (link kind)
     * @param name The relation name
     * @param stor The data storage to use
     * @param hidden The hidden relations (true) will not show up on public link
     * list places */
    RelationPtr createRelation(const std::string &name,
                               const DataStoragePtr &stor, bool hidden);

    /** Get relation given it's name
     * @param name The relation's name
     * @return A shared_pointer to the relation, or NULL if not found
     * @note The relation will be .isNull() if it was not found
     */
    RelationPtr getRelation(const std::string &name);

    /** Get relation given it's flavor
     * @param flavor The relation's flavor
     * @return A shared_pointer to the relation, or NULL if not found
     * @note The relation will be .isNull() if it was not found
     */
    RelationPtr getRelation(int flavor);

    /** A notification that object was destroyed (removes all links targetted or
     * pointed from the obj. ID)
     * @param id The object id that was removed
     * @note Do NOT call this directly unless you know what it does
     */
    void objectDestroyed(int id);

    /** load links from a single database
     * @param db The database to load from
     * @param objMask the mask of object id's to allow - both src and dst
     * objects have to be here, otherwise link gets ignored */
    void load(const FileGroupPtr &db, const BitArray &objMask);

    /** Saves the links and link data according to the saveMask */
    void save(const FileGroupPtr &db, uint saveMask);

    /** Clears all the data and the relation mappings */
    void clear();

    // --- link queries ---
    /** @see Relation::getAllLinks
    @param flavor The link flavor (relation type).
    @param src The source object ID
    @param dst The destination object ID
    @return LinkQueryResultPtr Link iterator for resulting links
    @throw If invalid flavor is specified, this method will return an empty
    iterator */
    LinkQueryResultPtr getAllLinks(int flavor, int src, int dst) const;

    /** @see Relation::getAllInherited
    @param flavor The link flavor (relation type).
    @param src The source object ID
    @return LinkQueryResultPtr Link iterator for resulting links
    @throw If invalid flavor is specified, this method will return an empty
    iterator */
    LinkQueryResultPtr getAllInherited(int flavor, int src, int dst) const;

    /** @see Relation::getOneLink
    @param flavor The link flavor (relation type).
    @param src The source object ID
    @param dst The destination object ID
    @return LinkPtr Link structure for the resulting link (or NULL if none
    found)
    @throw If invalid flavor is specified, this method will return a NULL link
    (invalid) */
    LinkPtr getOneLink(int flavor, int src, int dst) const;

    /** @see Relation::getLink */
    LinkPtr getLink(link_id_t id) const;

    /// Name to Relation instance. The primary storage of Relation instances.
    typedef std::map<std::string, RelationPtr> RelationNameMap;

    /** @returns a link name iterator over all link type names (both inverse and
     * normal ones) */
    StringIteratorPtr getAllLinkNames();

    /** A shortcut to Relation::getFieldDescIterator
     * @param flavor The flavor of the relation
     * @return the iterator over property field descriptions, or NULL if invalid
     * name was specified
     * @see Relation::getFieldDescIterator
     */
    const DataFields &getFieldDesc(int flavor);

protected:
    bool init();
    void bootstrapFinished();
    void shutdown();

    /** request a mapping Name->Flavor and reverse
     * @param id The flavor value requested
     * @param name The name for that flavor (Relation name)
     * @param rel The relation instance to associate with that id
     * @return false if conflict happened, true if all went ok, and new mapping
     * is inserted (or already was registered)
     */
    bool requestRelationFlavorMap(int id, const std::string &name,
                                  RelationPtr &rel);

    typedef std::map<int, std::string> FlavorToName;
    typedef std::map<std::string, int> NameToFlavor;

    /// ID to Relation instance. Secondary storage of Relation instances, mapped
    /// per request when loading
    typedef std::map<int, RelationPtr> RelationIDMap;

    FlavorToName mFlavorToName;
    NameToFlavor mNameToFlavor;
    RelationIDMap mRelationIDMap;
    RelationNameMap mRelationNameMap;

    /// Relations chunk versions
    uint mRelVMaj, mRelVMin;

    /// Database service
    DatabaseServicePtr mDatabaseService;
};

/// Shared pointer to Link service
typedef shared_ptr<LinkService> LinkServicePtr;

/// Factory for the LinkService objects
class OPDELIB_EXPORT LinkServiceFactory : public ServiceFactory {
public:
    LinkServiceFactory();
    ~LinkServiceFactory(){};

    /** Creates a LinkService instance */
    Service *createInstance(ServiceManager *manager);

    virtual const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
