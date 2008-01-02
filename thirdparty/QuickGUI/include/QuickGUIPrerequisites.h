#ifndef __QUICKPREQUISITES_H
#define __QUICKPREQUISITES_H

#include "Ogre.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	////////////DRAWING/////////////
	#define VERTICES_PER_QUAD 4

	class Rect;
	class Point;
	class Size;

	struct Vertex;
	class VertexBuffer;
	class QuadContainer;
	class Quad;

	// Stores the Texture of a quad, and the index of the first quad following it with a different texture.
	typedef std::pair<Ogre::String,unsigned int > TextureQuadIndex;
	typedef std::vector<TextureQuadIndex > TextureChangeList;

	typedef std::list<Quad*> QuadList;
	typedef std::vector<Quad*> QuadArray;

	typedef std::list<QuadContainer*> QuadContainerList;

	////////////EVENTS/////////////
	class MemberFunctionSlot;
	class EventArgs;
	class WidgetEventArgs;
	class MouseEventArgs;

	typedef std::vector<MemberFunctionSlot*> EventHandlerArray;

	////////////UTILS/////////////
	class MouseCursor;
	class Utility;
	class ConfigScriptLoader;

	////////////Handlers/////////////
	class GUIManager;
	class Sheet;
	class SkinSet;

	////////////Effects/////////////
	class Effect;
	class AlphaEffect;
	class MoveEffect;
	class SizeEffect;

	////////////Widgets/////////////
	class Widget;
	typedef std::vector<Widget*> WidgetArray;

	class Window;

	class Image;
	class Panel;
	class Border;
	class ScrollPane;
	class Button;
	class NStateButton;

	class Text;
	class Label;
	class MenuLabel;
	class LabelArea;
	class TextBox;
	class Console;
	class TextArea;
	class TextBox;

	class List;
	class ComboBox;
	class TitleBar;
	class Tree;

	class ProgressBar;
	class HorizontalTrackBar;
	class VerticalTrackBar;
	class VerticalScrollBar;
	class HorizontalScrollBar;

}
#endif //__QUICKPREQUISITES_H
