/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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
 *	  $Id$
 *
 *****************************************************************************/

// This source code is a fixed and maintained version of the Null render system written by xyzzy@ogre3d.org forum

#ifndef __NULLRENDERWINDOW_H
#define __NULLRENDERWINDOW_H

#include "stdafx.h"

namespace Ogre {
	
	class NULLRenderWindow : public RenderWindow
	{
		public:
			NULLRenderWindow();
			~NULLRenderWindow();
			void create(const String& name, unsigned int width, unsigned int height,
					bool fullScreen, const NameValuePairList *miscParams);
			void destroy(void);
			bool isClosed() const { return mClosed; }
			void reposition(int left, int top);
			void resize(unsigned int width, unsigned int height);
			void swapBuffers( bool waitForVSync = true ) {};
			int getWindowHandle() const { return mWindowHandle; }
			virtual void writeContentsToFile(const String& filename) {}
			virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer=FB_AUTO) {};
			virtual bool requiresTextureFlipping() const { return false;}
	
			/// @copydoc RenderTarget::update
			void update(bool swap);
		protected:
			bool	mIsExternal;			// window not created by Ogre
			bool	mSizing;
			bool	mClosed;
			bool	mIsSwapChain;			// Is this a secondary window?
			static	unsigned int mWindowHandle;
	
	};
}

#endif
