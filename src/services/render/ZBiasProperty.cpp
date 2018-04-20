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

#include "ZBiasProperty.h"
#include "RenderService.h"
#include "SingleFieldDataStorage.h"
#include "property/PropertyService.h"
#include "EntityInfo.h"

namespace Opde {
/*--------------------------------------------------------*/
/*-------------------- RenderAlphaProperty ----------------*/
/*--------------------------------------------------------*/
ZBiasProperty::ZBiasProperty(RenderService *rs, PropertyService *owner)
    : RenderedProperty(rs, owner, "RendererZBias", "Z-Bias", "always")
{

    mPropertyStorage = DataStoragePtr(new UIntDataStorage(NULL));

    setChunkVersions(2, 4);

    mSceneMgr = rs->getSceneManager();
};

// --------------------------------------------------------------------------
ZBiasProperty::~ZBiasProperty(void){};

// --------------------------------------------------------------------------
void ZBiasProperty::addProperty(int oid) {
    DVariant val;

    if (!get(oid, "", val))
        OPDE_EXCEPT("Property not defined for object.",
                    "RenderAlphaProperty::addProperty");

    setZBias(oid, val.toUInt());
};

// --------------------------------------------------------------------------
void ZBiasProperty::removeProperty(int oid) {
    // reinit to 0 - no bias
    setZBias(oid, 0);
};

// --------------------------------------------------------------------------
void ZBiasProperty::setPropertySource(int oid, int effid) {
    // re-read the property
    addProperty(oid);
};

// --------------------------------------------------------------------------
void ZBiasProperty::valueChanged(int oid, const std::string &field,
                                 const DVariant &value) {
    // just call the setter
    setZBias(oid, value.toUInt());
};

// --------------------------------------------------------------------------
void ZBiasProperty::setZBias(int oid, uint32_t bias) {
    EntityInfo *ei = getEntityInfo(oid);
    ei->setZBias(bias);
};
}; // namespace Opde
