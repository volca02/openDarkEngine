/*
Copied from the Ogre3d wiki: Article "Per renderable transparency"
Edited to fit the usage in open dark engine

$Id$

*/

#include "MaterialInstance.h"
#include <OgreBlendMode.h>

using namespace Ogre;

MaterialInstance::MaterialInstance () {
	mCurrentTransparency = 0.0f;
	mCurrentZBias = 0.0f;
	
	mCopyMat.setNull ();
    
	mSBT = SBT_TRANSPARENT_ALPHA;
}

MaterialInstance::~MaterialInstance () {
  clearCopyMaterial ();
}

void MaterialInstance::setSceneBlending (SceneBlendType sbt) {
  mSBT = sbt;

  if (!mCopyMat.isNull ()) {
    Material::TechniqueIterator techniqueIt = mCopyMat->getTechniqueIterator ();
    while (techniqueIt.hasMoreElements ()) {
      Technique *t = techniqueIt.getNext ();
      Technique::PassIterator passIt = t->getPassIterator ();
      while (passIt.hasMoreElements ()) {
        passIt.getNext ()->setSceneBlending (mSBT);
      }
    }
  }
}

void MaterialInstance::setTransparency (Real transparency) {
  mCurrentTransparency = transparency;
  if (mCurrentTransparency > 0.0f) {
    if (mCurrentTransparency > 1.0f)
      mCurrentTransparency = 1.0f;
    
    if (mCopyMat.isNull ()) {
      createCopyMaterial ();
    }
      
    unsigned short i = 0, j;
    ColourValue sc, dc; // Source colur, destination colour
    Material::TechniqueIterator techniqueIt = mCopyMat->getTechniqueIterator ();
    while (techniqueIt.hasMoreElements ()) {
      Technique *t = techniqueIt.getNext ();
      Technique::PassIterator passIt = t->getPassIterator ();
      j = 0;
      while (passIt.hasMoreElements ()) {
        sc = mOriginalMat->getTechnique (i)->getPass (j)->getDiffuse ();

        switch (mSBT) {
          case SBT_ADD:
            dc = sc;
            dc.r -= sc.r * mCurrentTransparency;
            dc.g -= sc.g * mCurrentTransparency;
            dc.b -= sc.b * mCurrentTransparency;
            passIt.peekNext ()->setAmbient (ColourValue::Black);
            break;
          case SBT_TRANSPARENT_ALPHA:
          default:
            dc = sc;
            dc.a = sc.a * (1.0f - mCurrentTransparency);
            passIt.peekNext ()->setAmbient (mOriginalMat->getTechnique (i)->getPass (j)->getAmbient ());
            break;
        }
        passIt.peekNext ()->setDiffuse (dc);
        passIt.peekNext ()->setAlphaRejectFunction(CMPF_ALWAYS_PASS);
        
        passIt.moveNext ();
            
        ++j;
      }
          
      ++i;
    }
  }
  else {
    mCurrentTransparency = 0.0f;
  }
}

void MaterialInstance::setZBias(Ogre::Real zbias) {
	mCurrentZBias = zbias;
	
	if (mCurrentZBias != 0.0f) {
		if (mCopyMat.isNull ()) {
			createCopyMaterial ();
		}
		
		Material::TechniqueIterator techniqueIt = mCopyMat->getTechniqueIterator ();

		while (techniqueIt.hasMoreElements ()) {
			Technique *t = techniqueIt.getNext ();
			
			Technique::PassIterator passIt = t->getPassIterator ();
			while (passIt.hasMoreElements ()) {
				Pass* p = passIt.getNext();
				
				// change the depth bias
				p->setDepthBias(mCurrentZBias, mCurrentZBias);
			}
		}
	}
}

MaterialPtr MaterialInstance::getCopyMaterial () {
  return mCopyMat;
}

void MaterialInstance::createCopyMaterial () {
  String name;
  // Avoid name collision
  do {
    name = mOriginalMat->getName () + StringConverter::toString (Math::UnitRandom ());
  } while (MaterialManager::getSingleton ().resourceExists (name));
          
  mCopyMat = mOriginalMat->clone (name);

  Material::TechniqueIterator techniqueIt = mCopyMat->getTechniqueIterator ();
  while (techniqueIt.hasMoreElements ()) {
    Technique *t = techniqueIt.getNext ();
    Technique::PassIterator passIt = t->getPassIterator ();
    while (passIt.hasMoreElements ()) {
      passIt.peekNext ()->setDepthWriteEnabled (false);
      passIt.peekNext ()->setSceneBlending (mSBT);
      passIt.moveNext ();
    }
  }
}

void MaterialInstance::clearCopyMaterial () {
  if (!mCopyMat.isNull ())
    MaterialManager::getSingleton ().remove (mCopyMat->getName ());
       
  mCopyMat.setNull ();
}

bool MaterialInstance::hasOverrides(void) {
	return (mCurrentTransparency != 0.0f) || (mCurrentZBias != 0.0f);
}
