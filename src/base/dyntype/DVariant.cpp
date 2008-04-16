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

#include <stdexcept>
#include "DVariant.h"

using namespace std;

namespace Opde {
	/*------------------------------------------------------*/
	/*-------------------- DVariant ------------------------*/
	/*------------------------------------------------------*/

	DVariant::DVariant() {
		mPrivate.isShared = false;
		mPrivate.type = DV_INVALID;
	}

	//------------------------------------
	DVariant::DVariant(Type t, void *val) {
		mPrivate.isShared = false;
		mPrivate.type = t;
		
		switch (t) {
			case DV_BOOL :
				mPrivate.data.dbool = val ? *static_cast<bool *>(val) : false;
				break;

			case DV_FLOAT :
				mPrivate.data.dfloat = val ? *static_cast<float *>(val) : 0;
				break;

			case DV_INT :
				mPrivate.data.dint = val ? *static_cast<int *>(val) : 0;
				break;

			case DV_UINT :
				mPrivate.data.duint = val ? *static_cast<uint *>(val) : 0;
				break;

			case DV_STRING :
				mPrivate.data.shared = new Shared<string>(*static_cast<string *>(val));
				mPrivate.isShared = 1;
				break;

			case DV_VECTOR :
				mPrivate.data.shared = new Shared<Vector3>(*static_cast<Vector3 *>(val));
				mPrivate.isShared = 1;
				break;
				
			case DV_QUATERNION :
				mPrivate.data.shared = new Shared<Quaternion>(*static_cast<Quaternion *>(val));
				mPrivate.isShared = 1;
				break;

			default :
				throw runtime_error("DVariant::DVariant() - invalid type");
		}
	}

	//------------------------------------
	DVariant::DVariant(Type t, const std::string& txtval) {
		mPrivate.isShared = false;
		mPrivate.type = t;
		
		switch (t) {
			case DV_BOOL :
				mPrivate.data.dbool = StringToBool(txtval);
				break;

			case DV_FLOAT :
				mPrivate.data.dfloat = StringToFloat(txtval);
				break;

			case DV_INT :
				mPrivate.data.dint = StringToInt(txtval);
				break;

			case DV_UINT :
				mPrivate.data.duint = StringToUInt(txtval);
				break;

			case DV_STRING :
				mPrivate.data.shared = new Shared<string>(txtval);
				mPrivate.isShared = true;
				break;

			case DV_VECTOR :
				mPrivate.data.shared = new Shared<Vector3>(StringToVector(txtval));
				mPrivate.isShared = true;
				break;
				
			case DV_QUATERNION :
				mPrivate.data.shared = new Shared<Quaternion>(StringToQuaternion(txtval));
				mPrivate.isShared = true;
				break;

			default :
				throw runtime_error("DVariant::DVariant() - invalid type");
		}
	}

	//------------------------------------
	DVariant::DVariant(const DVariant& b) {
		if (b.mPrivate.isShared) {
			mPrivate.isShared = true;
			mPrivate.type = b.mPrivate.type;
			mPrivate.data.shared = b.mPrivate.data.shared->clone();
		} else {
			mPrivate = b.mPrivate;
		}
	}

	//------------------------------------
	DVariant::DVariant(bool val) {
		mPrivate.type = DV_BOOL;
		mPrivate.data.dbool = val;
		mPrivate.isShared = false;
	}

	//------------------------------------
	DVariant::DVariant(float val) {
		mPrivate.type = DV_FLOAT;
		mPrivate.data.dfloat = val;
		mPrivate.isShared = false;
	}

	//------------------------------------
	DVariant::DVariant(int val) {
		mPrivate.type = DV_INT;
		mPrivate.data.dint = val;
		mPrivate.isShared = false;
	}

	//------------------------------------
	DVariant::DVariant(uint val) {
		mPrivate.type = DV_UINT;
		mPrivate.data.duint = val;
		mPrivate.isShared = false;
	}

	//------------------------------------
	DVariant::DVariant(const char* text, int length) {
		mPrivate.type = DV_STRING;

		if (length > 0) { // if length is present... (I could probably see what the default length param is, but it could differ)
			mPrivate.data.shared = new Shared<string>(string(text, length));
		} else {
			mPrivate.data.shared = new Shared<string>(text);
		}

		mPrivate.isShared = true;
	}

	//------------------------------------
	DVariant::DVariant(const std::string& text) {
		mPrivate.type = DV_STRING;
		mPrivate.data.shared = new Shared<string>(text);
		mPrivate.isShared = true;
	}

	//------------------------------------
	DVariant::DVariant(const Vector3& vec) {
		mPrivate.type = DV_VECTOR;
		mPrivate.data.shared = new Shared<Vector3>(vec);
		mPrivate.isShared = true;
	}
	
	//------------------------------------
	DVariant::DVariant(const Quaternion& ori) {
		mPrivate.type = DV_QUATERNION;
		mPrivate.data.shared = new Shared<Quaternion>(ori);
		mPrivate.isShared = true;
	}

	//------------------------------------
	DVariant::DVariant(float x, float y, float z) {
		mPrivate.type = DV_VECTOR;
		mPrivate.data.shared = new Shared<Vector3>(Vector3(x, y, z));
		mPrivate.isShared = true;
	}

	//------------------------------------
	DVariant::~DVariant() {	
		if (mPrivate.isShared)
			delete mPrivate.data.shared;
	}

	//------------------------------------
	const char* DVariant::typeToString() const {
		switch (mPrivate.type) {
			case DV_BOOL :
				return "Bool";
			case DV_FLOAT :
				return "Float";
			case DV_INT :
				return "Int";
			case DV_UINT :
				return "UInt";
			case DV_STRING :
				return "String";
			case DV_VECTOR :
				return "Vector";
			case DV_QUATERNION :
				return "Quaternion";
			default:
                return "Invalid"; // Be gentle. Do not throw
		}
	}

	//------------------------------------
	DVariant::operator std::string() const {
		std::ostringstream o;

		switch (mPrivate.type) {
			case DV_BOOL :
				return mPrivate.data.dbool ? "true" : "false";
			case DV_FLOAT :
				o << mPrivate.data.dfloat;
				return o.str();
			case DV_INT :
				o << mPrivate.data.dint;
				return o.str();
			case DV_UINT :
				o << mPrivate.data.duint;
				return o.str();
			case DV_STRING :
				assert(mPrivate.isShared);
				return (static_cast<Shared<string>*>(mPrivate.data.shared))->data;
			case DV_VECTOR : {
				const Vector3& vecd = (static_cast<Shared<Vector3>*>(mPrivate.data.shared))->data;
				o << vecd.x << ", " << vecd.y << ", " << vecd.z;
				return o.str();
			}
			case DV_QUATERNION : {
				const Quaternion& qd = (static_cast<Shared<Quaternion>*>(mPrivate.data.shared))->data;
				o << qd.w << ", " << qd.x << ", " << qd.y << ", " << qd.z;
				return o.str();
			}
			default:
				// TODO: We need a good replacement for exceptions here (and everywhere else as well)
				// I assert false for now...
				assert(false);
				return "<INVALID_TYPE>";
		}
	}

	//------------------------------------
	string DVariant::toString() const {
		return operator string();
	}

	//------------------------------------
	int DVariant::toInt() const {
		std::stringstream ssStream;

		switch (mPrivate.type) {
			case DV_BOOL :
				return mPrivate.data.dbool ? 1 : 0;
			case DV_FLOAT :
				return (int)mPrivate.data.dfloat;
			case DV_INT :
				return mPrivate.data.dint;
			case DV_UINT :
				return mPrivate.data.duint;
			case DV_STRING : // Convert the contained value to string
				return StringToInt(((static_cast<Shared<string>*>(mPrivate.data.shared))->data));
			case DV_VECTOR :
				throw runtime_error("DVariant::toInt() - vector cannot be converted to int");
			case DV_QUATERNION :
				throw runtime_error("DVariant::toInt() - quaternion cannot be converted to int");
			case DV_INVALID:
				throw runtime_error("DVariant::toInt() - invalid type specified");
			default:
				throw runtime_error("DVariant::toInt() - unknown type specified");
		}
	}

	//------------------------------------
	uint DVariant::toUInt() const {
		switch (mPrivate.type) {
			case DV_BOOL :
				return mPrivate.data.dbool ? 1 : 0;
			case DV_FLOAT :
				return (unsigned int)mPrivate.data.dfloat;
			case DV_INT :
				return mPrivate.data.dint;
			case DV_UINT :
				return mPrivate.data.duint;
			case DV_STRING :
				return StringToInt(((static_cast<Shared<string>*>(mPrivate.data.shared))->data));
			case DV_VECTOR :
				throw runtime_error("DVariant::toUInt() - vector cannot be converted to uint");
			case DV_QUATERNION :
				throw runtime_error("DVariant::toUInt() - quaternion cannot be converted to uint");
			default:
				throw runtime_error("DVariant::toUInt() - invalid type specified");
		}
	}

	//------------------------------------
	float DVariant::toFloat() const {
		std::stringstream ssStream;

		switch (mPrivate.type) {
			case DV_BOOL :
				return mPrivate.data.dbool ? 1 : 0;
			case DV_FLOAT :
				return mPrivate.data.dfloat;
			case DV_INT :
				return mPrivate.data.dint;
			case DV_UINT :
				return mPrivate.data.duint;
			case DV_STRING :
				return StringToFloat((static_cast<Shared<string>*>(mPrivate.data.shared))->data);
			case DV_VECTOR :
				throw runtime_error("DVariant::toFloat() - vector cannot be converted to int");
			case DV_QUATERNION :
				throw runtime_error("DVariant::toFloat() - quaternion cannot be converted to int");
			default:
				throw runtime_error("DVariant::toFloat() - invalid type specified");
		}
	}

	//------------------------------------
	bool DVariant::toBool() const {
		switch (mPrivate.type) {
			case DV_BOOL :
				return mPrivate.data.dbool;
			case DV_FLOAT :
				return mPrivate.data.dfloat != 0;
			case DV_INT :
				return mPrivate.data.dint != 0;
			case DV_UINT :
				return mPrivate.data.duint != 0;
			case DV_STRING :
				return StringToBool((static_cast<Shared<string>*>(mPrivate.data.shared))->data);
			case DV_VECTOR :
				throw runtime_error("DVariant::toBool() - vector cannot be converted to bool");
			case DV_QUATERNION :
				throw runtime_error("DVariant::toBool() - quaternion cannot be converted to bool");
			default:
				throw runtime_error("DVariant::toBool() - invalid type specified");
		}
	}

	//------------------------------------
	Vector3 DVariant::toVector() const {
		if (mPrivate.type == DV_VECTOR) {
			// simply return the value
			return ((static_cast<Shared<Vector3>*>(mPrivate.data.shared))->data);
		} else if (mPrivate.type == DV_STRING) {
			return StringToVector((static_cast<Shared<string>*>(mPrivate.data.shared))->data);
		} else {
			throw runtime_error("DVariant::toVector - Incompatible source type");
		}
	}

	//------------------------------------
	Quaternion DVariant::toQuaternion() const {
		if (mPrivate.type == DV_QUATERNION) {
			// simply return the value
			return ((static_cast<Shared<Quaternion>*>(mPrivate.data.shared))->data);
		} else if (mPrivate.type == DV_STRING) {
			return StringToQuaternion((static_cast<Shared<string>*>(mPrivate.data.shared))->data);
		} else {
			throw runtime_error("DVariant::toQuaternion - Incompatible source type");
		}
	}

	//------------------------------------
	const DVariant& DVariant::operator =(const DVariant& b) {
		// The rather unreadable ugly code is here to handle to self-asignment
		if (b.mPrivate.isShared) {

			if ((mPrivate.isShared)) {
				// we have shared data as well
			    if ((mPrivate.data.shared != b.mPrivate.data.shared)) {
			    	// only copy data if not the same pointer
					delete mPrivate.data.shared;
					mPrivate.data.shared = b.mPrivate.data.shared->clone();
				}
			} else // we did not have shared data, clone no matter what
				mPrivate.data.shared = b.mPrivate.data.shared->clone();
				
				
			mPrivate.isShared = true;
			mPrivate.type = b.mPrivate.type;

		} else {
			// if was shared, release
			if (mPrivate.isShared)
				delete mPrivate.data.shared;
			
			mPrivate = b.mPrivate;
		}

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(bool b) {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.isShared = false;
		mPrivate.data.dbool = b;
		mPrivate.type = DV_BOOL;

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(int i)  {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.type = DV_INT;
		mPrivate.isShared = false;
		mPrivate.data.dint = i;

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(uint u)  {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.type = DV_UINT;
		mPrivate.isShared = false;
		mPrivate.data.duint = u;

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(float f)  {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.type = DV_FLOAT;
		mPrivate.isShared = false;
		mPrivate.data.dfloat = f;

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(char* s) {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.isShared = true;
		mPrivate.type = DV_STRING;
		mPrivate.data.shared = new Shared<string>(string(s));

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(const std::string& s) {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.isShared = true;
		mPrivate.type = DV_STRING;
		mPrivate.data.shared = new Shared<string>(s);

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(const Vector3& v) {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.isShared = true;
		mPrivate.type = DV_VECTOR;
		mPrivate.data.shared = new Shared<Vector3>(v);

		return *this;
	}

	//------------------------------------
	const DVariant& DVariant::operator =(const Quaternion& q) {
		if (mPrivate.isShared)
			delete mPrivate.data.shared;

		mPrivate.isShared = true;
		mPrivate.type = DV_QUATERNION;
		mPrivate.data.shared = new Shared<Quaternion>(q);

		return *this;
	}

	//------------------------------------
	DVariant::Type DVariant::type() const {
		return mPrivate.type;
	}

	//------------------------------------
	// Returns a reference to a shared data, statically cast to the requested type, if possible
	template <typename T> T& DVariant::shared_cast() const {
		if (mPrivate.isShared == false)
			throw(runtime_error("Invalid shared_cast - cast on non-shared data"));

		Shared<T> *ct = static_cast<Shared<T>*>(mPrivate.data.shared);

		if (ct != NULL)
			return (ct->data);
		else
			throw(runtime_error("Invalid shared_cast - cast on different type or NULL"));

	}

	//------------------------------------
	bool DVariant::compare(const DVariant &b) const {
		if (b.type() == type()) {
			switch (mPrivate.type) {
				case DV_BOOL  :
					return b.mPrivate.data.dbool == mPrivate.data.dbool;
				case DV_FLOAT :
					return b.mPrivate.data.dfloat == mPrivate.data.dfloat;
				case DV_INT :
					return b.mPrivate.data.dint == mPrivate.data.dint;
				case DV_UINT :
					return b.mPrivate.data.duint == mPrivate.data.duint;
				case DV_STRING:
					return
						b.shared_cast<string>() ==
						shared_cast<string>();
				case DV_VECTOR:
					return 	b.shared_cast<Vector3>() ==
						shared_cast<Vector3>();

				case DV_QUATERNION:
					return 	b.shared_cast<Quaternion>() ==
						shared_cast<Quaternion>();

				default:
					throw(runtime_error("DVariant: Invalid compare type"));
			}
		} else {
			// Some basic comparison operations with conversion
			switch (mPrivate.type) {
				case DV_BOOL : // any nonzero value will be considered true
					return mPrivate.data.dbool == b.toBool();

				case DV_FLOAT :
					return b.toFloat() == mPrivate.data.dfloat;

				case DV_UINT :
					return toUInt() == b.toUInt();
				case DV_INT  :
					return toInt() == b.toInt();

				case DV_STRING :
					return shared_cast<string>() == b.toString();

				case DV_VECTOR :
					return shared_cast<Vector3>() == b.toVector();
					
				case DV_QUATERNION :
					return shared_cast<Quaternion>() == b.toQuaternion();

				default :
					throw runtime_error("DVariant::typeToString() - invalid type");
			}
		}

		return false;
	};

	//------------------------------------
	Vector3 DVariant::StringToVector(const std::string& str) {
		string src(str);

		float vec[3];
		vec[0] = vec[1] = vec[2] = 0;
		int cnt = 0;

		while (cnt <= 2) {
			// parse the string
			size_t comma_pos = src.find(',');

			// last value does not end with comma
			if (cnt == 2) {
					std::stringstream ssStream(src);
					ssStream >> vec[cnt];

					if (!ssStream)
						throw runtime_error(string("DVariant::StringToVector - Parse error for ") + str);
			} else {
				if (comma_pos != string::npos) {
					std::stringstream ssStream(src.substr(0, comma_pos));
					ssStream >> vec[cnt];
					ssStream.clear();

					if (!ssStream)
						throw runtime_error(string("DVariant::StringToVector - Parse error for ") + str);

					src = src.substr(comma_pos + 1, src.length() - (comma_pos + 1));
				} else {
					throw runtime_error(string("DVariant::StringToVector - Parse error for ") + str);
					// return Vector3(0,0,0);
				}
			}

			cnt++;
		}

		// make a vector3 out of the array, and return
		return Vector3(vec[0], vec[1], vec[2]);
	}
	
	//------------------------------------
	Quaternion DVariant::StringToQuaternion(const std::string& str) {
		string src(str);

		float quat[4];
		quat[0] = quat[1] = quat[2] = quat[3] = 0;
		int cnt = 0;

		while (cnt <= 3) {
			// parse the string
			size_t comma_pos = src.find(',');

			// last value does not end with comma
			if (cnt == 3) {
					std::stringstream ssStream(src);
					ssStream >> quat[cnt];

					if (!ssStream)
						throw runtime_error(string("DVariant::StringToQuaternion - Parse error for ") + str);
			} else {
				if (comma_pos != string::npos) {
					std::stringstream ssStream(src.substr(0, comma_pos));
					ssStream >> quat[cnt];
					ssStream.clear();

					if (!ssStream)
						throw runtime_error(string("DVariant::StringToQuaternion - Parse error for ") + str);

					src = src.substr(comma_pos + 1, src.length() - (comma_pos + 1));
				} else {
					throw runtime_error(string("DVariant::StringToQuaternion - Parse error for ") + str);
				}
			}

			cnt++;
		}

		// make a vector3 out of the array, and return
		return Quaternion(quat[0], quat[1], quat[2], quat[3]);
	}

	//------------------------------------
	int DVariant::StringToInt(const std::string& str) {
		std::stringstream ssStream;
		int iReturn;

		if (str.substr(0,2)=="0x") { // hexadecimal
			ssStream >> hex;
			ssStream << str.substr(2);
    			ssStream >> iReturn;

			if (!ssStream)
				throw runtime_error(string("DVariant::StringToInt - Parse error for ") + str);
		} else {
			ssStream << str;
    			ssStream >> iReturn;

			if (!ssStream)
				throw runtime_error(string("DVariant::StringToInt - Parse error for ") + str);
		}

		return iReturn;
	}

	//------------------------------------
	uint DVariant::StringToUInt(const std::string& str) {
		std::stringstream ssStream;
		uint iReturn;

		if (str.substr(0,2)=="0x") { // hexadecimal
			ssStream >> hex;
			ssStream << str.substr(2);
    			ssStream >> iReturn;

			if (!ssStream)
				throw runtime_error(string("DVariant::StringToInt - Parse error for ") + str);
		} else {
			ssStream << str;
    			ssStream >> iReturn;

			if (!ssStream)
				throw runtime_error(string("DVariant::StringToInt - Parse error for ") + str);
		}

		return iReturn;
	}

	//------------------------------------
	bool DVariant::StringToBool(const std::string& str) {
		string s(str);

		// HMM: VC does not like this.
		// transform(s.begin(), s.end(), s.begin(), (int(*)(int)) std::tolower);

		if ((s=="true") || (s=="1") || (s=="yes")) {
			return true;
		} else if ((s=="false") || (s=="0") || (s=="no")) {
			return false;
		} else
			throw runtime_error(string("DVariant::StringToBool - Parse error for ") + str);
	}

	//------------------------------------
	float DVariant::StringToFloat(const std::string& str) {
		std::stringstream ssStream;
		float fReturn;

		ssStream << str;
    		ssStream >> fReturn;

		if (!ssStream)
			throw runtime_error(string("DVariant::StringToFloat - Parse error for ") + str);

		return fReturn;
	}

}
