/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 *Software Foundation; either version 2 of the License, or (at your option) any
 *later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 *details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 *along with this program; if not, write to the Free Software Foundation, Inc.,
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *
 *	$Id$
 *
 *****************************************************************************/

#include <OgreDefaultHardwareBufferManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreSimpleRenderable.h>

#include "DarkBspNode.h"
#include "DarkPortal.h"
#include "DarkPortalFrustum.h"
#include "DarkSceneManager.h"

// Helping 'infinity' value for PortalRect coords
#define INF 100000
#define F_INF 1E37
#define MAX_PORTAL_POINTS 128

namespace Ogre {
#define POSITION_BINDING 0

const PortalRect PortalRect::EMPTY = PortalRect(INF, -INF, INF, -INF, F_INF);

// Some default values for screen
const PortalRect PortalRect::SCREEN = PortalRect(0, 1024, 0, 768);

// The static window resolution
int PortalRect::sScreenWidth2 = 512;
int PortalRect::sScreenHeight2 = 384;

// -----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &o, PortalRect &r) {
    o << "RECT [top:" << r.top << ", left:" << r.left
      << ", bottom: " << r.bottom << ", right:" << r.right << "]";
    return o;
}

void ScreenRectCache::startUpdate(size_t portalCount, size_t cellCount,
                                  unsigned int update)
{
    // see if we have enough room in both screen space rect caches.
    if (cellRects.size() < cellCount)
        cellRects.resize(cellCount);
    if (portalRects.size() < portalCount)
        portalRects.resize(portalCount);
    updateID = update;
}

// -----------------------------------------------------------------------------
// ----------------- Portal Class implementation -------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
Portal::Portal(unsigned int id, BspNode *source, BspNode *target, Plane plane)
    : ConvexPolygon(plane), mID(id)
{
    mSource = source;
    mTarget = target;

    // Setup the render op. for portal debug display.
    // This should be conditional to the DEBUG builds to stop eating precious
    // memory (10-30000 portals * few 100 bytes of class data is not as much
    // though) mRenderOp.vertexData = NULL;
}

// -----------------------------------------------------------------------------
Portal::~Portal()
{
    /*if (mRenderOp.vertexData)
            delete mRenderOp.vertexData;*/

    detach();
}

// -----------------------------------------------------------------------------
Portal::Portal(const Portal &src) : ConvexPolygon(src)
{
    this->mTarget = src.getTarget();
    this->mSource = src.getSource();
}

/** Move ctor */
Portal::Portal(Portal &&src)
    : ConvexPolygon(std::move(src))
{
    mID = src.mID;
    mTarget = src.mTarget;
    mSource = src.mSource;
    mPortalID = src.mPortalID;
    mCenter = src.mCenter;
    mRadius = src.mRadius;
}

// -----------------------------------------------------------------------------
void Portal::refreshBoundingVolume() {

    unsigned int pointcount = mPoints.size();
    if (pointcount == 0) {
        mCenter = Vector3(0, 0, 0);
        mRadius = -1;
        return;
    }
    // first get the center.
    Vector3 center(0, 0, 0);

    for (unsigned int x = 0; x < pointcount; x++)
        center += mPoints[x];

    center /= pointcount;

    mCenter = center;

    // now the maximal radius
    float radius = 0;

    for (unsigned int x = 0; x < pointcount; x++) {
        Vector3 vdist = mPoints[x] - center;

        float len = vdist.squaredLength();

        if (len > radius)
            radius = len;
    }

    mRadius = sqrt(radius);
}

// -----------------------------------------------------------------------------
void Portal::attach()
{
    mSource->attachOutgoingPortal(this);
    mTarget->attachIncommingPortal(this);
}

// -----------------------------------------------------------------------------
void Portal::detach() {
    mSource->detachPortal(this);
    mTarget->detachPortal(this);
}

// -----------------------------------------------------------------------------
Real Portal::getSquaredViewDepth(const Camera *cam) const {
    Vector3 dist = cam->getDerivedPosition() - mCenter;

    return dist.squaredLength();
}

// -----------------------------------------------------------------------------
Real Portal::getBoundingRadius(void) const { return mRadius; }

// -----------------------------------------------------------------------------
bool Portal::refreshScreenRect(const Vector3 &vpos, ScreenRectCache &rects,
                               const Matrix4 &toScreen, const Plane &cutp)
{
    // modified version of the ConvexPolygon::clipByPlane which does two things
    // at once: cuts by the specified plane, and projects the result to screen
    // (then, updates the rect to contain this point)
    PortalRectInfo &pi = rects.portal(getID());
    pi.invalidate();

    // Backface cull. The portal won't be culled if a vector camera-vertex
    // dotproduct normal will be greater than
    Vector3 camToV0 = mPoints[0] - vpos;
    float dotp = camToV0.dotProduct(mPlane.normal);
    pi.portalCull = (dotp > 0);

    // skip these expensive operations if we encounter a backface cull
    if (pi.portalCull)
        return false;

    // portal points plane cutting and to-screen proj.
    int positive = 0;
    int negative = 0;

    unsigned int pointcount = mPoints.size();

    if (pointcount == 0)
        return 0;

    assert(pointcount < MAX_PORTAL_POINTS);

    // first we mark the vertices
    Plane::Side sides[MAX_PORTAL_POINTS];

    unsigned int idx;

    PolygonPoints::const_iterator it = mPoints.begin();
    PolygonPoints::const_iterator end = mPoints.end();

    for (idx = 0; it != end; ++it, ++idx) {
        Plane::Side side = cutp.getSide(*it);
        sides[idx] =
            side; // push the side of the vertex into the side buffer...

        switch (side) {
        case Plane::POSITIVE_SIDE:
            positive++;
            break;
        case Plane::NEGATIVE_SIDE:
            negative++;
            break;
        default:;
        }
    }

    // Now that we have the poly's side classified, we can process it...
    if (positive == 0) {
        // we clipped away the whole portal. No need to cut
        return false;
    }

    // some vertices were on one side, some on the other
    long prev = pointcount - 1; // the last one

    for (idx = 0; idx < pointcount; idx++) {
        const Plane::Side side = sides[idx];

        if (side == Plane::POSITIVE_SIDE) {
            if (sides[prev] == Plane::POSITIVE_SIDE) {
                pi.screenRect.enlargeToContain(toScreen * mPoints.at(idx));
            } else {
                // calculate a new boundry positioned vertex
                const Vector3 &v1 = mPoints.at(prev);
                const Vector3 &v2 = mPoints.at(idx);
                Vector3 dv =
                    v2 - v1; // vector pointing from v2 to v1 (v1+dv*0=v2 *1=v1)

                // the dot product is there for a reason! (As I have a tendency
                // to overlook the difference)
                float t = cutp.getDistance(v2) / (cutp.normal.dotProduct(dv));

                pi.screenRect.enlargeToContain(
                    toScreen *
                    (v2 -
                     (dv * t))); // a new, boundry placed vertex is inserted
                pi.screenRect.enlargeToContain(toScreen * v2);
            }
        } else {
            if (sides[prev] == Plane::POSITIVE_SIDE) { // if we're going outside
                // calculate a new boundry positioned vertex
                const Vector3 v1 = mPoints[idx];
                const Vector3 v2 = mPoints[prev];
                const Vector3 dv = v2 - v1;

                float t = cutp.getDistance(v2) / (cutp.normal.dotProduct(dv));

                pi.screenRect.enlargeToContain(
                    toScreen *
                    (v2 -
                     (dv * t))); // a new, boundry placed vertex is inserted
            }
        }

        prev = idx;
    }

    return true;
}

void Portal::refreshScreenRect(const Vector3 &vpos, ScreenRectCache &rects,
                               const Matrix4 &toScreen,
                               const PortalFrustum &frust)
{
    PortalRectInfo &pi = rects.portal(getID());
    pi.invalidate();

    // Backface cull. The portal won't be culled if a vector camera-vertex
    // dotproduct normal will be greater than 0
    Vector3 camToV0 = mPoints[0] - vpos;
    float dotp = camToV0.dotProduct(mPlane.normal);
    pi.portalCull = (dotp > 0);

    // skip these expensive operations if we encounter a backface cull
    if (pi.portalCull)
        return;


    // We also can cull away the portal if it is behind the camera's near plane.
    // Can we? This needs distance calculation with the surrounding sphere

    // We have to cut the Portal using camera's frustum first, as this should
    // solve the to screen projection problems... the reason is that the coords
    // that are at the back side or near the view point do not get projected
    // right (that is right for our purpose) it should be sufficient to clip by
    // camera's plane (not near plane, too far away, just the plane that comes
    // throught the camera's origin and has normal == view vector of camera)

    int cl = frust.getPortalClassification(*this);

    // no onscreen
    if (cl == -1)
        return;

    // all onscreen
    if (cl == 1) {
        for (const auto &point : mPoints) {
            // This is one time-consuming line... I wonder how big eater this
            // line is.
            Vector3 hcsPosition = toScreen * point;
            pi.screenRect.enlargeToContain(hcsPosition);
        }

        return;
    }

    // NOTE: This is a bit costly, but it's only done once per frame/camera
    // most of the other portal projections use the other method with cut plane

    // need to clip. Costly...
    auto onScreen = frust.clipPortal(*this);

    // If we have a non-zero cut result
    if (onScreen) {
        const PortalPoints &scr_points = onScreen->getPoints();

        // project all the vertices to screen space
        for (const auto &point : scr_points) {
            // This is one time-consuming line... I wonder how big eater this
            // line is.
            Vector3 hcsPosition = toScreen * point;
            pi.screenRect.enlargeToContain(hcsPosition);
        }
    }
}

} // namespace Ogre
