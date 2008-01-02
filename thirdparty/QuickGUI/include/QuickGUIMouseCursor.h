#ifndef QUICKGUIMOUSECURSOR_H
#define QUICKGUIMOUSECURSOR_H

#include "OgreException.h"
#include "OgreImage.h"
#include "OgrePrerequisites.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"
#include "QuickGUIQuad.h"
#include "QuickGUIVertexBuffer.h"
#include "QuickGUIWidget.h"

#include <list>

namespace QuickGUI
{
	// Forward declarations
	class GUIManager;

	/** Mouse representation inside the primary render window
		@remarks
		Through GUIManager and its input handling, the
		mouse cursor gets updated and repositioned, simulating a mouse.
		@note
		MouseCursor is a Singleton.
	*/
	class _QuickGUIExport MouseCursor
	{
	public:
		enum CursorState
		{
			CURSOR_STATE_HOVER				=  0,
			CURSOR_STATE_NORMAL					,
			CURSOR_STATE_RESIZE_HORIZONTAL		,
			CURSOR_STATE_RESIZE_VERTICAL		,
			CURSOR_STATE_RESIZE_DIAGONAL_1		,
			CURSOR_STATE_RESIZE_DIAGONAL_2		,
			CURSOR_STATE_SPECIAL_1				,
			CURSOR_STATE_SPECIAL_2
		};
	public:
		/** Constructor
			@param
                size The width, and height of the window in Relative Dimensions.
			@param
				sizeMode The metrics mode used to interpret the size give. (relative/absolute/pixel)
			@param
				textureName Ogre material defining the cursor image.
        */
        MouseCursor(const Size& size, const Ogre::String& skinName, GUIManager* gm);
		/** Standard Destructor. */
		~MouseCursor();

		/**
		* Returns true if cursor is set to go invisible when mouse is off the screen, false otherwise.
		*/
		bool getHideWhenOffScreen();
		Point getPosition();
		Size getSize();
		/**
		* Hides the cursor.
		*/
		void hide();
		/**
		* Hides the cursor, but does not record the mouse is hidden.  Used for automatic hiding,
		* when mouse goes outside of screen.  When it comes back into window, we need to use mVisible
		* variable to tell us if user intends the mouse to be shown in window or not.
		*/
		void _hide();

		Ogre::String getTexture();
		
		bool isVisible();
		
		/**
		* Returns true if the mouse if on the bottom border of the screen, false otherwise.
		*/
		bool mouseOnBotBorder();
		/**
		* Returns true if the mouse if on the left border of the screen, false otherwise.
		*/
		bool mouseOnLeftBorder();
		/**
		* Returns true if the mouse if on the right border of the screen, false otherwise.
		*/
		bool mouseOnRightBorder();
		/**
		* Returns true if the mouse if on the top border of the screen, false otherwise.
		*/
		bool mouseOnTopBorder();
		/**
		* Applies the pixel offset to the current cursor position.
		*/
		void offsetPosition(const int& xPixelOffset, const int& yPixelOffset);
		/**
		* Render the mouse to screen.
		*/
		void render();
		void setCursorState(CursorState s);
		/**
		* if toggled, the cursor will become invisible when the mouse moves away from the primary render window.
		* only useful if you have the mouse created with the DISCL_NONEXCLUSIVE option. (Refer to OIS creation of the mouse)
		*/
		void setHideCursorWhenOSCursorOffscreen(bool hide);
		/**
		* Sets the position of the mouse cursor on the screen, in pixel coordinates.
		*/
		void setPosition(Ogre::Real pixelX, Ogre::Real pixelY);
		/**
		* Sets the Size of the mouse cursor on the screen, in pixel dimensions.
		*/
		void setSize(Ogre::Real pixelWidth, Ogre::Real pixelHeight);
		/**
		* Sets the Ogre material defining the mouse
		*/
		void setSkin(const Ogre::String& skinName);
		/**
		* Shows the cursor.
		*/
		void show();

	protected:
		GUIManager*		mGUIManager;

		CursorState		mCursorState;

		Ogre::String	mSkinName;
		Ogre::String	mSkinComponent;

		// Default texture.
		Ogre::String	mTextureName;

		// Width and Height in pixels
		Size			mPixelSize;
		Point			mPixelPosition;
		
		// Specifies the area (in screen pixels) that the mouse can move around in.
		Point			mConstraints;				
		bool			mOnTopBorder;
		bool			mOnBotBorder;
		bool			mOnLeftBorder;
		bool			mOnRightBorder;
		
		bool			mHideWhenOffScreen;
		bool			mVisible;

		Quad*			mQuad;
		QuadList mRenderObjectList;
		VertexBuffer*	mVertexBuffer;

		/**
		* Prevents the mouse cursor from going out of bounds.
		* The position can't exceed the primary render window dimensions (pixels)
		*/
		void constrainPosition();
	};
}

#endif
