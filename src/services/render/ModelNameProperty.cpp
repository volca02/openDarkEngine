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
 *
 *		$Id$
 *
 *****************************************************************************/

#include "ModelNameProperty.h"
#include "RenderService.h"
#include "SingleFieldDataStorage.h"
#include "property/PropertyService.h"

namespace Opde {
/*--------------------------------------------------------*/
/*-------------------- ModelNameProperty -----------------*/
/*--------------------------------------------------------*/
ModelNameProperty::ModelNameProperty(RenderService *rs, PropertyService *owner)
    : RenderedProperty(rs, owner, "ModelName", "ModelName", "always") {

    mPropertyStorage = DataStoragePtr(new FixedStringDataStorage<16>(NULL));

    setChunkVersions(2, 16);

    mSceneMgr = rs->getSceneManager();
};

// --------------------------------------------------------------------------
ModelNameProperty::~ModelNameProperty(void){};

// --------------------------------------------------------------------------
void ModelNameProperty::addProperty(int oid) {
    DVariant val;

    if (!get(oid, "", val))
        OPDE_EXCEPT("Property not defined for object.",
                    "ModelNameProperty::addProperty");

    setModel(oid, val.toString());
};

// --------------------------------------------------------------------------
void ModelNameProperty::removeProperty(int oid) { setModel(oid, ""); };

// --------------------------------------------------------------------------
void ModelNameProperty::setPropertySource(int oid, int effid) {
    // re-read the property
    addProperty(oid);
};

// --------------------------------------------------------------------------
void ModelNameProperty::valueChanged(int oid, const std::string &field,
                                     const DVariant &value) {
    // just call the setter
    setModel(oid, value.toString());
};

// --------------------------------------------------------------------------
void ModelNameProperty::setModel(int oid, const std::string &name) {
    mOwner->setObjectModel(oid, name);
};
}; // namespace Opde
