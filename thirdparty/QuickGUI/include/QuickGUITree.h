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

namespace QuickGUI
{
	/** Represents an Item in the Tree
		@remarks
			A Tree Widget can hold a number of TreeItems.
	*/
	class TreeItem
	{
	public:
		/** Constructor
            @param
                name The name to be given to the TreeItem (must be unique).
            @param
                skinName The name of the skin to get graphics from
        */
		TreeItem(const std::string& name, const std::string& skinName, Tree* tree, double iconsize, GUIManager* mGuiManager);
		~TreeItem();

		double getLag() { return _lag; }
		bool getVisible() { return _visible; }
		bool getPlace() { return _place; }

		std::vector<TreeItem *>& getChildren();
		
		TreeItem* getParent() { return _parent; }
		std::string& getName() { return _name; }
		MenuLabel* getItem() { return _item; }
		Image* getImage() { return _minus; }
		Image* getThumbtails() { return _thumbtails; }
		TreeItem* getPrevious() { return _previous; }
		TreeItem* getNext() { return _next; }

		QuickGUI::Image* addChild(TreeItem* Item);
		void removeChild(const std::string& name);

		void setYPosition(double y);
		void setVisible(bool vis);
		void setPrevious(TreeItem* prev) { _previous = prev; }
		void setNext(TreeItem* next) { _next = next; }
		
		void onMouseButtonDown(const EventArgs& args);
		void setParent(TreeItem* item) { _parent = item; }
		void setLag(double lag) { _lag = lag; }
		void setItem(MenuLabel* item) { _item = item; }
		void setThumbtails(const std::string& pic);

	private:
		GUIManager*	_guiManager;
		QuickGUI::Image* _minus;		// - + button
		QuickGUI::Image* _thumbtails;
		TreeItem* _parent;
		std::vector<TreeItem *> _child;
		double _lag;
		double _IconSize;
		bool _place;
		bool _visible;

		/* heritage */
		TreeItem* _next;
		TreeItem* _previous;
		Tree* _tree;
		std::string mSkinName;
		std::string _name;
		MenuLabel* _item;
	};

	/** Represents a Tree View Widget
	*/
	class _QuickGUIExport Tree :
		public List
	{
	public:
		/** Constructor
            @param
                name The name to be given to the Tree Widget (must be unique).
            @param
                mGUIManager
        */
		Tree(const std::string& name, QuickGUI::GUIManager* mGUIManager);
		
		void addInList(const std::string& name, const std::string& text, const std::string& parent = "root", const std::string& LoadAfter = "");
		void hideTreeItems(const std::string& name);
		void showTreeItems(const std::string& name);

		void setTitle(const std::string& title);
		void deleteItem(const std::string& name);
		void addAfterTreeItem(const std::string& name, TreeItem* item);
		std::string	GetLastChild(const std::string& name, const std::string& parent = "");
		void setThumbtails(const std::string& name, const std::string& pic);
	private:
		~Tree();
		bool hasParent(TreeItem* child, const std::string& name);
		bool parentIsPlace(TreeItem* child);
		void slideUp(TreeItem* child);
		void slideDown(TreeItem* child);
		void deleteChild(TreeItem* me);
	private:
		std::string _name;
		double _iconSize;
		TreeItem* _lastChild;
		std::map<std::string, TreeItem*> _lists;
		QuickGUI::GUIManager* _guiManager;
	};
}

#endif

