#ifndef QUICKGUITEXTHELPER_H
#define QUICKGUITEXTHELPER_H

#include "OgreException.h"
#include "OgreFont.h"
#include "OgreFontManager.h"
#include "OgrePrerequisites.h"
#include "OgreUTFString.h"

#include "QuickGUIExportDLL.h"
#include "QuickGUISize.h"

// How much to scale the text by for sharpness
#define TEXT_MULTIPLIER 0.9
// How many spaces per tab is standard
#define SPACES_PER_TAB 4

namespace QuickGUI
{
	class _QuickGUIExport TextHelper
	{
	public:
		enum 
		{
			UNICODE_NEL		= 0x0085,
			UNICODE_CR		= 0x000D,
			UNICODE_LF		= 0x000A,
			UNICODE_TAB		= 0x0009,
			UNICODE_SPACE	= 0x0020,
			UNICODE_ZERO	= 0x0030,
		};
	public:
		TextHelper();
		~TextHelper();

		Ogre::String getFont();
		Ogre::FontPtr getFontTexturePtr();
		Ogre::Real getFontTextureHeight();
		Ogre::Real getFontTextureWidth();
		Ogre::String getFontMaterialName();
		Size getGlyphSize(Ogre::UTFString::unicode_char c);
		Ogre::Real getGlyphWidth(Ogre::UTFString::unicode_char c);
		Ogre::Real getGlyphHeight();
		Ogre::Font::UVRect getGlyphTexCoords(Ogre::UTFString::unicode_char c);
		/*
		* Returns space width. (Actually, width of 'r')
		*/
		Ogre::Real getSpaceWidth();
		/*
		* Returns tab width. (Actually, width of 'r' times SPACES_PER_TAB)
		*/
		Ogre::Real getTabWidth();
		Ogre::Real getTextWidth(const Ogre::String& text);

		static inline bool	isTab			(Ogre::UTFString::unicode_char c) { return c == UNICODE_TAB; }
		static inline bool	isSpace			(Ogre::UTFString::unicode_char c) { return c == UNICODE_SPACE; }
		static inline bool	isNewLine		(Ogre::UTFString::unicode_char c) { return c == UNICODE_CR || c == UNICODE_LF || c == UNICODE_NEL; }
		static inline bool	isWhiteSpace	(Ogre::UTFString::unicode_char c) { return isTab(c) || isSpace(c) || isNewLine(c); }

		void setFont(const Ogre::String& fontName);

	protected:
		Ogre::FontPtr mFont;
		Ogre::TexturePtr mFontTexture;
		size_t mFontTextureWidth;
		size_t mFontTextureHeight;
	};
}

#endif
