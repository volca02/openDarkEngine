#ifndef QUICKGUIWIDGET_H
#define QUICKGUIWIDGET_H

#include "OgreException.h"
#include "OgrePrerequisites.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIEventArgs.h"
#include "QuickGUIMemberFunctionPointer.h"
#include "QuickGUIRect.h"
#include "QuickGUIQuad.h"
#include "QuickGUIQuadContainer.h"
#include "QuickGUISkinSetManager.h"
#include "QuickGUIExportDLL.h"

#include <vector>
#include <ctype.h>

namespace QuickGUI
{
	// Forward declarations
	class Border;
	class Window;
	class Sheet;
	class Panel;
	class GUIManager;

	/** Main QuickGUI components for creating GUI.
		@remarks
		This class outlines the interface for
		widgets, providing default functionality.
	*/
	class _QuickGUIExport Widget
	{
	public:
		// GUIManager is the only manager that can destroy widgets.
		friend class GUIManager;
		friend class Panel;
		friend class ScrollPane;
		friend class List;

		/**
		* Outlining Types of widgets in the library.
		*/
		enum Type
		{
			TYPE_BORDER				=  0,
			TYPE_BUTTON					,
			TYPE_CHECKBOX				,
			TYPE_COMBOBOX				,
			TYPE_CONSOLE				,
			TYPE_IMAGE					,
			TYPE_LABEL					,
			TYPE_LIST					,
			TYPE_MENULABEL				,
			TYPE_LABELAREA				,
			TYPE_TEXTAREA				,
			TYPE_NSTATEBUTTON			,
			TYPE_PANEL					,
			TYPE_PROGRESSBAR			,
			TYPE_SCROLL_PANE			,
			TYPE_SCROLLBAR_HORIZONTAL	,
			TYPE_SCROLLBAR_VERTICAL		,
			TYPE_SHEET					,
			TYPE_TEXTBOX				,
			TYPE_TITLEBAR				,
			TYPE_TRACKBAR_HORIZONTAL	,
			TYPE_TRACKBAR_VERTICAL		,
			TYPE_TREE					,
			TYPE_WINDOW
		};
		/**
		* All widgets must support these events
		*/
		enum Event
		{
			EVENT_CHARACTER_KEY		=  0,
			EVENT_CHILD_ADDED			,
			EVENT_CHILD_CREATED			,
			EVENT_CHILD_DESTROYED		,
			EVENT_CHILD_REMOVED			,
			EVENT_DISABLED				,
			EVENT_DRAGGED				,
			EVENT_DROPPED				,
			EVENT_ENABLED				,
			EVENT_GAIN_FOCUS			,
			EVENT_HIDDEN				,
			EVENT_KEY_DOWN				,
			EVENT_KEY_UP				,
			EVENT_LOSE_FOCUS			,
			EVENT_MOUSE_BUTTON_DOWN		,
			EVENT_MOUSE_BUTTON_UP		,
			EVENT_MOUSE_CLICK			,
			EVENT_MOUSE_CLICK_DOUBLE	,
			EVENT_MOUSE_CLICK_TRIPLE	,
			EVENT_MOUSE_ENTER			,
			EVENT_MOUSE_LEAVE			,
			EVENT_MOUSE_MOVE			,
			EVENT_MOUSE_WHEEL			,
			EVENT_PARENT_CHANGED		,
			EVENT_POSITION_CHANGED		,
			EVENT_SHOWN					,
			EVENT_SIZE_CHANGED			,
			EVENT_END_OF_LIST
		};
		/**
		* Specifies horizontal position/size relative to parent resizing.
		*/
		enum HorizontalAnchor
		{
			ANCHOR_HORIZONTAL_LEFT				=  0,
			ANCHOR_HORIZONTAL_RIGHT					,
			ANCHOR_HORIZONTAL_LEFT_RIGHT			,
			ANCHOR_HORIZONTAL_NONE
		};
		/**
		* Specifies vertical position/size relative to parent resizing.
		*/
		enum VerticalAnchor
		{
			ANCHOR_VERTICAL_TOP				=  0,
			ANCHOR_VERTICAL_BOTTOM				,
			ANCHOR_VERTICAL_TOP_BOTTOM			,
			ANCHOR_VERTICAL_NONE
		};
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
				textureName The name of the texture used to visually represent the widget. (ie "qgui.window.png")
			@param
				group The QuadContainer containing the Quad used by this Widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		Widget(const Ogre::String& name, GUIManager* gm);

		virtual void addChild(Widget* w);
		/** Adds an event handler to this widget
			@param
				EVENT Defined widget events, for example: EVENT_GAIN_FOCUS, EVENT_CHARACTER_KEY, EVENT_MOUSE_BUTTON_DOWN, etc
            @param
                function member function assigned to handle the event.  Given in the form of myClass::myFunction.
				Function must return bool, and take QuickGUI::EventArgs as its parameters.
            @param
                obj particular class instance that defines the handler for this event.  Here is an example:
				addEventHandler(QuickGUI::EVENT_MOUSE_BUTTON_DOWN,myClass::myFunction,this);
			@note
				Multiple user defined event handlers can be defined for an event.  All added event handlers will be called
				whenever the event is fired.
			@note
				You may see Visual Studio give an error warning such as "error C2660: 'QuickGUI::Widget::addEventHandler' : function does not take 3 arguments".
				This is an incorrect error message.  Make sure your function pointer points to a function which returns void, and takes parameter "const EventArgs&".
        */
		template<typename T> void addEventHandler(Event EVENT, void (T::*function)(const EventArgs&), T* obj)
		{
			mUserEventHandlers[EVENT].push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addEventHandler(Event EVENT, MemberFunctionSlot* function);

		template<typename T> void addEventListener(Event EVENT, void (T::*function)(const EventArgs&), T* obj)
		{
			mEventListeners.push_back(new MemberFunctionPointer<T>(function,obj));
		}
		void addEventListener(MemberFunctionSlot* function);

		void allowResizing(bool allow);
		/**
		* Alters the widgets offset to be higher than widget w.  Widget w must be in the
		* same QuadContainer and Layer.
		*/
		virtual void appearOverWidget(Widget* w);
		void constrainDragging(bool DragXOnly, bool DragYOnly);
		/**
		* Disable Widget, making it unresponsive to events.
		* NOTE: Sheets cannot be disabled.
		*/
		virtual void disable();
		/**
		* Moves draggingWidget.  By default, dragging widget is this widget, but this can be changed.
		* Allows dragging the titlebar or it's text to drag the window, for example.
		*/
		void drag(const Ogre::Real& pixelX, const Ogre::Real& pixelY);
		/**
		* Returns true if the widget is able to be dragged, false otherwise.
		*/
		bool draggingEnabled();
		/**
		* Enable Widget, allowing it to accept and handle events.
		* NOTE: Sheets cannot be enabled/disabled
		*/
		virtual void enable();
		/**
		* Returns true is widget is enabled, false otherwise.
		*/
		bool enabled();
		/**
		* Enable or Disable dragging.
		*/
		void enableDragging(bool enable);
		/**
		* Event Handler that executes the appropriate User defined Event Handlers for a given event.
		* Returns true if the event was handled, false otherwise.
		*/
		bool fireEvent(Event e, EventArgs& args);
		/**
		* Sets focus to the widget by firing an activation event.
		*/
		virtual void focus();
		Ogre::Real getActualOpacity();
		/**
		* Returns the position of the widget as it would be drawn on the screen.
		* NOTE: This is a convenience method. Actual Position is the same as
		*  getScreenPosition() + getScrollOffset().
		*/
		Point getActualPosition();
		WidgetArray* getChildWidgetList();
		Widget* getChildWidget(const Ogre::String& name);
		Widget* getChildWidget(Type t, unsigned int index);
		Rect getDimensions();
		GUIManager* getGUIManager();
		int getNumberOfHandlers(Event e);
		bool getInheritOpacity();
		Ogre::Real getOpacity();
		Point getPosition();
		Point getScrollOffset();
		Size getSize();

		Ogre::String getFontName();
		/**
		* Returns true if the widget will gain focus when clicked, false otherwise.
		*/
		bool getGainFocusOnClick();
		bool getGrabbed();
		Ogre::Real getHeight();
		/**
		* Returns true if this widget is hidden when its parent is hidden.
		*/
		bool getHideWithParent();
		/**
		* Iterates through all child widgets and retrieves the highest offset.
		*/
		int getHighestOffset();
		HorizontalAnchor getHorizontalAnchor();
		bool getInheritClippingWidget();
		bool getInheritQuadLayer();
		Ogre::String getInstanceName();
		/**
		* Returns true if window is able to be repositions, false otherwise.
		*/
		bool getMovingEnabled();
		/**
		* Returns the number of parent iterations required to get to Sheet widget.
		*/
		int getOffset();
		/**
		* Get Panel this widget belongs to.
		* NOTE: This value may be NULL.
		*/
		Panel* getParentPanel();
		/**
		* Get Sheet this widget belongs to.
		* NOTE: This value may be NULL.
		*/
		Sheet* getParentSheet();
		/**
		* Get Widget this widget belongs to.
		* NOTE: This value may be NULL.
		*/
		Widget* getParentWidget();
		/**
		* Get Window this widget belongs to.
		* NOTE: This value may be NULL.
		*/
		Window* getParentWindow();
		bool getPropogateEventFiring(Event e);
		/*
		* Get Render Object that visually represents the widget.
		*/
		Quad* getQuad();
		/*
		* Get Render Object Group this widget's Quad belongs in.
		*/
		QuadContainer* getQuadContainer();
		Quad::Layer getQuadLayer();
		/*
		* Get the screen pixel coordinates this widget is drawn at.
		* NOTE: This is not the same as getPosition, which returns a value relative to parent.
		* NOTE: This may not be the actual screen coordinates, since QuickGUI supports scrolling.
		*/
		Point getScreenPosition();
		bool getScrollPaneAccessible();
		Ogre::String getSkinComponent();
		/**
		* Get whether or not this widget is shown when its parent is shown.
		*/
		bool getShowWithParent();
		Ogre::String getSkin();
		/**
		* Iterates through visible Children widgets to find and return the widget that is *hit* by the point.
		* Returns NULL is nothing is *hit*.
		*/
		virtual Widget* getTargetWidget(const Point& pixelPosition);
		/**
		* Returns the type of the widget, as enumerated above. ex. TYPE_BUTTON.
		*/
		Type getWidgetType();
		VerticalAnchor getVerticalAnchor();
		Ogre::Real getWidth();
		Ogre::Real getXPosition();
		Ogre::Real getYPosition();
		bool hasMouseButtonHandlers();
		/**
		* Sets mVisible to false.  Widgets should override this to implement how they handle
		* hiding.
		*/
		virtual void hide();
		void hideSkin();
		/**
		* Returns true if pixel point p is inside the pixel dimensions of this widget.
		*/
		virtual bool isPointWithinBounds(const Point& pixelPosition);
		bool isVisible();
		/**
		* Returns true if Widget w is a child of this widget, false otherwise.
		*/
		bool isChild(Widget* w);

		/**
		* Offset the widget position.  Useful for dragging/moving widgets.
		*/
		void move(const Ogre::Real& pixelX, const Ogre::Real& pixelY);
		void move(const Point& pixelOffset);
		void moveX(Ogre::Real pixelX);
		void moveY(Ogre::Real pixelY);
		/*
		* Function disabling ability to change widget's texture.
		*/
		void lockTexture();
		/**
		* Determins if the mouse if over a transparent part of the image defining the widget.
		* Used to determin if the mouse is *over* a widget. (non transparent parts)
		*/
		bool overTransparentPixel(const Point& mousePixelPosition);
		/**
		* Force updating of the Widget's Quad position on screen.
		*/
		virtual void redraw();
		void removeChild(Widget* w);
		void removeChild(const Ogre::String& widgetName);
		/**
		* Properly cleans up all child widgets.
		*/
		void removeAndDestroyAllChildWidgets();
		void removeAndDestroyChild(Widget* w);
		void removeAndDestroyChild(const Ogre::String& widgetName);
		bool resizingAllowed();
		/**
		* Manually set the Dimensions of the widget.
		*/
		void setDimensions(const Rect& pixelDimensions);
		/**
		* This function specifies the widget that will be moved when the widget is *dragged*.
		* By default, the Dragging Widget is the widget itself, but this allows for the possibility
		* of moving a window by *dragging* the titlebar, or even the titlbar's text widget.
		*/
		void setDraggingWidget(Widget* w);

		virtual void setFont(const Ogre::String& fontScriptName, bool recursive = false);
		/**
		* Allows clicking on a widget to not change the active widget.
		*/
		void setGainFocusOnClick(bool gainFocus);
		/**
		* Manually set mGrabbed to true.
		*/
		void setGrabbed(bool grabbed);
		virtual void setHeight(Ogre::Real pixelHeight);
		/**
		* If set to true, this widget will be hidden when its parent's widget is hidden.
		* NOTE: All widgets have this set to true by default.
		*/
		void setHideWithParent(bool hide);
		void setHorizontalAnchor(HorizontalAnchor a);
		/**
		* When set to true, the widget will inherit it's parent's clipping widget.
		* NOTE: The clipping widget's bounds are the bounds used to clip the widget.
		*/
		void setInheritClippingWidget(bool inherit);
		void setInheritOpacity(bool inherit);
		void setInheritQuadLayer(bool inherit);
		/**
		* If set to false, widget cannot be moved.
		*/
		void setMovingEnabled(bool enable);
		/**
		* Manipulates the offset used to determine this widgets zOrder in rendering.
		*/
		virtual void setOffset(int offset);
		void setOpacity(Ogre::Real opacity);
		/**
		* Manually set position of widget.
		* NOTE: the values given are relative to the parent's top left corner, and not the screen!  For screen positioning,
		*  user the setScreenPosition function.
		*/
		virtual void setPosition(const Ogre::Real& pixelX, const Ogre::Real& pixelY);
		virtual void setPosition(const Point& pixelPoint);
		void setPropagateEventFiring(Event e, bool propogate);
		virtual void setQuadLayer(Quad::Layer l);
		/**
		* Manually set position of widget.
		* NOTE: the values given are relative to the render windows's top left corner, and not the parent widget!
		*/
		void setScreenPosition(const Ogre::Real& pixelX, const Ogre::Real& pixelY);
		void setScreenXPosition(const Ogre::Real& pixelX);
		void setScreenYPosition(const Ogre::Real& pixelY);
		void setScrollPaneAccessible(bool accessible);
		virtual void setSkin(const Ogre::String& skinName, bool recursive = false);
		void setSkinComponent(const Ogre::String& skinComponent);
		/**
		* Manually set size of widget.
		*/
		virtual void setSize(const Ogre::Real& pixelWidth, const Ogre::Real& pixelHeight);
		virtual void setSize(const Size& pixelSize);
		/**
		* If set to true, this widget will be shown when its parent's widget is shown.
		* NOTE: most widgets have this set to true by default. (Menu's are false by default)
		*/
		void setShowWithParent(bool show);
		/**
		* If set to true, borders will be created, provided the matching *.border.*.png textures exist.
		* If set to false, any borders that have been created will be destroyed.
		*/
		void setUseBorders(bool use);
		void setVerticalAnchor(VerticalAnchor a);
		virtual void setWidth(Ogre::Real pixelWidth);
		void setXPosition(Ogre::Real pixelX);
		virtual void setYPosition(Ogre::Real pixelY);
		/**
		* Sets mVisible to true.  Widgets should override this to implement how they handle
		* showing.
		*/
		virtual void show();
		void showSkin();
		/**
		* Function required for certain widgets/functions to function properly, ie TextBox and fade.
		*/
		virtual void timeElapsed(const Ogre::Real time);
		inline bool isUnderTiming() const {return mEnabled;};

		/*
		* Allows texture of widget to change. (behavior by default)
		*/
		void unlockTexture();
		/** Checks if this widget property are subject 
		*   to modification upon time
		*/
		inline bool getUnderEffect() const { return mUnderEffect; }
		/** set widget flag underEffect to that if user takes control
		*   he knows that he has to disable effect before.
		*/
		void setUnderEffect(bool val) { mUnderEffect = val; }

	protected:
		virtual void setClippingWidget(Widget* w, bool recursive = false);
		virtual void setGUIManager(GUIManager* gm);
		virtual void setParent(Widget* parent);
		virtual void setQuadContainer(QuadContainer* container);

		// Positions/sizes the widget according to parent's size.
		virtual void _applyAnchors();
		void _deriveAnchorValues();
	protected:
		virtual ~Widget();

		GUIManager*					mGUIManager;
		Ogre::String				mInstanceName;
		Type						mWidgetType;

		// PROPERTIES
		bool						mCanResize;
		Widget*						mClippingWidget;
		bool						mInheritClippingWidget;
		bool						mDragXOnly;
		bool						mDragYOnly;
		Ogre::String				mFontName;
		Ogre::String				mSkinName;
		bool						mHideSkin;
		bool						mVisible;
		bool						mEnabled;
		bool						mGainFocusOnClick;
		bool						mGrabbed;
		bool						mTextureLocked;
		Quad::Layer					mQuadLayer;
		bool						mInheritQuadLayer;
		bool						mMovingEnabled;
		bool						mDraggingEnabled;
		Ogre::String				mSkinComponent;
		bool						mScrollPaneAccessible;
		bool						mInheritOpacity;
		Ogre::Real					mOpacity;
		// number of parents iterated to get to sheet.
		int							mOffset;
		bool						mHideWithParent;
		bool						mShowWithParent;
		// used for transparency picking
		Ogre::Image*				mWidgetImage;
		Widget*						mParentWidget;
		Widget*						mWidgetToDrag;

		// ANCHORS
		HorizontalAnchor			mHorizontalAnchor;
		VerticalAnchor				mVerticalAnchor;
		Ogre::Real					mPixelsFromParentRight;
		Ogre::Real					mPixelsFromParentBottom;

		// Implement the Enter/Leave functionality.
		bool						mEntered;

		// state that this widget property are subject 
		// to modification upon time
		bool						mUnderEffect;

		Quad*						mQuad;
		// All widgets have at least 1 quad, but can use more.
		QuadArray					mQuads;
		Quad* _createQuad();

		// Keeping track of the QuadContainer this Quad belongs to.
		QuadContainer*				mQuadContainer;

		Widget*						_createChild(const Ogre::String& name, Type t);
		// List of any child widgets this widget may have.
		WidgetArray					mChildWidgets;

		virtual Widget*				_createComponent(const Ogre::String& name, Type t);
		WidgetArray					mComponents;

		// Pixel position relative to parent.  (0,0) is the Parent Widgets top Left corner.
		Point						mPosition;
		// Used for scrolling widgets.
		Point						mScrollOffset;
		// Size in pixels.
		Size						mSize;

		bool						mUseBorders;
		void _createBorders();
		void _destroyBorders();

		// Event handlers! One List per event per widget
		EventHandlerArray mUserEventHandlers[EVENT_END_OF_LIST];
		bool mPropogateEventFiring[EVENT_END_OF_LIST];

		EventHandlerArray mEventListeners;

		void _initEventHandlers();

		virtual void _setScrollXOffset(Ogre::Real pixelXOffset);
		virtual void _setScrollYOffset(Ogre::Real pixelYOffset);
	protected:
		virtual void onPositionChanged(const EventArgs& args);
		virtual void onSizeChanged(const EventArgs& args);
	};
}

#endif
