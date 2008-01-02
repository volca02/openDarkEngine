#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIManager.h"
#include "QuickGUITree.h"

namespace QuickGUI
{
	/**	TreeItem Constructor
	*/
	TreeItem::TreeItem(const std::string& name, const std::string& skinName, Tree* tree, double iconsize, GUIManager* mGUIManager):
		_item(NULL), _parent(NULL), _minus(NULL) , _place(true), mSkinName(skinName),
		_name(name), _tree(tree), _next(NULL), _previous(NULL), _visible(true), _thumbtails(NULL),
		_IconSize(iconsize), _guiManager(mGUIManager)
	{
		_child.clear();
	}

	TreeItem::~TreeItem()
	{
		//delete from parent child's list
		if (_parent)
			_parent->removeChild(_name);

		_guiManager->destroyWidget(_thumbtails);
		_guiManager->destroyWidget(_minus);
	}

	std::vector<TreeItem*>& TreeItem::getChildren()
	{
		return _child;
	}

	void TreeItem::removeChild(const std::string& name)
	{
		std::vector<TreeItem*>::iterator itb = _child.begin();
		for(; itb != _child.end(); ++itb)
			if ((*itb)->getName() == name)
			{
				_child.erase(itb);
				break;
			}
	}

	QuickGUI::Image* TreeItem::addChild(TreeItem* Item)
	{
		_child.push_back(Item);
		if (!_minus) // set image + - 
		{
			Sheet *mSheet = _guiManager->getDefaultSheet();
			_minus = mSheet->createImage();
			_minus->setSkinComponent(".tree.minus");
			_minus->setSize(_IconSize, _IconSize);
			_minus->setInheritClippingWidget(true);
			_minus->setInheritOpacity(true);
			_minus->setInheritQuadLayer(true);
			_minus->addEventHandler(Widget::EVENT_MOUSE_BUTTON_DOWN, &TreeItem::onMouseButtonDown, this);
			return _minus;
		}
		return NULL;
	}

	void TreeItem::onMouseButtonDown(const EventArgs& args)
	{
		std::cout << "pushed skin name = " << mSkinName << std::endl;
		_place = !_place;
		// change skin component and apply skin and resize list
		if(_place)
		{
			_tree->showTreeItems(_name);
			_minus->setSkinComponent(".tree.minus");
		}
		else
		{
			_tree->hideTreeItems(_name);
			_minus->setSkinComponent(".tree.plus");
		}
		_minus->setSkin(mSkinName, true);

		// for resize scrollbar CHANGE IT !!!!!
		//_tree->getScrollPane()->manageWidgets();
		_tree->setSize(_tree->getSize());
	}

	void TreeItem::setThumbtails(const std::string& pic)
	{
		if (!_thumbtails)
		{
			Sheet *mSheet = _guiManager->getDefaultSheet();
			_thumbtails = mSheet->createImage();
		}
		_thumbtails->setMaterial(pic);
		_thumbtails->setSize(_IconSize, _IconSize);
		if (_thumbtails->getParentSheet())
			_thumbtails->getParentSheet()->removeChild(_thumbtails);
		_tree->addChild(_thumbtails);
		_thumbtails->setPosition(_item->getXPosition() + (1.5 * _IconSize) / 2, _item->getYPosition() + (_item->getHeight() - _IconSize)  / 2);
	}

	void TreeItem::setYPosition(double y)
	{
		_item->setYPosition(y);
		if (_minus)
			_minus->setYPosition(y);
		if (_thumbtails)
			_thumbtails->setYPosition(y);
	}

	void TreeItem::setVisible(bool vis)
	{
		_visible = vis;
		if (_visible)
		{
			_item->show();
			if (_minus)
				_minus->show();
			if (_thumbtails)
				_thumbtails->show();
		}
		else
		{
			_item->hide();
			if (_minus)
				_minus->hide();
			if (_thumbtails)
				_thumbtails->hide();
		}
	}

	/** Tree Constructor
	*/
	Tree::Tree(const std::string& name, QuickGUI::GUIManager* mGUIManager) 
		: List(name, mGUIManager), _guiManager(mGUIManager), _lastChild(NULL), _iconSize(15), _name(name)
	{
		_lists["root"] = NULL;
	}

	Tree::~Tree()
	{
		std::map<std::string, TreeItem*>::iterator itb = _lists.begin();
		std::map<std::string, TreeItem*>::iterator ite = _lists.end();

		for (; itb != ite; ++itb)
		{
			itb->second->setParent(NULL);
			// segfault when distroy ???I don't know why
			//delete itb->second;
		}
	}

	void Tree::deleteChild(TreeItem* me)
	{
		// delete from List and destroy all childs
		std::vector<TreeItem *>::iterator childItr;
		for (childItr = me->getChildren().begin(); childItr != me->getChildren().end(); ++childItr)
		{
			if (!(*childItr)->getChildren().empty())
			{
				deleteChild(*childItr);
				continue;
			}

			WidgetArray::iterator it = mItems.begin();
			while(it != mItems.end())
			{
				bool deleted = false;
				if ((*childItr) && (*it) == (*childItr)->getItem())
				{
					_lists[(*childItr)->getName()] = NULL;
					(*childItr)->setParent(NULL);
					delete (*childItr);
					//mGUIManager->destroyWidget((*it));
					it = mItems.erase(it);
					deleted = true;
				}
				if (!deleted && (*it)== me->getItem())
				{
					//mGUIManager->destroyWidget((*it));
					it = mItems.erase(it);
					deleted = true;
				}
				if (!deleted)
				++it;
			}
		}
		_lists[me->getName()] = NULL;
		me->setParent(NULL);
		delete me;
	}

	void Tree::deleteItem(const std::string& name)
	{
		TreeItem *me = _lists[name];
		TreeItem *prev;
		TreeItem *next;

		if (!me || me == _lists["root"])
			return;
	
		next = _lists[GetLastChild(name)]->getNext();
		prev = me->getPrevious();

		if (_lastChild == me)
			_lastChild = prev;

		if (next)
			next->setPrevious(prev);
		if (prev)
			prev->setNext(next);

		me->getParent()->removeChild(name);
		deleteChild(me);

		slideDown(prev);

		// resize scrollbar change IT !!!
		setSize(getSize());
	}

	void Tree::addInList(const std::string& name, const std::string& text, const std::string& parent, const std::string& LoadAfter)
	{
		if (!_lists["root"])
		{
			// create the root Item
			TreeItem *root = new TreeItem(_name, "qgui", this, _iconSize, mGUIManager);
			_lists["root"] = root;
			_lists[_name] = root;

			MenuLabel *item = addMenuLabel();
			root->setItem(item);
			item->setXPosition(_iconSize / 2);
			root->setLag(0);
			item->setInheritClippingWidget(true);
			item->setInheritOpacity(true);
			item->setInheritQuadLayer(true);
			item->setWidth(getWidth() - (item->getXPosition() + root->getLag()));
		}

		TreeItem* par = _lists[parent];

		if (!par)
			return;

		TreeItem* child = new TreeItem(name, mSkinName, this, _iconSize, _guiManager);
		_lists[name] = child;
		child->setLag(par->getLag() + 20);

		Image* img = par->addChild(child);
		child->setParent(par);

		MenuLabel* item = this->addMenuLabel();
		item->setText(text);
		
		item->setHorizontalAlignment(QuickGUI::Label::HA_LEFT);
		if(img)
		{
			if(img->getParentWidget())
				img->getParentWidget()->removeChild(img);
			addChild(img);
			img->setPosition(par->getItem()->getPosition().x, par->getItem()->getPosition().y);
			img->setSkin(mSkinName, true);
			img->appearOverWidget(this);
			img->setPosition(par->getItem()->getXPosition() - (_iconSize / 2), par->getItem()->getYPosition() + (par->getItem()->getHeight() - _iconSize) / 2);
		}

		child->setItem(item);

		if ((parent == "root" || _lists["root"]->getName() == parent) && LoadAfter.empty()) // if we to add this item under all other
		{
			item->setPosition(item->getPosition().x + child->getLag(), item->getPosition().y);
			child->setPrevious(_lastChild);
			if (_lastChild)
				_lastChild->setNext(child);
			_lastChild = child;
		}
		else if (!LoadAfter.empty())// If we add this item after an other
			addAfterTreeItem(LoadAfter, child);
		else // else we add this item at the bootom of parent
			addAfterTreeItem(GetLastChild(parent), child);
		if (item->getWidth() == getWidth())
			item->setWidth(getWidth() - (item->getXPosition() + child->getLag()));	
	}

	void Tree::hideTreeItems(const std::string& name)
	{
		TreeItem* child = _lists[name];

		if (!child)
			return;
		
		std::vector<TreeItem*>::iterator itb = child->getChildren().begin();
		std::vector<TreeItem*>::iterator ite = child->getChildren().end();
		
		for (; itb != ite; ++itb)
		{
			if ((*itb))
			{
				if ((*itb)->getNext())
				{
					(*itb)->getNext()->setYPosition((*itb)->getItem()->getYPosition());
					hideTreeItems((*itb)->getName());
				}
				(*itb)->setVisible(false);
				
				if ((itb+1) == ite)
					slideUp((*itb));
			}
		}
	}

	void Tree::showTreeItems(const std::string& name)
	{
		TreeItem *child = _lists[name];

		if (!child)
			return;
		
		std::vector<TreeItem *>::iterator itb = child->getChildren().begin();
		std::vector<TreeItem *>::iterator ite = child->getChildren().end();
		
		for (; itb != ite; ++itb)
		{
			if ((*itb))
			{ 
				if ((*itb)->getPrevious() && (*itb)->getParent() && !parentIsPlace((*itb)))
					(*itb)->setYPosition((*itb)->getPrevious()->getItem()->getYPosition());
				else
				{
					if ((*itb)->getPrevious())
						(*itb)->setYPosition((*itb)->getPrevious()->getItem()->getYPosition() + (*itb)->getPrevious()->getItem()->getHeight());

					(*itb)->setVisible(true);
				}
				showTreeItems((*itb)->getName());
				
				if ((itb+1) == ite)
					slideDown((*itb));
			}
		}
	}

	void Tree::slideUp(TreeItem* child)
	{
		if (child && child->getNext())
		{
			double y = child->getItem()->getYPosition();
			if (child->getVisible())
				y += child->getItem()->getHeight();
			child->getNext()->setYPosition(y);
			slideUp(child->getNext());
		}
	}

	void Tree::slideDown(TreeItem* child)
	{
		if (child && child->getPrevious())
		{
			double y = child->getPrevious()->getItem()->getYPosition();
			if (child->getVisible())
				y += child->getPrevious()->getItem()->getHeight();

			child->setYPosition(y);
			slideDown(child->getNext());
		}
	}

	void Tree::setTitle(const std::string& title)
	{
		if (!_lists["root"])
		{
			// create the root Item
			TreeItem *root = new TreeItem(_name, "qgui", this, _iconSize, mGUIManager);
			_lists["root"] = root;
			_lists[_name] = root;

			MenuLabel *item = addMenuLabel();
			root->setItem(item);
			item->setXPosition(_iconSize / 2);
			root->setLag(0);
			item->setInheritClippingWidget(true);
			item->setInheritOpacity(true);
			item->setInheritQuadLayer(true);
			item->setWidth(getWidth() - (item->getXPosition() + root->getLag()));
		}
		_lists["root"]->getItem()->setText(title);
		_lists["root"]->getItem()->setHorizontalAlignment(Label::HA_LEFT);
	}

	void Tree::addAfterTreeItem(const std::string& name, TreeItem* item)
	{
		TreeItem* before = _lists[name];

		if (!before)
			return;

		if (before->getNext())
			before->getNext()->setPrevious(item);
		else
			_lastChild = item;
		item->setPrevious(before);
		item->setNext(before->getNext());
		before->setNext(item);

		item->getItem()->setXPosition(item->getItem()->getXPosition() + item->getLag());
		slideDown(item);
	}

	std::string	Tree::GetLastChild(const std::string& name,  const std::string& parentName)
	{
		TreeItem *child = _lists[name];
		if (!child)
			return std::string(name);
		std::string	parent = parentName;

		if (parentName.empty())
			parent = name;

		while (child->getNext() && hasParent(child->getNext(), parent))
			child = child->getNext();
		return child->getName();
	}

	bool Tree::hasParent(TreeItem* child, const std::string& name)
	{
		TreeItem *par = child->getParent();
		while (par)
		{
			if (par->getName() == name)
				return true;
			par = par->getParent();
		}
		return false;
	}

	bool Tree::parentIsPlace(TreeItem* child)
	{
		for (TreeItem* parent = child->getParent(); parent; parent = parent->getParent())
			if (!parent->getPlace())
				return false;
		return true;
	}

	void Tree::setThumbtails(const std::string& name, const std::string& pic)
	{
		TreeItem* it = _lists[name];

		if (!it)
			return;

		it->setThumbtails(pic);
	}
} // namespace