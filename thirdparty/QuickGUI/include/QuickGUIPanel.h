#ifndef QUICKGUIPANEL_H
#define QUICKGUIPANEL_H

#include "OgreStringConverter.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIBorder.h"
#include "QuickGUIButton.h"
#include "QuickGUICheckBox.h"
#include "QuickGUIComboBox.h"
#include "QuickGUIConsole.h"
#include "QuickGUIImage.h"
#include "QuickGUILabelArea.h"
#include "QuickGUINStateButton.h"
#include "QuickGUIProgressBar.h"
#include "QuickGUIScrollPane.h"
#include "QuickGUIText.h"
#include "QuickGUITextBox.h"
#include "QuickGUITree.h"
#include "QuickGUIHorizontalTrackBar.h"
#include "QuickGUIVerticalTrackBar.h"
#include "QuickGUIWidget.h"
#include "QuickGUIQuadContainer.h"

namespace QuickGUI
{
	/** Represents a Widget Container.
	@remarks
	The Panel class has the ability to create the majority of defined Widgets.
	The Sheet and Window Widgets derive from this widget (Panel), giving them the
	same abilities.
	@note
	Panels cannot create the TitleBar, Window, or Sheet widget.
	@note
	Panels are meant to be created via the Window and Sheet widget.
	*/
	class _QuickGUIExport Panel :
		public Widget,
		public QuadContainer
	{
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
				parentWidget parent widget which created this widget.
        */
		Panel(const Ogre::String& name, GUIManager* gm);		

		virtual void addChild(Widget* w);
		virtual void allowScrolling(bool allow);


		Button* createButton();
		Button* createButton(const Ogre::String& name);

		CheckBox* createCheckBox();
		CheckBox* createCheckBox(const Ogre::String& name);

		ComboBox* createComboBox();
		ComboBox* createComboBox(const Ogre::String& name);

		Console* createConsole();
		Console* createConsole(const Ogre::String& name);

		HorizontalScrollBar* createHorizontalScrollBar();
		HorizontalScrollBar* createHorizontalScrollBar(const Ogre::String& name);

		Image* createImage();
		Image* createImage(const Ogre::String& name);

		Label* createLabel();
		Label* createLabel(const Ogre::String& name);

		List* createList();
		List* createList(const Ogre::String& name);

		LabelArea* createMultiLineLabel();
		LabelArea* createMultiLineLabel(const Ogre::String& name);

		NStateButton* createNStateButton();
		NStateButton* createNStateButton(const Ogre::String& name);

		Panel* createPanel();
		Panel* createPanel(const Ogre::String& name);

		ProgressBar* createProgressBar();
		ProgressBar* createProgressBar(const Ogre::String& name);

		TextBox* createTextBox();
		TextBox* createTextBox(const Ogre::String& name);

		Tree* createTree();
		Tree* createTree(const Ogre::String& name);

		HorizontalTrackBar* createHorizontalTrackBar();
		HorizontalTrackBar* createHorizontalTrackBar(const Ogre::String& name);

		VerticalScrollBar* createVerticalScrollBar();
		VerticalScrollBar* createVerticalScrollBar(const Ogre::String& name);

		VerticalTrackBar* createVerticalTrackBar();
		VerticalTrackBar* createVerticalTrackBar(const Ogre::String& name);

		ScrollPane* getScrollPane();
		virtual Widget* getTargetWidget(const Point& pixelPosition);

		bool scrollingAllowed();
		virtual void show();

	protected:
		virtual ~Panel();
		virtual void setQuadContainer(QuadContainer* container);
		virtual Widget*	_createComponent(const Ogre::String& name, Type t);
	protected:
		ScrollPane* mScrollPane;
		bool mScrollingAllowed;

		VerticalScrollBar* mRightScrollBar;
		HorizontalScrollBar* mBottomScrollBar;

		void onChildAdded(const EventArgs& args);
		void onChildRemoved(const EventArgs& args);
		void onSizeChanged(const EventArgs& args);
	};
}

#endif
