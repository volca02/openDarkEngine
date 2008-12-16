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
#include "ModelScaleProperty.h"

namespace Opde {
	/*--------------------------------------------------------*/
	/*-------------------- ModelScaleProperty ----------------*/
	/*--------------------------------------------------------*/
	ModelScaleProperty::ModelScaleProperty(RenderService* rs, PropertyService* owner) :
			RenderedProperty(rs, owner, "ModelScale", "Scale", "always") {

		mPropertyStorage = new Vector3DataStorage(NULL);

		// TODO: Check the version
		setChunkVersions(2, 12);

		mSceneMgr = rs->getSceneManager();
	};

	// --------------------------------------------------------------------------
	ModelScaleProperty::~ModelScaleProperty(void) {
	};

	// --------------------------------------------------------------------------
	void ModelScaleProperty::addProperty(int oid) {
		DVariant val;

		if (!get(oid, "", val))
			OPDE_EXCEPT("Property not defined for object.", "ModelScaleProperty::addProperty");

		setScale(oid, val.toVector());
	};

	// --------------------------------------------------------------------------
	void ModelScaleProperty::removeProperty(int oid) {
		// reinit to 1.0 - no transparency
		setScale(oid, Vector3(1.0f, 1.0f, 1.0f));
	};

	// --------------------------------------------------------------------------
	void ModelScaleProperty::setPropertySource(int oid, int effid) {
		// re-read the property
		addProperty(oid);
	};

	// --------------------------------------------------------------------------
	void ModelScaleProperty::valueChanged(int oid, const std::string& field, const DVariant& value) {
		// just call the setter
		setScale(oid, value.toVector());
	};

	// --------------------------------------------------------------------------
	void ModelScaleProperty::setScale(int oid, const Vector3& scale) {
		EntityInfo* ei = getEntityInfo(oid);

		// Bugfix for zero sized scale. Dunno why those appear, but anyway
		// this helps...
		if ( (scale.x == 0) || (scale.y == 0) || (scale.z == 0))
			return;

		ei->setScale(scale);
	};
};

