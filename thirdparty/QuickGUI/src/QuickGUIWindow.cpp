#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIWindow.h"
// included to get access to default font/fontsize
#include "QuickGUIManager.h"

namespace QuickGUI
{
	Window::Window(const Ogre::String& name, GUIManager* gm) :
		Panel(name,gm),
		mTitleBar(0),
		mBringToFrontOnFocus(true)
	{
		mSkinComponent = ".window";
		mWidgetType = TYPE_WINDOW;
		mShowWithParent = false;
		mCanResize = true;
		addEventHandler(EVENT_GAIN_FOCUS,&Window::onGainFocus,this);

		// Create TitleBar - tradition titlebar dimensions: across the top of the window
		mTitleBar = dynamic_cast<TitleBar*>(_createComponent(mInstanceName+".TitleBar",TYPE_TITLEBAR));
		mTitleBar->setSize(mSize.width,25);
		mTitleBar->enableDragging(true);
		mTitleBar->setDraggingWidget(this);

		mTitleBar->getCloseButton()->addEventHandler(Widget::EVENT_MOUSE_CLICK,&Window::onMouseUpOverCloseButton,this);
		mTitleBar->getCloseButton()->addEventHandler(Widget::EVENT_MOUSE_BUTTON_UP,&Window::onMouseUpOverCloseButton,this);

		// Create borders.
		setUseBorders(true);
	}

	Window::~Window()
	{
		setQuadContainer(NULL);
	}

	void Window::allowScrolling(bool allow)
	{
		mScrollingAllowed = allow;

		if(mScrollingAllowed)
		{
			if(mScrollPane == NULL)
			{
				mScrollPane = dynamic_cast<ScrollPane*>(_createChild(mInstanceName+".ScrollPane",TYPE_SCROLL_PANE));
				mScrollPane->setSize(mSize);

				mScrollPane->removeChild(mScrollPane->mRightBar);
				// store reference to the scroll bar
				mRightScrollBar = mScrollPane->mRightBar;
				addChild(mRightScrollBar);
				mRightScrollBar->setPosition(mSize.width - 20,0);

				mScrollPane->removeChild(mScrollPane->mBottomBar);
				// store reference to the scroll bar
				mBottomScrollBar = mScrollPane->mBottomBar;
				addChild(mBottomScrollBar);
				mBottomScrollBar->setPosition(0,mSize.height - 20);

				if(mTitleBar->isVisible())
				{
					mRightScrollBar->setYPosition(mTitleBar->getHeight());
					mRightScrollBar->setHeight(mRightScrollBar->getHeight() - mTitleBar->getHeight());
				}

				mScrollPane->manageWidgets();
			}
		}
		else
		{
			if(mScrollPane != NULL)
			{
				delete mScrollPane;
				mScrollPane = NULL;

				mGUIManager->destroyWidget(mRightScrollBar);
				mRightScrollBar = NULL;
				mGUIManager->destroyWidget(mBottomScrollBar);
				mBottomScrollBar = NULL;
			}
		}
	}

	void Window::onGainFocus(const EventArgs& args)
	{
		if(mBringToFrontOnFocus)
			mQuadContainer->moveWindowGroupToEnd(this);
	}

	void Window::bringToFront()
	{
		mQuadContainer->moveWindowGroupToEnd(this);
	}

	bool Window::getBringToFrontOnFocus()
	{
		return mBringToFrontOnFocus;
	}

	TitleBar* Window::getTitleBar()
	{
		return mTitleBar;
	}

	void Window::hideCloseButton()
	{
		mTitleBar->hideCloseButton();
	}

	void Window::hideTitlebar()
	{
		mTitleBar->hide();
		mTitleBar->setShowWithParent(false);

		if(mRightScrollBar)
		{
			mRightScrollBar->setYPosition(0);
			mRightScrollBar->setHeight(mRightScrollBar->getHeight() + mTitleBar->getHeight());
		}
	}

	void Window::onMouseUpOverCloseButton(const EventArgs& args)
	{
		Panel::hide();
	}

	void Window::setBringToFrontOnFocus(bool BringToFront)
	{
		mBringToFrontOnFocus = BringToFront;
	}

	void Window::setQuadContainer(QuadContainer* c)
	{
		if((mQuadContainer != NULL) && (c != mQuadContainer))
			mQuadContainer->removeChildWindowContainer(this);

		mQuadContainer = c;

		if(mQuadContainer != NULL)
			mQuadContainer->addChildWindowContainer(this);
	}

	void Window::showCloseButton()
	{
		mTitleBar->showCloseButton();
	}

	void Window::showTitlebar()
	{
		mTitleBar->show();
		mTitleBar->setShowWithParent(true);

		mRightScrollBar->setYPosition(mTitleBar->getHeight());
		mRightScrollBar->setHeight(mRightScrollBar->getHeight() - mTitleBar->getHeight());

		Ogre::Real titlebarHeight = mTitleBar->getHeight();
		setYPosition(mPosition.y + titlebarHeight);
		setHeight(mSize.height - titlebarHeight);
	}
}
