#ifndef QUICKGUIVERTEX_H
#define QUICKGUIVERTEX_H

#include "OgreColourValue.h"
#include "OgreVector3.h"
#include "OgreVector2.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"

// From OpenGUI: Renderer_Ogre_Renderer.h

namespace QuickGUI
{
	struct _QuickGUIExport Vertex
	{
		Ogre::Vector3 pos;
		Ogre::RGBA color;
		Ogre::Vector2 uv;
	};
}

#endif
