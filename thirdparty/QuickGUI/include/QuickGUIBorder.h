#ifndef QUICKGUIBORDER_H
#define QUICKGUIBORDER_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIWidget.h"

namespace QuickGUI
{
	/** Represents a simple Imagel.
		@remarks
		Pretty much a Label, but without text.
		@note
		Images also support Render To Texture.
	*/
	class _QuickGUIExport Border :
		public Widget
	{
	public:
		enum BorderType
		{
			BORDER_TYPE_TOP_LEFT		=  0,
			BORDER_TYPE_TOP_RIGHT			,
			BORDER_TYPE_BOTTOM_LEFT			,
			BORDER_TYPE_BOTTOM_RIGHT		,
			BORDER_TYPE_LEFT				,
			BORDER_TYPE_TOP					,
			BORDER_TYPE_RIGHT				,
			BORDER_TYPE_BOTTOM
		};
	public:
		/** Constructor
            @param
                name The name to be given to the widget (must be unique).
            @param
                dimensions The x Position, y Position, width and height of the widget.
			@param
				positionMode The GuiMetricsMode for the values given for the position. (absolute/relative/pixel)
			@param
				sizeMode The GuiMetricsMode for the values given for the size. (absolute/relative/pixel)
			@param
				material Ogre material defining the widget image.
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		Border(const Ogre::String& name, GUIManager* gm);

		BorderType getBorderType();

		void setBorderType(BorderType t);
		void setSkin(const Ogre::String& skinName, Ogre::String extension = ".png", bool recursive = false);

	protected:
		virtual ~Border();
		virtual void setParent(Widget* parent);
	protected:

		BorderType mBorderType;

		Ogre::Real mThickness;
		Ogre::Real mOverlap;

		// used to restore mouse cursor when leaving.
		Ogre::String mMouseCursorTexture;

		void onDragged(const EventArgs& args);
		void onMouseEnter(const EventArgs& args);
		void onMouseLeave(const EventArgs& args);
	};
}

#endif
