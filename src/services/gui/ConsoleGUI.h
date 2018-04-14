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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __CONSOLEGUI_H
#define __CONSOLEGUI_H

#include "config.h"

#include "ConsoleBackend.h"
#include "draw/DrawService.h"
#include "draw/FontDrawSource.h"
#include "draw/TextureAtlas.h"
#include "input/InputService.h"

namespace Opde {
/// forward decl.
class GUIService;

class OPDELIB_EXPORT ConsoleGUI {
public:
    ConsoleGUI(GUIService *owner);
    ~ConsoleGUI();

    /** Injects an ois keyboard event into the console
     * @return true if the keyboard event was consumed, false otherwise (Console
     * not visible)
     */
    bool injectKeyPress(unsigned int keycode);

    void setActive(bool active);

    inline bool isActive() const { return mIsActive; };

    /** Frame update method. Call every time frame update happens */
    void update(int timems);

    /// Used to rebuild the console according to the new resolution
    void resolutionChanged(size_t width, size_t height);

private:
    bool mIsActive;

    Ogre::String mCommand;

    int mPosition;

    ConsoleBackend *mConsoleBackend;
    GUIService *mOwner;
    FontDrawSourcePtr mFont;
    TextureAtlasPtr mAtlas;
    DrawServicePtr mDrawSrv;
    InputServicePtr mInputSrv;

    RenderedRect *mConsoleBackground;
    RenderedRect *mCommandLineBackground;
    RenderedLabel *mConsoleText;
    RenderedLabel *mCommandLine;
    DrawSheetPtr mConsoleSheet;
    ClipRect mTextClipRect;
    ClipRect mCmdLineClipRect;
    std::vector<Ogre::ColourValue> mConsoleColors;
};

} // namespace Opde

#endif
