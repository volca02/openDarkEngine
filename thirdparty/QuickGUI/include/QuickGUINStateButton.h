#ifndef QUICKGUINSTATEBUTTON_H
#define QUICKGUINSTATEBUTTON_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIButton.h"

#include <vector>

namespace QuickGUI
{
	/** Represents a button with several states.
		@remarks
		A state is defined as an image and text. For a small
		example, a checkbox would be the same as a 2 state button.
		For each state, you need to define its original, mouse
		over, and mouse down state, and any text.
		@note
		A ButtonStateEvent was created for this widget.
		@note
		NStateButton must be created by a Window widget.
	*/
	class _QuickGUIExport NStateButton :
		public Button
	{
	public:
		/** A Particular Image and Text representing a Logical State.
			@note
			A State must be created by a NStateButton widget.
		*/
		class State
		{
		public:
			State(const Ogre::String& name, const Ogre::String& skin, const Ogre::String& skinComponent, Ogre::UTFString text="") :
			  mName(name),
			  mSkinName(skin),
			  mSkinComponentName(skinComponent),
			  mText(text)
			{}
			~State() {}

			Ogre::String getSkin() { return mSkinName; }
			Ogre::String getSkinComponent() { return mSkinComponentName; }
			Ogre::String getName() { return mName; }
			Ogre::UTFString getText() { return mText; }
			
		protected:
			Ogre::String mName;
			Ogre::String mSkinName;
			Ogre::String mSkinComponentName;
			Ogre::UTFString mText;
		};

		/** Constructor
            @param
                name The name to be given to the widget (must be unique).
            @param
                dimensions The x Position, y Position, width, and height of the widget.
			@param
				positionMode The GuiMetricsMode for the values given for the position. (absolute/relative/pixel)
			@param
				sizeMode The GuiMetricsMode for the values given for the size. (absolute/relative/pixel)
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		NStateButton(const Ogre::String& name, GUIManager* gm);

		/**
		* Add user defined event that will be called when the state of the button has changed.
		*/
		template<typename T> void addOnStateChangedEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnStateChangedUserEventHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnStateChangedEventHandler(MemberFunctionSlot* function);
		/**
		* Adds (and creates) a state to the button.  If it is the first state, the state will be applied.
		*/
		void addState(const Ogre::String& name, const Ogre::String& skin, const Ogre::String& skinComponent, Ogre::UTFString text="");
		/**
		* Useful when you want to simulate the button being pressed down by the mouse.
		* If you actually want to click the mouse, use the mouse, or call onMouseButtonDown.
		*/
		virtual void applyButtonDownTexture();
		virtual void applyButtonOverTexture();
		/**
		* If supplying a method to simulate the button being pressed down, we need a method
		* to restore the button to the normal looking state.
		*/
		virtual void applyDefaultTexture();

		/**
		* Removes (and destroys) all states.  Widget has no appearance after this, since no states are defined.
		*/
		void clearStates();

		/**
		* Returns the current state of the widget, if defined.
		*/
		State* getCurrentState();
		/**
		* Returns the index of the state provided.
		*/
		int getIndexOfState(State* s);
		/**
		* Returns the next state in the list of defined states.
		*/
		State* getNextState();
		/**
		* Returns the previous state in the list of defined states.
		*/
		State* getPreviousState();
		/**
		* Returns the state corresponding to the index in the list of defined states.
		* No exception is thrown if index is out of bounds.
		*/
		State* getState(Ogre::ushort index);
		/**
		* Returns the state with the name provided.
		* No exception is state does not exist.
		*/
		State* getState(const Ogre::String& name);

		/**
		* Default Handler for the EVENT_MOUSE_BUTTON_UP event.
		*/
		void onMouseButtonUp(const EventArgs& args);
		/**
		* Default Handler for handling state changes.
		*/
		void onStateChanged(const WidgetEventArgs& e);
		/**
		* Manually setting the current State of the widget. onStateChanged will be called.
		*/
		void setCurrentState(State* s);
		void setCurrentState(Ogre::ushort index);
		void setCurrentState(const Ogre::String& name);

		/**
		* Advancing the state of the Widget.(circular) onStateChanged will be called.
		*/
		void toggleNextState();
		/**
		* Setting State to Previously defined State.(circular) onStateChanged will be called.
		*/
		void togglePreviousState();
		
	protected:
		virtual ~NStateButton();

		std::vector<State*>	mStates;

		State* mCurrentState;
		Ogre::String mBaseTexture;
		Ogre::String mBase;
		Ogre::String mExtension;

		EventHandlerArray mOnStateChangedUserEventHandlers;
	};
}

#endif
