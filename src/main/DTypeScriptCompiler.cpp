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
#include "DTypeScriptCompiler.h"
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
    DTypeScriptCompiler::TokenActionMap DTypeScriptCompiler::mTokenActionMap;

    String DTypeScriptCompiler::dtypeScript_BNF =
		"<Script> ::= {<Script_Properties>} \n"
	
		"<Script_Properties> ::= <NameSpace> | <Definitions> \n"
	
		"<Definitions> ::= <Enum> | <Bitfield> | <Alias> | <Struct> | <Field>\n"
		
		"<NameSpace> ::= 'namespace' <Unquoted_Label> '{' {<Definitions>} '}' \n"
	
		"<Enum> ::= 'enum' <Type_Label> [<Space_Character>] ':' <Variant_Type> '{' <Enum_Keys> '}' \n"
		
		"<Bitfield> ::= 'bitfield' <Type_Label> '{' <Enum_Keys> '}' \n"
	
		"<Enum_Keys> ::= <Enum_Key> {<Enum_Key>} \n"
		"<Enum_Key> ::= 'key' <Flex_Label> <Flex_Label> \n"
		
		// Alias type to another name : alias type new_name [optional array_len] (searches actual namespace then global namespace)
		"<Alias> ::= 'alias' <Type_Label> [<Space_Character>] <Type_Label> [<Array_Spec>] \n"
		
		"<Struct_Header> ::= 'struct' | 'union' \n"
		
		// Struct definition 
		"<Struct> ::= <Struct_Header> <Type_Label> [<Array_Spec>] '{' {<Struct_Fields>} '}' \n"
	
		"<Struct_Fields> ::= <Struct_Field> {<Struct_Field>} \n"
		
		"<Struct_Field> ::= <Field> | <Struct> | <Alias> \n"
		
		// A type definition / struct field. 
		// Example: int32 use myEnum TenEnums [10] = 0 //(this will create array of 10 fields of int32, enumerated, with default value 0)
		// Example: char[10]  TenStrings [10] = "Hello" //(this will create array of 10 strings with default value "Hello")
		"<Field> ::= <Type_Definition> [<Enum_Ref>] <Type_Label> [<Array_Spec>] [<Default_Value>] \n"
		
		"<Enum_Ref> ::= 'use ' <Type_Label> [<Space_Character>]\n"
		
		"<Array_Spec> ::= '[' <Integer> ']' \n"
	
		"<Default_Value> ::= '=' <Flex_Label> \n"
	
		"<Type_Definition> ::= <Primitive_Type> | <String_Type> \n"
		
		"<Primitive_Type> ::= 'int32' | 'uint32' | 'int16' | 'uint16' | 'int8' | 'uint8' | 'float' | 'vector' | 'bool32' | 'bool16' | 'bool8' \n"
		
		"<Variant_Type> ::= 'uint' | 'int' | 'float' | 'bool' | 'string' | 'vector' \n"
		
		"<String_Type> ::= <Fixed_String> | 'varstr' \n"
		
		"<Fixed_String> ::= 'char' <Array_Spec> \n"
	
		"<Space_Character> ::= ' ' \n"
		
			
		// common rules
		"<Label> ::= <Quoted_Label> | <Unquoted_Label> \n"
		"<Flex_Label> ::= <Quoted_Label> | <Spaced_Label> \n"
		"<Quoted_Label> ::= -'\"' <Spaced_Label> -'\"' \n"
		"<Spaced_Label> ::= <Spaced_Label_Illegals> {<Spaced_Label_Illegals>} \n"
		
		"<Type_Label> ::= <Type_Label_legals> {<Type_Label_legals_Ext>} \n"
		
		"<Type_Label_legals> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_) \n"
		"<Type_Label_legals_Ext> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-0123456789) \n"
		
		"<Integer> ::= (0123456789) { (0123456789) } \n"
		
		"<Unquoted_Label> ::= <Unquoted_Label_Illegals> {<Unquoted_Label_Illegals>} \n"
			"<Spaced_Label_Illegals> ::= (!,:\n\r\t{}\"[]) \n"
			"<Unquoted_Label_Illegals> ::= (! :\n\r\t{}\"[]) \n"

        ;

	//-----------------------------------------------------------------------
	DTypeScriptCompiler::DTypeScriptCompiler(void) : mStateStack(), mBinaryService(NULL) {
		// default to global definition group
		mGroupName = "";

		ServiceManager* svcmgr = ServiceManager::getSingletonPtr();
		mBinaryService = static_cast<BinaryService*>(svcmgr->getService("BinaryService"));
	}
	
	//-----------------------------------------------------------------------
	DTypeScriptCompiler::~DTypeScriptCompiler(void) {

	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::setupTokenDefinitions(void) {
		addLexemeTokenAction("{", ID_OPENBRACE, &DTypeScriptCompiler::parseOpenBrace);
		addLexemeTokenAction("}", ID_CLOSEBRACE, &DTypeScriptCompiler::parseCloseBrace);
		
		addLexemeTokenAction("namespace", ID_NAMESPACE, &DTypeScriptCompiler::parseNameSpace);
		
		addLexemeTokenAction("enum", ID_ENUM, &DTypeScriptCompiler::parseEnum);
		addLexemeTokenAction("bitfield", ID_BITFIELD, &DTypeScriptCompiler::parseEnum);
		addLexemeTokenAction("key", ID_KEY, &DTypeScriptCompiler::parseEnumKey);
		addLexemeTokenAction("alias", ID_ALIAS, &DTypeScriptCompiler::parseAlias);
		
		addLexemeTokenAction("struct", ID_STRUCT, &DTypeScriptCompiler::parseStruct);
		addLexemeTokenAction("union", ID_UNION, &DTypeScriptCompiler::parseStruct);
		
		// common
		addLexemeTokenAction("=", ID_EQUALS);
		addLexemeTokenAction("use ", ID_USE);
		
		addLexemeTokenAction("[", ID_OPENBOX);
		addLexemeTokenAction("]", ID_CLOSEBOX);
		
		addLexemeTokenAction("int", ID_INT);
		addLexemeTokenAction("uint", ID_UINT);
		addLexemeTokenAction("string", ID_STRING);
		
		// primitive type names
		addLexemeTokenAction("int32", ID_INT32, &DTypeScriptCompiler::parseField);
		addLexemeTokenAction("int16", ID_INT16, &DTypeScriptCompiler::parseField);
		addLexemeTokenAction("int8", ID_INT8, &DTypeScriptCompiler::parseField);
		
		addLexemeTokenAction("uint32", ID_UINT32, &DTypeScriptCompiler::parseField);
		addLexemeTokenAction("uint16", ID_UINT16, &DTypeScriptCompiler::parseField);
		addLexemeTokenAction("uint8", ID_UINT8, &DTypeScriptCompiler::parseField);
		
		addLexemeTokenAction("float", ID_FLOAT, &DTypeScriptCompiler::parseField);
		addLexemeTokenAction("vector", ID_VECTOR, &DTypeScriptCompiler::parseField);
		
		addLexemeTokenAction("char", ID_CHAR, &DTypeScriptCompiler::parseField);
		
		addLexemeTokenAction("varstr", ID_VARSTR, &DTypeScriptCompiler::parseField);
	}

	//-----------------------------------------------------------------------
	size_t DTypeScriptCompiler::getAutoTokenIDStart() const {
		return ID_AUTOTOKEN;
	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::addLexemeTokenAction(const String& lexeme, const size_t token, const DTS_Action action) {
		addLexemeToken(lexeme, token, action != 0);
		mTokenActionMap[token] = action;
	}

	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::executeTokenAction(const size_t tokenID) {
		TokenActionIterator action = mTokenActionMap.find(tokenID);

		if (action == mTokenActionMap.end()) {
			// BAD command. BAD!
			logParseError("Unrecognised TypeDef Script command action");
			return;
		} else {
			try {
				(this->*action->second)();
			} catch (Exception& ogreException) {
				// an unknown token found or BNF Grammer rule was not successful
				// in finding a valid terminal token to complete the rule expression.
				logParseError(ogreException.getDescription());
			} catch (BasicException& ogreException) {
				// an unknown token found or BNF Grammer rule was not successful
				// in finding a valid terminal token to complete the rule expression.
				logParseError(ogreException.getDetails());
			}
		}
	}

	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::logParseError(const String& error) {
		LOG_ERROR("Error in script at line %d  of %s (last context: %s) : %s", mCurrentLine, mSourceName.c_str(), mCurrentState.name.c_str(), error.c_str());
		
		/*OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR,
                    "Error in script at line " + StringConverter::toString(mCurrentLine) +
			" of " + mSourceName + ": " + error, "DTypeScriptCompiler::logParseError");*/
	}
    
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::releaseAll(DTypeDefVector& toRel) {
		DTypeDefVector::iterator it = toRel.begin();
		    
		for (; it != toRel.end(); it++) {
			(*it)->release();
		}
		    
		toRel.clear();
	}
    
	//-----------------------------------------------------------------------
	DTypeScriptCompiler::CompileState DTypeScriptCompiler::popState() {
    		CompileState old = mCurrentState;
		
		if (mStateStack.size() > 0) {
			mCurrentState = mStateStack.top();
			mStateStack.pop();
			
			return old;
		} else {
			// state stack is empty!
			mCurrentState.state = CS_UNKNOWN;
			mCurrentState.enumeration = NULL;
			return old;
		}
	}
		
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::pushCurrentState() {
		mStateStack.push(mCurrentState);
		mCurrentState.state = CS_UNKNOWN;
		mCurrentState.enumeration = NULL;
	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseOpenBrace(void) {

	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseCloseBrace(void) {
		// the brace is closing. We have to evaluate the current state
		CompileState was = popState();
		
		if (was.state == CS_ENUM) { // Closing the enum/bitfield definition
			if (mCurrentState.state == CS_STRUCT) {
				was.enumeration->release();
				logParseError("Enum cannot be created inside structs, use pre-defined enums only.");
			}
			
			if (mCurrentState.state == CS_ENUM) {
				was.enumeration->release();
				logParseError("Enum inside enum error - found enum " + mCurrentState.name + " when closing " + was.name );
			}
			
			// register the finished enum, and release
			mBinaryService->addEnum(mGroupName, was.name, was.enumeration);
			
			// release the handle to the enumeration
			was.enumeration->release();
		}
		
		if (was.state == CS_STRUCT) {
			if (mCurrentState.state == CS_ENUM) 
				logParseError("Struct inside enum error.");
			
			DTypeDef *nt = new DTypeDef(was.name, was.types, was.unioned);
				
			// if specified an array, wrap it up so
			if (was.arraylen > 1) {
				// wrap up
				DTypeDef *nta = new DTypeDef(nt, was.arraylen);
				nt->release();
				
				nt = nta;
			}
			
			// create the struct according to the 'was' structure fields
			dispatchType(nt); // will release if needed
			releaseAll(was.types);
		}
		
 		if (was.state == CS_NAMESPACE) // closing namespace, erase the namespace's name
			mGroupName = "";
	}
	
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseNameSpace(void) {
		// Namespace block
		// namespace NAME {...}
		
		
		// Namespace does not push itself to the stack. it always is at the lowest position. Pushing happens while creating containers
		const String newGrp = getNextTokenLabel();
		
		if (mGroupName != "" || mCurrentState.state == CS_NAMESPACE)
			logParseError("Nesting namespaces is not possible - " + newGrp + " inside " + mGroupName);
		
		if (mCurrentState.state != CS_UNKNOWN)
		    logParseError("Namespace inside a declaration is not possible : " + newGrp);
		
		// okay, we can create the current state that namespace thing.
		mGroupName = newGrp;
		mCurrentState.state = CS_NAMESPACE;
		mCurrentState.name = mGroupName;
		mCurrentState.enumeration = NULL;
	}
	
	//-----------------------------------------------------------------------
	DVariant::Type DTypeScriptCompiler::getDVTypeFromStr(const std::string& name) {
		if (name == "int" || name == "int32" || name == "int16" || name == "int8") {
			return DVariant::DV_INT;
		} 
		else if (name == "uint" || name == "uint32" || name == "uint16" || name == "uint8") {
			return DVariant::DV_UINT;
		} 
		else if (name == "bool") {
			return DVariant::DV_BOOL;
		} 
		else if (name == "float") {
			return DVariant::DV_FLOAT;
		} 
		else if (name == "vector") {
			return DVariant::DV_FLOAT;
		} 
		else if (name == "string" || name == "char") {
			return DVariant::DV_STRING;
		} 
		
		OPDE_EXCEPT("Unknown type specified : " + name, "DTypeScriptCompiler::getDVTypeFromStr");
	}
	
	//-----------------------------------------------------------------------
	DTypeDef* DTypeScriptCompiler::getTypeDef(const std::string& name) {
		DTypeDef* nt;
		
		try {
			nt = mBinaryService->getType(mGroupName, name);
			return nt;
		} catch (BasicException& ex) {
			try {
				if (mGroupName != "") {
					nt = mBinaryService->getType("", name);
					return nt;
				} else {
					logParseError("Type definition not found : " + name);
				}
			} catch (BasicException& ex) {
				logParseError("Type definition not found : " + name);
			}
		}
	}
	
	//-----------------------------------------------------------------------
	DEnum* DTypeScriptCompiler::getEnum(const std::string& name) {
		DEnum* en;
		
		try {
			en = mBinaryService->getEnum(mGroupName, name);
			return en;
		} catch (BasicException& ex) {
			try {
				if (mGroupName != "") {
					en = mBinaryService->getEnum("", name);
					return en;
				} else {
					logParseError("Enum definition not found : " + name);
				}
			} catch (BasicException& ex) {
				logParseError("Enum definition not found : " + name);
			}
		}
	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::dispatchType(DTypeDef* def) {
		if (mCurrentState.state == CS_STRUCT) {
			// insert the alias into the queue of the struct's members
			mCurrentState.types.push_back(def); // will get released on closing brace
		} else if (mCurrentState.state == CS_ENUM) {
			def->release();
			logParseError("Alias inside the enum definition not possible");
		} else {
			mBinaryService->addType(mGroupName, def);
			def->release();
		}
	}

	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseEnum(void) {
		// This one is called by enum and bitfield, enum needs type as a parameter
		// bitfield name {}
		// enum name : type {}
		
		pushCurrentState();
		
		bool bitfield = getCurrentTokenID() == ID_BITFIELD ? true : false;
		
		mCurrentState.enumeration = NULL;
		mCurrentState.state = CS_ENUM;
		mCurrentState.name = getNextTokenLabel();
		
		if (!bitfield) {
			const size_t paramCount = getRemainingTokensForAction();
		
			if (paramCount != 2) 
				logParseError("Type specifier for enumeration '" + mCurrentState.name + "' expected");
			
			
			// get the type specifier. First comes the ':'
			getNextToken();
			
			TokenID type = static_cast<TokenID>(getNextTokenID());
			
			// convert the type to the DVariant::type
			DVariant::Type ntype = getDVTypeFromID(type);
			
			mCurrentState.enumvaltype = ntype;
			
			mCurrentState.enumeration = new DEnum(ntype, false);
		} else {
			mCurrentState.enumvaltype = DVariant::DV_UINT;
			mCurrentState.enumeration = new DEnum(DVariant::DV_UINT, true);
		}
	}

	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseEnumKey(void) {
		// enumeration/bitfield key
		// key "" ""

		assert(mCurrentState.enumeration != NULL);
		assert(mCurrentState.state == CS_ENUM);
		
		const String keyname = getNextTokenLabel();
		const String value = getNextTokenLabel();
		
		try {
			// parse the value and insert
			DVariant val(mCurrentState.enumvaltype, value);
			
			mCurrentState.enumeration->insert(keyname, val);
			
		} catch (runtime_error &e) {
			logParseError("error while constructing key value : " + value);
		}
	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseAlias(void) {
		const String type = getNextTokenLabel();
		const String newname = getNextTokenLabel();
		
		DTypeDef *typ = getTypeDef(type);
		DTypeDef *newt = typ->alias(newname);
		
		if (testNextTokenID(ID_OPENBRACE)) {
			int len = parseBoxBrace();
			
			DTypeDef *arrayt = new DTypeDef(newt, len);
			newt->release();
			dispatchType(arrayt);
		} else
			dispatchType(newt);
	}

	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseStruct(void) {
		// This one is called by struct and union
		// struct name [arr] {...}
		// union name [arr] {...}
		
		// push the current state to the stack, as we're gonna fill a nested one
		pushCurrentState();
		
		mCurrentState.unioned = getCurrentTokenID() == ID_UNION ? true : false;
		mCurrentState.state = CS_STRUCT;
		mCurrentState.name = getNextTokenLabel();
		mCurrentState.types.clear();
		mCurrentState.arraylen = 1; // not an array
		
		if (testNextTokenID(ID_OPENBOX))
			mCurrentState.arraylen = parseBoxBrace();
	}
	
	//-----------------------------------------------------------------------
	DVariant::Type DTypeScriptCompiler::getDVTypeFromID(TokenID id) {
		switch (id) {
			case ID_INT32 :
			case ID_INT16 :
			case ID_INT8  : 
			case ID_INT   : return DVariant::DV_INT;

			case ID_UINT32 :
			case ID_UINT16 : 
			case ID_UINT8  :
			case ID_UINT   : return DVariant::DV_UINT;

			case ID_BOOL32 :
			case ID_BOOL16 : 
			case ID_BOOL8  : return DVariant::DV_BOOL;
			
			case ID_FLOAT  : return DVariant::DV_FLOAT;
				
			case ID_STRING :
			case ID_CHAR:
			case ID_VARSTR : return DVariant::DV_STRING;
				
			
			case ID_VECTOR : return DVariant::DV_VECTOR;
				
			default : 
				logParseError("Invalid type specified for DVariant type");
		}
	}
	
	//-----------------------------------------------------------------------
	int DTypeScriptCompiler::getDataLenFromID(TokenID id) {
		switch (id) {
			case ID_INT32 :
			case ID_UINT32 :
			case ID_BOOL32 : return 4;
					
			case ID_INT16 :
			case ID_UINT16 : 
			case ID_BOOL16 : return 2;
			
			case ID_INT8  : 
			case ID_UINT8  : 
			case ID_BOOL8  : return 1;
			
			case ID_FLOAT  : return 4;
				
			case ID_VECTOR : return 12;
			
			case ID_VARSTR : return -1;
			
			default : 
				logParseError("Invalid type specified or size can't be determined");
		}
	}
	
	//-----------------------------------------------------------------------
	void DTypeScriptCompiler::parseField(void) {
		TokenID typei = static_cast<TokenID>(getCurrentTokenID());
		
		// get the field type out of the current token ID
		DVariant::Type type = getDVTypeFromID(typei);
		
		DEnum* enm = NULL;
		
		size_t array_len = 1;
		
		int datasize = 0;
		
		if (typei == ID_CHAR) {
			// Array spec
			datasize = parseBoxBrace();
		} else 
			datasize = getDataLenFromID(typei);
		
		// Look at the next token. it can either be 'use', array spec '[' or direct name of the field
		if (testNextTokenID(ID_USE)) {
			// use enumeration
			getNextToken(); // skip the use keyword
			
			String enm_name = getNextTokenLabel();

			enm = getEnum(enm_name);
		}
		
		// now comes the name
		String name = getNextTokenLabel();

		if (testNextTokenID(ID_OPENBOX)) // array typed
			array_len = parseBoxBrace();

		DVariant templ;
		bool hasDefault = false;
		
		// optional equals
		if (testNextTokenID(ID_EQUALS)) {
			getNextToken();
			
			try {
				templ = DVariant(type, getNextTokenLabel());
				
				hasDefault = true;
			} catch (runtime_error &ex) {
				logParseError("Exception while constructing the default field's '" + name + "' value");
			}
		}
		
		DTypeDef* ndef = NULL;
		
		if (hasDefault)
			try {
				ndef = new DTypeDef(name, templ, datasize, enm);
			} catch (Opde::BasicException &ex) {
				logParseError("Exception while constructing the field's '" + name + "' type : " + ex.getDetails());
			}
		else 
			try {
				ndef = new DTypeDef(name, type, datasize, enm);
			} catch (Opde::BasicException &ex) {
				logParseError("Exception while constructing the field's '" + name + "' type : " + ex.getDetails());
			}
			
		if (array_len <= 1) {
			dispatchType(ndef);
		} else {
			DTypeDef* arr = new DTypeDef(ndef, array_len);
			ndef->release();
		
			dispatchType(arr);
		}
	}
	
	int DTypeScriptCompiler::parseBoxBrace() {
		getNextToken(); // skip the [
		
		size_t id = getCurrentTokenID();
		
		if (id != ID_OPENBOX)
			logParseError("Expected open box brace character : " + ((int)id));
			
		
		int array_len;
		
		try {
			String slen = getNextTokenLabel();
		
			// convert to int
			array_len = StringConverter::parseLong(slen);
			
			if (array_len == 0)
				logParseError("Zero or invalid array length : " + slen);
			
		} catch (Ogre::Exception &ex) {
			array_len = 0;
			logParseError("Exception parsing the array len : " + ex.getDescription());
		}
			
		
		// getNextToken(); // skip the ]
		
		if (getNextTokenID() != ID_CLOSEBOX)
			logParseError("Box brace not closed");
		
		return array_len;
	}
}
