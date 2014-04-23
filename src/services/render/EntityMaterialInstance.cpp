/*
Copied from the Ogre3d wiki: Article "Per renderable transparency"
Edited to fit the usage in open dark engine

$Id$
*/

#include "EntityMaterialInstance.h"

using namespace Ogre;

Ogre::uint8 EntityMaterialInstance::msBaseRenderQueueGroup = 127; // Must be set!

EntityMaterialInstance::EntityMaterialInstance (Entity *e) {
	mEntity = e;
	mOriginalRenderQueueGroup = e->getRenderQueueGroup();
	mSceneBlendType = SBT_MODULATE;
	mCurrentTransparency = 0.0f;
	mZBias = 0;

	prepareSEMIs();
}

EntityMaterialInstance::~EntityMaterialInstance () {
	destroySEMIs();
}

void EntityMaterialInstance::setSceneBlending (SceneBlendType sbt) {
	mSceneBlendType = sbt;

	std::vector<SubEntityMaterialInstance *>::iterator it, iend;
	iend = mSEMIs.end ();
	for (it = mSEMIs.begin (); it != iend; ++it) {
		(*it)->setSceneBlending (sbt);
	}
}

void EntityMaterialInstance::setTransparency (Real transparency) {
	mCurrentTransparency = transparency;
	if (mCurrentTransparency > 1.0f)
		mCurrentTransparency = 1.0f;
	if (mCurrentTransparency < 0.0f)
		mCurrentTransparency = 0.0f;

	std::vector<SubEntityMaterialInstance *>::iterator it, iend;
	iend = mSEMIs.end ();
	for (it = mSEMIs.begin (); it != iend; ++it) {
		(*it)->setTransparency (mCurrentTransparency);
	}
}

SubEntityMaterialInstancesIterator EntityMaterialInstance::getSubEntityMaterialInstancesIterator () {
	return SubEntityMaterialInstancesIterator (mSEMIs.begin (), mSEMIs.end ());
}

void EntityMaterialInstance::setEntity(Ogre::Entity *e) {
	assert(e != NULL);

	// do nothing if the same entity is given
	if (e == mEntity)
		return;

	mEntity = e;

	mOriginalRenderQueueGroup = e->getRenderQueueGroup();

	// will destroy the current ones
	prepareSEMIs();

	setSceneBlending(mSceneBlendType);

	// set all the parameters
	if (mCurrentTransparency > 0.0f)
		setTransparency(mCurrentTransparency);

	if (mZBias != 0)
		setZBias(mZBias);
}

void EntityMaterialInstance::prepareSEMIs() {
	// just to be sure
	destroySEMIs();

	for (unsigned int i = 0; i < mEntity->getNumSubEntities (); i++) {
		mSEMIs.push_back (new SubEntityMaterialInstance (mEntity->getSubEntity (i)));
	}
}

void EntityMaterialInstance::destroySEMIs() {
	std::vector<SubEntityMaterialInstance *>::iterator it, iend;
	iend = mSEMIs.end ();

	for (it = mSEMIs.begin (); it != iend; ++it) {
		delete *it;
	}

	mSEMIs.clear();
}

void EntityMaterialInstance::setZBias(size_t zbias) {
	mZBias = zbias;

	// limit zbias to 0-16
	if (mZBias > 16)
		mZBias = 16;

	if (zbias != 0) {
		// modify the render queue accordingly
		mEntity->setRenderQueueGroup(msBaseRenderQueueGroup - mZBias);

		std::vector<SubEntityMaterialInstance *>::iterator it, iend;
		iend = mSEMIs.end ();
		for (it = mSEMIs.begin (); it != iend; ++it) {
			(*it)->setZBias(1 << mZBias);
		}
	} else {
		mEntity->setRenderQueueGroup(mOriginalRenderQueueGroup);

		std::vector<SubEntityMaterialInstance *>::iterator it, iend;
		iend = mSEMIs.end ();
		for (it = mSEMIs.begin (); it != iend; ++it) {
			(*it)->setZBias(0);
		}
	}
}

void EntityMaterialInstance::setBaseRenderQueueGroup(Ogre::uint8 baseRQ) {
	msBaseRenderQueueGroup = baseRQ;
}
