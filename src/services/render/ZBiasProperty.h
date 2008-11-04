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


#ifndef __ZBIASPROPERTY_H
#define __ZBIASPROPERTY_H

#include "RenderedProperty.h"

namespace Opde {

	/** a Z-Bias property implementation using rendered property handler.
	* Controls the rendering bias of the object (for Z-Fightning avoidance). SS2/T2 only.
	* Uses simple unsigned int data storage. Defaults to 0 - No bias. The stored bias is in bits. Inherits always.
	*/
	class ZBiasProperty : public RenderedProperty {
		public:
			/// constructor
			ZBiasProperty(RenderService* rs, PropertyService* owner);

			/// destructor
			virtual ~ZBiasProperty(void);
			
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
			void setZBias(int oid, uint32_t bias);
			
			Ogre::SceneManager* mSceneMgr;
	};
};

#endif
