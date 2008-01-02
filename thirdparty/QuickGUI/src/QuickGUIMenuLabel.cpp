#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIMenuLabel.h"

namespace QuickGUI
{
	MenuLabel::MenuLabel(const Ogre::String& name, GUIManager* gm) :
		Label(name,gm),
		mIcon(0),
		mIconMaterialName(""),
		mButton(0),
		mButtonTextureName("")
	{
		mWidgetType = TYPE_MENULABEL;
		mSkinComponent = ".menulabel";
		mSize = Size::ZERO;

		mTextBoundsPixelOffset.x = mSize.height;
		mTextBoundsRelativeSize.width = (mSize.width - (mSize.height * 2.0)) / mSize.width;

		setPropagateEventFiring(EVENT_MOUSE_BUTTON_UP,true);
	}

	MenuLabel::~MenuLabel()
	{
	}

	Ogre::String MenuLabel::getButtonSkin()
	{
		if(mButton == NULL)
			return "";

		return mButton->getSkin();
	}

	Ogre::String MenuLabel::getIconMaterial()
	{
		if(mIcon == NULL)
			return "";

		return mIcon->getMaterialName();
	}

	void MenuLabel::setMouseButtonUpOnButtonHandler(MemberFunctionSlot* function)
	{
		if(mButton == NULL)
			return;

		mButton->addEventHandler(EVENT_MOUSE_BUTTON_UP,function);
	}

	void MenuLabel::setButtonSkin(const Ogre::String& skinName)
	{
		if(mButton != NULL)
		{
			mButton->setSkin(skinName);
			return;
		}

		Point buttonPosition;
		HorizontalAnchor hAnchor;

		if(mLayout == LAYOUT_LEFT_TO_RIGHT)
		{
			buttonPosition = Point(mSize.width - mSize.height,0);
			hAnchor = ANCHOR_HORIZONTAL_LEFT;
		}
		else
		{
			buttonPosition = Point(0,0);
			hAnchor = ANCHOR_HORIZONTAL_RIGHT;
		}

		mButton = dynamic_cast<Button*>(_createChild(mInstanceName+".Button",TYPE_BUTTON));
		mButton->setSize(mSize.height,mSize.height);
		mButton->setPosition(buttonPosition);
		mButton->setAutoSize(false);
		mButton->setHorizontalAnchor(hAnchor);
		mButton->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
		mButton->setPropagateEventFiring(EVENT_MOUSE_BUTTON_UP,true);
	}

	void MenuLabel::setIconMaterial(const Ogre::String& materialName)
	{
		if(materialName == "")
			return;

		mIconMaterialName = materialName;

		if(mIcon != NULL)
		{
			mIcon->setMaterial(mIconMaterialName);
			return;
		}

		Point iconPosition;
		HorizontalAnchor hAnchor;

		if(mLayout == LAYOUT_LEFT_TO_RIGHT)
		{
			iconPosition = Point(0,0);
			hAnchor = ANCHOR_HORIZONTAL_RIGHT;
		}
		else
		{
			iconPosition = Point(mSize.width - mSize.height,0);
			hAnchor = ANCHOR_HORIZONTAL_LEFT;
		}

		mIcon = dynamic_cast<Image*>(_createChild(mInstanceName+".Image",TYPE_IMAGE));
		mIcon->setSize(mSize.height,mSize.height);
		mIcon->setPosition(iconPosition);
		mIcon->setHorizontalAnchor(hAnchor);
		mIcon->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);
		mIcon->setPropagateEventFiring(EVENT_MOUSE_BUTTON_UP,true);
		mIcon->setMaterial(mIconMaterialName);
	}

	void MenuLabel::onSizeChanged(const EventArgs& args)
	{
		Label::onSizeChanged(args);
		mTextBoundsPixelOffset.x = mSize.height;
		mTextBoundsRelativeSize.width = (mSize.width - (mSize.height * 2.0)) / mSize.width;
	}
}
