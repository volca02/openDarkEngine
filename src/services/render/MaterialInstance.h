// Copied from the Ogre3d wiki: Article "Per renderable transparency"
// Edited to fit the usage in open dark engine

#ifndef __MATERIALINSTANCE_H__
#define __MATERIALINSTANCE_H__

#include "Ogre.h"

/** An instance of a single material.
 * This class represents a single instance of a material. It's mainly 
 * an utility to allow a single Renderable to change its transparency without changing every 
 * other renderables' transparency which also use this material. 
 * It will create (and use) a copy of the original material only when needed. But it will keep 
 * the reference to the cloned material until it's no longer needed (to save re-clonation time).
 * Allows also changing the Renderable material through this class, so it's no need to destroy 
 * this instance and create a new one if the material changes.
 * @note Transparency is applied to all the passes in all the techniques.
 * @note Default blending method is alpha blending. Blending methods in the original material will be overriden. 
 * @note Modulative blending is not supported as it can't be done if some textures exist.
 * @author Kencho
 * @todo Check lighting enabled or not. Disabled lighting won't allow colour changing.
 * @todo Take care of shininess, specularity, and emissiveness.
 * @todo Add existing material recognising support (to allow existing transparency updating...).
 */
class MaterialInstance {
// Attributes =================================================================================
  protected:
    /** Reference to the original material.
     */
    Ogre::MaterialPtr mOriginalMat;
    /** Reference to the copy material.
     */
    Ogre::MaterialPtr mCopyMat;
    /** Keeps the current transparency value.
     */
    Ogre::Real mCurrentTransparency;
    /** Current blending method.
     */
    Ogre::SceneBlendType mSBT;
// Methods ====================================================================================
  public:
    /** Constructor. 
     * Initialises references and parameters.
     */
    MaterialInstance ();
    /** Destructor.
     * @note Destroys the copy material if needed.
     */
    ~MaterialInstance ();
    /** Sets the blending method to use to adjust transparency.
     * @param sbt The SceneBlendType desired.
     */
    void setSceneBlending (Ogre::SceneBlendType sbt);
    /** Changes this instance transparency. 
     * @param transparency The new transparency. Values will be clamped to [0..1].
     * @note This changes transparency. A value of 0 means full opacity, while 1 means full 
     *       transparency (invisible)
     * @note If transparency equals 0, it will use the original material instead of the copy 
     *       (the copy is mandatory transparent, and thus might be slower than the original).
     */
    void setTransparency (Ogre::Real transparency);
    /** Retrieves a shared pointer to its cloned material.
     * @return A MaterialPtr of the cloned material.
     */
    Ogre::MaterialPtr getCopyMaterial ();
  protected:
    /** Initialises the reference to the original material.
     */
    virtual void initOriginalMaterial () = 0;
    /** Clones the original material.
     */
    void createCopyMaterial ();
    /** If exists, removes the copy material, and clears the reference to it.
     */
    void clearCopyMaterial ();
};

#endif // __MATERIALINSTANCE_H__
