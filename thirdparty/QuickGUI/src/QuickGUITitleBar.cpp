#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUITitleBar.h"
#include "QuickGUIManager.h"

#include "QuickGUIWindow.h"
#include "QuickGUIMouseCursor.h"

namespace QuickGUI
{
	TitleBar::TitleBar(const Ogre::String& name, GUIManager* gm) :
		Label(name,gm)
	{	
		mWidgetType = TYPE_TITLEBAR;
		mSkinComponent = ".titlebar";
		mSize = Size(100,25);
		mScrollPaneAccessible = false;
		setQuadLayer(Quad::LAYER_MENU);
		mInheritQuadLayer = false;
		mHorizontalAnchor = ANCHOR_HORIZONTAL_LEFT_RIGHT;
		mHorizontalAlignment = HA_LEFT;

		mTextBoundsRelativeSize = Size(mSize.width - (mSize.height - 2),mSize.height) / mSize;

		// Create CloseButton
		Ogre::Real ButtonSize = mSize.height - 2;
		mCloseButton = dynamic_cast<Button*>(_createComponent(mInstanceName+".CloseButton",TYPE_BUTTON));
		mCloseButton->setSkinComponent(".titleBar.button");
		mCloseButton->setSize(ButtonSize,ButtonSize);
		mCloseButton->setPosition(mSize.width - ButtonSize - 1,1);
		mCloseButton->setAutoSize(false);
		mCloseButton->setHorizontalAnchor(ANCHOR_HORIZONTAL_RIGHT);
		mCloseButton->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
	}

	TitleBar::~TitleBar()
	{
		Widget::removeAndDestroyAllChildWidgets();
		mCloseButton = NULL;
	}

	Button* TitleBar::getCloseButton()
	{
		return mCloseButton;
	}

	void TitleBar::hideCloseButton()
	{
		mCloseButton->hide();
		mTextBoundsRelativeSize.width = 1;
		mText->redraw();
	}

	void TitleBar::setAutoSize(bool autoSize)
	{
		mAutoSize = autoSize;

		if(mAutoSize)
		{
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
			// setHeight sets mAutoSize to false..
			mAutoSize = true;
		}
	}

	void TitleBar::setCaption(const Ogre::UTFString& caption)
	{
		mText->setCaption(caption);
	}

	void TitleBar::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		Label::setFont(fontScriptName,recursive);
		mText->setFont(mFontName);

		if(mAutoSize)
		{
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
			// setHeight sets mAutoSize to false..
			mAutoSize = true;
			// The close button will have been resized vertically.  Need to match horizontally
			mCloseButton->setWidth(mCloseButton->getHeight());
			mCloseButton->setXPosition(mSize.width - mCloseButton->getWidth() - 1);
			mTextBoundsRelativeSize = Size(mSize.width - mCloseButton->getWidth(),mSize.height) / mSize;
		}
	}

	void TitleBar::setText(const Ogre::UTFString& text)
	{
		mText->setCaption(text);
	}

	void TitleBar::setWidth(Ogre::Real pixelWidth)
	{
		Label::setWidth(pixelWidth);

		mText->redraw();
	}

	void TitleBar::showCloseButton()
	{
		mCloseButton->show();
		mTextBoundsRelativeSize.width = (mSize.width - mCloseButton->getWidth()) / mSize.width;
	}
}
