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

#include "CustomImageCodec.h"
#include "FreeImage.h"

#include <OgreDataStream.h>
#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreString.h>

#define _TRANSPARENT_PAL_INDEX 0

namespace Ogre {
/*---------------------------------------------------------*/
/*-------------------- CustomImageCodec -------------------*/
/*---------------------------------------------------------*/

/// Static registration/unregistration methods, and that sorta stuff
CustomImageCodec::RegisteredCodecList CustomImageCodec::msCodecList;
CustomImageCodec::CodecList CustomImageCodec::msReplacedCodecs;

bool CustomImageCodec::msGIFFound = false;
bool CustomImageCodec::msPCXFound = false;

void CustomImageCodec::startup(void) {
    // We should have the freeimage library already started by ogre, but let's
    // see
    // TODO: FreeImage_Initialise(false);

    // We're only interested in gif and pcx files...
    // Let's see if we can find those
    for (int i = 0; i < FreeImage_GetFIFCount(); ++i) {
        // one format, many extensions. What's this FIF thingie anyway? ;)
        if ((FREE_IMAGE_FORMAT)i != FIF_GIF && (FREE_IMAGE_FORMAT)i != FIF_PCX)
            continue;

        if ((FREE_IMAGE_FORMAT)i != FIF_GIF)
            msGIFFound = true;

        if ((FREE_IMAGE_FORMAT)i != FIF_PCX)
            msPCXFound = true;

        String exts(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i));

        // Find out the extensions the format uses, and register those
        StringVector extsVector = StringUtil::split(exts, ",");

        for (StringVector::iterator v = extsVector.begin();
             v != extsVector.end(); ++v) {
            ImageCodec *codec = new CustomImageCodec(*v, i);

            msCodecList.push_back(codec);

            try {
                Codec *cod = Codec::getCodec(*v);
                msReplacedCodecs[*v] = cod;
                Codec::unregisterCodec(cod);
            } catch (Ogre::Exception) {
                // Nothing to do... that means the codec was not registered by
                // the previous codec impl.
            }

            Codec::registerCodec(codec);
        }
    }

    if (!msGIFFound)
        LogManager::getSingleton().logMessage(
            LML_NORMAL, "Warning: GIF support will fail-back - Free image "
                        "seems not to support GIF files");

    if (!msPCXFound)
        LogManager::getSingleton().logMessage(
            LML_NORMAL, "Warning: PCX support will fail-back - Free image "
                        "seems not to support PCX files");
}

void CustomImageCodec::shutdown(void) {
    // TODO: FreeImage_DeInitialise();

    // Loop no. 1. Remove all the registered codecs
    for (RegisteredCodecList::iterator i = msCodecList.begin();
         i != msCodecList.end(); ++i) {
        Codec::unregisterCodec(*i);
        delete *i;
    }

    // Loop no. 1 : Replace the codecs back to thei're originals
    for (CodecList::iterator it = msReplacedCodecs.begin();
         it != msReplacedCodecs.end(); ++it) {
        if (it->second) // only re-register if there was an original in the
                        // first place
            Codec::registerCodec(it->second);
    }

    msCodecList.clear();
    msReplacedCodecs.clear();
}

// ---------------------------------------------------------------------------------------------------
CustomImageCodec::CustomImageCodec(const String &type, unsigned int fiType)
    : mType(type), mFreeImageType(fiType){};

// ---------------------------------------------------------------------------------------------------
CustomImageCodec::~CustomImageCodec(){};

// ---------------------------------------------------------------------------------------------------
DataStreamPtr CustomImageCodec::encode(MemoryDataStreamPtr &input,
                                       CodecDataPtr &pData) const {
    OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                "Cannot code with CustomImageCodec", "CustomImageCodec::code");
}

// ---------------------------------------------------------------------------------------------------
void CustomImageCodec::encodeToFile(MemoryDataStreamPtr &input,
                                    const String &outFileName,
                                    CodecDataPtr &pData) const {
    OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                "Cannot code with CustomImageCodec",
                "CustomImageCodec::codeToFile");
}

// ---------------------------------------------------------------------------------------------------
Codec::DecodeResult CustomImageCodec::decode(DataStreamPtr &input) const {
    if (!input.get())
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot decode empty stream",
                    "CustomImageCodec::decode");

    // Buffer stream into memory (TODO: override IO functions instead?)
    MemoryDataStream memStream(input, true);

    FIMEMORY *fiMem = FreeImage_OpenMemory(
        memStream.getPtr(), static_cast<DWORD>(memStream.size()));

    FIBITMAP *fiBitmap =
        FreeImage_LoadFromMemory((FREE_IMAGE_FORMAT)mFreeImageType, fiMem);

    MemoryDataStreamPtr output;

    // Must derive format first, this may perform conversions

    FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(fiBitmap);
    FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType(fiBitmap);

    unsigned bpp = FreeImage_GetBPP(fiBitmap);

    if (imageType == FIT_BITMAP && colourType == FIC_PALETTE && bpp == 8) {
        // Convert to 24 bpp using our own method. Pal. index 0 will get alpha
        // 255, other will get 0
        RGBQUAD *pal = FreeImage_GetPalette(fiBitmap);

        if (pal == NULL) {
            FreeImage_Unload(fiBitmap);
            FreeImage_CloseMemory(fiMem);

            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "CustomImageCodec does not support 8 bit images "
                        "without palette",
                        "CustomImageCodec::decode");
        }

        ImageData *imgData = new ImageData();

        imgData->depth = 1; // only 2D formats handled by this codec
        imgData->width = FreeImage_GetWidth(fiBitmap);
        imgData->height = FreeImage_GetHeight(fiBitmap);
        imgData->num_mipmaps = 0; // no mipmaps in non-DDS
        imgData->flags = 0;
        imgData->format = PF_A8R8G8B8;

        // iterate through the image. Create a new one using the conversion...
        unsigned char *srcData = FreeImage_GetBits(fiBitmap);
        unsigned srcPitch = FreeImage_GetPitch(fiBitmap);

        // The new image data will have 32 bits
        size_t dstPitch =
            imgData->width * PixelUtil::getNumElemBytes(PF_A8R8G8B8);
        imgData->size = dstPitch * imgData->height;

        // Bind output buffer
        output.reset(new MemoryDataStream(imgData->size));

        uchar *pSrc;
        uchar *pDst = output->getPtr();

        for (size_t y = 0; y < imgData->height; ++y) {
            pSrc = srcData + (imgData->height - y - 1) * srcPitch;
            uchar *pDstRow = pDst;

            // X axis. Convert using pal
            for (size_t x = 0; x < imgData->width; ++x) {
                uchar pidx = pSrc[x];

                if (pidx == _TRANSPARENT_PAL_INDEX) {
                    *(pDstRow++) = 0; // Absolutely transparent black
                    *(pDstRow++) = 0; // This is to avoid pink edges
                    *(pDstRow++) = 0;
                    *(pDstRow++) = 0;
                } else {
                    *(pDstRow++) = pal[pidx].rgbBlue;
                    *(pDstRow++) = pal[pidx].rgbGreen;
                    *(pDstRow++) = pal[pidx].rgbRed;
                    *(pDstRow++) = 255;
                }
            }

            pDst += dstPitch;
        }

        FreeImage_Unload(fiBitmap);
        FreeImage_CloseMemory(fiMem);

        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);
        return ret;

    } else {

        // Use the underlying codec, if any
        // Unload the image
        FreeImage_Unload(fiBitmap);
        FreeImage_CloseMemory(fiMem);

        CodecList::iterator it = msReplacedCodecs.find(getType());

        if (it != msReplacedCodecs.end() && (it->second)) {
            return it->second->decode(input);
        } else {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "CustomImageCodec does not support other formats than "
                        "FIT_BITMAP, and no underlying Codec found for type " +
                            getType(),
                        "CustomImageCodec::decode");
        }
    };
}

// ---------------------------------------------------------------------------------------------------
String CustomImageCodec::getType() const { return mType; }

} // namespace Ogre
