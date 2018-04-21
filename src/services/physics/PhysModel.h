/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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

#ifndef __PHYSMODEL_H
#define __PHYSMODEL_H

#include "Array.h"
#include "File.h"
#include "PhysCommon.h"
#include "integers.h"

namespace Opde {
/** @brief Physics model. Contains all data on physics for particular object
 * (Both static and dynamic). This class presents the encapsulation of the whole
 * physical definition of the given object. Internally, this class is a
 * composition of a multiple of other classes, logically separating the
 * different aspects of the physical behavior of the model.
 */
class PhysModel {
public:
    PhysModel(int objid);
    ~PhysModel();

    /** standard read method. Additional parameter indicates which physics
     * version we are using.
     * @note This method should be called prior to the specializations used in
     * various phys. model types,\ as in the file the common parameters precede
     * the special ones.
     * @param sf The file stream to read from
     * @param physVersion The version of the PHYS_SYSTEM chunk(tag file) as
     * indicated by it's header
     */
    virtual void read(const FilePtr &sf, unsigned int physVersion);

    /** Writes the physical model object back to a file.
     * @see read For comments.
     */
    virtual void write(const FilePtr &sf, unsigned int physVersion);

    /// @return the object id of this model
    int32_t getID() const;

    struct Spring {
        float tension;
        float damping;
    };

    //  - go find current cell via BSP, discover the media, update the media of
    //  the submodels
    // broadcast media changed event
    void updateMedium();

    /** puts a model to sleep or wakes it up
     * @note Should handle all the intricacies related to the fact object fell
     * asleep including PhysFellAsleep/PhysWokeUp script messages
     * @note Will stop all the movement of the object
     * @param sleep The desired state of the object physical model (true will
     * put to sleep, false will wake up)
     */
    void setSleep(bool sleep);

protected:
    void clear(void);

    int32_t mObjectID;
    uint32_t mSubModelCount;
    uint32_t mFlags;
    float mGravity;
    std::vector<uint32_t> mSubModelTypes; // the same as the main type of the object
    float mFriction;
    /** Media type. Not entirelly the same as encoded in WR for some obscure
     * reason Mapping: 3->8, 2->1, 1->0, otherwise it stays the same
     */
    int32_t mMediaType;

    std::vector<Spring> mSprings;

    /// Some frame time accumulator
    float mTime;

    /// @todo Unknown meaning
    bool mRopeVsTerrain;

    /// nonzero if the object has attachments
    uint32_t mPhysAttachments;

    /// nonzero if the object is attached
    uint32_t mPhysAttached;

    /// Rotational flags
    uint32_t mRotAxes;

    /// Resting flags
    uint32_t mRestAxes;

    /// Mantling state
    uint32_t mMantlingState;

    /// Mantling related vector (or so it seems)
    Vector3 mMantlingVec;

    /// This contains the positions and orientations of SubModels
    SimpleArray<SubModel> mSubModels;
    SubModel mMainSubModel;

    /// relative submodel positions
    SimpleArray<Vector3> mRelPos;

    /// Object to which this one is attached (rope climbing)
    int32_t mRopeAttObjID;

    /// Submodel of the rope attachment
    int32_t mRopeAttSubModel;

    /// Position on the rope, between the two specified rope segments (i.e.
    /// mRopeAttSubModel && mRopeAttSubModel-1)
    float mRopeSegPos;
};
}; // namespace Opde

#endif
