/*
Copied from the Ogre3d wiki: Article "Per renderable transparency"
Edited to fit the usage in open dark engine

$Id$

*/

#ifndef __SUBENTITYMATERIALINSTANCE_H__
#define __SUBENTITYMATERIALINSTANCE_H__

#include "MaterialInstance.h"

/** Specialisation of the MaterialInstance class for SubEntities.
 * @author Kencho.
 * @see MaterialInstance.
 */
class SubEntityMaterialInstance : public MaterialInstance {
// Attributes =================================================================================
  public:
  protected:
    /** Reference to the affected SubEntity.
     */
    Ogre::SubEntity *mSubEntity;
// Methods ====================================================================================
  public:
    /** Constructor. 
     * Initialises references and parameters.
     * @param se The SubEntity this SubEntityMaterialInstance works on.
     */
    SubEntityMaterialInstance (Ogre::SubEntity *se);
    /** Destructor.
     * @note Destroys the copy material if needed.
     */
    virtual ~SubEntityMaterialInstance ();
    /** Changes this SubEntity material and does any needed operations to keep the previous
     * material instance parameters (transparency and such).
     * @param name Name of the new material.
     * @note This also changes the references SubEntity material, so there is no need to call 
     *       SubEntity::setMaterialName() if this method is called. Indeed it's recommended to 
     *       change it through this instance rather than changing it manually.
     */
    void setMaterialName (Ogre::String name);
    /** Changes this instance transparency. 
     * @param transparency The new transparency. Values will be clamped to [0..1].
     * @note This changes transparency. A value of 0 means full opacity, while 1 means full 
     *       transparency (invisible)
     * @note If transparency equals 0, it will use the original material instead of the copy 
     *       (the copy is mandatory transparent, and thus might be slower than the original).
     * @see MaterialInstance::setTransparency().
     */
    void setTransparency (Ogre::Real transparency);
    
    /** Changes this instance's z-bias setting.
    */
    void setZBias (Ogre::Real zbias);
    
    
  protected:
    /** Initialises the reference to the original material from the SubEntity's.
     * @see MaterialInstance::initOriginalMaterial().
     */
    void initOriginalMaterial ();
    
    
};

#endif // __SUBENTITYMATERIALINSTANCE_H__
