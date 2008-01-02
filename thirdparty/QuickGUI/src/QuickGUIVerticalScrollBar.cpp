#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIVerticalScrollBar.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	VerticalScrollBar::VerticalScrollBar(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mMinSliderPosition(0),
		mMaxSliderPosition(mSize.height),
		mMouseDownOnTrack(false),
		mSmallChange(0.1),
		mLargeChange(0.4),
		mRepeatTimer(0),
		mScrollRepeatTime(0.5),
		mButtonLayout(BUTTON_LAYOUT_OPPOSITE)
	{
		mWidgetType = TYPE_SCROLLBAR_VERTICAL;
		mSkinComponent = ".scrollbar.vertical";
		mSize = Size(20,100);

		mGUIManager->registerTimeListener(this);
		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalScrollBar::onMouseDownOnTrack,this);
		addEventHandler(EVENT_MOUSE_BUTTON_UP,&VerticalScrollBar::onMouseUpOnTrack,this);
		addEventHandler(EVENT_MOUSE_LEAVE,&VerticalScrollBar::onMouseLeaveTrack,this);

		mSlider = dynamic_cast<Button*>(_createComponent(mInstanceName+".Slider",TYPE_BUTTON));
		mSlider->setSkinComponent(".scrollbar.vertical.slider");
		mSlider->setSize(mSize.width,mSize.width);
		mSlider->setPosition(0,mSize.width);
		mSlider->setAutoSize(false);
		mSlider->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mSlider->enableDragging(true);
		mSlider->constrainDragging(false,true);
		mSlider->getQuad()->setLayer(mQuad->getLayer());
		mSlider->addEventHandler(EVENT_DRAGGED,&VerticalScrollBar::onSliderDragged,this);
		
		mScrollUp1 = dynamic_cast<Button*>(_createComponent(mInstanceName+".Up1",TYPE_BUTTON));
		mScrollUp1->setSkinComponent(".scrollbar.vertical.up");
		mScrollUp1->setSize(mSize.width,mSize.width);
		mScrollUp1->setAutoSize(false);
		mScrollUp1->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mScrollUp1->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalScrollBar::onScrollUpDown,this);

		mScrollDown1 = dynamic_cast<Button*>(_createComponent(mInstanceName+".Down1",TYPE_BUTTON));
		mScrollDown1->setSkinComponent(".scrollbar.vertical.down");
		mScrollDown1->setSize(mSize.width,mSize.width);
		mScrollDown1->setPosition(0,mSize.width);
		mScrollDown1->setAutoSize(false);
		mScrollDown1->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mScrollDown1->setShowWithParent(false);
		mScrollDown1->hide();
		mScrollDown1->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalScrollBar::onScrollDownDown,this);

		mScrollUp2 = dynamic_cast<Button*>(_createComponent(mInstanceName+".Up2",TYPE_BUTTON));
		mScrollUp2->setSkinComponent(".scrollbar.vertical.up");
		mScrollUp2->setSize(mSize.width,mSize.width);
		mScrollUp2->setPosition(0,mSize.height - (mSize.width*2.0));
		mScrollUp2->setAutoSize(false);
		mScrollUp2->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mScrollUp2->setVerticalAnchor(ANCHOR_VERTICAL_BOTTOM);
		mScrollUp2->setShowWithParent(false);
		mScrollUp2->hide();
		mScrollUp2->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalScrollBar::onScrollUpDown,this);

		mScrollDown2 = dynamic_cast<Button*>(_createComponent(mInstanceName+".Down2",TYPE_BUTTON));
		mScrollDown2->setSkinComponent(".scrollbar.vertical.down");
		mScrollDown2->setSize(mSize.width,mSize.width);
		mScrollDown2->setPosition(0,mSize.height - mSize.width);
		mScrollDown2->setAutoSize(false);
		mScrollDown2->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mScrollDown2->setVerticalAnchor(ANCHOR_VERTICAL_BOTTOM);
		mScrollDown2->addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&VerticalScrollBar::onScrollDownDown,this);

		mMinSliderPosition = mScrollDown1->getYPosition();
		mMaxSliderPosition = mScrollDown2->getYPosition();
	}

	VerticalScrollBar::~VerticalScrollBar()
	{
		mGUIManager->unregisterTimeListener(this);
	}

	void VerticalScrollBar::_constrainSlider()
	{
		Rect sliderDimensions = mSlider->getDimensions();
		Ogre::Real sliderStart = sliderDimensions.y;
		Ogre::Real sliderEnd = sliderDimensions.y + sliderDimensions.height;

		if(sliderStart < mMinSliderPosition)
			mSlider->setYPosition(mMinSliderPosition);
		else if(sliderEnd > mMaxSliderPosition)
			mSlider->setYPosition(mMaxSliderPosition - mSlider->getHeight());
	}

	void VerticalScrollBar::_determineMinMax()
	{
		switch(mButtonLayout)
		{
		case BUTTON_LAYOUT_ADJACENT_UP:
			mMinSliderPosition = mScrollDown1->getYPosition() + mScrollDown1->getHeight();
			mMaxSliderPosition = mSize.height;
			break;
		case BUTTON_LAYOUT_ADJACENT_DOWN:
			mMinSliderPosition = 0;
			mMaxSliderPosition = mScrollUp2->getYPosition();
			break;
		case BUTTON_LAYOUT_MULTIPLE_BOTH:
			mMinSliderPosition = mScrollDown1->getYPosition() + mScrollDown1->getWidth();
			mMaxSliderPosition = mScrollUp2->getYPosition();
			break;
		case BUTTON_LAYOUT_MULTIPLE_UP:
			mMinSliderPosition = mScrollDown1->getYPosition() + mScrollDown1->getWidth();
			mMaxSliderPosition = mScrollUp2->getYPosition();
			break;
		case BUTTON_LAYOUT_MULTIPLE_DOWN:
			mMinSliderPosition = mScrollDown1->getYPosition() + mScrollDown1->getWidth();
			mMaxSliderPosition = mScrollUp2->getYPosition();
			break;
		case BUTTON_LAYOUT_NONE:
			mMinSliderPosition = 0;
			mMaxSliderPosition = mSize.height;
			break;
		case BUTTON_LAYOUT_OPPOSITE:
			mMinSliderPosition = mScrollDown1->getYPosition();
			mMaxSliderPosition = mScrollDown2->getYPosition();
			break;
		}
	}

	void VerticalScrollBar::_scroll(Ogre::Real change, ScrollEventArgs args)
	{
		mSlider->moveY(change * (mMaxSliderPosition - mMinSliderPosition));

		_constrainSlider();

		EventHandlerArray::iterator it;
		for( it = mOnScrollHandlers.begin(); it != mOnScrollHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void VerticalScrollBar::_setValue(Ogre::Real value)
	{
		Ogre::Real pixelY = (value * (mMaxSliderPosition - mMinSliderPosition)) + mMinSliderPosition;

		mSlider->setYPosition(pixelY);
		_constrainSlider();
	}

	void VerticalScrollBar::_showButtons()
	{
		switch(mButtonLayout)
		{
		case BUTTON_LAYOUT_ADJACENT_UP:
			mScrollUp1->show();
			mScrollUp1->setShowWithParent(true);
			mScrollDown1->show();
			mScrollDown1->setShowWithParent(true);
			mScrollUp2->hide();
			mScrollUp2->setShowWithParent(false);
			mScrollDown2->hide();
			mScrollDown2->setShowWithParent(false);
			break;
		case BUTTON_LAYOUT_ADJACENT_DOWN:
			mScrollUp1->hide();
			mScrollUp1->setShowWithParent(false);
			mScrollDown1->hide();
			mScrollDown1->setShowWithParent(false);
			mScrollUp2->show();
			mScrollUp2->setShowWithParent(true);
			mScrollDown2->show();
			mScrollDown2->setShowWithParent(true);
			break;
		case BUTTON_LAYOUT_MULTIPLE_BOTH:
			mScrollUp1->show();
			mScrollUp1->setShowWithParent(true);
			mScrollDown1->show();
			mScrollDown1->setShowWithParent(true);
			mScrollUp2->show();
			mScrollUp2->setShowWithParent(true);
			mScrollDown2->show();
			mScrollDown2->setShowWithParent(true);
			break;
		case BUTTON_LAYOUT_MULTIPLE_UP:
			mScrollUp1->show();
			mScrollUp1->setShowWithParent(true);
			mScrollDown1->show();
			mScrollDown1->setShowWithParent(true);
			mScrollUp2->show();
			mScrollUp2->setShowWithParent(true);
			mScrollDown2->hide();
			mScrollDown2->setShowWithParent(false);
			break;
		case BUTTON_LAYOUT_MULTIPLE_DOWN:
			mScrollUp1->hide();
			mScrollUp1->setShowWithParent(false);
			mScrollDown1->show();
			mScrollDown1->setShowWithParent(true);
			mScrollUp2->show();
			mScrollUp2->setShowWithParent(true);
			mScrollDown2->show();
			mScrollDown2->setShowWithParent(true);
			break;
		case BUTTON_LAYOUT_NONE:
			mScrollUp1->hide();
			mScrollUp1->setShowWithParent(false);
			mScrollDown1->hide();
			mScrollDown1->setShowWithParent(false);
			mScrollUp2->hide();
			mScrollUp2->setShowWithParent(false);
			mScrollDown2->hide();
			mScrollDown2->setShowWithParent(false);
			break;
		case BUTTON_LAYOUT_OPPOSITE:
			mScrollUp1->show();
			mScrollUp1->setShowWithParent(true);
			mScrollDown1->hide();
			mScrollDown1->setShowWithParent(false);
			mScrollUp2->hide();
			mScrollUp2->setShowWithParent(false);
			mScrollDown2->show();
			mScrollDown2->setShowWithParent(true);
			break;
		}

		if(!mVisible)
			hide();
	}

	void VerticalScrollBar::addOnScrollEventHandler(MemberFunctionSlot* function)
	{
		mOnScrollHandlers.push_back(function);
	}

	VerticalScrollBar::ButtonLayout VerticalScrollBar::getButtonLayout()
	{
		return mButtonLayout;
	}

	Ogre::Real VerticalScrollBar::getLargeChange()
	{
		return mLargeChange;
	}

	Ogre::Real VerticalScrollBar::getRepeatTime()
	{
		return mScrollRepeatTime;
	}

	Ogre::Real VerticalScrollBar::getSliderHeight()
	{
		return mSlider->getHeight();
	}

	Ogre::Real VerticalScrollBar::getSmallChange()
	{
		return mSmallChange;
	}

	Ogre::Real VerticalScrollBar::getValue()
	{
		Ogre::Real retVal = ((mSlider->getYPosition() - mMinSliderPosition) / (mMaxSliderPosition - mMinSliderPosition));
		return retVal;
	}

	void VerticalScrollBar::onMouseDownOnTrack(const EventArgs& args)
	{
		QuickGUI::MouseButtonID id = dynamic_cast<const MouseEventArgs&>(args).button;
		if(id == MB_Left)
		{
			mMouseDownOnTrack = true;
			mButtonDown = MB_Left;

			Point mousePosition = dynamic_cast<const MouseEventArgs&>(args).position - getScreenPosition();

			ScrollEventArgs scrollArgs(this);
			// if mouse clicked above track, scroll up.
			if(mousePosition.y < mSlider->getYPosition())
			{
				scrollArgs.sliderIncreasedPosition = false;
				_scroll(-mLargeChange,scrollArgs);
			}
			else
			{
				scrollArgs.sliderIncreasedPosition = true;
				_scroll(mLargeChange,scrollArgs);
			}

			mRepeatTimer = 0;
		}
		else if(id == MB_Right)
		{
			mMouseDownOnTrack = true;
			mButtonDown = MB_Right;

			Point mousePosition = dynamic_cast<const MouseEventArgs&>(args).position - getScreenPosition();

			ScrollEventArgs scrollArgs(this);
			// if mouse clicked below track, scroll down.
			if(mousePosition.y < mSlider->getYPosition())
			{
				scrollArgs.sliderIncreasedPosition = false;
				_scroll(mLargeChange,scrollArgs);
			}
			else
			{
				scrollArgs.sliderIncreasedPosition = true;
				_scroll(-mLargeChange,scrollArgs);
			}

			mRepeatTimer = 0;
		}
	}

	void VerticalScrollBar::onMouseUpOnTrack(const EventArgs& args)
	{
		QuickGUI::MouseButtonID id = dynamic_cast<const MouseEventArgs&>(args).button;
		if((id == MB_Left) || (id == MB_Right))
		{
			mMouseDownOnTrack = false;
		}
	}

	void VerticalScrollBar::onMouseLeaveTrack(const EventArgs& args)
	{
		mMouseDownOnTrack = false;
	}

	void VerticalScrollBar::onSizeChanged(const EventArgs& args)
	{
		Widget::onSizeChanged(args);
		_determineMinMax();
		_constrainSlider();
	}

	void VerticalScrollBar::onScrollUpDown(const EventArgs& args)
	{
		QuickGUI::MouseButtonID id = dynamic_cast<const MouseEventArgs&>(args).button;
		if(id == MB_Left)
		{
			scrollUpSmall();

			mRepeatTimer = 0;
		}
		else if(id == MB_Right)
		{
			scrollDownSmall();
			
			mRepeatTimer = 0;
		}
	}

	void VerticalScrollBar::onScrollDownDown(const EventArgs& args)
	{
		QuickGUI::MouseButtonID id = dynamic_cast<const MouseEventArgs&>(args).button;
		if(id == MB_Left)
		{
			scrollDownSmall();

			mRepeatTimer = 0;
		}
		else if(id == MB_Right)
		{
			scrollUpSmall();
			
			mRepeatTimer = 0;
		}
	}

	void VerticalScrollBar::onSliderDragged(const EventArgs& args)
	{
		ScrollEventArgs scrollArgs(this);
		if(dynamic_cast<const MouseEventArgs&>(args).moveDelta.y < 0)
			scrollArgs.sliderIncreasedPosition = false;
		else
			scrollArgs.sliderIncreasedPosition = true;

		_scroll(0,scrollArgs);
	}

	void VerticalScrollBar::scrollUpLarge()
	{
		ScrollEventArgs scrollArgs(this);
		scrollArgs.sliderIncreasedPosition = false;
		_scroll(-mLargeChange,scrollArgs);
	}

	void VerticalScrollBar::scrollUpSmall()
	{
		ScrollEventArgs scrollArgs(this);
		scrollArgs.sliderIncreasedPosition = false;
		_scroll(-mSmallChange,scrollArgs);
	}

	void VerticalScrollBar::scrollDownLarge()
	{
		ScrollEventArgs scrollArgs(this);
		scrollArgs.sliderIncreasedPosition = true;
		_scroll(mLargeChange,scrollArgs);
	}

	void VerticalScrollBar::scrollDownSmall()
	{
		ScrollEventArgs scrollArgs(this);
		scrollArgs.sliderIncreasedPosition = true;
		_scroll(mSmallChange,scrollArgs);
	}

	void VerticalScrollBar::setButtonLayout(ButtonLayout layout)
	{
		mButtonLayout = layout;

		_showButtons();
		_determineMinMax();
	}

	void VerticalScrollBar::setLargeChange(Ogre::Real change)
	{
		mLargeChange = change;
	}

	void VerticalScrollBar::setScrollRepeatTime(Ogre::Real timeInSeconds)
	{
		mScrollRepeatTime = timeInSeconds;
	}

	void VerticalScrollBar::setSliderHeight(Ogre::Real relativeHeight)
	{
		mSlider->setHeight(relativeHeight * (mMaxSliderPosition - mMinSliderPosition));
		_constrainSlider();
	}

	void VerticalScrollBar::setSmallChange(Ogre::Real change)
	{
		mSmallChange = change;
	}

	void VerticalScrollBar::setValue(Ogre::Real value)
	{
		ScrollEventArgs scrollArgs(this);
		if(value < getValue())
			scrollArgs.sliderIncreasedPosition = false;
		else
			scrollArgs.sliderIncreasedPosition = true;

		Ogre::Real pixelY = (value * (mMaxSliderPosition - mMinSliderPosition)) + mMinSliderPosition;

		if( pixelY < 0.0 )
			pixelY = 0.0;
		if( pixelY > (mMaxSliderPosition - mSlider->getHeight()) )
			pixelY = (mMaxSliderPosition - mSlider->getHeight());

		mSlider->setYPosition(pixelY);			

		_scroll(0,scrollArgs);
	}

	void VerticalScrollBar::timeElapsed(const Ogre::Real time)
	{
		Widget::timeElapsed(time);
		mRepeatTimer += time;

		if(mRepeatTimer > mScrollRepeatTime)
		{
			if(mMouseDownOnTrack)
			{
				MouseEventArgs args(this);
				args.button = mButtonDown;
				args.position = mGUIManager->getMouseCursor()->getPosition();
				onMouseDownOnTrack(args);
				return;
			}

			if(mScrollUp1->isDown() || mScrollUp2->isDown())
			{
				scrollUpSmall();
				return;
			}
			
			if(mScrollDown1->isDown() || mScrollDown2->isDown())
			{
				scrollDownSmall();
				return;
			}

			mRepeatTimer = 0;
		}
	}
}
