/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __RENDERALPHAPROPERTY_H
#define __RENDERALPHAPROPERTY_H

#include "RenderedProperty.h"

namespace Opde {

/** a RenderAlpha property implementation using rendered property handler.
 * Controls the transparency of the object
 * Uses simple unsigned float data storage. Defaults to 1.0 - No transparency.
 * Inherits always.
 */
class RenderAlphaProperty : public RenderedProperty {
public:
    /// constructor
    RenderAlphaProperty(RenderService *rs, PropertyService *owner);

    /// destructor
    virtual ~RenderAlphaProperty(void);

protected:
    /// @see ActiveProperty::addProperty
    void addProperty(int oid);

    /// @see ActiveProperty::removeProperty
    void removeProperty(int oid);

    /// @see ActiveProperty::setPropertySource
    void setPropertySource(int oid, int effid);

    /// @see ActiveProperty::valueChanged
    void valueChanged(int oid, const std::string &field, const DVariant &value);

    /// core setter method. Called from other methods to set the render alpha
    /// value
    void setAlpha(int oid, float alpha);

    Ogre::SceneManager *mSceneMgr;
};
}; // namespace Opde

#endif
