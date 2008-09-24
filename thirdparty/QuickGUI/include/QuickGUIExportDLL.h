#ifndef QUICKGUIEXPORTDLL_H
#define QUICKGUIEXPORTDLL_H

#include "OgrePlatform.h"

#ifndef _QuickGUIExport
   #if defined(OGRE_PLATFORM)
      #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined ( __MINGW32__ )
      #   if defined( QUICKGUI_EXPORTS )
      #      define _QuickGUIExport __declspec( dllexport )
      #   else
      #      define _QuickGUIExport __declspec( dllimport )
      #   endif
      #else
      #   define _QuickGUIExport
      #endif
    #else
      #define _QuickGUIExport
    #endif

#endif
#endif
