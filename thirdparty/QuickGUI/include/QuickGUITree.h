#ifndef QUICKGUITREE_H
#define QUICKGUITREE_H

#include "OgreStringConverter.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIBorder.h"
#include "QuickGUIButton.h"
#include "QuickGUICheckBox.h"
#include "QuickGUIComboBox.h"
#include "QuickGUIConsole.h"
#include "QuickGUIImage.h"
#include "QuickGUIPanel.h"
#include "QuickGUILabelArea.h"
#include "QuickGUINStateButton.h"
#include "QuickGUIProgressBar.h"
#include "QuickGUIScrollPane.h"
#include "QuickGUIText.h"
#include "QuickGUITextBox.h"
#include "QuickGUIHorizontalTrackBar.h"
#include "QuickGUIVerticalTrackBar.h"
#include "QuickGUIWidget.h"

#include <iostream>
#include <vector>

using std::vector;
using std::string;
using std::map;

namespace QuickGUI
{
	class Tree;
	class TreeItem;

	/** Represents an Item in the Tree
		@remarks
			A Tree Widget can hold a number of TreeItems.
	*/
	class TreeItem
	{
	public:
		/** Constructor
            @param
                name The name to be given to the TreeItem .
			@param
                id The id to be given to the TreeItem (must be unique).
            @param
                skinName The name of the skin to get graphics from
        */
		TreeItem(const string &name,const string &id, const string &skinName, Tree *tree, double iconsize, GUIManager *mGuiManager);
		~TreeItem();

		double					getLag(){ return _lag; }
		bool					getVisible(){ return _visible; }
		bool					getPlace(){ return _place; }
		TreeItem				*getParent(){ return _parent; }
		vector<TreeItem *>		&getChilds();
		string					&getName(){ return _name;}
		string					&getId(){ return _id;}
		MenuLabel *				getItem() { return _item; }
		Image	*				getImage(){ return _minus; }
		Image	*				getThumbtails(){ return _thumbtails; }
		TreeItem				*getPrevious(){ return _previous; }
		TreeItem				*getNext(){ return _next; }


		void					removeChild(const string &name);
		void					setYPosition(double y);
		void					setVisible(bool vis);
		void					setItem(MenuLabel *item);
		void					setPrevious(TreeItem *pre){ _previous = pre; }
		void					setNext(TreeItem * next){ _next = next; }
		QuickGUI::Image			*addChild(TreeItem *Item);
		void					setParent(TreeItem *Item){ _parent = Item; }
		void					setLag(double lag){ _lag = lag; }
		void					setPlace(bool place){ _place = place; }
		void					setThumbtails(const string &pic);
		void					onMouseButtonDownItemLeave();
	private: //effects
		
		void onMouseClickDouble(const EventArgs& args);
		void onMouseButtonDown(const EventArgs& args);
		void onMouseEntersItem(const EventArgs& args);
		void onMouseLeavesItem(const EventArgs& args);
		void onMouseButtonDownItem(const EventArgs& args);

	private:
		GUIManager *				_guiManager;
		QuickGUI::Image				*_minus;		// - + button
		QuickGUI::Image				*_thumbtails;
		TreeItem					*_parent;
		vector<TreeItem *>		_child;
		double						_lag;
		double						_IconSize;
		bool						_place;
		bool						_visible;
		bool						_down;

		/* heritage */
		TreeItem					*_next;
		TreeItem					*_previous;
		Tree						*_tree;
		string					mSkinName;
		string					_id;
		string					_name;
		MenuLabel					*_item;
	};


	/** Represents a Tree View Widget
	*/

	class _QuickGUIExport Tree :
		public List
	{
	public:
		/** Constructor
            @param
                name The name to be given to the Tree Widget.
            @param
                mGUIManager
			@param
                padding ChildTreeItem's left padding inherited from parent TreeItem. Default value 20
        */
		Tree(const string &name, QuickGUI::GUIManager *mGUIManager, double padding = 20);

		const string addInList(const string &name, const string &text, const string &parent = "root", const string &LoadAfter = "");
		void		hideTreeItems(const string &name);
		void		showTreeItems(const string &name);

		void		deleteItem(const string &name);
		void		addAfterTreeItem(const string &name, TreeItem *item);
		string		GetLastChild(const string &name, const string &parent = "");
		void		setThumbtails(const string &name, const string &pic);
		void		onItemPushDown(const string &name);
		const string GetSelectedName();
		const string GetSelectedID();
		void					close(const string &id);
		void					open(const string &id);

	private:
		~Tree();
		bool		hasParent(TreeItem *child, const string &name);
		bool		parentIsPlace(TreeItem *child);
		void		slideUp(TreeItem *child);
		void		slideDown(TreeItem *child);
		void		deleteChild(TreeItem *me);
		string	createId(const string& name);
		void		resize();
		
	private:
		double											_padding;
		TreeItem										*_selected;
		string										_name;
		string										_id;
		double											_iconSize;
		TreeItem										*_lastChild;
		map<string, TreeItem *>				_lists;
		QuickGUI::GUIManager							*_guiManager;
	};
}

#endif

