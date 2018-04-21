/******************************************************************************
 *
 *	  This file is part of openDarkEngine project
 *	  Copyright (C) 2005-2006 openDarkEngine team
 *
 *	  This program is free software; you can redistribute it and/or modify
 *	  it under the terms of the GNU General Public License as published by
 *	  the Free Software Foundation; either version 2 of the License, or
 *	  (at your option) any later version.
 *
 *	  This program is distributed in the hope that it will be useful,
 *	  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	  GNU General Public License for more details.
 *
 *	  You should have received a copy of the GNU General Public License
 *	  along with this program; if not, write to the Free Software
 *	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *02111-1307	USA
 *
 *	  $Id$
 *
 *****************************************************************************/

/**
 @file CustomImageCodec.h
 @brief A custom image codec. This class hooks into the Ogre::Codec, providing
 transparency for palletized images (extensions .pcx, .gif)
*/

#ifndef __CUSTOMIMAGECODEC_H
#define __CUSTOMIMAGECODEC_H

#include "config.h"

#include <OgreCodec.h>
#include <OgreImageCodec.h>

namespace Ogre {

// Interface cloned from Ogre::FreeImageCodec

/// Custom image codec providing transparency for pallete index 0 on 8bit
/// palletized images of .PCX and .GIF formats
class CustomImageCodec : public ImageCodec {
private:
    String mType;
    unsigned int mFreeImageType;

    typedef std::list<ImageCodec *> RegisteredCodecList;
    static RegisteredCodecList msCodecList;

    // Hooks. These are the previous codecs registered for the given extensions
    static CodecList msReplacedCodecs;

    static bool msGIFFound;
    static bool msPCXFound;

public:
    CustomImageCodec(const String &type, unsigned int fiType);
    ~CustomImageCodec();

    /// A void implementation if code, throwing OGRE_EXCEPTION
    DataStreamPtr encode(MemoryDataStreamPtr &input, CodecDataPtr &pData) const;

    /// A void implementation if codeToFile, throwing
    /// OGRE_EXCEPTION
    void encodeToFile(MemoryDataStreamPtr &input, const String &outFileName,
                      CodecDataPtr &pData) const;

    /// @copydoc Codec::decode
    DecodeResult decode(DataStreamPtr &input) const;

    /// Type getter
    virtual String getType() const;

    /// Static method to startup FreeImage and register the FreeImage codecs
    static void startup(void);

    /// Static method to shutdown FreeImage and unregister the FreeImage codecs
    static void shutdown(void);

    String magicNumberToFileExt(const char *magicNumberPtr,
                                size_t maxbytes) const {
        return "";
    };
};

} // namespace Ogre
#endif
