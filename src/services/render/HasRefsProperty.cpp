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

#include "RenderService.h"
#include "PropertyService.h"
#include "SingleFieldDataStorage.h"
#include "HasRefsProperty.h"

namespace Opde {
	/*--------------------------------------------------------*/
	/*--------------------- HasRefsProperty ------------------*/
	/*--------------------------------------------------------*/
	HasRefsProperty::HasRefsProperty(RenderService* rs, PropertyService* owner) : 
			RenderedProperty(rs, owner, "HasRefs", "HasRefs", "always") {
		mPropertyStorage = new BoolDataStorage();
	};

	// --------------------------------------------------------------------------
	HasRefsProperty::~HasRefsProperty(void) {
	};

	// --------------------------------------------------------------------------
	void HasRefsProperty::addProperty(int oid) {
		// nothing special needed here. Just initialize the value of the hasrefs to the data value
		// as hasRefs is inherited, just use our methods
		DVariant val(false);

		if (!get(oid, "", val))
			OPDE_EXCEPT("Property not defined for object.", "HasRefsProperty::addProperty");
		
		setHasRefs(oid, val.toBool());
	};

	// --------------------------------------------------------------------------
	void HasRefsProperty::removeProperty(int oid) {
		// reinit to true - the object's default
		setHasRefs(oid, true);
	};

	// --------------------------------------------------------------------------
	void HasRefsProperty::setPropertySource(int oid, int effid) {
		// re-read the property
		addProperty(oid);
	};

	// --------------------------------------------------------------------------
	void HasRefsProperty::valueChanged(int oid, const std::string& field, const DVariant& value) {
		// just call the setter
		setHasRefs(oid, value.toBool());
	};

	// --------------------------------------------------------------------------
	void HasRefsProperty::setHasRefs(int oid, bool hasRefs) {
		EntityInfo* ei = getEntityInfo(oid);
		ei->setHasRefs(hasRefs);
	};
};

