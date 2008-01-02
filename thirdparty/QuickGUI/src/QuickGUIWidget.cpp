#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIWidget.h"
#include "QuickGUIManager.h"
#include "QuickGUIPanel.h"
#include "QuickGUIWindow.h"
#include "QuickGUISheet.h"
#include "QuickGUIBorder.h"
#include "QuickGUITextArea.h" 

namespace QuickGUI
{
	Widget::Widget(const Ogre::String& name, GUIManager* gm) :
		mCanResize(false),
		mClippingWidget(0),
		mDragXOnly(false),
		mDragYOnly(false),
		mDraggingEnabled(false),
		mEnabled(true),
		mEntered(false),
		mGainFocusOnClick(true),
		mGrabbed(false),
		mGUIManager(gm),
		mHideSkin(false),
		mHideWithParent(true),
		mHorizontalAnchor(ANCHOR_HORIZONTAL_LEFT),
		mInheritClippingWidget(true),
		mInheritOpacity(true),
		mInheritQuadLayer(true),
		mInstanceName(name),
		mMovingEnabled(true),
		mOffset(0),
		mOpacity(1),
		mPosition(Point(0,0)),
		mParentWidget(0),
		mQuadContainer(0),
		mQuadLayer(Quad::LAYER_CHILD),
		mScrollOffset(Point::ZERO),
		mScrollPaneAccessible(true),
		mShowWithParent(true),
		mSkinComponent(""),
		mSkinName(""),
		mTextureLocked(false),
		mUseBorders(false),
		mVerticalAnchor(ANCHOR_VERTICAL_TOP),
		mVisible(true),
		mWidgetImage(NULL)
	{
		_initEventHandlers();
		mQuad = _createQuad();
		mWidgetToDrag = this;

		// Add event handlers before call to setDimensions, if you want the widget handlers to be called.
		// This is important for widgets to position themself correctly.
		addEventHandler(EVENT_POSITION_CHANGED,&Widget::onPositionChanged,this);
		addEventHandler(EVENT_SIZE_CHANGED,&Widget::onSizeChanged,this);

		setPropagateEventFiring(EVENT_MOUSE_ENTER,true);
		setPropagateEventFiring(EVENT_MOUSE_LEAVE,true);
	}

	Widget::~Widget()
	{
		if(mParentWidget != NULL)
			mParentWidget->removeChild(this);

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
			delete (*it);
		mComponents.clear();

		// Safest route is to destroy children first thing.
		removeAndDestroyAllChildWidgets();

		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
			delete (*it);
		mQuads.clear();

		delete mWidgetImage;

		// Cleanup Event Handlers.
		int index = 0;
		while( index < (EVENT_END_OF_LIST) )
		{
			for(EventHandlerArray::iterator it = mUserEventHandlers[index].begin(); it != mUserEventHandlers[index].end(); ++it)
				delete (*it);
			mUserEventHandlers[index].clear();

			++index;
		}

		// Remove name from GUIManager name list. (So another widget can be created with this name, if desired)
		mGUIManager->notifyNameFree(mInstanceName);
	}

	void Widget::_applyAnchors()
	{
		if(mParentWidget == NULL)
			return;

		Size parentSize = mParentWidget->getSize();

		// Vertical anchor:
		switch(mVerticalAnchor)
		{
		case ANCHOR_VERTICAL_TOP_BOTTOM:
			setHeight(parentSize.height - mPixelsFromParentBottom - mPosition.y);
			break;
		case ANCHOR_VERTICAL_TOP:
			break;
		case ANCHOR_VERTICAL_BOTTOM:
			mPosition.y = parentSize.height - mPixelsFromParentBottom - mSize.height;
			break;
		case ANCHOR_VERTICAL_NONE:
			break;
		}

		// Horizontal anchor:
		switch(mHorizontalAnchor)
		{
		case ANCHOR_HORIZONTAL_LEFT_RIGHT:
			setWidth(parentSize.width - mPixelsFromParentRight - mPosition.x);
			break;
		case ANCHOR_HORIZONTAL_LEFT:
			break;
		case ANCHOR_HORIZONTAL_RIGHT:
			mPosition.x = parentSize.width - mPixelsFromParentRight - mSize.width;
			break;
		case ANCHOR_HORIZONTAL_NONE:
			break;
		}

		redraw();
	}

	void Widget::_createBorders()
	{
		// By default, borders have a 5 pixel thickness
		Ogre::Real overlap = 1;
		Ogre::Real thickness = 5;

		Border* b = dynamic_cast<Border*>(_createComponent(mInstanceName+".TopLeftBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_TOP_LEFT);
	
		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".TopRightBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_TOP_RIGHT);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".BottomLeftBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_BOTTOM_LEFT);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".BottomRightBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_BOTTOM_RIGHT);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".LeftBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_LEFT);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".TopBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_TOP);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".RightBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_RIGHT);

		b = dynamic_cast<Border*>(_createComponent(mInstanceName+".BottomBorder",TYPE_BORDER));
		b->setBorderType(Border::BORDER_TYPE_BOTTOM);
	}

	Widget* Widget::_createChild(const Ogre::String& name, Type t)
	{
		Widget* w;
		switch(t)
		{
			case TYPE_BORDER:				w = new Border(name,mGUIManager);				break;
			case TYPE_BUTTON:				w = new Button(name,mGUIManager);				break;
			case TYPE_CHECKBOX:				w = new CheckBox(name,mGUIManager);				break;
			case TYPE_COMBOBOX:				w = new ComboBox(name,mGUIManager);				break;
			case TYPE_CONSOLE:				w = new Console(name,mGUIManager);				break;
			case TYPE_IMAGE:				w = new Image(name,mGUIManager);				break;
			case TYPE_LABEL:				w = new Label(name,mGUIManager);				break;
			case TYPE_LABELAREA:			w = new LabelArea(name,mGUIManager);			break;
			case TYPE_LIST:					w = new List(name,mGUIManager);					break;
			case TYPE_MENULABEL:			w = new MenuLabel(name,mGUIManager);			break;
			case TYPE_NSTATEBUTTON:			w = new NStateButton(name,mGUIManager);			break;
			case TYPE_PANEL:				w = new Panel(name,mGUIManager);				break;
			case TYPE_PROGRESSBAR:			w = new ProgressBar(name,mGUIManager);			break;
			case TYPE_SCROLL_PANE:			w = new ScrollPane(name,mGUIManager);			break;
			case TYPE_SCROLLBAR_HORIZONTAL: w = new HorizontalScrollBar(name,mGUIManager);	break;
			case TYPE_SCROLLBAR_VERTICAL:	w = new VerticalScrollBar(name,mGUIManager);	break;
			case TYPE_TEXTAREA:				w = new TextArea(name,mGUIManager);				break;
			case TYPE_TEXTBOX:				w = new TextBox(name,mGUIManager);				break;
			case TYPE_TITLEBAR:				w = new TitleBar(name,mGUIManager);				break;
			case TYPE_TRACKBAR_HORIZONTAL:	w = new HorizontalTrackBar(name,mGUIManager);	break;
			case TYPE_TRACKBAR_VERTICAL:	w = new VerticalTrackBar(name,mGUIManager);		break;
			case TYPE_TREE:					w = new Tree(name,mGUIManager);					break;
			case TYPE_WINDOW:				w = new Window(name,mGUIManager);				break;
			default:						w = new Widget(name,mGUIManager);				break;
		}

		w->setSize(w->getSize());
		if(mSkinName != "")
			w->setSkin(mSkinName,true);
		w->setFont(mFontName,true);
		addChild(w);
		w->setPosition(0,0);

		WidgetEventArgs args(w);
		fireEvent(EVENT_CHILD_CREATED,args);

		if(!mVisible && w->getHideWithParent())
			w->hide();

		return w;
	}

	Widget* Widget::_createComponent(const Ogre::String& name, Type t)
	{
		Widget* w;
		switch(t)
		{
			case TYPE_BORDER:				w = new Border(name,mGUIManager);				break;
			case TYPE_BUTTON:				w = new Button(name,mGUIManager);				break;
			case TYPE_CHECKBOX:				w = new CheckBox(name,mGUIManager);				break;
			case TYPE_COMBOBOX:				w = new ComboBox(name,mGUIManager);				break;
			case TYPE_CONSOLE:				w = new Console(name,mGUIManager);				break;
			case TYPE_IMAGE:				w = new Image(name,mGUIManager);				break;
			case TYPE_LABEL:				w = new Label(name,mGUIManager);				break;
			case TYPE_LABELAREA:			w = new LabelArea(name,mGUIManager);			break;
			case TYPE_LIST:					w = new List(name,mGUIManager);					break;
			case TYPE_MENULABEL:			w = new MenuLabel(name,mGUIManager);			break;
			case TYPE_NSTATEBUTTON:			w = new NStateButton(name,mGUIManager);			break;
			case TYPE_PANEL:				w = new Panel(name,mGUIManager);				break;
			case TYPE_PROGRESSBAR:			w = new ProgressBar(name,mGUIManager);			break;
			case TYPE_SCROLL_PANE:			w = new ScrollPane(name,mGUIManager);			break;
			case TYPE_SCROLLBAR_HORIZONTAL: w = new HorizontalScrollBar(name,mGUIManager);	break;
			case TYPE_SCROLLBAR_VERTICAL:	w = new VerticalScrollBar(name,mGUIManager);	break;
			case TYPE_TEXTAREA:				w = new TextArea(name,mGUIManager);				break;
			case TYPE_TEXTBOX:				w = new TextBox(name,mGUIManager);				break;
			case TYPE_TITLEBAR:				w = new TitleBar(name,mGUIManager);				break;
			case TYPE_TRACKBAR_HORIZONTAL:	w = new HorizontalTrackBar(name,mGUIManager);	break;
			case TYPE_TRACKBAR_VERTICAL:	w = new VerticalTrackBar(name,mGUIManager);		break;
			case TYPE_WINDOW:				w = new Window(name,mGUIManager);				break;
			default:						w = new Widget(name,mGUIManager);				break;
		}

		w->setSize(w->getSize());
		// Some Composition widgets will create components before inheritting skin name.
		if(mSkinName != "")
			w->setSkin(mSkinName,true);
		w->setFont(mFontName,true);
		mComponents.push_back(w);
		//addChild(w);
		w->setParent(this);
		w->setPosition(0,0);

		if(!mVisible && w->getHideWithParent())
			w->hide();

		return w;
	}

	void Widget::_deriveAnchorValues()
	{
		if(mParentWidget == NULL)
			return;

		Size parentSize = mParentWidget->getSize();
		mPixelsFromParentRight = parentSize.width - (mPosition.x + mSize.width);
		mPixelsFromParentBottom = parentSize.height - (mPosition.y + mSize.height);
	}

	void Widget::_destroyBorders()
	{
		int index = 0;
		while( index < static_cast<int>(mComponents.size()) )
		{
			if(mComponents[index]->getWidgetType() == TYPE_BORDER)
			{
				Widget* w = mComponents[index];
				mComponents.erase(std::find(mComponents.begin(),mComponents.end(),mComponents[index]));
				mGUIManager->_destroyWidget(w);
			}

			++index;
		}
	}

	Quad* Widget::_createQuad()
	{
		Quad* newQuad = new Quad(this);
		mQuads.push_back(newQuad);
		return newQuad;
	}

	void Widget::_initEventHandlers()
	{
		int index = 0;
		// 22 common types of events currently
		while( index < (EVENT_END_OF_LIST) )
		{
			mUserEventHandlers[index].clear();
			mPropogateEventFiring[index] = false;
			++index;
		}
	}

	void Widget::_setScrollXOffset(Ogre::Real pixelXOffset)
	{
		mScrollOffset.x = pixelXOffset;
		mQuad->setXPosition(getScreenPosition().x + getScrollOffset().x);
		redraw();
	}

	void Widget::_setScrollYOffset(Ogre::Real pixelYOffset)
	{
		mScrollOffset.y = pixelYOffset;
		mQuad->setYPosition(getScreenPosition().y + getScrollOffset().y);
		redraw();
	}

	void Widget::addChild(Widget* w)
	{
		if(w->getParentWidget() != NULL)
			return;

		mChildWidgets.push_back(w);

		w->setParent(this);

		// Convert Widget's position to be relative to new parent.
		w->setPosition(w->getScreenPosition() - getScreenPosition());

		WidgetEventArgs args(w);
		fireEvent(EVENT_CHILD_ADDED,args);
	}

	void Widget::addEventHandler(Event EVENT, MemberFunctionSlot* function)
	{
		mUserEventHandlers[EVENT].push_back(function);
	}

	void Widget::allowResizing(bool allow)
	{
		mCanResize = allow;
	}

	void Widget::appearOverWidget(Widget* w)
	{
		if( (w->getQuadContainer() == NULL) || (mQuadContainer == NULL) )
			return;

		if( (w->getQuadContainer() != mQuadContainer) ||
			(w->getQuad()->getLayer() != mQuad->getLayer()) )
			return;

		setOffset(w->getHighestOffset() + 1);
	}

	void Widget::constrainDragging(bool DragXOnly, bool DragYOnly)
	{
		mDragXOnly = false;
		mDragYOnly = false;

		if(DragXOnly && DragYOnly)
			return;

		mDragXOnly = DragXOnly;
		mDragYOnly = DragYOnly;
	}

	void Widget::disable()
	{
		if((mWidgetType == Widget::TYPE_SHEET)) 
			return;

		//setTexture(mDisabledTextureName,false);

		WidgetEventArgs args(this);
		fireEvent(EVENT_DISABLED,args);

		mEnabled = false;
	}

	void Widget::drag(const Ogre::Real& pixelX, const Ogre::Real& pixelY)
	{
		if(!mDraggingEnabled) 
			return;

		MouseEventArgs args(this);
		args.position = mGUIManager->getMouseCursor()->getPosition();
		args.moveDelta.x = pixelX;
		args.moveDelta.y = pixelY;
		
		if(mWidgetToDrag != NULL) 
		{
			if(mDragXOnly)
				mWidgetToDrag->moveX(pixelX);
			else if(mDragYOnly)
				mWidgetToDrag->moveY(pixelY);
			else
				mWidgetToDrag->move(pixelX,pixelY);
		}

		// fire onDragged Event.		
		fireEvent(EVENT_DRAGGED,args);
	}

	bool Widget::draggingEnabled()
	{
		return mDraggingEnabled;
	}

	void Widget::enable()
	{
		if(mWidgetType == Widget::TYPE_SHEET) 
			return;

		mEnabled = true;

		//setTexture(mFullTextureName,false);

		WidgetEventArgs args(this);
		fireEvent(EVENT_ENABLED,args);
	}

	bool Widget::enabled()
	{
		return mEnabled;
	}

	void Widget::enableDragging(bool enable)
	{
		mDraggingEnabled = enable;
	}

	void Widget::focus()
	{
		mGUIManager->setActiveWidget(this);
	}

	Ogre::Real Widget::getActualOpacity()
	{
		if((mParentWidget == NULL) || (!mInheritOpacity))
			return mOpacity;

		return mParentWidget->getActualOpacity() * mOpacity;
	}

	Point Widget::getActualPosition()
	{
		return getScreenPosition() + getScrollOffset();
	}

	WidgetArray* Widget::getChildWidgetList()
	{
		return &mChildWidgets;
	}

	Widget* Widget::getChildWidget(const Ogre::String& name)
	{
		if( name.empty() ) 
			return NULL;
		if( mInstanceName == name ) 
			return this;

		Widget* w = NULL;
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( (w = (*it)->getChildWidget(name)) != NULL )
			{
				return w;
			}
		}

		return NULL;
	}

	Widget* Widget::getChildWidget(Type t, unsigned int index)
	{
		int count = -1;
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if((*it)->getWidgetType() == t)
				++count;

			if(count == index)
				return (*it);
		}

		return NULL;
	}

	bool Widget::getGainFocusOnClick()
	{
		return mGainFocusOnClick;
	}

	bool Widget::getGrabbed()
	{
		return mGrabbed;
	}

	Ogre::Real Widget::getHeight()
	{
		return mSize.height;
	}

	bool Widget::getHideWithParent()
	{
		return mHideWithParent;
	}

	int Widget::getHighestOffset()
	{
		// iterate through child widgets..
		Widget* w = NULL;
		// Get the widget with the highest offset
		int widgetOffset = mOffset;
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( (*it)->getWidgetType() == TYPE_SCROLL_PANE )
				continue;

			int temp = (*it)->getHighestOffset();
			if( temp > widgetOffset )
				widgetOffset = temp;
		}

		return widgetOffset;
	}

	Widget::HorizontalAnchor Widget::getHorizontalAnchor()
	{
		return mHorizontalAnchor;
	}

	bool Widget::getInheritClippingWidget()
	{
		return mInheritClippingWidget;
	}

	bool Widget::getInheritQuadLayer()
	{
		return mInheritQuadLayer;
	}

	Ogre::String Widget::getInstanceName()
	{
		return mInstanceName;
	}

	bool Widget::getMovingEnabled()
	{
		return mMovingEnabled;
	}

	int Widget::getOffset()
	{
		return mOffset;
	}

	Panel* Widget::getParentPanel()
	{
		Widget* w = mParentWidget;
		while((w != NULL) && (w->getWidgetType() != TYPE_PANEL))
			w = w->getParentWidget();

		return dynamic_cast<Panel*>(w);
	}

	Sheet* Widget::getParentSheet()
	{
		Widget* w = mParentWidget;
		while((w != NULL) && (w->getWidgetType() != TYPE_SHEET))
			w = w->getParentWidget();

		return dynamic_cast<Sheet*>(w);
	}

	Widget* Widget::getParentWidget()
	{
		return mParentWidget;
	}

	Window* Widget::getParentWindow()
	{
		Widget* w = mParentWidget;
		while((w != NULL) && (w->getWidgetType() != TYPE_WINDOW))
			w = w->getParentWidget();

		return dynamic_cast<Window*>(w);
	}

	bool Widget::getPropogateEventFiring(Event e)
	{
		return mPropogateEventFiring[e];
	}

	Quad* Widget::getQuad()
	{
		return mQuad;
	}

	QuadContainer* Widget::getQuadContainer()
	{
		return mQuadContainer;
	}

	Quad::Layer Widget::getQuadLayer()
	{
		return mQuadLayer;
	}

	Point Widget::getScreenPosition()
	{
		if(mParentWidget == NULL)
			return Point::ZERO;
		
		return mParentWidget->getScreenPosition() + mPosition;
	}

	bool Widget::getScrollPaneAccessible()
	{
		return mScrollPaneAccessible;
	}

	Ogre::String Widget::getSkinComponent()
	{
		return mSkinComponent;
	}

	bool Widget::getShowWithParent()
	{
		return mShowWithParent;
	}

	Ogre::String Widget::getSkin()
	{
		return mSkinName;
	}

	Rect Widget::getDimensions()
	{
		return Rect(mPosition,mSize);
	}

	Ogre::String Widget::getFontName()
	{
		return mFontName;
	}

	GUIManager* Widget::getGUIManager()
	{
		return mGUIManager;
	}

	bool Widget::getInheritOpacity()
	{
		return mInheritOpacity;
	}

	int Widget::getNumberOfHandlers(Event e)
	{
		return static_cast<int>(mUserEventHandlers[e].size());
	}

	Ogre::Real Widget::getOpacity()
	{
		return mOpacity;
	}

	Point Widget::getPosition()
	{
		return mPosition;
	}

	Point Widget::getScrollOffset()
	{
		if( mParentWidget == NULL )
			return mScrollOffset;

		return mParentWidget->getScrollOffset() + mScrollOffset;
	}

	Size Widget::getSize()
	{
		return mSize;
	}

	Widget* Widget::getTargetWidget(const Point& pixelPosition)
	{
		if( !mQuad->visible() || !mEnabled )
			return NULL;

		Widget* w = NULL;

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
		{
			if((w = (*it)->getTargetWidget(pixelPosition)) != NULL)
				return w;
		}

		// If position is not inside this widget, it can't be inside a child widget. (except menus, which are handled differently)
		if( !isPointWithinBounds(pixelPosition) ) 
			return NULL;		

		// Iterate through Menu Layer Child Widgets.
		int widgetOffset = 0;
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( (*it)->getQuadLayer() == Quad::LAYER_CHILD )
				continue;

			Widget* temp = (*it)->getTargetWidget(pixelPosition);
			if( (temp != NULL) && (temp->getOffset() > widgetOffset) )
			{
				widgetOffset = temp->getOffset();
				w = temp;
			}
		}
		if(w != NULL)
			return w;

		// Iterate through Child Layer Child Widgets.
		widgetOffset = 0;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( (*it)->getQuadLayer() == Quad::LAYER_MENU )
				continue;

			Widget* temp = (*it)->getTargetWidget(pixelPosition);
			if( (temp != NULL) && (temp->getOffset() > widgetOffset) )
			{
				widgetOffset = temp->getOffset();
				w = temp;
			}
		}
		if(w != NULL)
			return w;

		return this;
	}

	Widget::Type Widget::getWidgetType()
	{
		return mWidgetType;
	}

	Widget::VerticalAnchor Widget::getVerticalAnchor()
	{
		return mVerticalAnchor;
	}

	Ogre::Real Widget::getWidth()
	{
		return mSize.width;
	}

	Ogre::Real Widget::getXPosition()
	{
		return mPosition.x;
	}

	Ogre::Real Widget::getYPosition()
	{
		return mPosition.y;
	}

	void Widget::hide()
	{
		mGrabbed = false;

		bool currentlyVisible = mVisible;
		mVisible = false;

		for( QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it )
			(*it)->setVisible(false);

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
		{
			if((*it)->getHideWithParent())
				(*it)->hide();
		}

		// hide children
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if((*it)->getHideWithParent())
				(*it)->hide();
		}

		// if widget is the active widget at time of being hidden, set sheet to active widget.
		if(mGUIManager->getActiveWidget() == this)
			mGUIManager->setActiveWidget(mGUIManager->getActiveSheet());
		// if mouse cursor is over widget at time of being hidden, tell GUIManager to find the next widget mouse is over
		if(mGUIManager->getMouseOverWidget() == this)
			mGUIManager->injectMouseMove(0,0);

		// Only fire event if we change visibility.  If we were already hidden, don't fire.
		if(currentlyVisible)
		{
			WidgetEventArgs args(this);
			fireEvent(EVENT_HIDDEN,args);
		}
	}

	void Widget::hideSkin()
	{
		mHideSkin = true;

		mQuad->setMaterial("");

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
			hideSkin();
	}

	bool Widget::isPointWithinBounds(const Point& pixelPosition)
	{
		if(!mQuad->visible())
			return false;

		if(!mQuad->isPointWithinBounds(pixelPosition))
			return false;

		if(overTransparentPixel(pixelPosition))
			return false;

		return true;
	}

	bool Widget::isVisible()
	{
		return mVisible;
	}

	bool Widget::isChild(Widget* w)
	{
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( w == (*it) || (*it)->isChild(w) )
				return true;
		}

		return false;
	}

	void Widget::move(const Ogre::Real& pixelX, const Ogre::Real& pixelY)
	{
		if(!mMovingEnabled) 
			return;

		setPosition(mPosition.x + pixelX,mPosition.y + pixelY);
	}

	void Widget::move(const Point& pixelOffset)
	{
		move(pixelOffset.x,pixelOffset.y);
	}

	void Widget::moveX(Ogre::Real pixelX)
	{
		if(!mMovingEnabled) 
			return;

		setXPosition(mPosition.x + pixelX);
	}

	void Widget::moveY(Ogre::Real pixelY)
	{
		if(!mMovingEnabled) 
			return;

		setYPosition(mPosition.y + pixelY);
	}

	bool Widget::fireEvent(Event e, const EventArgs& args)
	{
		if(!mEnabled || (mUserEventHandlers[e].empty() && !mPropogateEventFiring[e])) 
			return false;

		if(e == EVENT_MOUSE_ENTER)
		{
			if(mEntered)
				return false;
			else
				mEntered = true;
		}
		else if(e == EVENT_MOUSE_LEAVE)
		{
			if(!mEntered)
				return false;
			else
				mEntered = false;
		}

		EventHandlerArray::iterator it;
		EventHandlerArray* userEventHandlers = &(mUserEventHandlers[e]);
		for( it = userEventHandlers->begin(); it != userEventHandlers->end(); ++it )
			(*it)->execute(args);

		if(mPropogateEventFiring[e] && (mParentWidget != NULL))
			mParentWidget->fireEvent(e,args);

		return true; 
	}

	void Widget::lockTexture()
	{
		mTextureLocked = true;
	}

	void Widget::onPositionChanged(const EventArgs& args)
	{
		// maintain child widget positions
		WidgetArray::iterator it;

		for( it = mComponents.begin(); it != mComponents.end(); ++it )
			(*it)->redraw();

		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->redraw();
	}

	void Widget::onSizeChanged(const EventArgs& args)
	{
		WidgetArray::iterator it;
		
		for( it = mComponents.begin(); it != mComponents.end(); ++it )
			(*it)->_applyAnchors();

		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->_applyAnchors();
	}

	bool Widget::overTransparentPixel(const Point& mousePixelPosition)
	{
		if( mWidgetImage == NULL ) 
			return false;

		Point pt = mousePixelPosition - getScreenPosition() - getScrollOffset();

		if (pt.x <= 0 || pt.y <= 0)
		{
			// something is wrong here
			Ogre::LogManager::getSingletonPtr()->logMessage("Quickgui : error in Widget::overTransparentPixel getting correct Mouse to widget position");
			return false;
		}

		Ogre::Real relX = pt.x / mSize.width;
		Ogre::Real relY = pt.y / mSize.height;

		Ogre::Real xpos = (relX * mWidgetImage->getWidth()); 
		Ogre::Real ypos = (relY * mWidgetImage->getHeight());

		// Reason I subtract 1 from width and height: Cannot access pixel 10 in an image of width 10. 0-9..
		if(xpos < 0 )
			xpos = 0; 
		else if(xpos >= mWidgetImage->getWidth())
			xpos = mWidgetImage->getWidth() - 1;

		if(ypos < 0)
			ypos = 0;
		else if(ypos >= mWidgetImage->getHeight())
			ypos = mWidgetImage->getHeight() - 1;

		const Ogre::ColourValue c = mWidgetImage->getColourAt(xpos,ypos,0);
		if( c.a <= 0.0 ) 
			return true;

		return false;
	}

	void Widget::redraw()
	{
		mQuad->setDimensions(Rect(getScreenPosition() + getScrollOffset(),mSize));
		mQuad->setOpacity(getActualOpacity());

		WidgetArray::iterator it;

		for( it = mComponents.begin(); it != mComponents.end(); ++it )
			(*it)->redraw();

		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->redraw();
	}

	void Widget::removeChild(Widget* w)
	{
		removeChild(w->getInstanceName());
	}

	void Widget::removeChild(const Ogre::String& widgetName)
	{
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if((*it)->getInstanceName() == widgetName)
			{
				WidgetEventArgs args((*it));
				fireEvent(EVENT_CHILD_REMOVED,args);

				(*it)->setQuadContainer(NULL);
				(*it)->setParent(NULL);
				mChildWidgets.erase(it);
				return;
			}
		}
	}

	void Widget::removeAndDestroyAllChildWidgets()
	{
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			WidgetEventArgs args((*it));
			fireEvent(EVENT_CHILD_REMOVED,args);

			(*it)->setQuadContainer(NULL);
			(*it)->setParent(NULL);
			delete (*it);
		}
		mChildWidgets.clear();
	}

	void Widget::removeAndDestroyChild(Widget* w)
	{
		if((w == NULL) || (mGUIManager == NULL) || (std::find(mChildWidgets.begin(),mChildWidgets.end(),w) == mChildWidgets.end()))
			return;

		removeChild(w);
		mGUIManager->_destroyWidget(w);

		WidgetEventArgs args(w);
		fireEvent(EVENT_CHILD_DESTROYED,args);
	}

	void Widget::removeAndDestroyChild(const Ogre::String& widgetName)
	{
		removeAndDestroyChild(getChildWidget(widgetName));
	}

	bool Widget::resizingAllowed()
	{
		return mCanResize;
	}

	void Widget::timeElapsed(const Ogre::Real time) 
	{
		if(!mEnabled)
			return;
	}

	void Widget::setClippingWidget(Widget* w, bool recursive)
	{
		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
		{
			if( (*it)->getInheritClippingWidget() )
				(*it)->setClippingWidget(w);
		}

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
		{
			if( (*it)->getInheritClippingWidget() )
					(*it)->setClippingWidget(w);
		}

		if(recursive)
		{
			for(WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it)
			{
				if( (*it)->getInheritClippingWidget() )
					(*it)->setClippingWidget(w);
			}
		}
	}

	void Widget::setDimensions(const Rect& pixelDimensions)
	{
		setPosition(pixelDimensions.x,pixelDimensions.y);
		setSize(pixelDimensions.width,pixelDimensions.height);
	}

	void Widget::setDraggingWidget(Widget* w)
	{
		mWidgetToDrag = w;
	}

	void Widget::setFont(const Ogre::String& fontScriptName, bool recursive) 
	{ 
		if(fontScriptName == "")
			return;

		mFontName = fontScriptName;

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
			(*it)->setFont(fontScriptName,true);

		if(recursive)
		{
			for(WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it)
				(*it)->setFont(fontScriptName,recursive);
		}
	}

	void Widget::setGainFocusOnClick(bool gainFocus)
	{
		mGainFocusOnClick = gainFocus;
	}

	void Widget::setGrabbed(bool grabbed)
	{
		mGrabbed = grabbed;
	}

	void Widget::setHeight(Ogre::Real pixelHeight)
	{
		mSize.height = pixelHeight;

		// update vertical anchor
		if(mParentWidget != NULL)
			mPixelsFromParentBottom = mParentWidget->getHeight() - (mPosition.y + mSize.height);

		WidgetEventArgs args(this);
		fireEvent(EVENT_SIZE_CHANGED,args);

		mQuad->setHeight(mSize.height);
	}

	void Widget::setHideWithParent(bool hide)
	{
		mHideWithParent = hide;
	}

	void Widget::setHorizontalAnchor(Widget::HorizontalAnchor a)
	{
		mHorizontalAnchor = a;
	}

	void Widget::setInheritClippingWidget(bool inherit)
	{
		mInheritClippingWidget = inherit;
	}

	void Widget::setInheritOpacity(bool inherit)
	{
		mInheritOpacity = inherit;

		redraw();
	}

	void Widget::setInheritQuadLayer(bool inherit)
	{
		mInheritQuadLayer = inherit;
	}

	void Widget::setMovingEnabled(bool enable)
	{
		mMovingEnabled = enable;
	}

	void Widget::setOffset(int offset)
	{
		int delta = offset - mOffset;
		mOffset = offset;

		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
			(*it)->setOffset((*it)->getOffset() + delta);

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
			(*it)->setOffset((*it)->getOffset() + delta);

		for(WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->setOffset((*it)->getOffset() + delta);
	}

	void Widget::setOpacity(Ogre::Real opacity)
	{
		mOpacity = opacity;

		redraw();
	}

	void Widget::setPosition(const Ogre::Real& pixelX, const Ogre::Real& pixelY)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		mPosition.x = pixelX;
		mPosition.y = pixelY;

		WidgetEventArgs args(this);
		fireEvent(EVENT_POSITION_CHANGED,args);

		// update anchors
		_deriveAnchorValues();
		
		mQuad->setPosition(getScreenPosition() + getScrollOffset());
	}

	void Widget::setPosition(const Point& pixelPoint)
	{
		setPosition(pixelPoint.x,pixelPoint.y);
	}

	void Widget::setPropagateEventFiring(Event e, bool propogate)
	{
		mPropogateEventFiring[e] = propogate;
	}

	void Widget::setScreenPosition(const Ogre::Real& pixelX, const Ogre::Real& pixelY)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		setPosition(Point(pixelX,pixelY) - mParentWidget->getScreenPosition());
	}

	void Widget::setScreenXPosition(const Ogre::Real& pixelX)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		setXPosition(pixelX - mParentWidget->getScreenPosition().x);
	}

	void Widget::setScreenYPosition(const Ogre::Real& pixelY)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		setYPosition(pixelY - mParentWidget->getScreenPosition().y);
	}

	void Widget::setScrollPaneAccessible(bool accessible)
	{
		mScrollPaneAccessible = accessible;
	}

	void Widget::setSkin(const Ogre::String& skinName, bool recursive)
	{
		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(skinName);
		if(ss == NULL)
			throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND,"Skin \"" + skinName + "\" does not exist!  Did you forget to load it using the SkinSetManager?","Widget::setSkin");

		mSkinName = skinName;

		Ogre::String textureName = mSkinName + mSkinComponent + ss->getImageExtension();

		if(!ss->containsImage(textureName))
			return;

		mQuad->setMaterial(ss->getMaterialName());
		mQuad->setTextureCoordinates(ss->getTextureCoordinates(textureName));

		// Load Image, used for transparency picking.
		if(Utility::textureExistsOnDisk(textureName))
		{
			Ogre::Image i;
			i.load(textureName,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			delete mWidgetImage;
			mWidgetImage = new Ogre::Image(i);
		}

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
			(*it)->setSkin(skinName,true);

		if(recursive)
		{
			for(WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it)
				(*it)->setSkin(skinName,recursive);
		}

		if(mHideSkin)
			hideSkin();
	}

	void Widget::setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight)
	{
		mSize.width = pixelWidth;
		mSize.height = pixelHeight;

		// update anchor
		_deriveAnchorValues();

		WidgetEventArgs args(this);
		fireEvent(EVENT_SIZE_CHANGED,args);

		mQuad->setSize(mSize);
	}

	void Widget::setSize(const Size& pixelSize)
	{
		Widget::setSize(pixelSize.width,pixelSize.height);
	}

	void Widget::setShowWithParent(bool show)
	{
		mShowWithParent = show;
	}

	void Widget::setQuadLayer(Quad::Layer l)
	{
		mQuadLayer = l;

		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
		{
			if((*it)->getInheritLayer())
				(*it)->setLayer(mQuadLayer);
		}

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
		{
			if( (*it)->getInheritQuadLayer() )
				(*it)->setQuadLayer(mQuadLayer);
		}

		for(WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if( (*it)->getInheritQuadLayer() )
				(*it)->setQuadLayer(mQuadLayer);
		}
	}

	void Widget::setUseBorders(bool use)
	{
		mUseBorders = use;
		if(mUseBorders)
			_createBorders();
		else
			_destroyBorders();
	}

	void Widget::setVerticalAnchor(Widget::VerticalAnchor a)
	{
		mVerticalAnchor = a;
	}

	void Widget::setWidth(Ogre::Real pixelWidth)
	{
		mSize.width = pixelWidth;

		// update horizontal anchor
		if(mParentWidget != NULL)
			mPixelsFromParentRight = mParentWidget->getWidth() - (mPosition.x + mSize.width);

		WidgetEventArgs args(this);
		fireEvent(EVENT_SIZE_CHANGED,args);

		mQuad->setWidth(mSize.width);
	}

	void Widget::setXPosition(Ogre::Real pixelX)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		mPosition.x = pixelX;

		// update horizontal anchor
		if(mParentWidget != NULL)
			mPixelsFromParentRight = mParentWidget->getWidth() - (mPosition.x + mSize.width);

		WidgetEventArgs args(this);
		fireEvent(EVENT_POSITION_CHANGED,args);
		
		mQuad->setXPosition(getScreenPosition().x + getScrollOffset().x);
	}

	void Widget::setYPosition(Ogre::Real pixelY)
	{
		if((mWidgetType == Widget::TYPE_SHEET) || (mParentWidget == NULL))
			return;

		mPosition.y = pixelY;

		// update vertical anchor
		if(mParentWidget != NULL)
			mPixelsFromParentBottom = mParentWidget->getHeight() - (mPosition.y + mSize.height);

		WidgetEventArgs args(this);
		fireEvent(EVENT_POSITION_CHANGED,args);
		
		mQuad->setYPosition(getScreenPosition().y + getScrollOffset().y);
	}

	void Widget::show()
	{
		bool currentlyVisible = mVisible;
		mVisible = true;
		for( QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it )
		{
			if((*it)->getShowWithOwner())
				(*it)->setVisible(true);
		}

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it )
		{
			if((*it)->getShowWithParent())
				(*it)->show();
		}

		// show children, except for Windows and lists of MenuList or ComboBox Widget.
		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
		{
			if((*it)->getShowWithParent())
				(*it)->show();
		}

		// Only fire event if we change visibility.  If we were already visible, don't fire.
		if(!currentlyVisible)
		{
			WidgetEventArgs args(this);
			fireEvent(EVENT_SHOWN,args);
		}
	}

	void Widget::showSkin()
	{
		mHideSkin = false;
		setSkin(mSkinName);
	}

	void Widget::setGUIManager(GUIManager* gm)
	{
		mGUIManager = gm;

		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
			(*it)->setGUIManager(mGUIManager);

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
			(*it)->setGUIManager(mGUIManager);

		WidgetArray::iterator it;
		for( it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->setGUIManager(gm);
	}

	void Widget::setParent(Widget* parent)
	{
		mParentWidget = parent;

		if(mParentWidget != NULL)
		{
			setQuadContainer(parent->getQuadContainer());
			setGUIManager(parent->getGUIManager());

			// set the correct offset
			setOffset(mParentWidget->getOffset() + 1);
			// calculated properties
			_deriveAnchorValues();
			// inheritted properties
			if(mInheritClippingWidget)
				setClippingWidget(mParentWidget,true);
			if(!mParentWidget->isVisible())
				hide();
			if(mInheritQuadLayer)
				setQuadLayer(mParentWidget->getQuadLayer());
			mGainFocusOnClick = mParentWidget->getGainFocusOnClick();
		}

		WidgetEventArgs args(this);
		fireEvent(EVENT_PARENT_CHANGED,args);
	}

	void Widget::setQuadContainer(QuadContainer* container)
	{
		mQuadContainer = container;
		
		for(QuadArray::iterator it = mQuads.begin(); it != mQuads.end(); ++it)
			(*it)->_notifyQuadContainer(mQuadContainer);

		for(WidgetArray::iterator it = mComponents.begin(); it != mComponents.end(); ++it)
			(*it)->setQuadContainer(mQuadContainer);

		for( WidgetArray::iterator it = mChildWidgets.begin(); it != mChildWidgets.end(); ++it )
			(*it)->setQuadContainer(mQuadContainer);
	}

	void Widget::setSkinComponent(const Ogre::String& skinComponent)
	{
		mSkinComponent = skinComponent;

		// update widget appearance
		if(mSkinName != "")
			setSkin(mSkinName);
	}

	void Widget::unlockTexture()
	{
		mTextureLocked = false;
	}
}
