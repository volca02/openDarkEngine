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

#include <OgrePlane.h>

#include "DarkPortal.h"
#include "DarkPortalFrustum.h"

namespace Ogre {

/*---------------------------------------------------------*/
// construct a new Frustum out of a camera and a Portal (which has been clipped
// as needed previously)
PortalFrustum::PortalFrustum(const Camera *cam, const Portal &poly) {
    // we create a new frustum planes using a 3*Vector3 constructor
    const PortalPoints &pnts = poly.getPoints();

    if (pnts.size() < 3)
        // we have a degenerated poly!!!
        return;

    Vector3 cam_pos = cam->getDerivedPosition();

    planes.clear();

    // should we check whether the normal is faced to the center of the poly?
    PortalPoints::const_iterator endpt = pnts.end();
    auto prev = *--endpt;

    for (const auto &point : pnts) {
        Plane plane;

        // Dunno why, the constructor was failing to
        // intialize the values directly...
        plane.redefine(prev, point, cam_pos);

        planes.push_back(plane);
        prev = point;
    }

    // add the poly's self plane as well (near plane, sort of)
    planes.push_back(poly.getPlane());
}

/*---------------------------------------------------------*/
PortalFrustum::PortalFrustum(const Camera *cam) {
    // read out the LEFT, RIGHT, TOP and BOTTOM planes from the camera, and push
    // them into our planes var...
    planes.clear();

    planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_BOTTOM));
    planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_LEFT));
    planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_TOP));
    planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_RIGHT));
}

/*---------------------------------------------------------*/
PortalFrustum::PortalFrustum(const Vector3 &point, const Portal &poly) {
    // we create a new frustum planes using a 3*Vector3 constructor
    const PortalPoints &pnts = poly.getPoints();

    if (pnts.size() < 3)
        // we have a degenerated poly!!!
        return;

    planes.clear();

    // should we check whether the normal is faced to the center of the poly?
    PortalPoints::const_iterator endpt = pnts.end();
    auto prev = *--endpt;

    for (const auto &point : pnts) {
        Plane plane;

        plane.redefine(prev, point, point);

        planes.push_back(plane);
        prev = point;
    }
}
/*---------------------------------------------------------*/
PortalFrustum::PortalFrustum(const Vector3 &point, const Vector3 &direction,
                             Radian ang, size_t sides)
{
    // The planes will be generated as a cone approximation

    // An up vector to be used
    Vector3 up = direction.perpendicular();

    Quaternion q;

    // Rotate the up vector by the desired amount
    q.FromAngleAxis(Radian(0), direction);

    Vector3 current = q * up;

    // Now rotate by our desired angle
    q.FromAngleAxis(ang, current);

    // We have the first result to use
    current = q * current;

    for (size_t o = 0; o < sides; ++o) {
        // Define the rotation now, as a rotated variant of the axis
        q.FromAngleAxis(Radian(2 * Math::PI * (o + 1) / sides), direction);

        Vector3 next = q * up;

        // Now rotate by our desired angle
        q.FromAngleAxis(ang, next);

        // This is the resulting normal
        next = q * next;

        Plane plane;

        plane.redefine(point + current, point + next, point);

        planes.push_back(plane);

        current = next;
    }
}

/*---------------------------------------------------------*/
void PortalFrustum::addPlane(const Plane &a) { planes.push_back(a); }

/*---------------------------------------------------------*/
const FrustumPlanes &PortalFrustum::getPlanes() const { return planes; }

/*---------------------------------------------------------*/
int PortalFrustum::getPortalClassification(const Portal &src) const {
    // test the absolute distance of the Portal center to each plane.
    // if it is equal or smaller than radius, we intersect
    // otherwise if the distance is below zero, we are away, then end up
    // immedietally telling so
    const Vector3 center = src.mCenter;
    float radius = src.mRadius;

    for (const auto &plane : planes) {
        float dist = plane.getDistance(center);

        if (fabs(dist) < radius) // intersection
            return 0;

        if (dist < -radius) // totally outside, so we can safely say we do not
                            // see the poly by our frustum
            return -1;
    }

    return 1;
}

/*---------------------------------------------------------*/
std::unique_ptr<Portal> PortalFrustum::clipPortal(const Portal &src) const {
    // todo: test if the bounding box (sphere) is completely: inside/outside, or
    // intersecting. todo: only clip the poly if intersecting.

    int cl = getPortalClassification(src);

    std::unique_ptr<Portal> new_poly(new Portal(src));

    if (cl == 1) // all inside
        return new_poly;

    if (cl == -1) // all outside
        return nullptr;

    // The poly intersects with its bounding volume, so clip it using all planes
    // we have
    for (const auto &plane : planes) {
        bool ignore;
        if (new_poly->clipByPlane(plane, ignore) <= 2) {
            // was totally clipped away
            return nullptr;
        }
    }

    return new_poly;
}

} // namespace Ogre
