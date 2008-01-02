//This code is public domain - you can do whatever you want with it
//Original author: John Judnich
#include "QuickGUIPrecompiledHeaders.h"

#include "OgreScriptLoader.h"

#include "OgreResourceGroupManager.h"
#include "OgreException.h"
#include "OgreLogManager.h"

#include "QuickGUISkinSetManager.h"

#include "QuickGUIConfigScriptParser.h"

using namespace Ogre;
namespace QuickGUI
{
	void registerScriptParser()
	{
		// calling getSingleton will instantiate the class, 
		// causing it to register itself
		ConfigScriptLoader::getSingleton();
	}

	ConfigScriptLoader::ConfigScriptLoader()
	{
		//Register as a ScriptLoader
		mLoadOrder = 100.0f;
		mScriptPatterns.push_back("*.skinset");
		//mScriptPatterns.push_back("*.sheet");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);
	}

	ConfigScriptLoader::~ConfigScriptLoader()
	{
		//Delete all scripts
		HashMap<String, ConfigNode*>::iterator i;
		for (i = scriptList.begin(); i != scriptList.end(); i++){
			delete i->second;
		}
		scriptList.clear();

		//Unregister with resource group manager
		if (ResourceGroupManager::getSingletonPtr())
			ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
	}

	Real ConfigScriptLoader::getLoadingOrder() const
	{
		return mLoadOrder;
	}

	const StringVector &ConfigScriptLoader::getScriptPatterns() const
	{
		return mScriptPatterns;
	}

	ConfigNode *ConfigScriptLoader::getConfigScript(const String &type, const String &name)
	{
		HashMap<String, ConfigNode*>::iterator i;

		String key = type + ' ' + name;
		i = scriptList.find(key);

		//If found..
		if (i != scriptList.end())
			return i->second;
		else
			return NULL;
	}

	void ConfigScriptLoader::parseScript(DataStreamPtr &stream, const String &groupName)
	{
		//Copy the entire file into a buffer for fast access
		parseBuffLen = stream->size();
		parseBuff = new char[parseBuffLen];
		buffPtr = parseBuff;
		stream->read(parseBuff, parseBuffLen);
		parseBuffEnd = parseBuff + parseBuffLen;

		//Close the stream (it's no longer needed since everything is in parseBuff)
		stream->close();

		//Get first token
		_nextToken();
		if (tok == TOKEN_EOF)
		{
			//Delete the buffer
			delete[] parseBuff;
			return;
		}

		//Parse the script
		_parseNodes(0);

		if (tok == TOKEN_CLOSEBRACE)
		{
			//Delete the buffer
			delete[] parseBuff;
			OGRE_EXCEPT(3, "Parse Error: Closing brace out of place", "ConfigScript::load()");
		}

		//Delete the buffer
		delete[] parseBuff;
	}


	void ConfigScriptLoader::_nextToken()
	{
		lastTok = tok;
		lastTokVal = tokVal;
		lastTokPos = buffPtr;

		//EOF token
		if (buffPtr >= parseBuffEnd){
			tok = TOKEN_EOF;
			return;
		}

		//(Get next character)
		int ch = *buffPtr++;
		while (ch == ' ' || ch == 9){	//Skip leading spaces / tabs
			ch = *buffPtr++;
		}

		//Newline token
		if (ch == '\r' || ch == '\n'){
			do {
				ch = *buffPtr++;
			} while (ch == '\r' || ch == '\n');
			buffPtr--;

			tok = TOKEN_NEWLINE;
			return;
		}

		//Open brace token
		else if (ch == '{'){
			tok = TOKEN_OPENBRACE;
			return;
		}

		//Close brace token
		else if (ch == '}'){
			tok = TOKEN_CLOSEBRACE;
			return;
		}

		//Text token
		if (ch < 32 || ch > 122)	//Verify valid char
			OGRE_EXCEPT(4, "Parse Error: Invalid character", "ConfigScript::load()");

		tokVal = "";
		tok = TOKEN_TEXT;
		do {
			//Skip comments
			if (ch == '/'){
				int ch2 = *buffPtr;

				//C++ style comment (//)
				if (ch2 == '/'){
					buffPtr++;
					do {
						ch = *buffPtr++;
					} while (ch != '\r' && ch != '\n' && buffPtr < parseBuffEnd);

					tok = TOKEN_NEWLINE;
					return;
				}
			}

			//Add valid char to tokVal
			tokVal += ch;

			//Next char
			ch = *buffPtr++;
		} while (ch > 32 && ch <= 122 && buffPtr < parseBuffEnd);

		return;
	}

	void ConfigScriptLoader::_prevToken()
	{
		tok = lastTok;
		tokVal = lastTokVal;
		buffPtr = lastTokPos;
	}

	void ConfigScriptLoader::_parseNodes(ConfigNode *parent)
	{
		typedef std::pair<String, ConfigNode*> ScriptItem;

		while (1) {
			switch (tok){
				//Node
			case TOKEN_TEXT:
				//Add the new node
				ConfigNode *newNode;
				if (parent)
					newNode = parent->addChild(tokVal);
				else
					newNode = new ConfigNode(0, tokVal);

				//Get values
				_nextToken();
				while (tok == TOKEN_TEXT){
					newNode->addValue(tokVal);
					_nextToken();
				}

				//Add root nodes to scriptList
				if (!parent){
					String key;

					if (newNode->getValues().empty())
						key = newNode->getName() + ' ';
					else
						key = newNode->getName() + ' ' + newNode->getValues().front();

					scriptList.insert(ScriptItem(key, newNode));
				}

				//Skip any blank spaces
				while (tok == TOKEN_NEWLINE)
					_nextToken();

				//Add any sub-nodes
				if (tok == TOKEN_OPENBRACE){
					//Parse nodes
					_nextToken();
					_parseNodes(newNode);

					//Skip blank spaces
					while (tok == TOKEN_NEWLINE)
						_nextToken();

					//Check for matching closing brace
					if (tok != TOKEN_CLOSEBRACE)
						OGRE_EXCEPT(1, "Parse Error: Expecting closing brace", "ConfigScript::load()");
				} else {
					//If it's not a opening brace, back up so the system will parse it properly
					_prevToken();
				}

				break;

				//Out of place brace
			case TOKEN_OPENBRACE:
				OGRE_EXCEPT(2, "Parse Error: Opening brace out of plane", "ConfigScript::load()");
				break;

				//Return if end of nodes have been reached
			case TOKEN_CLOSEBRACE:
				return;

				//Return if reached end of file
			case TOKEN_EOF:
				return;
			}

			//Next token
			_nextToken();
		};
	}





	ConfigNode::ConfigNode(ConfigNode *parent, const String &name)
	{
		ConfigNode::name = name;
		ConfigNode::parent = parent;
		_removeSelf = true;	//For proper destruction

		//Add self to parent's child list (unless this is the root node being created)
		if (parent != NULL){
			parent->children.push_back(this);
		}
	}

	ConfigNode::~ConfigNode()
	{
		//Delete all children
		std::vector<ConfigNode*>::iterator i;
		for (i = children.begin(); i != children.end(); i++){
			ConfigNode *node = *i;
			node->_removeSelf = false;
			delete node;
		}
		children.clear();

		//Remove self from parent's child list
		if (_removeSelf && parent != NULL)
		{
			for(std::vector<ConfigNode*>::iterator it = parent->children.begin(); it != parent->children.end(); ++it) 
			{ 
				if((*it) == this) 
				{ 
					parent->children.erase(it); 
					break; 
				} 
			}
		}
	}

	ConfigNode *ConfigNode::addChild(const String &name)
	{
		return new ConfigNode(this, name);
	}

	ConfigNode *ConfigNode::findChild(const String &name, bool recursive)
	{
		std::vector<ConfigNode*>::iterator i;

		//Search for node
		for (i = children.begin(); i != children.end(); i++){
			ConfigNode *node = *i;
			if (node->name == name)
				return node;
		}

		//If not found, search child nodes (if recursive == true)
		if (recursive){
			for (i = children.begin(); i != children.end(); i++){
				(*i)->findChild(name, recursive);
			}
		}

		//Not found anywhere
		return NULL;
	}

	void ConfigNode::setParent(ConfigNode *newParent)
	{
		//Remove self from current parent
		for(std::vector<ConfigNode*>::iterator it = parent->children.begin(); it != parent->children.end(); ++it) 
		{ 
			if((*it) == this) 
			{ 
				parent->children.erase(it); 
				break; 
			} 
		}

		//Set new parent
		parent = newParent;

		//Add self to new parent
		parent->children.push_back(this);
	}


	//-----------------------------------------------------------------------
	void ConfigScriptSerializer::beginSection(const unsigned short level)
	{
		String& buffer = mBuffer;
		buffer += "\n";
		for (unsigned short i = 0; i < level; ++i)
		{
			buffer += "\t";
		}
		buffer += "{";
	}
	//-----------------------------------------------------------------------
	void ConfigScriptSerializer::endSection(const unsigned short level)
	{
		String& buffer = mBuffer;
		buffer += "\n";
		for (unsigned short i = 0; i < level; ++i)
		{
			buffer += "\t";
		}
		buffer += "}";
	}
	//-----------------------------------------------------------------------
	void ConfigScriptSerializer::writeAttribute(const unsigned short level, const String& att)
	{
		String& buffer = mBuffer;
		buffer += "\n";
		for (unsigned short i = 0; i < level; ++i)
		{
			buffer += "\t";
		}
		buffer += att;
	}

	//-----------------------------------------------------------------------
	void ConfigScriptSerializer::writeValue(const String& val)
	{
		String& buffer = mBuffer;
		buffer += (" " + val);
	}

	//-----------------------------------------------------------------------
	void ConfigScriptSerializer::clearQueue()
	{
		mBuffer.clear();
	}
    //-----------------------------------------------------------------------
    void ConfigScriptSerializer::exportQueued(const String &fileName)
    {
        if (mBuffer.empty())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Queue is empty !", "ConfigScriptSerializer::exportQueued");

		Ogre::LogManager::getSingleton().logMessage("QuickGui ConfigScriptSerializer : saving to" + fileName, LML_CRITICAL);
        FILE *fp;
        fp = fopen(fileName.c_str(), "w");
        if (!fp)
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create QuickGUI file.",
            "ConfigScriptSerializer::exportQueued");

        // output main buffer holding material script
        fputs(mBuffer.c_str(), fp);
        fclose(fp);

        LogManager::getSingleton().logMessage("ConfigScriptSerializer : done.", LML_CRITICAL);
        clearQueue();
    }
}
