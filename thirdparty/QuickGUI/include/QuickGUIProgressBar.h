#ifndef QUICKGUIPROGRESSBAR_H
#define QUICKGUIPROGRESSBAR_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIWidget.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"

namespace QuickGUI
{
	/** Represents a vertical or horizontal bar displaying
		visual status showing a current position relative to
		the beginning and end of the bar.
		@remarks
		Useful for Life Bars, or anything similar.
		@note
		ProgressBars must be created by the Window class.
	*/
	class _QuickGUIExport ProgressBar :
		public Widget
	{
	public:
		/**
		* Allows setting the direction the progressbar fills.
		* FILLS_NEGATIVE_TO_POSITIVE: 
		*	For Vertical Layouts, bar moves bottom to top. For Horizontal, bar moves left to right.
		* FILLS_POSITIVE_TO_NEGATIVE: 
		*	For Vertical Layouts, bar moves top to bottom. For Horizontal, bar moves right to left.
		*/
		enum FillDirection
		{
			FILLS_NEGATIVE_TO_POSITIVE	=  0,
			FILLS_POSITIVE_TO_NEGATIVE
		};
		enum Layout
		{
			LAYOUT_HORIZONTAL	=  0,
			LAYOUT_VERTICAL
		};
		/**
		* Dictates what side of the texture to *chop*.
		* CHOP_NEGATIVE: 
		*	For Vertical Layouts, texture chopped at the bottom. For Horizontal, texture chopped at the left.
		* CHOP_POSITIVE: 
		*	For Vertical Layouts, texture chopped at the top. For Horizontal, texture chopped at the right.
		*/
		enum ClippingEdge
		{
			CLIP_LEFT_BOTTOM		=  0,
			CLIP_RIGHT_TOP
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
				fillDirection The direction that the bar fills as progress increases
			@param
				material Ogre material defining the widget image.
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		ProgressBar(const Ogre::String& name, GUIManager* gm);

		/**
		* Add user defined event that will be called when amount of progress has changed.
		*/
		template<typename T> void addOnProgressChangedEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnProgressChangedHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnProgressChangedEventHandler(MemberFunctionSlot* function);
		Ogre::Real getProgress();
		/**
		* Default Handler for handling progress changes.
		* Note that this event is not passed to its parents, the event is specific to this widget.
		*/
		void onProgressChanged(const WidgetEventArgs& e);
		void onPositionChanged(const EventArgs& args);
		void onSizeChanged(const EventArgs& args);
		/**
		* Force updating of the Widget's Quad position on screen.
		*/
		void redraw();
		void setClippingEdge(ClippingEdge e);
		void setFillDirection(FillDirection d);
		/**
		* Set the initial pixel padding added so that the progressbar will begin to show progress
		* immediately, instead of ignoring small progress levels. 
		*/
		void setInitialPixelOffset(unsigned int width);
		void setLayout(Layout l);
		/**
		* Sets progress.  Value should be between 0.0 and 1.0
		*/
		void setProgress(Ogre::Real progress);
		virtual void setSkin(const Ogre::String& skinName, bool recursive = false);

	protected:
		virtual ~ProgressBar();

		void _getBarExtents();
		void _modifyBarTexture(Ogre::Real progress);

	private:
		Quad* mBarPanel;
		Layout mLayout;

		Ogre::Real mProgress;

		Ogre::String mSkinExtension;
		Ogre::TexturePtr mBarTexture;
		Ogre::String mBarTextureName;
		Ogre::Image mBarImage;
		Ogre::MaterialPtr mBarMaterial;
		int mBarMinWidth;
		int mBarMaxWidth;
		int mBarMinHeight;
		int mBarMaxHeight;

		EventHandlerArray mOnProgressChangedHandlers;

		// How many pixels to add to the initial edge
		int mInitialPixelOffset;

		// Stores how this bar should fill as progress increases
		ProgressBar::FillDirection mFillDirection;

		// Stores how the bar texture is truncated.
		ProgressBar::ClippingEdge mClippingEdge;
	};
}

#endif
