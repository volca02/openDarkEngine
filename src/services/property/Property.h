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

#ifndef __PROPERTYGROUP_H
#define __PROPERTYGROUP_H

#include "config.h"

#include "BitArray.h"
#include "DTypeDef.h"
#include "DataStorage.h"
#include "FileGroup.h"
#include "NonCopyable.h"
#include "OpdeException.h"
#include "PropertyCommon.h"
#include "inherit/InheritService.h"
#include "logger.h"

namespace Opde {
// forward decl.
class PropertyService;

/** @brief Property - a collection of per-object values (or value sets).
 * Property holds all the properties of the same kind for all the objects.
 * Properties contain a set of values via key->value mapping.
 */
class OPDELIB_EXPORT Property : public NonCopyable,
                                public MessageSource<PropertyChangeMsg> {
public:
    /** Property Constructor
     * @param owner The owner property service
     * @param name The property name
     * @param chunk_name The name of the chunk (without the P$)
     * @param storage The property storage created to hold the data
     */
    Property(PropertyService *owner, const std::string &name,
             const std::string &chunk_name, const DataStoragePtr &storage,
             std::string inheritorName);

    /** Setter for the property chunk version */
    inline void setChunkVersions(uint verMaj, uint verMin) {
        mVerMaj = verMaj;
        mVerMin = verMin;
    };

    /** gets the major version of the chunk */
    inline uint getChunkVersionMajor(void) { return mVerMaj; };

    /** gets the minor version of the chunk */
    inline uint getChunkVersionMinor(void) { return mVerMin; };

    /** Destructor */
    virtual ~Property();

    /** Shutdown routine. All dependencies must be be released here */
    virtual void shutdown();

    /// Sets this property's flag mBuiltIn to true, meaning this Property was
    /// created by code as a core property
    inline void setBuiltin() { mBuiltin = true; };

    /// Gets the builtin flag
    inline bool getBuiltin() { return mBuiltin; };

    /// Name getter. Returns the name of this property
    const std::string &getName() { return mName; };

    /** Determines whether an object with id obj_id stores or inherits property
     * of this type
     * @param obj_id The object id of the object
     * @return true if the object has the given property
     */
    bool has(int obj_id) const {
        // Get the effective id for the object
        int eoid = _getEffectiveObject(obj_id);

        if (eoid == 0)
            return false;

        // This code could be removed, if we knew the nonzero eoid will always
        // mean prop exists
        return owns(eoid);
    }

    /** Determines whether an object with id obj_id stores property of this type
     * @param obj_id The object id of the object
     * @return true if the object has the given property
     * @note Will return false if the object only inherits the property, but
     * does not own it (use has for that)
     */
    bool owns(int obj_id) const { return mPropertyStorage->has(obj_id); }

    /** Loads properties from a file group
     * @param db The database to load from
     * @param objMask The BitArray of objects to be loaded (other properties are
     * skipped)
     */
    void load(const FileGroupPtr &db, const BitArray &objMask);

    /** Saves properties to a file group
     * @param db The database to save to
     * @param objMask The BitArray of objects to be saved
     */
    void save(const FileGroupPtr &db, const BitArray &objMask);

    /** Clears the whole property (all data on all objects)
     * Clears out all the properties, and broadcasts PROP_CLEARED
     */
    virtual void clear();

    /** Creates a property for given object ID, using the default values for the
     * property fields
     * @param obj_id The id of the object to create the property for
     * @return true if the property was created, false if the object ID already
     * holds the given property
     * @note Broadcasts a PROP_ADDED on success
     * @note Notifies inheritor about the change */
    bool createProperty(int obj_id);

    /** Creates a property for given object ID
     * @param obj_id The id of the object to create the property for
     * @return true if the property was removed, false if the object ID didn't
     * hold the property
     * @note Broadcasts a PROP_REMOVED on success
     * @note Notifies inheritor about the change */
    bool removeProperty(int obj_id);

    /** Creates a new property, or replaces all the values on the current, by
     * cloning a given property on object ID
     * @param obj_id Target object ID (id to create)
     * @param src_id The id of the object to clone property from */
    bool cloneProperty(int obj_id, int src_id);

    // ----------------- Data manipulation related methods --------------------
    /** Direct data setter.
     * @param id object id
     * @param field the field name
     * @param value New value for the field
     * @return true if the change was sucessful
     * @see owns
     * @note Will log error when object id does not own the property to be
     * changed */
    virtual bool set(int id, const std::string &field, const DVariant &value);

    /** Direct data getter
     * @param id object id
     * @param field the field name
     * @param target The target value holder to be filled from the field
     * indicated, or untouched if field invalid
     * @see owns
     * @return false if field name was invalid, true if value was set in target
     */
    virtual bool get(int id, const std::string &field, DVariant &target);

    /** Notification that an object was destroyed. @see
     * PropertyService::objectDestroyed */
    void objectDestroyed(int id);

    /** @return A reference to const DataFieldDesc iterator, usable for data
     * description, automatic gui composition, etc. Internally, this is just a
     * wrapper around getFieldDescIterator call to PropertyStorage.
     * @todo It should be decided if it is guaranteed to have this iterator in a
     * storable quality - if it could be used to load/save data.
     */
    DataFieldDescIteratorPtr getFieldDescIterator(void);

    /** Grows the property to allow the storage of minID to maxID object id's
     */
    void grow(int minID, int maxID);

protected:
    // storage-less property constructor. Used by properties which want to
    // construct their storage on their own
    Property(PropertyService *owner, const std::string &name,
             const std::string &chunk_name, std::string inheritorName);

    /** Does the internal handling related to the creation of a property for
     * object
     * @param objID The object id to which a property was added
     */
    void _addProperty(int objID);

    /// The listener to the inheritance messages
    void onInheritChange(const InheritValueChangeMsg &msg);

    /** A connection point usable for descendants to implement property
     * behavior. Called from onInheritChange. In it's default this does nothing.
     * @see ActiveProperty
     */
    virtual void onPropertyModification(const InheritValueChangeMsg &msg);

    /** Returns an ID of the object which is responsible for the current
     * property values As the properties can be inherited using both archetype
     * links and MetaProps, there must be a way to know what object currently
     * represents the property values
     * @param obj_id The object id to query
     * @return object ID of the effective property holder, or zero (0) if not
     * found */
    int _getEffectiveObject(int obj_id) const {
        return mInheritor->getEffectiveID(obj_id);
    }

    /// The name of the property
    std::string mName;

    /// The name of the chunk data (reduced of the P$)
    std::string mChunkName;

    /// chunk version - major
    uint mVerMaj;
    /// chunk version - minor
    uint mVerMin;

    /// Inheritor used to determine property inheritance
    Inheritor *mInheritor;
    InheritServicePtr mInheritService;

    /// Inheritor value changes listener
    Inheritor::ListenerID mInheritorListenerID;

    /// Property storage used to store data for the property
    DataStoragePtr mPropertyStorage;

    /// Owner service
    PropertyService *mOwner;

    /// Builtin flag - properties created in code as builtin have true here
    bool mBuiltin;
};

/** Common ancestor to engine-implemented properties - those that handle
 * property value in a "visible" way. Every class inheriting from this should
 * implement all the abstract methods (sure) and also some of the virtual ones,
 * for example clear, which can happen to be called instead of removeProperty
 * when unloading data.
 * @note All the methods introduced here are only called on concrete objects!
 */
class OPDELIB_EXPORT ActiveProperty : public Property {
public:
    ActiveProperty(PropertyService *owner, const std::string &name,
                   const std::string &chunk_name, std::string inheritorName);

protected:
    /// Overriden from the ancestor, which handles the property life/value
    /// events
    void onPropertyModification(const InheritValueChangeMsg &msg);

    /** Handles the addition of the property to the object.
     * Called when a property is effectively added to the object (regardless of
     * it's source - inherited or direct). A function hooked here should
     * initialise the internal structure that is used per object
     */
    virtual void addProperty(int oid) = 0;

    /** Removes the property from the object.
     * Called when the property stops it's existence
     */
    virtual void removeProperty(int oid) = 0;

    /** Re-reads the values of the property from different source, rebuilds it's
     * values. Called when, thanks to the inheritance/metaproperty changes, the
     * oid object should re-set the internal values based on a different source
     * object.
     * @note Only properties which inherit values need this.
     */
    virtual void setPropertySource(int oid, int effid){};

    /** A field of the property changed event.
     * Called on all objects effectively inheriting values from some source
     * object (not specified).
     * @note This kind of event is typically only fired when changing the
     * property of the object itself, or in editor mode. This is thanks to the
     * fact that no archetype changes are possible in gameplay mode
     * @param oid The object id of the object onto which the change applies
     * @param field the field name of the property onto which the change applies
     * (or empty for one-valued properties)
     * @param value the new value of the propertie's field
     */
    virtual void valueChanged(int oid, const std::string &field,
                              const DVariant &value) = 0;
};
} // namespace Opde

#endif
