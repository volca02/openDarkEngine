#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUISize.h"

namespace QuickGUI
{
	Size::Size() :
		width(0),
		height(0)
	{
	}

	Size::Size(Ogre::Real Width, Ogre::Real Height) :
		width(Width),
		height(Height)
	{
	}

	const Size Size::ZERO( 0, 0 );
}
