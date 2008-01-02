#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUITextBox.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	TextBox::TextBox(const Ogre::String& name, GUIManager* gm) :
		Label(name,gm),
		mMaskUserInput(0),
		mBackSpaceDown(0),
		mBackSpaceTimer(0.0),
		mDeleteDown(0),
		mDeleteTimer(0.0),
		mLeftArrowDown(0),
		mRightArrowDown(0),
		mMoveCursorTimer(0.0),
		mCursorVisibilityTimer(0.0),
		mReadOnly(0),
		mCursorPixelWidth(4),
		mCursorIndex(0),
		mCaption(""),
		mVisibleStart(0),
		mVisibleEnd(0),
		mHasFocus(false),
		mMouseLeftDown(false),
		mLShiftDown(false),
		mRShiftDown(false),
		mLCtrlDown(false),
		mRCtrlDown(false),
		mTextCursorSkinComponent(".textbox.textcursor")
	{
		mWidgetType = TYPE_TEXTBOX;
		mSkinComponent = ".textbox";
		mSize = Size(100,0);

		mHorizontalAlignment = HA_LEFT;
		mMouseCursor = mGUIManager->getMouseCursor();
		mGUIManager->registerTimeListener(this);

		addEventHandler(EVENT_LOSE_FOCUS,&TextBox::onLoseFocus,this);
		addEventHandler(EVENT_CHARACTER_KEY,&TextBox::onCharacter,this);
		addEventHandler(EVENT_MOUSE_BUTTON_DOWN,&TextBox::onMouseButtonDown,this);
		addEventHandler(EVENT_MOUSE_BUTTON_UP,&TextBox::onMouseButtonUp,this);
		addEventHandler(EVENT_MOUSE_CLICK,&TextBox::onMouseClicked,this);
		addEventHandler(EVENT_KEY_DOWN,&TextBox::onKeyDown,this);
		addEventHandler(EVENT_KEY_UP,&TextBox::onKeyUp,this);

		mTextCursor = _createQuad();
		mTextCursor->setLayer(Quad::LAYER_CHILD);
		mTextCursor->setShowWithOwner(false);
		mTextCursor->setOffset(mOffset+1);
		mTextCursor->setVisible(false);
		mTextCursor->setSize(Size(mCursorPixelWidth,mSize.height));
		mTextCursor->_notifyQuadContainer(mQuadContainer);
	}

	TextBox::~TextBox()
	{
		mGUIManager->unregisterTimeListener(this);

		EventHandlerArray::iterator it;
		for( it = mOnEnterPressedUserEventHandlers.begin(); it != mOnEnterPressedUserEventHandlers.end(); ++it )
			delete (*it);
		mOnEnterPressedUserEventHandlers.clear();
	}

	void TextBox::_determineTextSelectionBounds(int cursorIndex, bool clearSelection)
	{
		// clear selection, or grow/shrink selection.
		if(clearSelection)
			mSelectStart = mSelectEnd = mSelectPrevious = cursorIndex;
		else
		{
			if( cursorIndex < mSelectPrevious )
			{
				if(mSelectPrevious > mSelectStart)
					mSelectEnd = mSelectStart;
				
				// Text Selection: determine whether we are growing left, or shrinking right.
				if(cursorIndex < mSelectStart)
					mSelectStart = cursorIndex;
				// shrink selection from the right.
				else if(cursorIndex >= mSelectStart)
					mSelectEnd = cursorIndex;
			}
			else if( cursorIndex > mSelectPrevious )
			{
				if(mSelectPrevious < mSelectEnd)
					mSelectStart = mSelectEnd;

				// Text Selection: determine whether we are growing right, or shrinking left.
				if(cursorIndex > mSelectEnd)
					mSelectEnd = cursorIndex;
				// shrink selection from the left.
				else if(cursorIndex <= mSelectEnd)
					mSelectStart = cursorIndex;
			}
			else // cursorIndex == mSelectPrevious
			{
				if((mCursorIndex != static_cast<int>(mCaption.length())) && (mCursorIndex != 0))
					mSelectStart = mSelectEnd = cursorIndex;
			}

			mSelectPrevious = cursorIndex;
		}
	}

	void TextBox::_determineVisibleTextBounds(int cursorIndex)
	{
		Ogre::Real maxTextWidth = getTextBounds().width;

		Ogre::Real width = 0;
		// Shift text left to show portion of caption starting at cursorIndex.
		if( cursorIndex < mVisibleStart )
		{
			mVisibleStart = cursorIndex;
			mVisibleEnd = mVisibleStart + 1;

			width = mText->getTextWidth(mCaption.substr(mVisibleStart,mVisibleEnd - mVisibleStart));
			while( (mVisibleEnd < static_cast<int>(mCaption.length())) && (width <= maxTextWidth) )
			{
				width += mText->getGlyphWidth(mCaption[mVisibleEnd]);
				++mVisibleEnd;
			}

			// When we exit the while loop, we are either at visibleStart == mCaption.length(), or we have exceeded the width limitation.
			if( mVisibleEnd != static_cast<int>(mCaption.length()) )
				--mVisibleEnd;
		}
		// Shift text right to show portion of caption ending at cursorIndex.
		else if( cursorIndex > mVisibleEnd )
		{
			mVisibleEnd = cursorIndex;
			mVisibleStart = mVisibleEnd - 1;

			width = mText->getTextWidth(mCaption.substr(mVisibleStart,mVisibleEnd - mVisibleStart));
			while( (mVisibleStart > 0) && (width <= maxTextWidth) )
			{
				--mVisibleStart;
				width += mText->getGlyphWidth(mCaption[mVisibleStart]);
			}

			// When we exit the while loop, we are either at visibleStart == 0, or we have exceeded the width limitation.
			if( mVisibleStart > 0 )
				++mVisibleStart;
		}
		// start from visible start and display the maximum number of characters until text bounds reached.
		else
		{
			mVisibleEnd = mVisibleStart + 1;

			width = mText->getTextWidth(mCaption.substr(mVisibleStart,mVisibleEnd - mVisibleStart));
			while( (mVisibleEnd < static_cast<int>(mCaption.length())) && (width <= maxTextWidth) )
			{
				width += mText->getGlyphWidth(mCaption[mVisibleEnd]);
				++mVisibleEnd;
			}

			// When we exit the while loop, we are either at visibleStart == mCaption.length(), or we have exceeded the width limitation.
			if( mVisibleEnd != static_cast<int>(mCaption.length()) )
				--mVisibleEnd;
		}
	}

	void TextBox::_positionCursor()
	{
		Rect textBounds = getTextBounds();
		Point cursorPos;

		// If the Text was not rendered, as is the case when the glyph (font) height is bigger than the area provided,
		// place the cursor according to horizontal/vertical alignment.
		if( mText->getNumberOfCharacters() == 0 )
		{
			switch(mHorizontalAlignment)
			{
			case HA_LEFT:
				cursorPos.x = textBounds.x;
				break;
			case HA_MID:
				cursorPos.x = (textBounds.x + (textBounds.width / 2));
				break;
			case HA_RIGHT:
				cursorPos.x = (textBounds.x + textBounds.width);
				break;
			}

			switch(mVerticalAlignment)
			{
			case VA_TOP:
				cursorPos.y = textBounds.y;
				break;
			case VA_MID:
				cursorPos.y = (textBounds.y + (textBounds.height / 2));
				break;
			case VA_BOTTOM:
				cursorPos.y = (textBounds.y + textBounds.height);
				break;
			}
		}
		else
		{
			// Now that the correct text will be displayed, we can correctly position the text cursor.
			if( (mCursorIndex - mVisibleStart) == 0 )
			{
				if( mCaption.empty() )
				{
					switch(mHorizontalAlignment)
					{
					case HA_LEFT:
						cursorPos.x = textBounds.x;
						break;
					case HA_MID:
						cursorPos.x = (textBounds.x + (textBounds.width / 2));
						break;
					case HA_RIGHT:
						cursorPos.x = (textBounds.x + textBounds.width);
						break;
					}

					switch(mVerticalAlignment)
					{
					case VA_TOP:
						cursorPos.y = textBounds.y;
						break;
					case VA_MID:
						cursorPos.y = (textBounds.y + (textBounds.height / 2));
						break;
					case VA_BOTTOM:
						cursorPos.y = (textBounds.y + textBounds.height);
						break;
					}
				}
				else
				{
					Quad* character = mText->getCharacter(mCursorIndex - mVisibleStart);
					Point pos = character->getPosition();
					Size size = character->getSize();
					cursorPos.x = pos.x;
					cursorPos.y = pos.y + (size.height / 2);
				}
			}
			else
			{
				Quad* character = mText->getCharacter((mCursorIndex - mVisibleStart) - 1);
				Point pos = character->getPosition();
				Size size = character->getSize();
				cursorPos.x = pos.x + size.width;
				cursorPos.y = pos.y + (size.height / 2);
			}
		}
		
		// horizontally center the text cursor image at the desired position.
		Size s = mTextCursor->getSize();
		cursorPos.x -= (s.width / 2);
		cursorPos.y -= (s.height / 2);
		mTextCursor->setPosition(cursorPos);
	}

	void TextBox::_updateText()
	{
		_updateVisibleText();
		_updateHighlightedText();
	}

	void TextBox::_updateVisibleText()
	{
		// Handle masking of characters.
		mOutput.clear();
		if(mMaskUserInput)
			mOutput.append(mCaption.length(),mMaskSymbol);
		else 
			mOutput = mCaption;

		mText->setCaption(mOutput.substr(mVisibleStart,mVisibleEnd - mVisibleStart));
	}

	void TextBox::_updateHighlightedText()
	{
		if(mSelectStart == mSelectEnd)
			mText->clearSelection();
		else
		{
			if((mSelectStart < mVisibleEnd) && (mSelectEnd > mVisibleStart))
			{
				int visibleSelectStart = mSelectStart;
				if( mSelectStart < mVisibleStart )
					visibleSelectStart = mVisibleStart;

				int visibleSelectEnd = mSelectEnd;
				if( mSelectEnd > mVisibleEnd )
					visibleSelectEnd = mVisibleEnd;
				
				mText->selectCharacters(visibleSelectStart - mVisibleStart,(visibleSelectEnd - 1) - mVisibleStart);
			}
		}
	}

	void TextBox::addCharacter(Ogre::UTFString::code_point cp)
	{
		if(mReadOnly)
			return;

		mMouseLeftDown = false;
		mHasFocus = true;

		// Remove selection if a selection has been made.
		if(mSelectStart != mSelectEnd)
		{
			mCaption = mCaption.erase(mSelectStart,mSelectEnd-mSelectStart);
			mCursorIndex = mSelectStart;
		}

		// Insert a character right before the text cursor.
		mCaption.insert(mCursorIndex,1,cp);
		
		setCursorIndex(mCursorIndex + 1);
	}

	void TextBox::addOnEnterPressedEventHandler(MemberFunctionSlot* function)
	{
		mOnEnterPressedUserEventHandlers.push_back(function);
	}

	void TextBox::backSpace()
	{
		if(mReadOnly || (mCaption.empty()))
			return;

		mMouseLeftDown = false;
		mHasFocus = true;

		// Remove selection if a selection has been made.
		if(mSelectStart != mSelectEnd)
		{
			mCaption = mCaption.erase(mSelectStart,mSelectEnd-mSelectStart);
			setCursorIndex(mSelectStart);
			return;
		}

		// If there is no selection, but we're at the beginning of the string, return.
		if(mCursorIndex == 0)
			return;

		// Decrement visible start.
		--mVisibleStart;
		if(mVisibleStart < 0)
			mVisibleStart = 0;

		mCaption.erase(mCursorIndex - 1,1);

		setCursorIndex(mCursorIndex - 1);
	}

	void TextBox::clearText()
	{
		setText("");
	}

	void TextBox::deleteCharacter()
	{
		if(mReadOnly || (mCaption.empty()))
			return;

		mMouseLeftDown = false;
		mHasFocus = true;

		// Remove selection if a selection has been made.
		if(mSelectStart != mSelectEnd)
		{
			mCaption = mCaption.erase(mSelectStart,mSelectEnd-mSelectStart);
			setCursorIndex(mSelectStart);
			return;
		}

		// Unlike Backspace, we do not need to modify (decrement) visible end.

		mCaption.erase(mCursorIndex,1);

		setCursorIndex(mCursorIndex);
	}

	void TextBox::focus()
	{
		if(!mEnabled) 
			return;

		mTextCursor->setVisible(true);

		Rect cursorOrigin;
		cursorOrigin.x = mTextCursor->getPosition().x;
		cursorOrigin.y = mTextCursor->getPosition().y + (mTextCursor->getSize().height / 2);
		cursorOrigin.width = mTextCursor->getSize().width;
		cursorOrigin.height = mTextCursor->getSize().height;

		mSelectStart = mSelectEnd = mSelectPrevious = mVisibleStart + mText->getTextCursorIndex(cursorOrigin);
		setCursorIndex(mSelectStart);
		mHasFocus = true;

		mGUIManager->setActiveWidget(this);
	}

	int TextBox::getNextWordIndex()
	{
		// cover bounds.
		if(mCaption.empty())
			return 0;
		else if(mCursorIndex == (static_cast<int>(mCaption.length())))
			return mCursorIndex;

		int whiteSpaceStart = mCursorIndex;
		// iterate through string until whitespace found.
		while((whiteSpaceStart < (static_cast<int>(mCaption.length()) - 1)) && !((mCaption[whiteSpaceStart] == ' ') || (mCaption[whiteSpaceStart] == '\t')))
			++whiteSpaceStart;

		int whiteSpaceEnd = whiteSpaceStart;
		// iterate through string until non whitespace found.
		while((whiteSpaceEnd < (static_cast<int>(mCaption.length()) - 1)) && ((mCaption[whiteSpaceEnd] == ' ') || (mCaption[whiteSpaceEnd] == '\t')))
			++whiteSpaceEnd;

		if(whiteSpaceEnd == (static_cast<int>(mCaption.length()) - 1))
			return (whiteSpaceEnd + 1);
		else
			return whiteSpaceEnd;
	}

	int TextBox::getPreviousWordIndex()
	{
		// cover bounds.
		if(mCaption.empty())
			return 0;
		else if(mCursorIndex == 0)
			return mCursorIndex;

		int whiteSpaceEnd = mCursorIndex - 1;
		// iterate through string until non whitespace found.  Use the string "   abc def[cursor]" as an example.
		while((whiteSpaceEnd > -1) && ((mCaption[whiteSpaceEnd] == ' ') || (mCaption[whiteSpaceEnd] == '\t')))
			--whiteSpaceEnd;

		// "   abc de[cursor]f".

		int whiteSpaceStart = whiteSpaceEnd;
		// iterate through string until whitespace found.
		while((whiteSpaceStart > -1) && !((mCaption[whiteSpaceStart] == ' ') || (mCaption[whiteSpaceStart] == '\t')))
			--whiteSpaceStart;

		// "   abc [cursor]def".

		return (whiteSpaceStart + 1);
	}

	bool TextBox::getReadOnly()
	{
		return mReadOnly;
	}

	Ogre::UTFString TextBox::getText()
	{
		return mCaption;
	}

	void TextBox::hide()
	{
		Label::hide();

		mBackSpaceDown = false;
		mLeftArrowDown = false;
		mRightArrowDown = false;
		mHasFocus = false;
		mMouseLeftDown = false;
	}

	void TextBox::maskUserInput(const Ogre::UTFString::unicode_char& symbol)
	{
		mMaskSymbol = symbol;

		// if there was previously text, we now need to mask it.
		if( (mCaption != "") && !mMaskUserInput ) 
			setText(mCaption);

		mMaskUserInput = true;
	}

	void TextBox::moveCursorLeft()
	{
		// if previous selection exists, place cursor at start of selection, and remove selection.
		if((mSelectStart != mSelectEnd) && !(mLShiftDown || mRShiftDown))
			setCursorIndex(mSelectStart);
		else if(mLCtrlDown || mRCtrlDown)
			setCursorIndex(getPreviousWordIndex(),!(mLShiftDown || mRShiftDown));
		else
			setCursorIndex(mCursorIndex - 1,!(mLShiftDown || mRShiftDown));
	}

	void TextBox::moveCursorRight()
	{
		// if previous selection exists, place cursor at end of selection, and remove selection.
		if((mSelectStart != mSelectEnd) && !(mLShiftDown || mRShiftDown))
			setCursorIndex(mSelectEnd);
		else if(mLCtrlDown || mRCtrlDown)
			setCursorIndex(getNextWordIndex(),!(mLShiftDown || mRShiftDown));
		else
			setCursorIndex(mCursorIndex + 1,!(mLShiftDown || mRShiftDown));
	}

	void TextBox::onLoseFocus(const EventArgs& args)
	{
		mText->clearSelection();
		mTextCursor->setVisible(false);
		mBackSpaceDown = false;
		mLeftArrowDown = false;
		mRightArrowDown = false;
		mHasFocus = false;
		mMouseLeftDown = false;
	}

	void TextBox::onEnterPressed(const KeyEventArgs& args)
	{
		EventHandlerArray::iterator it;
		for( it = mOnEnterPressedUserEventHandlers.begin(); it != mOnEnterPressedUserEventHandlers.end(); ++it )
			(*it)->execute(args);
	}

	void TextBox::onCharacter(const EventArgs& args) 
	{ 
		if(!mReadOnly) 
		{
			mBackSpaceDown = false;
			mLeftArrowDown = false;
			mRightArrowDown = false;

			addCharacter(dynamic_cast<const KeyEventArgs&>(args).codepoint);
		}
	}

	void TextBox::onKeyDown(const EventArgs& args) 
	{ 
		switch(dynamic_cast<const KeyEventArgs&>(args).scancode)
		{
		case KC_BACK:
			if(!mReadOnly)
			{
				mBackSpaceDown = true;
				mLeftArrowDown = false;
				mRightArrowDown = false;
				mBackSpaceTimer = 0.0;
				backSpace();
			}
			break;
		case KC_LEFT:
			// Make sure tapping the left key moves the cursor left.
			moveCursorLeft();

			mMoveCursorTimer = 0.0;
			mLeftArrowDown = true;
			mRightArrowDown = false;
			break;
		case KC_RIGHT:
			// Make sure tapping the right key moves the cursor right.
			moveCursorRight();

			mMoveCursorTimer = 0.0;
			mRightArrowDown = true;
			mLeftArrowDown = false;
			break;
		case KC_DELETE:
			if(!mReadOnly)
			{
				mDeleteTimer = 0.0;
				mDeleteDown = true;
				mBackSpaceDown = false;
				mLeftArrowDown = false;
				mRightArrowDown = false;
				deleteCharacter();
			}
			break;
		case KC_LSHIFT:
			mLShiftDown = true;
			break;
		case KC_RSHIFT:
			mRShiftDown = true;
			break;
		case KC_LCONTROL:
			mLCtrlDown = true;
			break;
		case KC_RCONTROL:
			mRCtrlDown = true;
			break;
		case KC_HOME:
			setCursorIndex(0,!(mLShiftDown || mRShiftDown));
			break;
		case KC_END:
			setCursorIndex(static_cast<int>(mCaption.length()),!(mLShiftDown || mRShiftDown));
			break;
		}
	}

	void TextBox::onKeyUp(const EventArgs& args)
	{
		switch(dynamic_cast<const KeyEventArgs&>(args).scancode)
		{
		case KC_BACK:
			if(!mReadOnly)
				mBackSpaceDown = false;
			break;
		case KC_LEFT:
			mLeftArrowDown = false;
			break;
		case KC_RIGHT:
			mRightArrowDown = false;
			break;
		case KC_DELETE:
			mDeleteDown = false;
			break;
		case KC_NUMPADENTER:
			{
				KeyEventArgs args(this);
				args.scancode = KC_NUMPADENTER;
				onEnterPressed(args);
			}
			break;
		case KC_RETURN:
			{
				KeyEventArgs args(this);
				args.scancode = KC_RETURN;
				onEnterPressed(args);
			}
			break;
		case KC_LSHIFT:
			mLShiftDown = false;
			break;
		case KC_RSHIFT:
			mRShiftDown = false;
			break;
		case KC_LCONTROL:
			mLCtrlDown = false;
			break;
		case KC_RCONTROL:
			mRCtrlDown = false;
			break;
		}
	}

	void TextBox::onMouseButtonDown(const EventArgs& args)
	{
		if(mReadOnly)
			return;

		mHasFocus = true;

		const MouseEventArgs mea = dynamic_cast<const MouseEventArgs&>(args);
		if(mea.button == MB_Left)
		{
			mMouseLeftDown = true;

			setCursorIndex(mVisibleStart + mText->getTextCursorIndex(mea.position));
		}
	}

	void TextBox::onMouseButtonUp(const EventArgs& args)
	{
		if(dynamic_cast<const MouseEventArgs&>(args).button == MB_Left)
			mMouseLeftDown = false;
	}

	void TextBox::onMouseClicked(const EventArgs& args)
	{
		if(!mGainFocusOnClick)
			return;

		mHasFocus = true;

		const MouseEventArgs mea = dynamic_cast<const MouseEventArgs&>(args);
		if(mea.button == MB_Left)
		{
			mMouseLeftDown = false;

			setCursorIndex(mVisibleStart + mText->getTextCursorIndex(mea.position));
		}
	}

	void TextBox::setAutoSize(bool autoSize)
	{
		mAutoSize = autoSize;

		if(mAutoSize)
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
	}

	void TextBox::setCursorIndex(int cursorIndex, bool clearSelection)
	{
		if( cursorIndex < 0 )
			cursorIndex = 0;
		else if( cursorIndex > static_cast<int>(mCaption.length()) )
			cursorIndex = static_cast<int>(mCaption.length());

		// Shifts the text if needbe, and displays the maximum portion of the caption, given the text bounds and cursor index.
		_determineVisibleTextBounds(cursorIndex);
		// Grows, shrinks, or clears the text selection. (determines mSelectStart and mSelectEnd)
		_determineTextSelectionBounds(cursorIndex,clearSelection);

		// render the proper section of text, and update selection accordingly.
		_updateText();

		mCursorIndex = cursorIndex;

		_positionCursor();

		// make sure cursor is visible.
		mTextCursor->setVisible(true);
		mCursorVisibilityTimer = 0.0;
	}

	void TextBox::setCursorIndex(Point position, bool clearSelection)
	{
		setCursorIndex(mVisibleStart + mText->getTextCursorIndex(position),clearSelection);
	}

	void TextBox::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		Widget::setFont(fontScriptName,recursive);
		mText->setFont(mFontName);

		if(mAutoSize)
		{
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
			mTextCursor->setHeight(mSize.height);
			// setHeight sets mAutoSize to false..
			mAutoSize = true;
		}
		else
			alignText();

		// re-initialize index variables
		mVisibleStart = 0;
		mVisibleEnd = 0;
		mSelectStart = 0;
		mSelectEnd = 0;

		setCursorIndex(static_cast<int>(mCaption.length()));
		mTextCursor->setVisible(false);
	}

	void TextBox::setReadOnly(bool readOnly)
	{
		mReadOnly = readOnly;
	}

	void TextBox::setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight)
	{
		Label::setSize(pixelWidth,pixelHeight);

		if(pixelHeight == 0)
			mAutoSize = true;
	}

	void TextBox::setSize(const Size& pixelSize)
	{
		TextBox::setSize(pixelSize.width,pixelSize.height);
	}

	void TextBox::setSkin(const Ogre::String& skinName, bool recursive)
	{
		Label::setSkin(skinName,recursive);

		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(mSkinName);
		if(ss != NULL)
		{
			mTextCursor->setMaterial(ss->getMaterialName());
			mTextCursor->setTextureCoordinates(ss->getTextureCoordinates(skinName + mTextCursorSkinComponent + ss->getImageExtension()));
		}
	}

	void TextBox::setText(const Ogre::UTFString& text)
	{
		if(text == mCaption)
			return;

		mCaption = text;

		mVisibleStart = 0;
		mVisibleEnd = 0;
		mSelectStart = 0;
		mSelectEnd = 0;

		// This function determines the visibleStart and visibleEnd, and
		// updates the text before setting the cursor position.
		setCursorIndex(static_cast<int>(mCaption.length()));
		mTextCursor->setVisible(false);
	}

	void TextBox::setWidth(Ogre::Real pixelWidth)
	{
		bool temp = mAutoSize;
		Label::setWidth(pixelWidth);
		mAutoSize = temp;

		mText->redraw();
		alignText();
	}

	void TextBox::timeElapsed(const Ogre::Real time)
	{
		if(mHasFocus && mMouseLeftDown)
		{
			int index = mVisibleStart + mText->getTextCursorIndex(mMouseCursor->getPosition());
			if(index != mCursorIndex)
				setCursorIndex(index,false);
		}

		if(!mReadOnly)
		{
			mBackSpaceTimer += time;
			mCursorVisibilityTimer += time;
			mMoveCursorTimer += time;
			mDeleteTimer += time;

			// Hard coding the time to allow repetitive operations to be every .5 seconds
			if( mBackSpaceTimer > 0.125 )
			{
				if(mBackSpaceDown && !mReadOnly) backSpace();
				mBackSpaceTimer = 0.0;
			}

			if( mCursorVisibilityTimer > 0.5 )
			{
				if(mHasFocus && !mReadOnly) 
					mTextCursor->setVisible(!mTextCursor->visible());
				mCursorVisibilityTimer = 0.0;
			}

			if( mMoveCursorTimer > 0.125 )
			{
				if(mLeftArrowDown)
					moveCursorLeft();
				else if(mRightArrowDown && (mCursorIndex < static_cast<int>(mCaption.length())))
					moveCursorRight();
					
				mMoveCursorTimer = 0.0;
			}

			if( mDeleteTimer > 0.125 )
			{
				if(mDeleteDown && !mReadOnly) deleteCharacter();
				mDeleteTimer = 0.0;
			}
		}

		Widget::timeElapsed(time);
	}
}
