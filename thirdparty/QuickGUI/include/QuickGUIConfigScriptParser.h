//This code is public domain - you can do whatever you want with it
//Original author: John Judnich
#pragma once
#ifndef __QuickGuiSkinSetParser_h_
#define __QuickGuiSkinSetParser_h_

#include "OgreScriptLoader.h"
#include "OgrePrerequisites.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	class ConfigNode;

	// necessary to make use of QuickGUI's script parser.
	void _QuickGUIExport registerScriptParser();

	class _QuickGUIExport ConfigScriptLoader: public Ogre::ScriptLoader
	{
	public:
		static ConfigScriptLoader& getSingleton() 
		{
			static ConfigScriptLoader theLog;
			return theLog;
		}

		inline static ConfigScriptLoader *getSingletonPtr() { return &getSingleton(); }

		Ogre::Real getLoadingOrder() const;
		const Ogre::StringVector &getScriptPatterns() const;

		ConfigNode *getConfigScript(const Ogre::String &type, const Ogre::String &name);

		void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &groupName);

	private:
		ConfigScriptLoader();   // ctor is hidden
		ConfigScriptLoader(ConfigScriptLoader const&);	// copy ctor is hidden
		ConfigScriptLoader& operator=(ConfigScriptLoader const&);	// assign op is hidden
		~ConfigScriptLoader();	// dtor is hidden

		Ogre::Real mLoadOrder;
		Ogre::StringVector mScriptPatterns;

		HashMap<Ogre::String, ConfigNode*> scriptList;

		//Parsing
		char *parseBuff, *parseBuffEnd, *buffPtr;
		size_t parseBuffLen;

		enum Token
		{
			TOKEN_TEXT,
			TOKEN_NEWLINE,
			TOKEN_OPENBRACE,
			TOKEN_CLOSEBRACE,
			TOKEN_EOF,
		};

		Token tok, lastTok;
		Ogre::String tokVal, lastTokVal;
		char *lastTokPos;

		void _parseNodes(ConfigNode *parent);
		void _nextToken();
		void _prevToken();
	};


	class _QuickGUIExport ConfigNode
	{
	public:
		ConfigNode(ConfigNode *parent, const Ogre::String &name = "untitled");
		~ConfigNode();

		inline void setName(const Ogre::String &name)
		{
			this->name = name;
		}

		inline Ogre::String &getName()
		{
			return name;
		}

		inline void addValue(const Ogre::String &value)
		{
			values.push_back(value);
		}

		inline void clearValues()
		{
			values.clear();
		}

		inline Ogre::StringVector &getValues()
		{
			return values;
		}

		ConfigNode *addChild(const Ogre::String &name = "untitled");
		ConfigNode *findChild(const Ogre::String &name, bool recursive = false);

		inline std::vector<ConfigNode*> &getChildren()
		{
			return children;
		}

		void setParent(ConfigNode *newParent);

		inline ConfigNode *getParent()
		{
			return parent;
		}

	private:
		Ogre::String name;
		Ogre::StringVector values;
		std::vector<ConfigNode*> children;
		ConfigNode *parent;

		bool _removeSelf;
	};


	class _QuickGUIExport ConfigScriptSerializer
	{
	public:
		void beginSection(const unsigned short level);
		void endSection(const unsigned short level);
		void writeAttribute(const unsigned short level, const Ogre::String& att);
		void writeValue(const Ogre::String& val);
		void clearQueue();
		void exportQueued(const Ogre::String &fileName);

	private:
		Ogre::String mBuffer;
	};
}

#endif //__QuickGuiSkinSetParser_h_
