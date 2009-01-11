/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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


#ifndef __DRAWBUFFER_H
#define __DRAWBUFFER_H

#include "DrawOperation.h"

#include <OgreRenderable.h>
#include <OgreMaterial.h>

namespace Opde {

	/** A single renderable representing all drawn quads for particular rendered settings combination (DrawSheet stores N of these for N materials) */
	class DrawBuffer : public Ogre::Renderable {
		public:
			DrawBuffer(const Ogre::String& imageName);
			
			virtual ~DrawBuffer();

			void addDrawOperation(DrawOperation* op);

			void removeDrawOperation(DrawOperation* op);

			void queueUpdate(DrawOperation* drawOp);

			inline bool isDirty() const { return mIsDirty; };
			
			void update();

			//--- Renderable mandatory ---
			const Ogre::MaterialPtr& getMaterial(void) const;
			void getRenderOperation(Ogre::RenderOperation&);
			void getWorldTransforms(Ogre::Matrix4*) const;
			Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;
			const Ogre::LightList& getLights() const;

		protected:
			DrawOperationMap mDrawOpMap;

			Ogre::MaterialPtr mMaterial;
			bool mIsDirty;
	};

	/// Draw buffer map for all render op. combinations (currently, we index by image name, we could reindex with image ID later for performance)
	typedef std::map<Ogre::String, DrawBuffer*> DrawBufferMap;
}

#endif
