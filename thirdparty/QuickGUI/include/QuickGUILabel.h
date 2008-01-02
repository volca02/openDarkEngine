#ifndef QUICKGUILABEL_H
#define QUICKGUILABEL_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIText.h"
#include "QuickGUIWidget.h"

namespace QuickGUI
{
	/** Represents a traditional Label.
		@remarks
		Labels are QuickGUI's method to showing text.
		@note
		Labels must be created by the Window class.
	*/
	class _QuickGUIExport Label :
		public Widget
	{
	public:
		/**
		* Specifies the horizontal alignment of text
		*/
		enum HorizontalAlignment
		{
			HA_LEFT			=  0,
			HA_MID				,
			HA_RIGHT
		};
		/**
		* Specifies the vertical alignment of text
		*/
		enum VerticalAlignment
		{
			VA_TOP				=  0,
			VA_MID					,
			VA_BOTTOM
		};
	public:
		/** Constructor
            @param
                name The name to be given to the widget (must be unique).
            @param
                dimensions The x Position, y Position, width and height of the widget.
			@param
				positionMode The GuiMetricsMode for the values given for the position. (absolute/relative/pixel)
			@param
				sizeMode The GuiMetricsMode for the values given for the size. (absolute/relative/pixel)
			@param
				textureName The name of the texture used to visually represent the widget. (ie "qgui.window.png")
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		Label(const Ogre::String& name, GUIManager* gm);
		
		/**
		* Aligns the child Label widget horizontally and vertically
		*/
		virtual void alignText();
		virtual void clearText();
		/**
		* Disable Widget, making it unresponsive to events.
		*/
		virtual void disable();
		/**
		* Enable Widget, allowing it to accept and handle events.
		*/
		virtual void enable();
		bool getAutoSize();
		int getHorizontalPixelPadWidth();
		/**
		* Convenience method to return the Text object's caption.
		*/
		virtual Ogre::UTFString getText();
		Quad* getTextCharacter(unsigned int index);
		/*
		* Returns the dimensions of the area used for text aligning and displaying.
		*/
		Rect getTextBounds();
		int getVerticalPixelPadHeight();
		/*
		* Hides the widget, including text.
		*/
		void hide();
		/**
		* Force updating of the Widget's Quad position on screen.
		*/
		void redraw();
		/**
		* Set true if the label's size should match it's font height and text width.
		* NOTE: AutoSize is set to true by default.  If you set this to false, you may
		*  end up with empty label's, as text that doesn't fit in the label won't be rendered.
		*/
		virtual void setAutoSize(bool autoSize);
		/**
		* Sets the color of the text when the widget is disabled.
		*/
		virtual void setDisabledTextColor(const Ogre::ColourValue& c);
		virtual void setFont(const Ogre::String& fontScriptName, bool recursive = false);
		virtual void setHeight(Ogre::Real pixelHeight);
		virtual void setQuadLayer(Quad::Layer l);
		virtual void setText(const Ogre::UTFString& text);
		/**
		* Sets text vertical alignment.
		*/
		virtual void setVerticalAlignment(VerticalAlignment va);
		/**
		* Sets text horizontal alignment.
		*/
		virtual void setHorizontalAlignment(HorizontalAlignment ha);
		void setHorizontalPixelPadWidth(unsigned int width);
		/**
		* Manipulates the offset used to determine this widgets zOrder in rendering.
		*/
		void setOffset(int offset);
		/**
		* Manually set size of widget.
		*/
		virtual void setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight);
		virtual void setSize(const Size& pixelSize);
		/**
		* Sets the dimensions of the area used for text aligning and displaying.
		*/
		void setTextBounds(const Point& relativePixelOffset, const Size& relativePixelSize);
		virtual void setTextColor(Ogre::ColourValue color);
		void setVerticalPixelPadHeight(unsigned int height);
		virtual void setWidth(Ogre::Real pixelWidth);
		/*
		* Shows the widget, including text.
		*/
		void show();

	protected:
		virtual ~Label();
		virtual void onPositionChanged(const EventArgs& args);
		virtual void onSizeChanged(const EventArgs& args);
		virtual void setClippingWidget(Widget* w, bool recursive = false);
		virtual void setGUIManager(GUIManager* gm);
		virtual void setQuadContainer(QuadContainer* container);
	protected:

		Text* mText;
		VerticalAlignment mVerticalAlignment;
		HorizontalAlignment	mHorizontalAlignment;

		int mHPixelPadWidth;
		int mVPixelPadHeight;

		Point mTextBoundsPixelOffset;
		Size mTextBoundsRelativeSize;

		Ogre::ColourValue mTextColor;
		Ogre::ColourValue mDisabledTextColor;

		bool mAutoSize;
	};
}

#endif
