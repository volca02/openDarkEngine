#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIHorizontalTrackBar.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	HorizontalTrackBar::HorizontalTrackBar(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mNumRegions(1),
		mCurrentValue(0),
		mLargeChange(3)
	{
		mWidgetType = TYPE_TRACKBAR_HORIZONTAL;
		mSkinComponent = ".trackbar.horizontal";
		mSize = Size(80,20);

		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&HorizontalTrackBar::onMouseButtonDown,this);

		mOnValueChangedHandlers.clear();

		// Create slider button at the beginning of the HorizontalTrackBar.
		mSliderButton = dynamic_cast<Button*>(_createComponent(mInstanceName+".Slider",TYPE_BUTTON));
		mSliderButton->setSkinComponent(".trackbar.horizontal.slider");
		mSliderButton->setSize(13,getHeight());
		mSliderButton->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
		mSliderButton->enableDragging(true);
		mSliderButton->constrainDragging(true,false);
		mSliderButton->setAutoSize(false);
		mSliderButton->addEventHandler(EVENT_DRAGGED,&HorizontalTrackBar::onSliderDragged,this);
		mSliderButton->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&HorizontalTrackBar::onMouseDownOnSlider,this);
		mSliderButton->addEventHandler(EVENT_MOUSE_BUTTON_UP,&HorizontalTrackBar::onMouseUpOnSlider,this);
		mSliderButton->addEventHandler(EVENT_LOSE_FOCUS,&HorizontalTrackBar::onMouseUpOnSlider,this);
		
		_getSliderPositions();

		setValue(0);
	}

	HorizontalTrackBar::~HorizontalTrackBar()
	{
		Widget::removeAndDestroyAllChildWidgets();
		mSliderButton = NULL;

		EventHandlerArray::iterator it;
		for( it = mOnValueChangedHandlers.begin(); it != mOnValueChangedHandlers.end(); ++it )
			delete (*it);
		mOnValueChangedHandlers.clear();
	}

	void HorizontalTrackBar::_getSliderPositions()
	{
		mSliderPositions.clear();
		
		Ogre::Real sliderPixelWidth = mSliderButton->getWidth();
		Ogre::Real trackLength = mSize.width - sliderPixelWidth;
		mRegionLength = trackLength / mNumRegions;

		Ogre::Real xStart = (sliderPixelWidth/2.0);
		mSliderPositions.push_back(xStart);
		for( int i = 0; i < mNumRegions; ++i )
		{
			mSliderPositions.push_back(xStart + ((i + 1) * mRegionLength));
		}
	}

	void HorizontalTrackBar::addOnValueChangedEventHandler(MemberFunctionSlot* function)
	{
		mOnValueChangedHandlers.push_back(function);
	}

	int HorizontalTrackBar::getClosestSliderPosition(const Point& pixelPoint)
	{
		Point p = pixelPoint - getScreenPosition();

		if( p.x <= mSliderPositions[0] )
			return 0;
		if( p.x >= mSliderPositions[mSliderPositions.size() - 1] )
			return (static_cast<int>(mSliderPositions.size()) - 1);

		int i;
		for(  i = 1; i < static_cast<int>(mSliderPositions.size()); ++i )
		{
			if( p.x < (mSliderPositions[i] - (mRegionLength/2.0)) )
				return (i - 1);
		}

		return i;
	}

	int HorizontalTrackBar::getLargeChange()
	{
		return mLargeChange;
	}

	int HorizontalTrackBar::getNumRegions()
	{
		return mNumRegions;
	}
	
	int HorizontalTrackBar::getNumTicks()
	{
		return (mNumRegions + 1);
	}

	int HorizontalTrackBar::getValue()
	{
		return mCurrentValue;
	}

	Ogre::Real HorizontalTrackBar::getPercentage()
	{
		return static_cast<Ogre::Real>(mCurrentValue) / static_cast<Ogre::Real>(mNumRegions);
	}

	void HorizontalTrackBar::onMouseDownOnSlider(const EventArgs& args)
	{	
		mSliderButton->lockTexture();
	}

	void HorizontalTrackBar::onMouseUpOnSlider(const EventArgs& args)
	{
		mSliderButton->unlockTexture();

		if(mGUIManager->getMouseOverWidget()->getInstanceName() != mSliderButton->getInstanceName())
			mSliderButton->applyDefaultTexture();
		else
			mSliderButton->applyButtonOverTexture();
	}

	void HorizontalTrackBar::onMouseButtonDown(const EventArgs& args)
	{
		Point mousePos = dynamic_cast<const MouseEventArgs&>(args).position;
		mousePos = mousePos - getScreenPosition();

		if( (mousePos.x > (mSliderPositions[mCurrentValue] + ((mRegionLength + mSliderButton->getWidth())/2.0))) && (mCurrentValue < mNumRegions) )
			setValue(mCurrentValue + mLargeChange);
		else if( (mousePos.x < (mSliderPositions[mCurrentValue] - ((mRegionLength - mSliderButton->getWidth())/2.0))) && (mCurrentValue > 0) )
			setValue(mCurrentValue - mLargeChange);
	}

	void HorizontalTrackBar::onSliderDragged(const EventArgs& args)
	{
		int indexFromCursor = getClosestSliderPosition(mGUIManager->getMouseCursor()->getPosition());
		setValue(indexFromCursor);
	}

	void HorizontalTrackBar::onValueChanged(const WidgetEventArgs& args)
	{
		if(!mEnabled)
			return;

		EventHandlerArray::iterator it;
		for( it = mOnValueChangedHandlers.begin(); it != mOnValueChangedHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void HorizontalTrackBar::setLargeChange(unsigned int LargeChange)
	{
		mLargeChange = LargeChange;
	}

	void HorizontalTrackBar::setNumRegions(unsigned int NumRegions)
	{
		mNumRegions = NumRegions;
		_getSliderPositions();
	}

	void HorizontalTrackBar::setNumTicks(unsigned int NumTicks)
	{
		setNumRegions(NumTicks - 1);
	}

	void HorizontalTrackBar::setSliderSize(Size pixelSize)
	{
		mSliderButton->setSize(pixelSize);
	}

	void HorizontalTrackBar::setValue(int Value)
	{
		if( Value < 0 )
			Value = 0;
		else if( Value > mNumRegions )
			Value = mNumRegions;

		mCurrentValue = Value;

		mSliderButton->setXPosition(mSliderPositions[mCurrentValue] - (mSliderButton->getWidth()/2.0));

		WidgetEventArgs e(this);
		e.handled = false;
		onValueChanged(e);
	}
}
