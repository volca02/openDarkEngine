// Copied from the Ogre3d wiki: Article "Per renderable transparency"
// Edited to fit the usage in open dark engine

#include "EntityMaterialInstance.h"

using namespace Ogre;

EntityMaterialInstance::EntityMaterialInstance (Entity *e) {
  for (unsigned int i = 0; i < e->getNumSubEntities (); i++) {
    mSEMIs.push_back (new SubEntityMaterialInstance (e->getSubEntity (i)));
  }
}

EntityMaterialInstance::~EntityMaterialInstance () {
  std::vector<SubEntityMaterialInstance *>::iterator it, iend;
  iend = mSEMIs.end ();
  for (it = mSEMIs.begin (); it != iend; ++it) {
    delete *it;
  }
}

void EntityMaterialInstance::setMaterialName (String name) {
  std::vector<SubEntityMaterialInstance *>::iterator it, iend;
  iend = mSEMIs.end ();
  for (it = mSEMIs.begin (); it != iend; ++it) {
    (*it)->setMaterialName (name);
  }
}

void EntityMaterialInstance::setSceneBlending (SceneBlendType sbt) {
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
