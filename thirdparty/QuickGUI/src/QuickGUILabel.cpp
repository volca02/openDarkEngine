#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUILabel.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	Label::Label(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mVerticalAlignment(VA_MID),
		mHorizontalAlignment(HA_MID),
		mTextBoundsPixelOffset(Point::ZERO),
		mTextBoundsRelativeSize(Size(1,1)),
		mTextColor(Ogre::ColourValue::White),
		mDisabledTextColor(Ogre::ColourValue(0.75,0.75,0.75,1)),
		mAutoSize(true),
		mHPixelPadWidth(6),
		mVPixelPadHeight(6)
	{
		mWidgetType = TYPE_LABEL;
		mSkinComponent = ".label";
		mSize = Size::ZERO;

		mText = new Text(mInstanceName+".Text",mQuadContainer,this);
		mText->setQuadLayer(mQuadLayer);	
	}

	Label::~Label()
	{
		delete mText;
	}

	void Label::alignText()
	{
		Rect textDimensions = mText->getDimensions();
		// 1 pixel buffer used
		Ogre::Real horizontalBuffer = 1.0 / mGUIManager->getViewportWidth();
		Ogre::Real verticalBuffer = 1.0 / mGUIManager->getViewportHeight();

		Rect textBounds = getTextBounds();

		// Horizontal alignment
		if( mHorizontalAlignment == HA_LEFT )
			mText->setPosition(Point(textBounds.x + horizontalBuffer,textDimensions.y));
		else if( mHorizontalAlignment == HA_MID )
			mText->setPosition(Point(textBounds.x + ((textBounds.width / 2.0) - (textDimensions.width / 2.0)),textDimensions.y));
		else if( mHorizontalAlignment == HA_RIGHT )
			mText->setPosition(Point(textBounds.x + textBounds.width - (horizontalBuffer + textDimensions.width),textDimensions.y));

		textDimensions = mText->getDimensions();

		// Vertical alignment
		if( mVerticalAlignment == VA_TOP )
			mText->setPosition(Point(textDimensions.x,textBounds.y + verticalBuffer));
		else if( mVerticalAlignment == VA_MID )
			mText->setPosition(Point(textDimensions.x,textBounds.y + ((textBounds.height / 2.0) - (textDimensions.height / 2.0))));
		else if( mVerticalAlignment == VA_BOTTOM )
			mText->setPosition(Point(textDimensions.x,textBounds.y + textBounds.height - (verticalBuffer + textDimensions.height)));
	}

	void Label::clearText()
	{
		mText->clearCaption();
	}

	void Label::disable()
	{
		mTextColor = mText->getColor();
		mText->setColor(mDisabledTextColor);

		Widget::disable();
	}

	void Label::enable()
	{
		mText->setColor(mTextColor);

		Widget::enable();
	}

	bool Label::getAutoSize()
	{
		return mAutoSize;
	}

	int Label::getHorizontalPixelPadWidth()
	{
		return mHPixelPadWidth;
	}

	Ogre::UTFString Label::getText()
	{
		return mText->getCaption();
	}

	Quad* Label::getTextCharacter(unsigned int index)
	{
		return mText->getCharacter(index);
	}

	Rect Label::getTextBounds()
	{
		return Rect(getScreenPosition() + getScrollOffset() + mTextBoundsPixelOffset,mTextBoundsRelativeSize * mSize);
	}

	int Label::getVerticalPixelPadHeight()
	{
		return mVPixelPadHeight;
	}

	void Label::hide()
	{
		Widget::hide();
		mText->hide();
	}

	void Label::onPositionChanged(const EventArgs& args)
	{
		Widget::onPositionChanged(args);

		mText->redraw();
	}

	void Label::onSizeChanged(const EventArgs& args)
	{
		Widget::onSizeChanged(args);

		mText->redraw();
	}

	void Label::setAutoSize(bool autoSize)
	{
		mAutoSize = autoSize;

		if(mAutoSize)
		{
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
			setWidth(mText->getTextWidth(mText->getCaption()) + mHPixelPadWidth);
		}
	}

	void Label::setVerticalAlignment(VerticalAlignment va)
	{
		mVerticalAlignment = va;

		if(mText->getDimensions() != Rect::ZERO)
			alignText();
	}

	void Label::setHorizontalAlignment(HorizontalAlignment ha)
	{
		mHorizontalAlignment = ha;

		if(mText->getDimensions() != Rect::ZERO)
			alignText();
	}

	void Label::setHorizontalPixelPadWidth(unsigned int width)
	{
		mHPixelPadWidth = width;
	}

	void Label::setClippingWidget(Widget* w, bool recursive)
	{
		Widget::setClippingWidget(w,recursive);
		mText->_clipToWidgetDimensions(w);
	}

	void Label::setGUIManager(GUIManager* gm)
	{
		Widget::setGUIManager(gm);
		mText->setGUIManager(mGUIManager);
	}

	void Label::setOffset(int offset)
	{
		Widget::setOffset(offset);
		mText->setOffset(mOffset+1);
	}

	void Label::setQuadContainer(QuadContainer* container)
	{
		Widget::setQuadContainer(container);
		mText->_notifyQuadContainer(mQuadContainer);
	}

	void Label::setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight)
	{
		if((pixelWidth == 0) && (pixelHeight == 0))
			return;

		mAutoSize = false;
		Widget::setSize(pixelWidth,pixelHeight);

		mText->redraw();
		alignText();
	}

	void Label::setSize(const Size& pixelSize)
	{
		Label::setSize(pixelSize.width,pixelSize.height);
	}

	void Label::redraw()
	{
		Widget::redraw();
		alignText();
	}

	void Label::setDisabledTextColor(const Ogre::ColourValue& c)
	{
		mDisabledTextColor = c;

		if(!mEnabled)
			mText->setColor(c);
	}

	void Label::setFont(const Ogre::String& fontScriptName, bool recursive)
	{
		if(fontScriptName == "")
			return;

		Widget::setFont(fontScriptName,recursive);
		mText->setFont(mFontName);

		if(mAutoSize)
		{
			setHeight(mText->getNewlineHeight() + mVPixelPadHeight);
			setWidth(mText->getTextWidth(mText->getCaption()) + mHPixelPadWidth);
			// setHeight sets mAutoSize to false..
			mAutoSize = true;
		}
	}

	void Label::setHeight(Ogre::Real pixelHeight)
	{
		mAutoSize = false;
		Widget::setHeight(pixelHeight);

		mText->redraw();
	}

	void Label::setQuadLayer(Quad::Layer l)
	{
		Widget::setQuadLayer(l);
		mText->setQuadLayer(l);
	}

	void Label::setText(const Ogre::UTFString& text)
	{
		if(mAutoSize)
		{
			mText->setCaption(text);
			setWidth(mText->getTextWidth(text) + mHPixelPadWidth);
			// setWidth sets mAutoSize to false..
			mAutoSize = true;
		}
		else
		{
			mText->setCaption(text);
		}
	}

	void Label::setTextBounds(const Point& pixelOffset, const Size& pixelSize)
	{
		mTextBoundsPixelOffset = pixelOffset;
		mTextBoundsRelativeSize = pixelSize / mSize;

		alignText();
	}

	void Label::setTextColor(Ogre::ColourValue color)
	{
		mTextColor = color;
		mText->setColor(mTextColor);
	}

	void Label::setVerticalPixelPadHeight(unsigned int height)
	{
		mVPixelPadHeight = height;
	}

	void Label::setWidth(Ogre::Real pixelWidth)
	{
		mAutoSize = false;
		Widget::setWidth(pixelWidth);

		mText->redraw();
	}

	void Label::show()
	{
		Widget::show();
		mText->show();
	}
}
