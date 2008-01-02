#ifndef QUICKGUIHORIZONTALSCROLLBAR_H
#define QUICKGUIHORIZONTALSCROLLBAR_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIButton.h"
#include "QuickGUIWidget.h"

namespace QuickGUI
{
	class _QuickGUIExport HorizontalScrollBar :
		public Widget
	{
	public:
		friend class ScrollPane;
		friend class Panel;

		enum ButtonLayout
		{
			BUTTON_LAYOUT_ADJACENT_LEFT		=  0,
			BUTTON_LAYOUT_ADJACENT_RIGHT		,
			BUTTON_LAYOUT_MULTIPLE_BOTH			,
			BUTTON_LAYOUT_MULTIPLE_LEFT			,
			BUTTON_LAYOUT_MULTIPLE_RIGHT		,
			BUTTON_LAYOUT_NONE					,
			BUTTON_LAYOUT_OPPOSITE			
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
			@note
				Vertical or Horizontal TrackBars are derived from a comparison between width and height.
        */
		HorizontalScrollBar(const Ogre::String& name, GUIManager* gm);

		// Same as setValue, except that the scroll event is not fired.
		void _setValue(Ogre::Real value);

		/**
		* Add user defined event that will be called when amount of progress has changed.
		*/
		template<typename T> void addOnScrollEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnScrollHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnScrollEventHandler(MemberFunctionSlot* function);

		ButtonLayout getButtonLayout();
		Ogre::Real getLargeChange();
		/**
		* Gets the amount of time the left mouse button is down over a button or bar
		* before another scroll occurs.
		*/
		Ogre::Real getRepeatTime();
		Ogre::Real getSliderWidth();
		Ogre::Real getSmallChange();
		/**
		* Gets the numerical value representing the position of the left end of the slider, relative to the track bounds.
		* NOTE: value should be between 0.0 and 1.0
		*/
		Ogre::Real getValue();

		void scrollLeftLarge();
		void scrollLeftSmall();
		void scrollRightLarge();
		void scrollRightSmall();
		void setButtonLayout(ButtonLayout layout);
		void setLargeChange(Ogre::Real change);
		/**
		* Sets the amount of time the left mouse button is down over a button or bar
		* before another scroll occurs.
		*/
		void setScrollRepeatTime(Ogre::Real timeInSeconds);
		void setSmallChange(Ogre::Real change);
		/**
		* Sets the numerical value representing the position of the left end of the slider, relative to the track bounds.
		* NOTE: value should be between 0.0 and 1.0
		*/
		void setValue(Ogre::Real value);

		void timeElapsed(const Ogre::Real time);

	protected:
		virtual ~HorizontalScrollBar();

		// time in seconds, ie 0.5
		Ogre::Real mScrollRepeatTime;
		Ogre::Real mRepeatTimer;

		// last recorded slider position
		Point mSliderPosition;

		Ogre::Real mMinSliderPosition;
		Ogre::Real mMaxSliderPosition;
		void _determineMinMax();

		Ogre::Real mLargeChange;
		Ogre::Real mSmallChange;

		ButtonLayout mButtonLayout;

		Button* mSlider;
		Ogre::String mSliderTextureName;

		void setSliderWidth(Ogre::Real relativeWidth);
		void _constrainSlider();

		Ogre::String mScrollLeftTextureName;
		Ogre::String mScrollRightTextureName;

		Button* mScrollLeft1;
		Button* mScrollLeft2;
		Button* mScrollRight1;
		Button* mScrollRight2;

		void _scroll(Ogre::Real change, ScrollEventArgs args);
		void onScrollLeftDown(const EventArgs& args);
		void onScrollRightDown(const EventArgs& args);

		bool mMouseDownOnTrack;
		MouseButtonID mButtonDown;

		void onMouseDownOnTrack(const EventArgs& args);
		void onMouseUpOnTrack(const EventArgs& args);
		void onMouseLeaveTrack(const EventArgs& args);
		void onSizeChanged(const EventArgs& args);
		void onSliderDragged(const EventArgs& args);

		EventHandlerArray mOnScrollHandlers;

		void _showButtons();
	};
}

#endif
