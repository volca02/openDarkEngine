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
 *		$Id: RenderAlphaProperty.h 959 2008-11-03 20:28:27Z volca $
 *
 *****************************************************************************/


#ifndef __MODELSCALEPROPERTY_H
#define __MODELSCALEPROPERTY_H

#include "RenderedProperty.h"

namespace Opde {

	/** a ModelScale property implementation using rendered property handler.
	* Controls the scale of the object
	* Uses Vector3. Defaults to 1.0,1.0,1.0 - No scale. Inherits always.
	*/
	class ModelScaleProperty : public RenderedProperty {
		public:
			/// constructor
			ModelScaleProperty(RenderService* rs, PropertyService* owner);

			/// destructor
			virtual ~ModelScaleProperty(void);
			
		protected:
			/// @see ActiveProperty::addProperty
			void addProperty(int oid);
			
			/// @see ActiveProperty::removeProperty
			void removeProperty(int oid);
			
			/// @see ActiveProperty::setPropertySource
			void setPropertySource(int oid, int effid);
			
			/// @see ActiveProperty::valueChanged
			void valueChanged(int oid, const std::string& field, const DVariant& value);

			/// core setter method. Called from other methods to set the hasrefs value
			void setScale(int oid, const Vector3& scale);
			
			Ogre::SceneManager* mSceneMgr;
	};
};

#endif
