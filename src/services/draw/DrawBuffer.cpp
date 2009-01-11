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


#include "DrawBuffer.h"

#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>

using namespace Ogre;

namespace Opde {

	DrawBuffer::DrawBuffer(const Ogre::String& imageName) : mIsDirty(false) {
		mMaterial = Ogre::MaterialManager::getSingleton().create("Draw_" + imageName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		// get the autocreated pass
		Ogre::Pass* pass = mMaterial->getTechnique(0)->getPass(0);

		// order of rendering will influence the result
		pass->setDepthCheckEnabled(true);
		pass->setDepthWriteEnabled(true);
		pass->setLightingEnabled(false);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	};

	DrawBuffer::~DrawBuffer() {
		// destroy the material again
		// Ogre::MaterialManager::getSingleton().remove(mMaterial);
	};

	void DrawBuffer::addDrawOperation(DrawOperation* op) {
		mIsDirty = true;
	};

	void DrawBuffer::removeDrawOperation(DrawOperation* op) {
		mIsDirty = true;
	};

	void DrawBuffer::queueUpdate(DrawOperation* drawOp) {
		mIsDirty = true;
	};

	void DrawBuffer::update() {
		// rebuild the queue. First calculate the vertex count
		// STUB
	}

	const MaterialPtr& DrawBuffer::getMaterial(void) const {
		return mMaterial;
	};

	void DrawBuffer::getRenderOperation(Ogre::RenderOperation& op) {
		/*
	        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

        	op.vertexData = mVertexData;
	        op.vertexData->vertexStart = 0;
	        op.vertexData->vertexCount = quadList.size() * 4;

        	op.useIndexes = true;
	        op.indexData = mIndexData;
	        op.indexData->indexStart = 0;
	        op.indexData->indexCount = quadList.size() * 6;
		*/
	};

	void DrawBuffer::getWorldTransforms(Ogre::Matrix4* trans) const {
		// Identity
		*trans = Matrix4::IDENTITY;
	};

	Ogre::Real DrawBuffer::getSquaredViewDepth(const Ogre::Camera* cam) const {
		// TODO: What?
		return 1.0f;
	};

	const Ogre::LightList& DrawBuffer::getLights() const {
		static LightList ll;
		return ll;
	};


}

