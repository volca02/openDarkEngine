#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIComboBox.h"
#include "QuickGUIManager.h"
#include "QuickGUIMouseCursor.h"
#include "QuickGUISheet.h"

namespace QuickGUI
{
	ComboBox::ComboBox(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mRightToLeft(false),
		mSelectedItem(0),
		mAutoSize(true),
		mVPixelPadHeight(10)
	{
		mWidgetType = TYPE_COMBOBOX;
		mSkinComponent = ".combobox";
		mSize = Size(125,25);

		addEventHandler(EVENT_LOSE_FOCUS,&ComboBox::onLoseFocus,this);
		addEventHandler(EVENT_MOUSE_ENTER,&ComboBox::onMouseEnters,this);
		addEventHandler(EVENT_MOUSE_LEAVE,&ComboBox::onMouseLeaves,this);
		addEventHandler(EVENT_MOUSE_BUTTON_UP,&ComboBox::onMouseButtonUp,this);
		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&ComboBox::onMouseButtonDown,this);

		mTextHelper = new TextHelper();

		mList = dynamic_cast<List*>(_createComponent(mInstanceName+".DropDownList",TYPE_LIST));
		mList->setSize(mSize.width,100);
		mList->setPosition(0,mSize.height);
		mList->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mList->setVerticalAnchor(ANCHOR_VERTICAL_BOTTOM);
		mList->setQuadLayer(Quad::LAYER_MENU);
		mList->setInheritQuadLayer(false);
		mList->setClippingWidget(NULL);
		mList->setInheritClippingWidget(false);
		mList->setShowWithParent(false);
		mList->setOffset(mOffset + 2);
		mList->allowScrolling(true);
		mList->hide();
		mList->setPropagateEventFiring(EVENT_MOUSE_BUTTON_UP,true);
		mList->setPropagateEventFiring(EVENT_MOUSE_BUTTON_DOWN,true);
		mList->setPropagateEventFiring(EVENT_MOUSE_ENTER,true);
		mList->setPropagateEventFiring(EVENT_MOUSE_LEAVE,true);

		mMenuLabel = dynamic_cast<MenuLabel*>(_createComponent(mInstanceName+".SelectedItem",TYPE_MENULABEL));
		mMenuLabel->setSize(mSize.width - mSize.height,mSize.height);
		mMenuLabel->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
		mMenuLabel->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mMenuLabel->setPosition(0,0);
		mMenuLabel->setPropagateEventFiring(EVENT_LOSE_FOCUS,true);
		mMenuLabel->setPropagateEventFiring(EVENT_MOUSE_ENTER,true);
		mMenuLabel->setPropagateEventFiring(EVENT_MOUSE_LEAVE,true);

		mButton = dynamic_cast<Button*>(_createComponent(mInstanceName+".DropDownButton",TYPE_BUTTON));
		mButton->setSkinComponent(".combobox.button");
		mButton->setSize(mSize.height,mSize.height);
		mButton->setPosition(mSize.width - mSize.height,0);
		mButton->setAutoSize(false);
		mButton->setHorizontalAnchor(ANCHOR_HORIZONTAL_RIGHT);
		mButton->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
		mButton->setPropagateEventFiring(EVENT_MOUSE_BUTTON_UP,true);
		mButton->setPropagateEventFiring(EVENT_MOUSE_ENTER,true);
		mButton->setPropagateEventFiring(EVENT_MOUSE_LEAVE,true);

		mHighlightSkinComponent = ".combobox.highlight";

		// create highlight container for the list
		mHighlightPanel = _createQuad();
		mHighlightPanel->setClippingWidget(mList);
		mHighlightPanel->setInheritClippingWidget(false);
		mHighlightPanel->setLayer(Quad::LAYER_MENU);
		mHighlightPanel->setInheritLayer(false);
		mHighlightPanel->setShowWithOwner(false);
		// offset + 3, to be able to show over ListItems with Images and Buttons and Text
		mHighlightPanel->setOffset(mOffset+3);
		mHighlightPanel->_notifyQuadContainer(mQuadContainer);
	}

	ComboBox::~ComboBox()
	{
		delete mTextHelper;

		Widget::removeAndDestroyAllChildWidgets();

		EventHandlerArray::iterator it;
		for( it = mOnSelectUserEventHandlers.begin(); it != mOnSelectUserEventHandlers.end(); ++it )
			delete (*it);
		mOnSelectUserEventHandlers.clear();
	}

	MenuLabel* ComboBox::addItem()
	{
		MenuLabel* newMenuLabel = mList->addMenuLabel();
		newMenuLabel->setPropagateEventFiring(EVENT_MOUSE_ENTER,true);
		newMenuLabel->setPropagateEventFiring(EVENT_MOUSE_LEAVE,true);
		return newMenuLabel;
	}

	void ComboBox::addOnSelectionEventHandler(MemberFunctionSlot* function)
	{
		mOnSelectUserEventHandlers.push_back(function);
	}

	void ComboBox::clearDropDownList()
	{
		mList->clear();
	}

	void ComboBox::clearSelection()
	{
		mHighlightPanel->setVisible(false);
		mSelectedItem = NULL;
		mMenuLabel->hide();
	}

	int ComboBox::getItemIndex(MenuLabel* l)
	{
		return mList->getItemIndex(l);
	}

	int ComboBox::getNumberOfItems()
	{
		return mList->getNumberOfItems();
	}

	MenuLabel* ComboBox::getSelectedItem()
	{
		return mSelectedItem;
	}

	int ComboBox::getSelectedItemIndex()
	{
		if(mSelectedItem == NULL)
			return -1;

		return mList->getItemIndex(mSelectedItem);
	}

	int ComboBox::getVerticalPixelPadHeight()
	{
		return mVPixelPadHeight;
	}

	void ComboBox::hideDropDownList(const EventArgs& args)
	{
		mList->hide();
	}

	void ComboBox::highlightListItem(MenuLabel* l)
	{
		if(l == NULL)
			return;

		mHighlightPanel->setPosition(l->getScreenPosition() + l->getScrollOffset());
		mHighlightPanel->setSize(l->getSize());
		mHighlightPanel->setVisible(true);
	}

	// Called when the user clicks outside the widget
	void ComboBox::onLoseFocus(const EventArgs& args)
	{
		const MouseEventArgs mea = dynamic_cast<const MouseEventArgs&>(args);

		// Selected Item (MenuLabel Widget) is set to call ComboBox::onLoseFocus.  Add this in to
		// handle situation where user clicks on Selected Item, and then ComboBox.
		if(isPointWithinBounds(mea.position))
			return;

		// Drop Down List children call this function when they lose focus, so this check is needed because you can enter this
		// function when you haven't actually clicked outside the combobox.
		if(mList->isPointWithinBounds(mea.position))
			return;

		mList->hide();
	}

	void ComboBox::onMouseEnters(const EventArgs& args)
	{
		const MouseEventArgs& mea = dynamic_cast<const MouseEventArgs&>(args);
		if(mList->isPointWithinBounds(mea.position))
		{
			highlightListItem(dynamic_cast<MenuLabel*>(mea.widget));
		}
		
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ".over" + ss->getImageExtension()));
		mButton->applyButtonOverTexture();
	}

	void ComboBox::onMouseLeaves(const EventArgs& args)
	{
		mHighlightPanel->setVisible(false);
		
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ss->getImageExtension()));
		mButton->applyDefaultTexture();
	}

	void ComboBox::onMouseButtonDown(const EventArgs& args)
	{
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ".down" + ss->getImageExtension()));
		mButton->applyButtonDownTexture();
	}

	void ComboBox::onMouseButtonUp(const EventArgs& args)
	{
		const MouseEventArgs& mea = dynamic_cast<const MouseEventArgs&>(args);
		if(mList->isPointWithinBounds(mea.position))
			onSelection(args);
		else
		{
			if(mList->isVisible())
			{
				mList->hide();
				mGUIManager->_menuClosed(mList);
			}
			else
			{
				mList->show();
				mGUIManager->_menuOpened(mList);
			}
		}
	}

	void ComboBox::onSelection(const EventArgs& args)
	{
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ss->getImageExtension()));
		mButton->applyDefaultTexture();
		mList->hide();

		Widget* targetWidget = dynamic_cast<const WidgetEventArgs&>(args).widget;
		// If the combobox drop list is bigger than the number of items, the user can click an empty space.
		if(targetWidget->getWidgetType() == TYPE_LIST)
			return;

		selectItem(dynamic_cast<MenuLabel*>(targetWidget));

		EventHandlerArray::iterator it;
		for( it = mOnSelectUserEventHandlers.begin(); it != mOnSelectUserEventHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void ComboBox::selectItem(unsigned int index)
	{
		Widget* w = mList->getItem(index);
		if( w == NULL )
			return;

		selectItem(dynamic_cast<MenuLabel*>(w));
	}

	void ComboBox::selectItem(MenuLabel* l)
	{
		if(l == NULL)
			return;

		mSelectedItem = l;
		mMenuLabel->setText(l->getText());
		mMenuLabel->setSkin(l->getSkin());
		mMenuLabel->setIconMaterial(l->getIconMaterial());
		if(mVisible)
			mMenuLabel->show();
		mHighlightPanel->setVisible(false);
	}

	void ComboBox::setDropDownHeight(Ogre::Real pixelHeight)
	{
		mDropDownHeight = pixelHeight;
		mList->setHeight(mDropDownHeight);
	}

	void ComboBox::setDropDownWidth(Ogre::Real pixelWidth)
	{
		mDropDownWidth = pixelWidth;
		mList->setWidth(mDropDownWidth);
		mList->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT);
	}

	void ComboBox::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		Widget::setFont(fontScriptName,recursive);
		mTextHelper->setFont(fontScriptName);

		if(mAutoSize)
		{
			setHeight(mTextHelper->getGlyphHeight() + mVPixelPadHeight);
			mAutoSize = true;
		}
	}

	void ComboBox::setHeight(Ogre::Real pixelHeight)
	{
		Widget::setHeight(pixelHeight);
		mAutoSize = false;
	}

	void ComboBox::setRightToLeft(bool rightToLeft)
	{
		if(mRightToLeft == rightToLeft)
			return;

		mRightToLeft = rightToLeft;

		if(mRightToLeft)
		{
			mButton->setXPosition(0);
			mButton->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT);
			mMenuLabel->setPosition(mSize.height,0);
		}
		else
		{
			mButton->setXPosition(mSize.width - mButton->getWidth());
			mButton->setHorizontalAnchor(ANCHOR_HORIZONTAL_RIGHT);
			mMenuLabel->setPosition(0,0);
		}

		//mMenuLabel->setRightToLeft(mRightToLeft);
	}

	void ComboBox::setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight)
	{
		Widget::setSize(pixelWidth,pixelHeight);
		mAutoSize = false;
	}

	void ComboBox::setSize(const Size& pixelSize)
	{
		ComboBox::setSize(pixelSize.width,pixelSize.height);
	}

	void ComboBox::setSkin(const Ogre::String& skinName, bool recursive)
	{
		Widget::setSkin(skinName,recursive);

		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mHighlightPanel->setMaterial(ss->getMaterialName());
		mHighlightPanel->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mHighlightSkinComponent+ ss->getImageExtension()));
	}

	void ComboBox::setVerticalPixelPadHeight(unsigned int height)
	{
		mVPixelPadHeight = height;

		if(mAutoSize)
		{
			setHeight(mTextHelper->getGlyphHeight() + mVPixelPadHeight);
			mAutoSize = true;
		}
	}
}
