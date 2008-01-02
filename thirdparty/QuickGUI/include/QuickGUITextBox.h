#ifndef QUICKGUITEXTBOX_H
#define QUICKGUITEXTBOX_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUILabel.h"
#include "QuickGUIMouseCursor.h"

namespace QuickGUI
{
	/** Represents a TextBox.
		@remarks
		TextBoxes allow the user to input data on the screen,
		which can be used for other purposes.  The TextBox class 
		requires at least 2 materials to define it's image:
		Background Image, Border.  For example, if you pass
		the constructor "sample.textbox" as its arguement for the 
		material, the class will expect "sample.textbox.border"
		to exist.
		@note
		TextBoxes must be created by the Window widget.
	*/
	class _QuickGUIExport TextBox :
		public Label
	{
	public:
		friend class LabelArea;
	public:
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
				material Ogre material defining the widget image.
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		TextBox(const Ogre::String& name, GUIManager* gm);

		/**
		* Adds a character to the textBox right before text cursor.
		*/
		void addCharacter(Ogre::UTFString::code_point cp);
		/**
		* Add user defined event that will be called when user presses Enter key with Textbox Activated.
		*/
		template<typename T> void addOnEnterPressedEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnEnterPressedUserEventHandlers.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addOnEnterPressedEventHandler(MemberFunctionSlot* function);
		/**
		* Method to erase the character right before the text cursor.
		*/
		void backSpace();
		void clearText();
		/**
		* Method to erase the character right after the text cursor.
		*/
		void deleteCharacter();
		/**
		* Sets focus to the widget, displaying the text cursor.
		*/
		void focus();
		/**
		* Returns the index of the beginning of the next word.  If next word
		* does not exist, the last index of the previous word is returned.
		* NOTE: This function is relative to the Text Cursor.
		*/
		int getNextWordIndex();
		/**
		* Returns the index of the beginning of the previous word.  If previous word
		* does not exist, the first index of the previous word is returned.
		* NOTE: This function is relative to the Text Cursor.
		*/
		int getPreviousWordIndex();
		bool getReadOnly();
		Ogre::UTFString getText();
		/**
		* Sets mVisible to false.  Widgets should override this to implement how they handle
		* hiding.
		*/
		void hide();
		/**
		* Hides the actual user input and writes the designated character
		* in its place.  Great for visible password protection.
		*/
		void maskUserInput(const Ogre::UTFString::unicode_char& symbol=' ');
		void moveCursorLeft();
		void moveCursorRight();

		// Overridden Event Handling functions
		/**
		* Handler for the EVENT_LOSE_FOCUS event, and deactivates all child widgets (if exist)
		*/
		void onLoseFocus(const EventArgs& args);
		/**
		* User defined event handler called when user presses Enter.
		* Note that this event is not passed to its parents, the event is specific to this widget.
		*/
		void onEnterPressed(const KeyEventArgs& args);
		/**
		* Handler for the EVENT_KEY_DOWN event.  If not handled, it will be passed
		* to the parent widget (if exists)
		*/
		void onKeyDown(const EventArgs& args);
		/**
		* Handler for the EVENT_KEY_UP event.  If not handled, it will be passed
		* to the parent widget (if exists)
		*/
		void onKeyUp(const EventArgs& args);
		/**
		* Handler for the EVENT_CHARACTER_KEY event.  Appends character to the end of the Label's text.
		*/
		void onCharacter(const EventArgs& args);
		/**
		* Handler for the EVENT_MOUSE_BUTTON_DOWN event.  Used to implement text highlighting.
		*/
		void onMouseButtonDown(const EventArgs& args);
		/**
		* Handler for the EVENT_MOUSE_BUTTON_UP event.  Used to implement text highlighting.
		*/
		void onMouseButtonUp(const EventArgs& args);
		/**
		* Handler for the EVENT_MOUSE_CLICK event.  Sets the Cursor position.
		*/
		void onMouseClicked(const EventArgs& args);
		/**
		* Set true if the label's size should match it's font height and text width.
		* NOTE: AutoSize is set to true by default.  If you set this to false, you may
		*  end up with empty label's, as text that doesn't fit in the label won't be rendered.
		*/
		virtual void setAutoSize(bool autoSize);
		/**
		* Text Cursor Indices start at 0, and are to the left of Text Indices.  
		* Let () represent Text Cursor Indices and [] represent Text Indices: 
		*	(0)[0](1)[1](2)[2](3)
		*/
		void setCursorIndex(int cursorIndex, bool clearSelection = true);
		void setCursorIndex(Point position, bool clearSelection = true);
		virtual void setFont(const Ogre::String& fontScriptName, bool recursive = false);
		/**
		* If set to true, cannot input text to textbox
		*/
		void setReadOnly(bool readOnly);
		/**
		* Manually set size of widget.
		*/
		virtual void setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight);
		virtual void setSize(const Size& pixelSize);
		virtual void setSkin(const Ogre::String& skinName, bool recursive = false);
		void setText(const Ogre::UTFString& text);
		virtual void setWidth(Ogre::Real pixelWidth);
		/**
		* Default Handler for injecting Time.
		*/
		void timeElapsed(const Ogre::Real time);

	protected:
		virtual ~TextBox();

		MouseCursor* mMouseCursor;

		// Text Cursor Properties
		Quad* mTextCursor;
		size_t mCursorPixelWidth;
		Ogre::String mTextCursorSkinComponent;

		// The full unmodified String stored by this textbox. Text boundaries and character
		// masking will affect what the user sees on screen.
		Ogre::UTFString mCaption;
		// String used for visual display.
		Ogre::UTFString mOutput;

		// Text Cursor Indices start at 0, and are to the left of Text Indices.
		// Let () represent Text Cursor Indices and [] represent Text Indices: (0)[0](1)[1](2)
		int mCursorIndex;
		// Text Cursor Indices that represent the portion of text visible, given the textBounds.
		int mVisibleStart;
		int mVisibleEnd;
		// Text Cursor Indices that represent the portion of text that is selected.  If within
		// visible indices, will appear highlighted.
		int mSelectStart;
		int mSelectEnd;
		// previous index recorded to keep track of direction.
		int mSelectPrevious;

		// Masking, used for password textboxes.
		bool mMaskUserInput;
		Ogre::UTFString::unicode_char mMaskSymbol;

		Ogre::Real mBackSpaceTimer;
		bool mBackSpaceDown;

		Ogre::Real mDeleteTimer;
		bool mDeleteDown;

		Ogre::Real mMoveCursorTimer;
		bool mLeftArrowDown;
		bool mRightArrowDown;

		// Special keys used for text manipulation.
		bool mLShiftDown;
		bool mRShiftDown;
		bool mLCtrlDown;
		bool mRCtrlDown;

		Ogre::Real mCursorVisibilityTimer;

		bool mMouseLeftDown;
		// Used to determine if cursor should blink.
		bool mHasFocus;

		bool mReadOnly;

		EventHandlerArray mOnEnterPressedUserEventHandlers;

		// setCursorIndex functionality broken into parts.  Not to be called outside this function,
		// and is order dependent.
		void _determineTextSelectionBounds(int cursorIndex, bool clearSelection);
		void _determineVisibleTextBounds(int cursorIndex);
		void _positionCursor();

		// Updates the visible portion of the text, and the selection within the text.
		void _updateText();
		void _updateVisibleText();
		void _updateHighlightedText();
	};
}

#endif
