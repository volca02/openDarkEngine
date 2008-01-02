#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUITextHelper.h"

namespace QuickGUI
{
	TextHelper::TextHelper() :
		mFontTextureHeight(0),
		mFontTextureWidth(0)
	{
	}

	TextHelper::~TextHelper()
	{
		mFont.setNull();
		mFontTexture.setNull();
	}

	Ogre::String TextHelper::getFont()
	{
		if(mFont.isNull())
			return "";

		return mFont->getName();
	}

	Ogre::FontPtr TextHelper::getFontTexturePtr()
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get font texture!","Text::getFontTexturePtr" );

		return (Ogre::FontPtr)mFontTexture; 
	}

	Ogre::Real TextHelper::getFontTextureHeight()
	{
		return mFontTextureHeight;
	}

	Ogre::Real TextHelper::getFontTextureWidth()
	{
		return mFontTextureWidth;
	}

	Ogre::String TextHelper::getFontMaterialName()
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get font material name!","Text::getFontMaterialName" );

		return mFont->getMaterial()->getName();
	}

	Size TextHelper::getGlyphSize(Ogre::UTFString::unicode_char c)
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get glyph size!","Text::getGlyphSize" );

		bool tab = false;

		// Use 'r' for the space character, because its width is most similar
		if( isSpace(c) || (tab = isTab(c)) )
			c = 'r';

		Ogre::Font::UVRect uvRect = mFont->getGlyphTexCoords(c);
		float width = ((uvRect.right - uvRect.left) * mFontTextureWidth);
		float height = ((uvRect.bottom - uvRect.top) * mFontTextureHeight);

		// shrink size a little bit to increase sharpness and solve weird blurry issue.
		if(tab)
			return Size(width * SPACES_PER_TAB,height) * TEXT_MULTIPLIER;
		else
			return Size(width,height) * TEXT_MULTIPLIER;
	}

	Ogre::Real TextHelper::getGlyphWidth(Ogre::UTFString::unicode_char c)
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get glyph width!","Text::getGlyphWidth" );

		bool tab = false;

		// Use 'r' for the space character, because its width is most similar
		if( isSpace(c) || (tab = isTab(c)) )
			c = 'r';

		Ogre::Font::UVRect uvRect = mFont->getGlyphTexCoords(c);
		float width = ((uvRect.right - uvRect.left) * mFontTextureWidth);

		// shrink size a little bit to increase sharpness and solve weird blurry issue.
		if(tab)
			return (width * SPACES_PER_TAB * TEXT_MULTIPLIER);
		else
			return (width * TEXT_MULTIPLIER);
	}

	Ogre::Real TextHelper::getGlyphHeight()
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get glyph height!","Text::getGlyphHeight" );

		Ogre::Font::UVRect uvRect = mFont->getGlyphTexCoords('0');
		float height = ((uvRect.bottom - uvRect.top) * mFontTextureHeight);

		// shrink size a little bit to increase sharpness and solve weird blurry issue.
		return (height * TEXT_MULTIPLIER);
	}

	Ogre::Font::UVRect TextHelper::getGlyphTexCoords(Ogre::UTFString::unicode_char c)
	{
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Font has not been set, cannot get glyph coords!","Text::getGlyphTexCoords" );

		return mFont->getGlyphTexCoords(c);
	}

	Ogre::Real TextHelper::getSpaceWidth()
	{
		return getGlyphWidth('r');
	}

	Ogre::Real TextHelper::getTabWidth()
	{
		return (getSpaceWidth() * SPACES_PER_TAB);
	}

	Ogre::Real TextHelper::getTextWidth(const Ogre::String& text)
	{
		Ogre::Real width = 0;
		
		Ogre::Font::UVRect uvRect;
		unsigned int index = 0;
		while( index < text.length() )
		{
			width += getGlyphWidth(text[index]);
			++index;
		}

		return width;
	}

	void TextHelper::setFont(const Ogre::String& fontName)
	{
		if(fontName == "")
			return;

		mFont = static_cast<Ogre::FontPtr>(Ogre::FontManager::getSingleton().getByName(fontName));
		 
		if(mFont.isNull())
			throw Ogre::Exception( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Could not find font \"" + fontName + "\".","Text::setFont" );

		// make sure font is loaded.
		mFont->load();
		mFontTexture = 
			static_cast<Ogre::TexturePtr>(
			Ogre::TextureManager::getSingleton().getByName(mFont->getMaterial()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName())
			);
		mFontTextureWidth = mFontTexture->getWidth();
		mFontTextureHeight = mFontTexture->getHeight();
	}
}
