/*
 Copied from the Ogre3d wiki: Article "Per renderable transparency"
 Edited to fit the usage in open dark engine

$Id$

TODO: This is here to let us support:
    * Per object transparency (Property RenderAlpha)
    * Self light objects - needs more work (per material, etc)
    * Full bright objects - f.e. this is how original dark indicates object to be frobbed
    * OTHERS?
*/


#ifndef __ENTITYMATERIALINSTANCE_H__
#define __ENTITYMATERIALINSTANCE_H__

#include "SubEntityMaterialInstance.h"

#include <OgreIteratorWrappers.h>

/** Iterator to traverse the SubEntityMaterialInstance's.
 * @author Kencho.
 */
typedef Ogre::VectorIterator<std::vector<SubEntityMaterialInstance *> > SubEntityMaterialInstancesIterator;

/** An instance of a full Entity material.
 * This is like a hub for all the underlying SubEntities material instances.
 * It keeps a list of each SubEntity's material instance.
 * @see SubEntityMaterialInstance.
 * @author Kencho
 */
class EntityMaterialInstance {
// Methods ====================================================================================
  public:
    /** Constructor. 
     * Initialises the list of SubEntities' material instances.
     * @param e The entity this material instance will affect to.
     * @note This will create material instances for all the underlying SubEntities.
     */
    EntityMaterialInstance (Ogre::Entity *e);
    /** Destructor.
     * Destroys all the underlying SubEntityMaterialInstances.
     */
    ~EntityMaterialInstance ();

    /** Sets the scene blending method for all the SubEntities.
     * @param sbt The desired SceneBlendType.
     * @see SubEntityMaterialInstance::setSceneBlending().
     */
    void setSceneBlending (Ogre::SceneBlendType sbt);

    /** Changes the whole Entity transparency, through all the underlying SubEntityMaterialInstances.
     * @param transparency New transparency.
     * @see SubEntityMaterialInstance::setTransparency().
     */
    void setTransparency (Ogre::Real transparency);

    /** Returns an iterator to traverse all the underlying MaterialInstances.
     * @return The SubEntityMaterialInstances iterator.
     */
    SubEntityMaterialInstancesIterator getSubEntityMaterialInstancesIterator ();
    
    /** Sets a new entity to handle by this class, preserving the previous values
    */
	void setEntity(Ogre::Entity *e);
  
  protected:
	void prepareSEMIs();
	void destroySEMIs();

// Attributes =================================================================================
	Ogre::Entity* mEntity;
	
    /** List of SubEntities' material instances.
     */
    std::vector<SubEntityMaterialInstance *> mSEMIs;
    /** Keeps the current transparency value.
     * @see SubEntityMaterialInstance::mCurrentTransparency.
     */
    Ogre::Real mCurrentTransparency;
  
	Ogre::SceneBlendType mSceneBlendType;
};

#endif // __ENTITYMATERIALINSTANCE_H__
