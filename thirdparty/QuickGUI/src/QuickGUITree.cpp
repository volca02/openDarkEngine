#include "QuickGUIPrecompiledHeaders.h"
#include "QuickGUIManager.h"
#include "QuickGUITree.h"

namespace QuickGUI
{
	/**	TreeItem Constructor
	*/
	TreeItem::TreeItem(const std::string &name, const std::string &id, const std::string &skinName, Tree *tree, double iconsize, GUIManager *mGUIManager):
		_item(NULL), _parent(NULL), _minus(NULL) , _place(true), mSkinName(skinName),
		_name(name), _id(id), _tree(tree), _next(NULL), _previous(NULL), _visible(true), _thumbtails(NULL), _down(false),
		_IconSize(iconsize), _guiManager(mGUIManager)
	{
		_child.clear();		
	}

	void TreeItem::setItem(MenuLabel *item)
	{
		_item = item; 
		//effects
		_item->addEventHandler(Widget::EVENT_MOUSE_ENTER,&TreeItem::onMouseEntersItem,this);
		_item->addEventHandler(Widget::EVENT_MOUSE_LEAVE,&TreeItem::onMouseLeavesItem,this);
		_item->addEventHandler(Widget::EVENT_MOUSE_BUTTON_DOWN,&TreeItem::onMouseButtonDownItem,this);
		_item->addEventHandler(Widget::EVENT_MOUSE_CLICK_DOUBLE, &TreeItem::onMouseClickDouble, this);
	}
	void TreeItem::onMouseEntersItem(const EventArgs& args)
	{
		if (_down)
			return;
		this->getItem()->setSkinComponent(".tree.button.over");
		this->getItem()->setSkin(_item->getSkin());
	}
	void TreeItem::onMouseLeavesItem(const EventArgs& args)
	{
		if (_down)
			return;
		this->getItem()->setSkinComponent(".tree.button");
		this->getItem()->setSkin(_item->getSkin());
	}
	void	TreeItem::onMouseButtonDownItem(const EventArgs& args)
	{
		_down = true;
		this->getItem()->setSkinComponent(".tree.button.down");
		this->getItem()->setSkin(_item->getSkin());
		_tree->onItemPushDown(_id);
	}

	void	TreeItem::onMouseButtonDownItemLeave()
	{
		_down = false;
		this->getItem()->setSkinComponent(".tree.button");
		this->getItem()->setSkin(_item->getSkin());
	}

	void	TreeItem::onMouseClickDouble(const EventArgs& args)
	{
		_place = !_place;
		if(_place)
			_tree->open(_id);
		else
			_tree->close(_id);
	}

	void	TreeItem::onMouseButtonDown(const EventArgs& args)
	{
		_place = !_place;

		if(_place)
			_tree->open(_id);
		else
			_tree->close(_id);
	}

	TreeItem::~TreeItem()
	{
		//delete from parent child's list
		if (_parent)
			_parent->removeChild(_id);

		setVisible(false);
		_tree->removeAndDestroyChild(_thumbtails);
		_tree->removeAndDestroyChild(_minus);
	}

	std::vector<TreeItem *>	&TreeItem::getChilds()
	{
		return _child;
	}

	void					TreeItem::removeChild(const std::string &id)
	{
		std::vector<TreeItem *>::iterator itb = _child.begin();
		for(; itb != _child.end(); ++itb)
			if ((*itb)->getId() == id)
			{
				_child.erase(itb);
				break;
			}
	}
	QuickGUI::Image			*TreeItem::addChild(TreeItem *Item)
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

	void					TreeItem::setThumbtails(const std::string &pic)
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
	void					TreeItem::setYPosition(double y)
	{
		_item->setYPosition(y);
		if (_minus)
			_minus->setYPosition(y);
		if (_thumbtails)
			_thumbtails->setYPosition(y);
	}

	void					TreeItem::setVisible(bool vis)
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
	Tree::Tree(const std::string &name, QuickGUI::GUIManager *mGUIManager, double padding) 
		: List(name,mGUIManager), _guiManager(mGUIManager), _lastChild(NULL), _iconSize(15), _name(name), _selected(NULL), _padding(padding)
	{
		// create the root Item
		TreeItem *root = new TreeItem("root", "root", "qgui", this, 0, mGUIManager);
		_lists["root"] = root;

		MenuLabel *item = addMenuLabel();
		root->setItem(item);
		item->setXPosition(0);
		item->setYPosition(- item->getHeight());
		root->setLag(-_padding / 2);
	}

	Tree::~Tree()
	{
		std::map<std::string, TreeItem *>::iterator itb = _lists.begin();
		std::map<std::string, TreeItem *>::iterator ite = _lists.end();

		for (; itb != ite; ++itb)
			delete itb->second;
	}

	void Tree::deleteChild(TreeItem *me)
	{
		// delete from List and destroy all childs
		std::vector<TreeItem *>::iterator childItr;
		for (childItr = me->getChilds().begin(); childItr != me->getChilds().end(); ++childItr)
		{
			if (!(*childItr)->getChilds().empty())
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
					_lists[(*childItr)->getId()] = NULL;
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
		_lists[me->getId()] = NULL;
		me->setParent(NULL);
		delete me;
	}

	void Tree::deleteItem(const std::string &id)
	{
		TreeItem *me = _lists[id];
		TreeItem *prev;
		TreeItem *next;

		if (!me || me == _lists["root"])
			return;
	
		next = _lists[GetLastChild(id)]->getNext();
		prev = me->getPrevious();

		if (_lastChild == me)
			_lastChild = prev;

		if (next)
			next->setPrevious(prev);
		if (prev)
			prev->setNext(next);

		me->getParent()->removeChild(id);
		deleteChild(me);

		slideDown(prev);

		resize();
	}

	const std::string Tree::addInList(const std::string &name, const std::string &text, const std::string &parent, const std::string &LoadAfter)
	{
		TreeItem *par = _lists[parent];

		if (!par)
			return "";
		
		// check for unique id; and create Item menulabel
		std::string id = createId(name);
		TreeItem *child = new TreeItem(name, id, mSkinName, this, _iconSize, _guiManager);
		_lists[id] = child;
		child->setLag(par->getLag() + 20);
		child->setParent(par);
		MenuLabel *item = this->addMenuLabel();
		item->setText(text);
		item->setHorizontalAlignment(QuickGUI::Label::HA_LEFT);
		child->setItem(item);

		// setImage + -
		Image *img = par->addChild(child);
		if (img)
		{
			if (img->getParentWidget())
				img->getParentWidget()->removeChild(img);
			addChild(img);
			img->setPosition(par->getItem()->getPosition().x, par->getItem()->getPosition().y);
			img->setSkin(mSkinName, true);
			img->appearOverWidget(this);
			img->setPosition(par->getItem()->getXPosition() - (_iconSize / 2), par->getItem()->getYPosition() + (par->getItem()->getHeight() - _iconSize) / 2);
		}

		
		if ((parent == "root" || _lists["root"]->getId() == parent) && LoadAfter.empty()) // if we to add this item under all other
		{
			item->setXPosition(item->getPosition().x + child->getLag());
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


		//look if parent is visible

		if (child->getVisible() && !par->getVisible())
		{
			child->setVisible(false);
			return id;
		}

		//Now we will check if Item is in good place on Y
		if (!child->getPrevious())
			return id;

		// try to find the last previous visible item;
		TreeItem *previousVisible;
		for (previousVisible = child->getPrevious(); previousVisible && !previousVisible->getVisible(); previousVisible = previousVisible->getPrevious())
			;
		if (previousVisible && previousVisible->getVisible())
			child->setYPosition(previousVisible->getItem()->getYPosition() + child->getItem()->getHeight());
		return id;
	}

	std::string	Tree::createId(const std::string& name)
	{
		if (_lists.find(name) == _lists.end())
			return name;
		std::string id = name + "_0";
		for (unsigned int i = 1; _lists.find(id) != _lists.end(); ++i)
		{
			id = name + "_";
			std::ostringstream	os;
			os << i;
			id += os.str();
		}
		return id;
	}

	void Tree::hideTreeItems(const std::string &id)
	{
		TreeItem *child = _lists[id];

		if (!child)
			return;
		
		std::vector<TreeItem *>::iterator itb = child->getChilds().begin();
		std::vector<TreeItem *>::iterator ite = child->getChilds().end();
		
		for (; itb != ite; ++itb)
			if ((*itb))
			{
				if ((*itb)->getNext())
				{
					(*itb)->getNext()->setYPosition((*itb)->getItem()->getYPosition());
					hideTreeItems((*itb)->getId());
				}
				(*itb)->setVisible(false);
				
				if ((*itb)->getName() == "load_intro")
					std::cout << "fait ?\n";

				if ((itb+1) == ite)
					slideUp((*itb));
			}
	}

	void Tree::showTreeItems(const std::string &id)
	{
		TreeItem *child = _lists[id];

		if (!child)
			return;
		
		std::vector<TreeItem *>::iterator itb = child->getChilds().begin();
		std::vector<TreeItem *>::iterator ite = child->getChilds().end();
		
		for (; itb != ite; ++itb)
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
				showTreeItems((*itb)->getId());
				
				if ((itb+1) == ite)
					slideDown((*itb));
			}
	}


	void Tree::slideUp(TreeItem *child)
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

	void Tree::slideDown(TreeItem *child)
	{
		// try to find the last previous visible item;
		TreeItem *previousVisible;
		TreeItem *children;
		for (previousVisible = child->getPrevious(); previousVisible && !previousVisible->getVisible(); previousVisible = previousVisible->getPrevious())
			;
		if (!previousVisible)
			previousVisible = _lists["root"];

		for (children = child; children; children = children->getNext())
		{
			double y = previousVisible->getItem()->getYPosition();
			if (children->getVisible())
			{
				y += previousVisible->getItem()->getHeight();
				children->setYPosition(y);
				previousVisible = children;
			}
		}
	}

	void Tree::addAfterTreeItem(const std::string &id, TreeItem *item)
	{
		TreeItem *before = _lists[id];

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

	std::string	Tree::GetLastChild(const std::string &id,  const std::string &parentId)
	{
		TreeItem *child = _lists[id];
		if (!child)
			return std::string(id);
		std::string	parent = parentId;

		if (parentId.empty())
			parent = id;

		while (child->getNext() && hasParent(child->getNext(), parent))
			child = child->getNext();
		return child->getId();
	}
	bool Tree::hasParent(TreeItem *child, const std::string &id)
	{
		TreeItem *par = child->getParent();
		while (par)
		{
			if (par->getId() == id)
				return true;
			par = par->getParent();
		}
		return false;
	}
	bool Tree::parentIsPlace(TreeItem *child)
	{
		for (TreeItem *parent = child->getParent(); parent; parent = parent->getParent())
			if (!parent->getPlace())
				return false;
		return true;
	}
	void Tree::setThumbtails(const std::string &id, const std::string &pic)
	{
		TreeItem *it = _lists[id];

		if (!it)
			return;

		it->setThumbtails(pic);
	}

	void Tree::onItemPushDown(const std::string &id)
	{
		if (_selected && _selected != _lists[id])
			_selected->onMouseButtonDownItemLeave();
		_selected = _lists[id];
	}

	const std::string Tree::GetSelectedName()
	{
		if (_selected)
			return _selected->getName();
		return "";
	}
	const std::string Tree::GetSelectedID()
	{
		if (_selected)
			return _selected->getId();
		return "";
	}
	void	Tree::close(const std::string &id)
	{
		TreeItem *item = _lists[id];
		if (!item || id == "root")
			return;
		if (item->getChilds().empty())
			return;
		
		item->setPlace(false);
		hideTreeItems(id);
		QuickGUI::Image * img = item->getImage();
		img->setSkinComponent(".tree.plus");
		img->setSkin(mSkinName, true);

		resize();
	}

	void	Tree::open(const std::string &id)
	{
		TreeItem *item = _lists[id];
		if (!item || id == "root")
			return;
		if (item->getChilds().empty())
			return;
		
		item->setPlace(true);
		showTreeItems(id);
		QuickGUI::Image * img = item->getImage();
		img->setSkinComponent(".tree.minus");
		img->setSkin(mSkinName, true);
		resize();
	}

	void		Tree::resize()
	{
		// for resize scrollbar CHANGE IT !!!!!
		//getScrollPane()->manageWidgets();
		setSize(getSize());
	}
} // namespace
