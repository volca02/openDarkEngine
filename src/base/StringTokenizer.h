/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *****************************************************************************/


#ifndef __STRINGTOKENIZER_H
#define __STRINGTOKENIZER_H

#include "Iterator.h"
#include <functional>
#include <string>

namespace Opde {
	/// char classifier (is Space)
	struct IsChar : public std::unary_function<char, bool> {
		IsChar(char c) : mChar(c) {};
		IsChar(IsChar& b) : mChar(b.mChar) {};

		bool operator()(char c) {
			return c == mChar;
		}

		char mChar;
	};

	/** An universal string tokenizer. Splits the text using given IsChar instance
	*/
	class StringTokenizer : public Iterator<std::string> {
		public:
			/** Constructor
			* @param src the string to tokenize
			* @param splitter the splitter instance to use
			* @param eatEmpty if true, eats consecutive separators, not returning empty strings on such ocasions
			*/
			StringTokenizer(const std::string& src, IsChar splitter, bool eatEmpty = true) : mStr(src), mIsChar(splitter), mEatEmpty(eatEmpty) {
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

		protected:
			const std::string& mStr;
			IsChar mIsChar;
			bool mEatEmpty;

			std::string::const_iterator mCurPos;
	};


	/** @brief Whitespace String tokenizer - splits given string on whitespaces, optionally watching for double quotes */
	class WhitespaceStringTokenizer : public Iterator<std::string> {
		protected:
			/// char classifier (is Space)
			struct IsSpace : public std::unary_function<char, bool> {
				bool operator()(char c) {
					return isspace(c);
				}
			};

			/// char classifier (is Quote)
			struct IsQuote : public std::unary_function<char, bool> {
				bool operator()(char c) {
					return (c == '\"');
				}
			};

		public:
			WhitespaceStringTokenizer(const std::string& src, bool ignoreQuotes = true) : mStr(src), mIgnoreQuotes(ignoreQuotes) {
				mCurPos = mStr.begin();
			};

			/// Returns next token, or empty string if no token is available
			std::string next() {
				// We tokenize a hardcoded way - all spaces, except those which are in quotes, are splitters
				std::string::const_iterator tok_end;

				if (mCurPos != mStr.end()) {
					while (mIsSpaceP(*mCurPos)) // eat all the spaces
						++mCurPos;

					// Look if the current char is quote. if it is, split on quotes
					if (mIsQuoteP(*mCurPos) && !mIgnoreQuotes) {
							tok_end = std::find_if(mCurPos, mStr.end(), mIsQuoteP);
					} else {
							tok_end = std::find_if(mCurPos, mStr.end(), mIsSpaceP);
					}

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

		protected:
			bool mIgnoreQuotes;
			const std::string& mStr;
			IsSpace mIsSpaceP;
			IsQuote mIsQuoteP;

			std::string::const_iterator mCurPos;
	};
}


#endif
