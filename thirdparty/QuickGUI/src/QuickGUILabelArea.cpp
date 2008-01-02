#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUILabelArea.h"

namespace QuickGUI
{
	LabelArea::LabelArea(const Ogre::String& name, GUIManager* gm) :
		Label(name,gm),
		mTextOffset(0.0),
		mCaption("")
	{
		mWidgetType = TYPE_LABELAREA;
		mSkinComponent = ".labelarea";
		mSize = Size(200,100);
		mAutoSize = false;

		mTextList = dynamic_cast<List*>(_createComponent(mInstanceName+".List",TYPE_LIST));
		mTextList->setSize(mSize);
		mTextList->setHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT_RIGHT);
		mTextList->setVerticalAnchor(ANCHOR_VERTICAL_TOP_BOTTOM);

		allowScrolling(true);

		mTextHelper = new TextHelper();
	}

	LabelArea::~LabelArea()
	{
		delete mTextHelper;
	}

	int LabelArea::_getLine(int startIndex)
	{
		int end = startIndex + 1;
		
		Ogre::Real width = 0;
		while( (end < static_cast<int>(mCaption.length())) && (width <= (mSize.width - mTextOffset)))
		{
			width += mTextHelper->getGlyphWidth(mCaption[end]);
			++end;
		}

		// The rest of the caption is less than the width of the Widget.
		if( end == static_cast<int>(mCaption.length()) )
			return end;
		// Remaining portion of caption is larger than the width of the widget.
		else
		{
			// end was incremented until larger than text width, decrement to ensure its less than text width.
			--end;

			int space = end;
			// Iterate backwards until we find a space.
			while( (space > 0) && (!mTextHelper->isSpace(mCaption[space])) )
				--space;
			
			// Gaurd against one long word
			if(space == (startIndex - 1))
				return end;
			else
				return space;
		}
	}

	void LabelArea::alignText()
	{
	}

	void LabelArea::allowScrolling(bool allow)
	{
		mScrollingAllowed = allow;
		mTextList->allowScrolling(mScrollingAllowed);

		if(allow)
			mTextOffset = mTextList->mRightScrollBar->getWidth();
		else
			mTextOffset = 0.0;
	}

	void LabelArea::clearText()
	{
		setText("");
	}

	void LabelArea::disable()
	{
		mTextColor = mText->getColor();

		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setTextColor(mDisabledTextColor);
		}

		Widget::disable();
	}

	void LabelArea::enable()
	{
		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setTextColor(mTextColor);
		}

		Widget::enable();
	}

	Ogre::UTFString LabelArea::getText()
	{
		return mCaption;
	}

	bool LabelArea::scrollingAllowed()
	{
		return mScrollingAllowed;
	}

	void LabelArea::setDisabledTextColor(const Ogre::ColourValue& c)
	{
		mDisabledTextColor = c;

		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setDisabledTextColor(mDisabledTextColor);
		}

		if(!mEnabled)
			disable();
	}

	void LabelArea::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		mTextHelper->setFont(fontScriptName);
		Widget::setFont(fontScriptName,recursive);
		setText(mCaption);
	}

	void LabelArea::setHorizontalAlignment(HorizontalAlignment ha)
	{
		mHorizontalAlignment = ha;

		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setHorizontalAlignment(mHorizontalAlignment);
		}
	}

	void LabelArea::setText(const Ogre::UTFString& text)
	{
		mCaption = text;

		// Caption Iterator
		int textIndex = 0;
		// Index denoting the end of a line that can fix within the widgets width.
		int lineEnd = 0;
		// The index of the text box to use, or create and use.
		int textBoxIndex = 0;

		while( textIndex < static_cast<int>(mCaption.size()) )
		{
			lineEnd = _getLine(textIndex);

			TextBox* tb;
			if( textBoxIndex >= mTextList->getNumberOfItems() )
			{
				tb = mTextList->addTextBox();
			}
			else
				tb = dynamic_cast<TextBox*>(mTextList->getItem(textBoxIndex));

			tb->setReadOnly(true);
			tb->setUseBorders(false);
			tb->setText(mCaption.substr(textIndex,lineEnd - textIndex));
			++textBoxIndex;

			textIndex = lineEnd + 1;
		}

		// destroy remaining unused textboxes.
		int numExtraTextBoxes = mTextList->getNumberOfItems() - textBoxIndex;
		while( numExtraTextBoxes > 0 )
		{
			mTextList->removeItem(textBoxIndex);
			--numExtraTextBoxes;
		}
	}

	void LabelArea::setTextColor(Ogre::ColourValue color)
	{
		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setTextColor(color);
		}
	}

	void LabelArea::setVerticalAlignment(VerticalAlignment va)
	{
		mVerticalAlignment = va;

		for( int i = 0; i < mTextList->getNumberOfItems(); ++i )
		{
			dynamic_cast<TextBox*>(mTextList->getItem(i))->setVerticalAlignment(mVerticalAlignment);
		}
	}
}
