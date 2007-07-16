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
	
		"<Script_Properties> ::= <LinkDefinition> | <PropertyDefinition> | <DChunkVersion> | <LChunkVersion> | <RelChunkVersion> | <PChunkVersion> \n" // so I can define default version for the chunks
	
		// Link definition part
		"<LinkDefinition> ::= 'relation' <Label> '{' {<LinkParams>} '}' \n"
		
		"<LinkParams> ::= <DTypeName> | <FakeSize> | <LinkHidden> | <NoDType> | <DChunkVersion> | <LChunkVersion> \n"
	
		"<DTypeName> ::= 'dtype' <Flex_Label> \n"
		
		"<NoDType> ::= 'no_data' \n"
		
		"<FakeSize> ::= 'fake_size' <Integer> \n" // if dark reports a different size, report that one
		
		"<LinkHidden> ::= 'hidden' \n" 
		
		"<DChunkVersion> ::= 'd_ver' <Version> \n"
		"<LChunkVersion> ::= 'l_ver' <Version> \n"
		    
		"<RelChunkVersion> ::= 'rel_ver' <Version> \n" // version of the relations chunk
		    
		// Property definition part
		"<PropertyDefinition> ::= 'property' <Label> '{' {<PropertyParams>} '}' \n"
		
		"<PropertyParams> ::= <DTypeName> | <PropLabel> | <PChunkVersion> | <PInheritor> \n"
		    
		"<PropLabel> ::= 'label' <Quoted_Label> \n"
		    
		"<PInheritor> ::= 'inherit' <Unquoted_Label> \n"

		"<PChunkVersion> ::= 'p_ver' <Version> \n" // property chunk version
		    
		// common rules
		"<Version> ::= <Integer> '.' <Integer> \n"
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
		mPropertyService = svcmgr->getService("PropertyService").as<PropertyService>();		
		
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
		addLexemeTokenAction("fake_size", ID_FAKESIZE, &PLDefScriptCompiler::parseFakeSize);
		addLexemeTokenAction("no_data", ID_NO_DATA, &PLDefScriptCompiler::parseNoData);
		
		addLexemeTokenAction("d_ver", ID_D_VER, &PLDefScriptCompiler::parseVersion);
		addLexemeTokenAction("l_ver", ID_L_VER, &PLDefScriptCompiler::parseVersion);
		
		addLexemeTokenAction("rel_ver", ID_REL_VER, &PLDefScriptCompiler::parseVersion);
		
		addLexemeTokenAction("property", ID_PROPERTY, &PLDefScriptCompiler::parseProperty);
		
		addLexemeTokenAction("p_ver", ID_P_VER, &PLDefScriptCompiler::parseVersion);
		
		addLexemeTokenAction("label", ID_LABEL, &PLDefScriptCompiler::parseLabel);
		
		addLexemeTokenAction("inherit", ID_INHERIT, &PLDefScriptCompiler::parseInherit);
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
			
			RelationPtr rel = mLinkService->createRelation(mCurrentState.name, dt, mCurrentState.hidden);
			
			if (mCurrentState.fake_size >= 0)
				rel->setFakeSize(mCurrentState.fake_size);
			
			rel->setChunkVersions(	mCurrentState.lmajor, 
					     	mCurrentState.lminor, 
					     	mCurrentState.dmajor, 
					     	mCurrentState.dminor
					     );
		} else if (mCurrentState.state == CS_PROPERTY) {
			DTypeDefPtr dt;
			
			dt = getTypeDef("properties", mCurrentState.dtypename);
			
			if (mCurrentState.label == "") // default to property chunk name silently
				mCurrentState.label = mCurrentState.name;
			
			PropertyGroupPtr rel = mPropertyService->createPropertyGroup(mCurrentState.label, mCurrentState.name, dt, mCurrentState.pmajor, mCurrentState.pminor);
		} else {
			logParseError("Closing an unknown state!");
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
		mCurrentState.fake_size = -1; // no fake size
		mCurrentState.dmajor = mDefaultDMajor; // fill in the default versions
		mCurrentState.dminor = mDefaultDMinor;
		mCurrentState.lmajor = mDefaultLMajor;
		mCurrentState.lminor = mDefaultLMinor;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseProperty(void) {
		// Next token contains the relation name
		mCurrentState.state = CS_PROPERTY;
		mCurrentState.name = getNextTokenLabel();
		mCurrentState.label = "";
		mCurrentState.dtypename = mCurrentState.name; // default
		mCurrentState.inherit = "always";
		mCurrentState.pmajor = mDefaultPMajor; // fill in the default versions
		mCurrentState.pminor = mDefaultPMinor;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseDType(void) {
		// Next token contains the Dtype name
		mCurrentState.dtypename = getNextTokenLabel();
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseLabel(void) {
		mCurrentState.label = getNextTokenLabel();
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseInherit(void) {
		mCurrentState.inherit = getNextTokenLabel();
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseFakeSize(void) {
		String slen = getNextTokenLabel();
		
		// convert to int
		int fake_size = StringConverter::parseLong(slen);
			
		if (fake_size == 0)
			logParseError("Zero or invalid fake_size : " + slen);
		
		mCurrentState.fake_size = fake_size;
	}
		
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseHidden(void) {
		mCurrentState.hidden = false;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseNoData(void) {
		mCurrentState.no_data = true;
	}
	
	//-----------------------------------------------------------------------
	void PLDefScriptCompiler::parseVersion(void) {
		int vid = getCurrentTokenID();
		 ID_D_VER ? true : false;
		
		// now read next two tokens, and set the resulting version numbers
		String svmaj = getNextTokenLabel();
		
		getNextToken(); // skip the dot
		
		String svmin = getNextTokenLabel();
		
		// convert to int
		uint maj = StringConverter::parseLong(svmaj);
		uint min = StringConverter::parseLong(svmin);
		
		if (mCurrentState.state == CS_RELATION) { 
			// fill the current state only
			if (vid == ID_D_VER) {
				mCurrentState.dmajor = maj;
				mCurrentState.dminor = min;
			} else if (vid == ID_L_VER) {
				mCurrentState.lmajor = maj;
				mCurrentState.lminor = min;
			} else if (vid == ID_P_VER) {
				mCurrentState.pmajor = maj;
				mCurrentState.pminor = min;
			} else {
				logParseError("Only data/link/property versions are allowed while in property/relation definition");
			}
			
		} else { // global default
			if (vid == ID_D_VER) {
				mDefaultLMajor = maj;
				mDefaultLMinor = min;
			} else if (vid == ID_L_VER) {
				mDefaultDMajor = maj;
				mDefaultDMinor = min;
			} else if (vid == ID_REL_VER) {
				// Just call version setting on the LinkService
				mLinkService->setChunkVersion(maj, min);
			} else if (vid == ID_P_VER) {
				mDefaultPMajor = maj;
				mDefaultPMinor = maj;
			}
		}
	}

	
}
