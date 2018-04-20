/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *****************************************************************************/

#ifndef __RENDERCOMMON_H
#define __RENDERCOMMON_H

namespace Opde {

const unsigned int RENDER_TYPE_NORMAL = 0;
const unsigned int RENDER_TYPE_NOT_RENDERED = 1;
const unsigned int RENDER_TYPE_NO_LIGHTMAP = 2;
const unsigned int RENDER_TYPE_EDITOR_ONLY = 3;

/// Render System message type
enum RenderMessageType {
    /// Render window changed the size
    RENDER_WINDOW_SIZE_CHANGE = 1
};

/// window size message details
struct RenderWindowSize {
    /// New width of the window (pixels)
    unsigned int width;
    /// New height of the window (pixels)
    unsigned int height;
    /// The new window is fullscreen (or windowed)
    bool fullscreen;
    /// Display id, or -1 if not found
    int display;
    // TODO: Pixelformat

    RenderWindowSize() : width(0), height(0), fullscreen(false), display(-1) {}

    RenderWindowSize(unsigned int w, unsigned int h, bool fs = false)
        : width(w), height(h), fullscreen(fs), display(-1) {}
};

/// Render service message (Used to signalize a change in the renderer setup)
struct RenderServiceMsg {
    RenderMessageType msg_type;

    RenderWindowSize size;
};

} // namespace Opde

#endif /* __RENDERCOMMON_H */
