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
