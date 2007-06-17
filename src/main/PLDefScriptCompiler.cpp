/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/

// Inspired by Ogre's material compiler, parts of the code were taken from the Ogre projects
#include "PLDefScriptCompiler.h"
#include "OpdeException.h"
#include <OgreException.h>
#include <OgreStringConverter.h>
#include "OpdeServiceManager.h"
#include "logger.h"

using namespace std;
using namespace Ogre;

namespace Opde {

    //-----------------------------------------------------------------------
    // Static definitions
    //-----------------------------------------------------------------------
    PLDefScriptCompiler::TokenActionMap PLDefScriptCompiler::mTokenActionMap;

    String PLDefScriptCompiler::pldefScript_BNF =
		"<Script> ::= {<Script_Properties>} \n"
	
		"<Script_Properties> ::= <LinkDefinition> \n"
	
		"<LinkDefinition> ::= 'relation' <Unquoted_Label> '{' {<LinkParams>} '}' \n"
		
		"<LinkParams> ::= <DTypeName> | <QueryCache> | <LinkHidden> | <NoDType> \n"
	
		"<DTypeName> ::= 'dtype' <Flex_Label> \n"
		
		"<NoDType> ::= 'no_data' \n"
		
		"<QueryCache> ::= 'cache' <QueryCacheType>\n"
	
		"<QueryCacheType> ::= 'none' | 'all' | 'incoming' | 'outgoing' \n"
		
		"<LinkHidden> ::= 'hidden' \n" // So the link will not show up in editor or such
		
		// common rules
		"<Label> ::= <Quoted_Label> | <Unquoted_Label> \n"
		"<Flex_Label> ::= <Quoted_Label> | <Spaced_Label> \n"
		"<Quoted_Label> ::= -'\"' <Spaced_Label> -'\"' \n"
		"<Spaced_Label> ::= <Spaced_Label_Illegals> {<Spaced_Label_Illegals>} \n"
		
		"<Integer> ::= (0123456789) { (0123456789) } \n"
		
		"<Unquoted_Label> ::= <Unquoted_Label_Illegals> {<Unquoted_Label_Illegals>} \n"
			"<Spaced_Label_Illegals> ::= (!,:\n\r\t{}\"[]) \n"
			"<Unquoted_Label_Illegals> ::= (! :\n\r\t{}\"[]) \n"

        ;

	//-----------------------------------------------------------------------
	PLDefScriptCompiler::PLDefScriptCompiler(void) {
		// default to global definition group
		ServiceManager* svcmgr = ServiceManager::getSingletonPtr();
		
		mLinkService = svcmgr->getService("LinkService").as<LinkService>();
		mBinaryService = svcmgr->getService("BinaryService").as<BinaryService>();
		
		mCurrentState.state = CS_UNKNOWN;
	}
	
	//-----------------------------------------------------------------------
	PLDefScriptCompiler::~PLDefScriptCompiler(void) {

	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::setupTokenDefinitions(void) {
		addLexemeTokenAction("{", ID_OPENBRACE, &PLDefScriptCompiler::parseOpenBrace);
		addLexemeTokenAction("}", ID_CLOSEBRACE, &PLDefScriptCompiler::parseCloseBrace);
		
		addLexemeTokenAction("relation", ID_RELATION, &PLDefScriptCompiler::parseRelation);
		addLexemeTokenAction("dtype", ID_DTYPE, &PLDefScriptCompiler::parseDType);
		addLexemeTokenAction("cache", ID_CACHE, &PLDefScriptCompiler::parseCache);
		addLexemeTokenAction("no_data", ID_NO_DATA, &PLDefScriptCompiler::parseNoData);
		
	}

	//-----------------------------------------------------------------------
	size_t PLDefScriptCompiler::getAutoTokenIDStart() const {
		return ID_AUTOTOKEN;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::addLexemeTokenAction(const String& lexeme, const size_t token, const PLDS_Action action) {
		addLexemeToken(lexeme, token, action != 0);
		mTokenActionMap[token] = action;
	}

	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::executeTokenAction(const size_t tokenID) {
		TokenActionIterator action = mTokenActionMap.find(tokenID);

		if (action == mTokenActionMap.end()) {
			// BAD command. BAD!
			logParseError("Unrecognised PLDef Script command action");
			return;
		} else {
			try {
				(this->*action->second)();
			} catch (Exception& ogreException) {
				// an unknown token found or BNF Grammer rule was not successful
				// in finding a valid terminal token to complete the rule expression.
				logParseError(ogreException.getDescription());
			} catch (BasicException& ogreException) {
				logParseError(ogreException.getDetails());
			}
		}
	}

	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::logParseError(const String& error) {
		// LOG_ERROR("Error in script at line %d  of %s (last context: %s) : %s", mCurrentLine, mSourceName.c_str(), mCurrentState.name.c_str(), error.c_str());
		
		OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,
                    "Error in script at line " + StringConverter::toString(mCurrentLine) +
			" of " + mSourceName + ": " + error, "DTypeScriptCompiler::logParseError");
	}
    
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseOpenBrace(void) {

	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseCloseBrace(void) {
		if (mCurrentState.state == CS_RELATION) {
			DTypeDefPtr dt;
			
			if (!mCurrentState.no_data) // if data definition is requested
				dt = getTypeDef("links", mCurrentState.dtypename); // TODO: Hardcoded. A problem?
			
			mLinkService->createRelation(mCurrentState.name, dt, mCurrentState.hidden);
		}
		
		
		mCurrentState.state = CS_UNKNOWN;
	}

	
	//-----------------------------------------------------------------------
	DTypeDefPtr PLDefScriptCompiler::getTypeDef(const std::string& group, const std::string& name) {
		try {
			return mBinaryService->getType(group, name);
		} catch (BasicException& ex) {
			try {
				if (group != "") {
					return mBinaryService->getType("", name);
				} else {
					logParseError("Type definition not found : '" + name + "'");
				}
			} catch (BasicException& ex) {
				logParseError("Type definition not found : '" + name +  "'");
			}
		}
	}
	
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseRelation(void) {
		// Next token contains the relation name
		mCurrentState.state = CS_RELATION;
		mCurrentState.name = getNextTokenLabel();
		mCurrentState.dtypename = mCurrentState.name; // default
		mCurrentState.hidden = false;
		mCurrentState.no_data = false;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseDType(void) {
		// Next token contains the Dtype name
		mCurrentState.dtypename = getNextTokenLabel();
	}
		
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseCache(void) {
		mCurrentState.cachetype = getNextTokenLabel();
	}
		
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseHidden(void) {
		mCurrentState.hidden = false;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseNoData(void) {
		mCurrentState.no_data = true;
	}

	
}
