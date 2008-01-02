#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIVerticalTrackBar.h"
#include "QuickGUIManager.h"
#include "QuickGUIMouseCursor.h"

namespace QuickGUI
{
	VerticalTrackBar::VerticalTrackBar(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mNumRegions(1),
		mCurrentValue(0),
		mLargeChange(3)
	{
		mWidgetType = TYPE_TRACKBAR_VERTICAL;
		mSkinComponent = ".trackbar.vertical";
		mSize = Size(20,80);

		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalTrackBar::onMouseButtonDown,this);

		mOnValueChangedHandlers.clear();

		// Create slider button at the beginning of the VerticalTrackBar.
		mSliderButton = dynamic_cast<Button*>(_createComponent(mInstanceName+".Slider",TYPE_BUTTON));
		mSliderButton->setSkinComponent(".trackbar.vertical.slider");
		mSliderButton->setSize(getWidth(),13);
		mSliderButton->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mSliderButton->enableDragging(true);
		mSliderButton->constrainDragging(false,true);
		mSliderButton->setAutoSize(false);
		mSliderButton->addEventHandler(EVENT_DRAGGED,&VerticalTrackBar::onSliderDragged,this);
		mSliderButton->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalTrackBar::onMouseDownOnSlider,this);
		mSliderButton->addEventHandler(EVENT_MOUSE_BUTTON_UP,&VerticalTrackBar::onMouseUpOnSlider,this);
		mSliderButton->addEventHandler(EVENT_LOSE_FOCUS,&VerticalTrackBar::onMouseUpOnSlider,this);
		
		_getSliderPositions();

		setValue(0);
	}

	VerticalTrackBar::~VerticalTrackBar()
	{
		Widget::removeAndDestroyAllChildWidgets();
		mSliderButton = NULL;

		EventHandlerArray::iterator it;
		for( it = mOnValueChangedHandlers.begin(); it != mOnValueChangedHandlers.end(); ++it )
			delete (*it);
		mOnValueChangedHandlers.clear();
	}

	void VerticalTrackBar::_getSliderPositions()
	{
		mSliderPositions.clear();

		Ogre::Real sliderPixelHeight = mSliderButton->getHeight();
		Ogre::Real trackLength = mSize.height - sliderPixelHeight;
		mRegionLength = trackLength / mNumRegions;

		Ogre::Real yStart = mSize.height - (sliderPixelHeight/2.0);
		mSliderPositions.push_back(yStart);
		for( int i = 0; i < mNumRegions; ++i )
		{
			mSliderPositions.push_back(yStart - ((i + 1) * mRegionLength));
		}
	}

	void VerticalTrackBar::addOnValueChangedEventHandler(MemberFunctionSlot* function)
	{
		mOnValueChangedHandlers.push_back(function);
	}

	int VerticalTrackBar::getClosestSliderPosition(const Point& pixelPoint)
	{
		Point p = pixelPoint - getScreenPosition();

		if( p.y >= mSliderPositions[0] )
			return 0;
		if( p.y <= mSliderPositions[mSliderPositions.size() - 1] )
			return (static_cast<int>(mSliderPositions.size()) - 1);

		int i;
		for( i = 1; i < static_cast<int>(mSliderPositions.size()); ++i )
		{
			if( p.y > (mSliderPositions[i] + (mRegionLength/2.0)) )
				return (i - 1);
		}

		return i;
	}

	int VerticalTrackBar::getLargeChange()
	{
		return mLargeChange;
	}

	int VerticalTrackBar::getNumRegions()
	{
		return mNumRegions;
	}
	
	int VerticalTrackBar::getNumTicks()
	{
		return (mNumRegions + 1);
	}

	int VerticalTrackBar::getValue()
	{
		return mCurrentValue;
	}

	Ogre::Real VerticalTrackBar::getPercentage()
	{
		return static_cast<Ogre::Real>(mCurrentValue) / static_cast<Ogre::Real>(mNumRegions);
	}

	void VerticalTrackBar::onMouseDownOnSlider(const EventArgs& args)
	{
		mSliderButton->lockTexture();
	}

	void VerticalTrackBar::onMouseUpOnSlider(const EventArgs& args)
	{
		mSliderButton->unlockTexture();

		if(mGUIManager->getMouseOverWidget()->getInstanceName() != mSliderButton->getInstanceName())
			mSliderButton->applyDefaultTexture();
		else
			mSliderButton->applyButtonOverTexture();
	}

	void VerticalTrackBar::onMouseButtonDown(const EventArgs& args)
	{
		Point mousePos = dynamic_cast<const MouseEventArgs&>(args).position;
		mousePos = mousePos - getScreenPosition();

		if( (mousePos.y < (mSliderPositions[mCurrentValue] - ((mRegionLength - mSliderButton->getHeight())/2.0))) && (mCurrentValue < mNumRegions) )
			setValue(mCurrentValue + mLargeChange);
		else if( (mousePos.y > (mSliderPositions[mCurrentValue] + ((mRegionLength + mSliderButton->getHeight())/2.0))) && (mCurrentValue > 0) )
			setValue(mCurrentValue - mLargeChange);
	}

	void VerticalTrackBar::onSliderDragged(const EventArgs& args)
	{
		int indexFromCursor = getClosestSliderPosition(mGUIManager->getMouseCursor()->getPosition());
		setValue(indexFromCursor);
	}

	void VerticalTrackBar::onValueChanged(const WidgetEventArgs& args)
	{
		if(!mEnabled)
			return;

		EventHandlerArray::iterator it;
		for( it = mOnValueChangedHandlers.begin(); it != mOnValueChangedHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void VerticalTrackBar::setLargeChange(unsigned int jumpValue)
	{
		mLargeChange = jumpValue;
	}

	void VerticalTrackBar::setNumRegions(unsigned int NumRegions)
	{
		mNumRegions = NumRegions;
		_getSliderPositions();
	}

	void VerticalTrackBar::setSliderSize(Size pixelSize)
	{
		mSliderButton->setSize(pixelSize);
	}

	void VerticalTrackBar::setNumTicks(unsigned int NumTicks)
	{
		setNumRegions(NumTicks - 1);
	}

	void VerticalTrackBar::setValue(int Value)
	{
		if( Value < 0 )
			Value = 0;
		else if( Value > mNumRegions )
			Value = mNumRegions;

		mCurrentValue = Value;

		mSliderButton->setYPosition(mSliderPositions[mCurrentValue] - (mSliderButton->getHeight()/2.0));

		WidgetEventArgs e(this);
		e.handled = false;
		onValueChanged(e);
	}
}
