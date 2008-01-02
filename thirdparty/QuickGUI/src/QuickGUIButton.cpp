#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUISkinSetManager.h"

#include "QuickGUIButton.h"
#include "QuickGUIManager.h"
#include "OgreFontManager.h"
#include "OgreFont.h"

namespace QuickGUI
{
	Button::Button(const Ogre::String& name, GUIManager* gm) :
		Label(name,gm),
		mButtonDown(false)
	{
		mWidgetType = TYPE_BUTTON;
		mSkinComponent = ".button";
		mSize = Size(75,25);

		addEventHandler(EVENT_MOUSE_ENTER,&Button::onMouseEnters,this);
		addEventHandler(EVENT_MOUSE_LEAVE,&Button::onMouseLeaves,this);
		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&Button::onMouseButtonDown,this);
		addEventHandler(EVENT_MOUSE_BUTTON_UP,&Button::onMouseButtonUp,this);
	}

	Button::~Button()
	{
	}

	void Button::applyButtonDownTexture()
	{
		// apply button ".down" texture
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ".down" + ss->getImageExtension()));
	}

	void Button::applyButtonOverTexture()
	{
		// apply button ".over" texture
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ".over" + ss->getImageExtension()));
	}

	void Button::applyDefaultTexture()
	{
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(mSkinName + mSkinComponent + ss->getImageExtension()));
	}

	bool Button::isDown()
	{
		return mButtonDown;
	}

	void Button::onMouseButtonDown(const EventArgs& args) 
	{
		if(dynamic_cast<const MouseEventArgs&>(args).button == MB_Left)
		{
			applyButtonDownTexture();
			mButtonDown = true;
		}
	}

	void Button::onMouseButtonUp(const EventArgs& args) 
	{ 
		if(dynamic_cast<const MouseEventArgs&>(args).button == MB_Left)
		{
			applyButtonOverTexture();
			mButtonDown = false;
		}
	}

	void Button::onMouseEnters(const EventArgs& args) 
	{ 
		if(mGrabbed) 
		{
			applyButtonDownTexture();
			mButtonDown = true;
		}
		else 
			applyButtonOverTexture();
	}

	void Button::onMouseLeaves(const EventArgs& args) 
	{ 
		applyDefaultTexture();
		mButtonDown = false;
	}
}
