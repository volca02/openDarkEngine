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

#include "DrawCommon.h"
#include "DrawOperation.h"

#include <OgreRenderable.h>
#include <OgreMaterial.h>
#include <OgreMovableObject.h>

namespace Opde {

	/** A single renderable representing all drawn quads for particular rendered settings combination (DrawSheet stores N of these for N materials) */
	class OPDELIB_EXPORT DrawBuffer : public Ogre::Renderable {
		public:
			/** Constructor
			 * @param materialName The name of the material to use for rendering. The constructor will look for the specified material and if it does not find it, it will create a new one
			 */
			DrawBuffer(const Ogre::MaterialPtr& material);

			/// Destructor
			virtual ~DrawBuffer();

			/// Adds a render operation to the buffer
			void addDrawOperation(DrawOperation* op);

			/// Removes a render operation from the buffer
			void removeDrawOperation(DrawOperation* op);

			/** A draw operation changed, queue an update
			 * @note The parameter is just a hint, the whole buffer is rebuilt
			 * @note If the draw op. would stay the same length, we could introduce ibo and vbo pos markers to the quads (smart updates)
			 */
			void queueUpdate(DrawOperation* drawOp);

			/// is dirty (needs update) getter
			inline bool isDirty() const { return mIsDirty; };
			
			/// manually marks this buffer as dirty - informs it that it needs a rebuild
			inline void markDirty() { mIsDirty = true; };

			/// Does a forced update (ignoring isDirty state)
			void update();

			/// Called by DrawOperation::visitDrawBuffer, this method queues the quads emited by the operation for sorting and rendering
			void _queueDrawQuad(const DrawQuad* dq);

			/// Called by sheet on setActive to inform the buffer is now rendered within a different sheet (this also means we'll have to rebuild usually)
			void _parentChanged(DrawSheet* parent);

			//--- Renderable mandatory ---
			const Ogre::MaterialPtr& getMaterial(void) const;

			void getRenderOperation(Ogre::RenderOperation&);

			void getWorldTransforms(Ogre::Matrix4*) const;

			Ogre::Real getSquaredViewDepth(const Ogre::Camera*) const;

			const Ogre::LightList& getLights() const;

			const Ogre::Quaternion& getWorldOrientation(void) const;

			const Ogre::Vector3& getWorldPosition(void) const;


			inline Ogre::uint8 getRenderQueueID() const { return mRenderQueueID; };

			inline bool isDirty() { return mIsDirty; };
			
			/// returns true if this buffer contains no rendered objects
			inline bool isEmpty() { return mQuadList.size() == 0; };
			
		protected:
			/// (re)builds the VBO according to the mQuadList
			void buildBuffer();

			/// destroys the rendering buffers
			void destroyBuffers();

			DrawOperationMap mDrawOpMap;
			DrawQuadList mQuadList;

			Ogre::MaterialPtr mMaterial;
			bool mIsDirty;
			bool mIsUpdating;

			Ogre::HardwareVertexBufferSharedPtr mBuffer;
			Ogre::VertexData* mVertexData;
			Ogre::IndexData* mIndexData;

			/// This is the reserved quad count, not the rendered quad count (which is less usually)
			size_t mQuadCount;

			Ogre::uint8 mRenderQueueID;

			/// Parent sheet - the one currently displaying the buffer
			DrawSheet* mParent;
	};

	/// Draw buffer map for all render op. combinations (indexed with texture source ID)
	typedef std::map<DrawSource::ID, DrawBuffer*> DrawBufferMap;
}

#endif
