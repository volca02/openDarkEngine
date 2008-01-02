#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUINStateButton.h"
#include "QuickGUIUtility.h" 

namespace QuickGUI
{
	NStateButton::NStateButton(const Ogre::String& name, GUIManager* gm) :
		Button(name,gm),
		mBaseTexture("")
	{
		mWidgetType = TYPE_NSTATEBUTTON;
		mSkinComponent = "";
		mSize = Size(75,25);
		mAutoSize = false;

		addEventHandler(EVENT_MOUSE_BUTTON_UP,&NStateButton::onMouseButtonUp,this);

		mOnStateChangedUserEventHandlers.clear();
	}

	NStateButton::~NStateButton()
	{
		clearStates();
	}

	void NStateButton::addOnStateChangedEventHandler(MemberFunctionSlot* function)
	{
		mOnStateChangedUserEventHandlers.push_back(function);
	}

	void NStateButton::addState(const Ogre::String& name, const Ogre::String& skin, const Ogre::String& skinComponent, Ogre::UTFString text)
	{
		NStateButton::State* s = new NStateButton::State(name,skin,skinComponent,text);
		mStates.push_back(s);

		// The widget has an image when the first state gets added
		if(static_cast<int>(mStates.size()) == 1)
		{
			setCurrentState(mStates[0]);
		}
	}

	void NStateButton::applyButtonDownTexture()
	{
		// apply button ".down" texture
		Ogre::String skin = mCurrentState->getSkin();
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(skin);
		if(ss == NULL)
			throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND,"SkinSet " + skin + " has not been loaded!","NStateButton::applyButtonDownTexture");

		mQuad->setMaterial(ss->getMaterialName());
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(skin + mCurrentState->getSkinComponent() + ".down" + ss->getImageExtension()));
	}

	void NStateButton::applyButtonOverTexture()
	{
		// apply button ".over" texture
		Ogre::String skin = mCurrentState->getSkin();
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(skin);
		if(ss == NULL)
			throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND,"SkinSet " + skin + " has not been loaded!","NStateButton::applyButtonDownTexture");

		mQuad->setMaterial(ss->getMaterialName());
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(skin + mCurrentState->getSkinComponent() + ".over" + ss->getImageExtension()));
	}

	void NStateButton::applyDefaultTexture()
	{
		Ogre::String skin = mCurrentState->getSkin();
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(skin);
		if(ss == NULL)
			throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND,"SkinSet " + skin + " has not been loaded!","NStateButton::applyButtonDownTexture");

		mQuad->setMaterial(ss->getMaterialName());
		Ogre::String texture = skin + mCurrentState->getSkinComponent() + ss->getImageExtension();
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(texture));

		// Load Image, used for transparency picking.
		if(Utility::textureExistsOnDisk(texture))
		{
			Ogre::Image i;
			i.load(texture,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			delete mWidgetImage;
			mWidgetImage = new Ogre::Image(i);
		}
	}

	void NStateButton::clearStates()
	{
		std::vector<State*>::iterator it;
		for( it = mStates.begin(); it != mStates.end(); ++it )
		{
			delete (*it);
		}
		mStates.clear();
	}

	NStateButton::State* NStateButton::getCurrentState()
	{
		return mCurrentState;
	}

	int NStateButton::getIndexOfState(NStateButton::State* s)
	{
		int index = 0;
		std::vector<State*>::iterator it;
		for( it = mStates.begin(); it != mStates.end(); ++it )
		{
			if( (*it)->getName() == s->getName() ) return index;
			++index;
		}

		return -1;
	}

	NStateButton::State* NStateButton::getNextState()
	{
		int index = getIndexOfState(mCurrentState);
		
		int nextStateIndex = index + 1;
		
		if( nextStateIndex >= static_cast<int>(mStates.size()) ) 
			nextStateIndex =  0;

		return mStates[nextStateIndex];
	}

	NStateButton::State* NStateButton::getPreviousState()
	{
		int index = getIndexOfState(mCurrentState);

		int prevStateIndex = index - 1;
		
		if( prevStateIndex < 0 ) 
			prevStateIndex = static_cast<int>(mStates.size()) - 1;

		return mStates[prevStateIndex];
	}

	NStateButton::State* NStateButton::getState(Ogre::ushort index)
	{
		if( mStates.size() < index ) return NULL;
		return mStates[index];
	}

	NStateButton::State* NStateButton::getState(const Ogre::String& name)
	{
		std::vector<State*>::iterator it;
		for( it = mStates.begin(); it != mStates.end(); ++it )
		{
			if( (*it)->getName() == name ) return (*it);
		}

		return NULL;
	}

	void NStateButton::onMouseButtonUp(const EventArgs& args)
	{
		const MouseEventArgs mea = dynamic_cast<const MouseEventArgs&>(args);

		if( mea.button == QuickGUI::MB_Left ) 
			toggleNextState();
		else if( mea.button == QuickGUI::MB_Right ) 
			togglePreviousState();

		Button::onMouseButtonUp(args);
	}

	void NStateButton::onStateChanged(const WidgetEventArgs& args)
	{
		EventHandlerArray::iterator it;
		for( it = mOnStateChangedUserEventHandlers.begin(); it != mOnStateChangedUserEventHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void NStateButton::setCurrentState(State* s)
	{
		if(!mEnabled)
			return;

		// Make sure s is a state within this State Button
		if(getState(s->getName()) == NULL) 
			return;

		mCurrentState = s;
		applyDefaultTexture();
		mText->setCaption(mCurrentState->getText());

		WidgetEventArgs args(this);
		onStateChanged(args);
	}

	void NStateButton::setCurrentState(Ogre::ushort index)
	{
		State* s = getState(index);

		if( s == NULL ) 
			return;

		setCurrentState(s);
	}

	void NStateButton::setCurrentState(const Ogre::String& name)
	{
		State* s = getState(name);

		if( s == NULL ) 
			return;
		
		setCurrentState(s);
	}

	void NStateButton::toggleNextState()
	{
		int index = getIndexOfState(mCurrentState);
		
		int nextStateIndex = index + 1;
		
		if( nextStateIndex >= static_cast<int>(mStates.size()) ) 
			nextStateIndex =  0;

		setCurrentState(mStates[nextStateIndex]);
	}

	void NStateButton::togglePreviousState()
	{
		int index = getIndexOfState(mCurrentState);

		int prevStateIndex = index - 1;
		
		if( prevStateIndex < 0 ) 
			prevStateIndex = static_cast<int>(mStates.size()) - 1;

		setCurrentState(mStates[prevStateIndex]);
	}
}
