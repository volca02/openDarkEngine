#ifndef QUICKGUIVERTICALTRACKBAR_H
#define QUICKGUIVERTICALTRACKBAR_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIButton.h"
#include "QuickGUIWidget.h"

#include <vector>

namespace QuickGUI
{
	/** Also known as a Slider.
		Consists of a Bar and a Button that slides
		from one end to the other.
		@remarks
		Layout (vertical/horizontal) determined by dimensions.
		@note
		Must be created by a Panel, Window, or Sheet.
	*/
	class _QuickGUIExport VerticalTrackBar :
		public Widget
	{
	public:
		friend class Panel;
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
		VerticalTrackBar(const Ogre::String& name, GUIManager* gm);

		/**
		* Add user defined event that will be called when amount of progress has changed.
		*/
		template<typename T> void addOnValueChangedEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnValueChangedHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnValueChangedEventHandler(MemberFunctionSlot* function);
		/*
		* Given a point P, returns the closest slider position.
		* NOTE: Point must be relative to the screen for accurate results.
		*/
		int getClosestSliderPosition(const Point& pixelPoint);
		/*
		* Returns the value added to Current Value when user clicks on the Track Bar.
		*/
		int getLargeChange();
		/*
		* Returns the number of divisible regions that make up the trackbar.
		*/
		int getNumRegions();
		/*
		* Returns the number of positions the slider button can be positioned at.
		*/
		int getNumTicks();
		/**
		* Returns a decimal between 0.0 and 1.0 signifying the location of the slider.
		*/
		Ogre::Real getPercentage();
		/**
		* Returns the Value, or current slider index, of the TrackBar.
		*/
		int getValue();
		/**
		* Default Handler for handling progress changes.
		* Note that this event is not passed to its parents, the event is specific to this widget.
		*/
		void onValueChanged(const WidgetEventArgs& args);
		/**
		* Overridden Handler.  Allows changing of slider position by clicking on trackbar.
		*/
		void onMouseButtonDown(const EventArgs& args);
		/*
		* Set the value applied to the currentValue when the track bar is clicked.
		*/
		void setLargeChange(unsigned int LargeChange);
		/**
		* Every region has a beginning and end, shared with neighboring regions. There will
		* always be (Region + 1) number of region boundaries (Ticks). Related to setNumTicks.
		*/
		void setNumRegions(unsigned int NumRegions);
		/**
		* Sets the number of positions Slider can be moved to. Related to setNumRegions.
		*/
		void setNumTicks(unsigned int NumTicks);
		void setSliderSize(Size pixelSize);
		/**
		* Sets the index of the Slider along the TrackBar.
		* NOTE: In horizontal TrackBars, left side is index 0.  In vertical, bottom is 0.
		* NOTE: Values below 0 will be set to 0, and values above the number of regions will be set to number of regions.
		*/
		void setValue(int Value);

	protected:
		virtual ~VerticalTrackBar();

		int mCurrentValue;
		// Value added to current value when user clicks on the track bar.
		int mLargeChange;

		// ----- TRACK PROPERTIES --------------------
		// number of available positions for positioning the slider.
		int mNumRegions;
		// Length between a region begin and endpoint;
		Ogre::Real mRegionLength;

		std::vector<Ogre::Real> mSliderPositions;
		void _getSliderPositions();

		// ----- SLIDER BUTTON PROPERTIES --------------------
		Button* mSliderButton;
		Ogre::String mSliderTextureName;

		void onSliderDragged(const EventArgs& args);

		// ----- EVENT HANDLERS --------------------
		void onMouseDownOnSlider(const EventArgs& args);
		void onMouseUpOnSlider(const EventArgs& args);

		EventHandlerArray mOnValueChangedHandlers;
	};
}

#endif
