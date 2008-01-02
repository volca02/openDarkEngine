#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIList.h"
#include "QuickGUIManager.h"
#include "QuickGUIMouseCursor.h"

namespace QuickGUI
{
	List::List(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mScrollPane(0),
		mAutoNameWidgetCounter(0),
		mScrollingAllowed(false),
		mItemHeight(20),
		mAutoSizeListItems(true),
		mVPixelPadHeight(6)
	{
		mWidgetType = TYPE_LIST;
		mSkinComponent = ".list";
		mSize = Size(100,100);

		mTextHelper = new TextHelper();

		addEventHandler(EVENT_CHILD_ADDED,&List::onChildAdded,this);
		addEventHandler(EVENT_CHILD_REMOVED,&List::onChildRemoved,this);

		mItems.clear();
	}

	List::~List()
	{
		mScrollPane = NULL;
		delete mTextHelper;

		mItems.clear();
	}

	MenuLabel* List::addMenuLabel()
	{
		Point p(0,(mAutoNameWidgetCounter * mItemHeight));
		Size s(mSize.width,mItemHeight);

		++mAutoNameWidgetCounter;

		Ogre::String name = mInstanceName+".Item"+Ogre::StringConverter::toString(mAutoNameWidgetCounter);
		mGUIManager->notifyNameUsed(name);

		MenuLabel* newMenuLabel = dynamic_cast<MenuLabel*>(_createChild(mInstanceName+".ChildMenuLabel",TYPE_MENULABEL));
		newMenuLabel->setSize(s);
		newMenuLabel->setPosition(p);
		newMenuLabel->setAutoSize(false);
		newMenuLabel->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);

		mItems.push_back(newMenuLabel);

		return newMenuLabel;
	}

	TextBox* List::addTextBox()
	{
		Point p(0,(mAutoNameWidgetCounter * mItemHeight));
		Size s(mSize.width,mItemHeight);

		++mAutoNameWidgetCounter;

		Ogre::String name = mInstanceName+".Item"+Ogre::StringConverter::toString(mAutoNameWidgetCounter);
		mGUIManager->notifyNameUsed(name);

		TextBox* newTextBox = dynamic_cast<TextBox*>(_createChild(mInstanceName+".ChildTextBox"+Ogre::StringConverter::toString(mItems.size()),TYPE_TEXTBOX));
		newTextBox->setSize(s);
		newTextBox->hideSkin();
		newTextBox->setPosition(p);
		newTextBox->setAutoSize(false);
		newTextBox->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);

		mItems.push_back(newTextBox);

		return newTextBox;
	}

	void List::allowScrolling(bool allow)
	{
		mScrollingAllowed = allow;

		if(mScrollingAllowed)
		{
			if(mScrollPane == NULL)
			{
				mScrollPane = dynamic_cast<ScrollPane*>(_createComponent(mInstanceName+".ScrollPane",TYPE_SCROLL_PANE));
				mScrollPane->setSize(mSize);
//				mScrollPane->setHorizontalSliderMinWidth(20);
//				mScrollPane->setVerticalSliderMinHeight(20);

				mRightScrollBar = mScrollPane->mRightBar;
				if(mSkinName != "")
					mRightScrollBar->setSkin(mSkinName,true);
				addChild(mRightScrollBar);
				mRightScrollBar->setPosition(mSize.width - 20,0);
				
				mBottomScrollBar = mScrollPane->mBottomBar;
				if(mSkinName != "")
					mBottomScrollBar->setSkin(mSkinName,true);
				addChild(mBottomScrollBar);
				mBottomScrollBar->setPosition(0,mSize.height - 20);

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

	void List::clear()
	{
		WidgetArray::iterator it;
		for( it = mItems.begin(); it != mItems.end(); ++it )
			mGUIManager->destroyWidget((*it));
		mItems.clear();
	}

	bool List::getAutoSizeListItems()
	{
		return mAutoSizeListItems;
	}

	Widget* List::getItem(unsigned int index)
	{
		if( index >= mItems.size() )
			return NULL;

		return mItems[index];
	}

	int List::getItemIndex(Widget* w)
	{
		Ogre::String name = w->getInstanceName();

		int counter = 0;
		WidgetArray::iterator it;
		for( it = mItems.begin(); it != mItems.end(); ++it )
		{
			if( name == (*it)->getInstanceName() )
				return counter;

			++counter;
		}

		return -1;
	}

	int List::getNumberOfItems()
	{
		return static_cast<int>(mItems.size());
	}

	ScrollPane* List::getScrollPane()
	{
		return mScrollPane;
	}

	int List::getVerticalPixelPadHeight()
	{
		return mVPixelPadHeight;
	}

	void List::onChildAdded(const EventArgs& args)
	{
		if(mScrollPane != NULL)
			mScrollPane->onChildAddedToParent(args);
	}

	void List::onChildRemoved(const EventArgs& args)
	{
		if(mScrollPane != NULL)
			mScrollPane->onChildRemovedFromParent(args);
	}

	void List::onSizeChanged(const EventArgs& args)
	{
		Widget::onSizeChanged(args);

		if(mScrollPane != NULL)
			mScrollPane->onParentSizeChanged(args);
	}

	void List::removeItem(unsigned int index)
	{
		if( index >= mItems.size() )
			return;

		Widget* w = NULL;

		unsigned int counter = 0;
		WidgetArray::iterator it;
		for( it = mItems.begin(); it != mItems.end(); ++it )
		{
			if( counter == index )
			{
				w = (*it);
				mItems.erase(it);
				break;
			}

			++counter;
		}

		if(w != NULL)
		{
			for( ; counter < mItems.size(); ++counter )
			{
				mItems[counter]->setYPosition(counter * mItemHeight);
			}

			// Remove child so that ScrollPane, if exists, will be notified.
			// Make sure this is done after all remaining items are correctly positioned,
			// since ScrollPane will adjust dimensions and scrollbars according to managed widgets.
			removeChild(w);
			mGUIManager->destroyWidget(w);
		}
	}

	bool List::scrollingAllowed()
	{
		return mScrollingAllowed;
	}

	void List::setAutoSizeListItems(bool autoSize)
	{
		mAutoSizeListItems = autoSize;

		if(mAutoSizeListItems)
		{
			setItemPixelHeight(mTextHelper->getGlyphHeight());
			mAutoSizeListItems = true;

			if(mScrollPane != NULL)
				mScrollPane->_determinePaneBounds();
		}
	}

	void List::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		Widget::setFont(fontScriptName,recursive);
		mTextHelper->setFont(fontScriptName);

		if(mAutoSizeListItems)
		{
			setItemPixelHeight(mTextHelper->getGlyphHeight() + mVPixelPadHeight);
			mAutoSizeListItems = true;

			if(mScrollPane != NULL)
				mScrollPane->_determinePaneBounds();
		}
	}

	void List::setItemPixelHeight(const Ogre::Real& heightInPixels)
	{
		mItemHeight = heightInPixels;
		mAutoSizeListItems = false;

		Ogre::Real counter = 0;
		WidgetArray::iterator it;
		for( it = mItems.begin(); it != mItems.end(); ++it )
		{
			(*it)->setYPosition(mItemHeight * counter);
			(*it)->setHeight(mItemHeight);
			++counter;
		}
	}

	void List::setVerticalPixelPadHeight(unsigned int height)
	{
		mVPixelPadHeight = height;

		if(mAutoSizeListItems)
		{
			setItemPixelHeight(mItemHeight + mVPixelPadHeight);
			mAutoSizeListItems = true;

			if(mScrollPane != NULL)
				mScrollPane->_determinePaneBounds();
		}
	}

	void List::show()
	{
		Widget::show();

		if(mScrollPane != NULL)
			mScrollPane->_syncBarWithParentDimensions();
	}
}
