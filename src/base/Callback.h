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

#ifndef __CALLBACK_H
#define __CALLBACK_H

#include "config.h"

namespace Opde {

/** An abstract callback functor definition. */
template <class M> class Callback {
public:
    virtual void operator()(const M &msg) = 0;

    virtual ~Callback(){};
};

/** A callback definition for a class method. One parameter callback
 * implementation. The template parameter I is the instance class */
template <class MSG, class I> class ClassCallback : public Callback<MSG> {
public:
    typedef void (I::*Method)(const MSG &msg);

    ClassCallback(I *instance, Method method)
        : mInstance(instance), mMethod(method){};

    virtual ~ClassCallback(){};

    virtual void operator()(const MSG &msg) { (*mInstance.*mMethod)(msg); }

protected:
    I *mInstance;
    Method mMethod;
};
} // namespace Opde

#endif
