#ifndef QUICKGUITEXT_H
#define QUICKGUITEXT_H

#include "OgreException.h"
#include "OgreFont.h"
#include "OgreFontManager.h"
#include "OgreStringConverter.h"
#include "OgreUTFString.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIEventArgs.h"
#include "QuickGUIExportDLL.h"
#include "QuickGUIMemberFunctionPointer.h"
#include "QuickGUIQuad.h"
#include "QuickGUIQuadContainer.h"
#include "QuickGUITextHelper.h"

#include <vector>

namespace QuickGUI
{
	// forward declarations
	class Widget;
	class Label;
	class GUIManager;

	class _QuickGUIExport Text
	{
	public:
		// Giving class ability to destroy this Object.
		friend class Label;

		// Text can be either Horizontal or Vertically printed
		typedef enum Layout
		{
			LAYOUT_HORIZONTAL	=  0,
			LAYOUT_VERTICAL
		};
		// How the text in aligned, line-by-line
		typedef enum Alignment
		{
			ALIGNMENT_LEFT		=  0,
			ALIGNMENT_MID,
			ALIGNMENT_RIGHT
		};
	public:
		Text(const Ogre::String& name, QuadContainer* container, Label* owner);
		~Text();

		// Internal function that sets the widget dimensions to use for clipping.
		void _clipToWidgetDimensions(Widget* w);
		void _notifyQuadContainer(QuadContainer* container);

		/**
		* Add user defined events, that will be called when text has changed.
		*/
		template<typename T> void addOnTextChangedEventHandler(bool (T::*function)(const EventArgs&), T* obj)
		{
			mOnTextChangedUserEventHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnTextChangedEventHandler(MemberFunctionSlot* function);

		/*
		* Clears selection by hiding the Selection Quad.
		*/
		void clearSelection();
		void clearCaption();
		/*
		* Returns the x,y position and width,height of the area encasing the text.
		* Form: (x,y,w,h)
		* NOTE: value will be Vector4::ZERO is there is no caption for this text.
		*/
		Rect getDimensions();
		Ogre::UTFString getCaption();
		Quad* getCharacter(unsigned int index);
		Ogre::ColourValue getColor();
		Ogre::String getFont();
		Ogre::Real getGlyphHeight();
		Size getGlyphSize(Ogre::UTFString::code_point c);
		Ogre::Real getGlyphWidth(Ogre::UTFString::code_point c);
		Ogre::ColourValue getInverseColor(const Ogre::ColourValue& c);
		/*
		* Get the current linespacing this text is using
		*/
		Ogre::Real getLineSpacing();
		Ogre::Real getNewlineHeight();
		/*
		* Returns the number of characters rendered to the screen.
		* NOTE: this may not be the same as the Caption length, since
		* the rendered text has a bounded region, and may not fit.
		*/
		int getNumberOfCharacters();
		Point getPosition();
		/*
		* Returns the selections portion of the caption.
		*/
		Ogre::UTFString getSelection();
		/*
		* Returns the index of the first character in the text selection.
		* If no selection is present, -1 is returned.
		*/
		int getSelectionEnd();
		/*
		* Returns the index of the last character in the text selection.
		* If no selection is present, -1 is returned.
		*/
		int getSelectionStart();
		/*
		* Returns the index of the glyph in the caption, not the code point.
		* NOTE: If position is less than beginning of text, index 0 is returned.  If
		*  position is more than end of text, index (length - 1) is returned.  If caption
		*  is empty, -1 is returned.
		*/
		int getTextIndex(const Point& pixelPosition);
		/*
		* Tests if absolute position is within any character bounds of the text caption.
		* Each cursor location to the left and right of a character is a cursor index.  
		* The cursor index to the left of character 0 is 0, for example.
		*/
		int getTextCursorIndex(const Point& pixelPosition);
		/*
		* Tests if the given rectangle intersects with any characters of the text caption.
		* Each cursor location to the left and right of a character is a cursor index.  
		* The cursor index to the left of character 0 is 0, for example.
		*/
		int getTextCursorIndex(const Rect& pixelDimensions);
		Ogre::Real getTextWidth(const Ogre::String& text);
		bool getVisible();

		void hide();

		/*
		* This function could potentially move the text anywhere on the screen, but its main use is
		* to maintain its position relative to the widget that owns the text.  Only call this function
		* if you want to override the default positioning of the text, relative to its owner.
		*/
		void move(const Point& pixelPosition);

		/**
		* Specific Handler for the Text object.  Called when the Text font, color, or caption changes.
		*/
		void onTextChanged(const TextEventArgs& e);

		/*
		* Rerenders the text, with current font and text color.  Useful to call
		* if Widget dimensions have changed.
		*/
		void redraw();
		/*
		* Remove selected characters from the caption.
		*/
		void removeSelection();
		/*
		* Highlights entire caption, denoting a selection.
		*/
		void selectCharacters();
		/*
		* Highlights a substring of the caption, denoting a selection.
		*/
		void selectCharacters(unsigned int startIndex, unsigned int endIndex);
		/*
		* Sets the text Caption.  Size will be calculated from glyph dimensions.
		*/
		void setCaption(const Ogre::UTFString& text, Layout l = LAYOUT_HORIZONTAL, Alignment a = ALIGNMENT_LEFT);
		void setFont(const Ogre::String& fontName);
		void setQuadLayer(Quad::Layer layer);
		void setOffset(int offset);
		/*
		* This function could potentially place the text anywhere on the screen, but its main use is
		* to maintain its position relative to the widget that owns the text.  Only call this function
		* if you want to override the default positioning of the text, relative to its owner.
		*/
		void setPosition(const Point& pixelPosition);
		/*
		* Sets the color of all characters of the text.
		*/
		void setColor(Ogre::ColourValue color);
		void setGUIManager(GUIManager* gm);
		/*
		* Sets the multiple of normal line spacing that will be used to draw multiple lines of text.
		*/
		void setLineSpacing(Ogre::Real spacing);
		void show();

	private:
		Label* mOwner;
		Quad::Layer mLayer;
		GUIManager* mGUIManager;

		Ogre::String mName;
		QuadContainer* mQuadContainer;
		int mOffset;
		bool mVisible;

		Widget* mClippingWidget;

		Ogre::UTFString mCaption;
		Layout mLayout;
		Alignment mAlignment;
		Ogre::Real mLineSpacing;

		TextHelper* mTextHelper;

		Ogre::ColourValue mColor;

		QuadArray mCharacters;
		// used for selecting/highlighting characters
		Quad* mCharacterBackground;
		Ogre::ColourValue mSelectColor;
		Ogre::ColourValue mBackgroundSelectColor;
		int mSelectStart;
		int mSelectEnd;

		Rect mPixelDimensions;

		// User defined event handlers that are called when a Selection is made.
		EventHandlerArray mOnTextChangedUserEventHandlers;

		void _calculateDimensions();
		void _clearCharacters();

		void _setCaptionHorizontal(const Ogre::UTFString& text);
		void _setCaptionVertical(const Ogre::UTFString& text);
	};
}

#endif
