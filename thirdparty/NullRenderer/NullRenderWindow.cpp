#include "stdafx.h"
#include "NullRenderWindow.h"

namespace Ogre {

	unsigned int NULLRenderWindow::mWindowHandle = 1;

	NULLRenderWindow::NULLRenderWindow() 
	{
		mIsFullScreen = false;
		mIsExternal = false;
		mSizing = false;
		mClosed = false;
	}

	NULLRenderWindow::~NULLRenderWindow()
	{
		destroy();
	}


	void NULLRenderWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		mWindowHandle++;

	}

	void NULLRenderWindow::destroy()
	{
		
	}

	void NULLRenderWindow::reposition(int top, int left)
	{
	}

	void NULLRenderWindow::resize(unsigned int width, unsigned int height)
	{
	}

	//-----------------------------------------------------------------------------
	void NULLRenderWindow::update(bool swap)
	{
		RenderWindow::update(swap);
	}

}
