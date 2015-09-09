/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *	   $Id$
 *
 *****************************************************************************/


#ifndef __STRINGTOKENIZER_H
#define __STRINGTOKENIZER_H

#include "config.h"

#include "Iterator.h"
#include <functional>
#include <string>

namespace Opde {
	/// char classifier (is Space)
	struct IsChar : public std::unary_function<char, bool> {
		IsChar(char c) : mChar(c) {};
		IsChar(const IsChar& b) : mChar(b.mChar) {};

		bool operator()(char c) const {
			return c == mChar;
		}

		char mChar;
	};

	/** An universal string tokenizer. Splits the text using given IsChar instance
	*/
	class StringTokenizer {
		public:
			/** Constructor
			* @param src the string to tokenize
			* @param splitter the splitter instance to use
			* @param eatEmpty if true, eats consecutive separators, not returning empty strings on such ocasions
			*/
			StringTokenizer(const std::string& src, IsChar splitter, bool eatEmpty = true) : mStr(src), mIsChar(splitter), mEatEmpty(eatEmpty) {
				mCurPos = mStr.begin();
			};
			
			/** Constructor
			* @param src the string to tokenize
			* @param splitter the splitter instance to use
			* @param eatEmpty if true, eats consecutive separators, not returning empty strings on such ocasions
			*/
			StringTokenizer(const std::string& src, char splitchar, bool eatEmpty = true) : mStr(src), mIsChar(splitchar), mEatEmpty(eatEmpty) {
				mCurPos = mStr.begin();
			};

			/// Returns next token, or empty string if no token is available
			std::string next() {
				std::string::const_iterator tok_end;

				if (mCurPos != mStr.end()) {

					if (mEatEmpty) {
						while (mIsChar(*mCurPos))
							++mCurPos;

					} else { // Or I'll end up with empty token on second
						if (mIsChar(*mCurPos))
							++mCurPos;
					}

					tok_end = std::find_if(mCurPos, mStr.end(), mIsChar);

					if (mCurPos < tok_end) {
						std::string toRet = std::string(mCurPos, tok_end);
						mCurPos = tok_end;
						return toRet;
					}
				}

				// Failback to empty string
				return std::string("");
			};

			/// Returns true if no token is available
			bool end() const {
				return mCurPos == mStr.end();
			};

			/// Returns the rest of the string
			std::string rest() {
				return std::string(mCurPos, mStr.end());
			}

			/** Pulls one string into the target
			* @param target The string to be filled with next token
			* @return true if sucessful, false if end encountered */			
			bool pull(std::string& target) {
				if (end())
					return false;
					
				target = next();
				return true;
			}

		protected:
			const std::string& mStr;
			IsChar mIsChar;
			bool mEatEmpty;

			std::string::const_iterator mCurPos;
	};


	/** @brief Whitespace String tokenizer - splits given string on whitespaces, optionally watching for double quotes */
	class WhitespaceStringTokenizer {
		protected:
			/// char classifier (is Space)
			struct IsSpace : public std::unary_function<char, bool> {
				bool operator()(char c) {
					return (isspace(c) != 0);
				}
			};

			/// char classifier (is Quote)
			struct IsQuote : public std::unary_function<char, bool> {
				bool operator()(char c) {
					return (c == '\"');
				}
			};

		public:
			WhitespaceStringTokenizer(const std::string& src, bool ignoreQuotes = true) : mIgnoreQuotes(ignoreQuotes), mStr(src) {
				mCurPos = mStr.begin();
			};

			/// Returns next token, or empty string if no token is available
			std::string next() {
				// We tokenize a hardcoded way - all spaces, except those which are in quotes, are splitters
				std::string::const_iterator tok_end;
				std::string::const_iterator strend = mStr.end();
	
				bool canEnd = false;

				if (mCurPos != strend) {
					while (mIsSpaceP(*mCurPos)) // eat all the spaces
						++mCurPos;

					// Look if the current char is quote. if it is, split on quotes
					if (mIsQuoteP(*mCurPos) && !mIgnoreQuotes) {
							tok_end = std::find_if(++mCurPos, strend, mIsQuoteP);
					} else {
							tok_end = std::find_if(mCurPos, strend, mIsSpaceP);
							// we can tolerate end here
							canEnd = true;
					}

					if (tok_end != strend || canEnd) {
						std::string toRet(mCurPos, tok_end);
						mCurPos = tok_end;
						return toRet;
					}
				}

				// Failback to empty string
				return std::string("");
			};

			/// Returns true if no token is available
			bool end() const {
				return mCurPos == mStr.end();
			};

			/// Returns the rest of the string
			std::string rest() {
				return std::string(mCurPos, mStr.end());
			}

			/** Pulls one string into the target
			* @param target The string to be filled with next token
			* @return true if sucessful, false if end encountered */			
			bool pull(std::string& target) {
				if (end())
					return false;
					
				target = next();
				return true;
			}


		protected:
			bool mIgnoreQuotes;
			const std::string& mStr;
			IsSpace mIsSpaceP;
			IsQuote mIsQuoteP;

			std::string::const_iterator mCurPos;
	};
}


#endif
