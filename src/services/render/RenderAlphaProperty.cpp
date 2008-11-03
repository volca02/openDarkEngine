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
#include "RenderAlphaProperty.h"

namespace Opde {
	/*--------------------------------------------------------*/
	/*-------------------- RenderAlphaProperty ----------------*/
	/*--------------------------------------------------------*/
	RenderAlphaProperty::RenderAlphaProperty(RenderService* rs, PropertyService* owner) : 
			RenderedProperty(rs, owner, "RenderAlpha", "RenderAlp", "always") {
		
		mPropertyStorage = new FloatDataStorage();
		
		// TODO: Check the version
		setChunkVersions(2, 65540);
		
		mSceneMgr = rs->getSceneManager();
	};

	// --------------------------------------------------------------------------
	RenderAlphaProperty::~RenderAlphaProperty(void) {
	};

	// --------------------------------------------------------------------------
	void RenderAlphaProperty::addProperty(int oid) {
		DVariant val;

		if (!get(oid, "", val))
			OPDE_EXCEPT("Property not defined for object.", "RenderAlphaProperty::addProperty");
		
		setAlpha(oid, val.toFloat());
	};

	// --------------------------------------------------------------------------
	void RenderAlphaProperty::removeProperty(int oid) {
		// reinit to 1.0 - no transparency
		setAlpha(oid, 1.0f);
	};

	// --------------------------------------------------------------------------
	void RenderAlphaProperty::setPropertySource(int oid, int effid) {
		// re-read the property
		addProperty(oid);
	};

	// --------------------------------------------------------------------------
	void RenderAlphaProperty::valueChanged(int oid, const std::string& field, const DVariant& value) {
		// just call the setter
		setAlpha(oid, value.toFloat());
	};

	// --------------------------------------------------------------------------
	void RenderAlphaProperty::setAlpha(int oid, float alpha) {
		EntityInfo* ei = getEntityInfo(oid);
		// clamp to 0-1
		if (alpha < 0.0f)
			alpha = 0.0f;
		
		if (alpha > 1.0f)
			alpha = 1.0f;
		
		// and set the alpha on the entity
		ei->setAlpha(alpha);
	};
};

