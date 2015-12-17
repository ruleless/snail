#ifndef __INI_H__
#define __INI_H__

#include "common.h"

#include <map>

class Ini
{
public:
	Ini(const char *pathname);
	virtual ~Ini();

	void _parse();

	inline char _toSymbol(char c)
	{
		char s = 'e';
		switch (c)
		{
		case ' ':
		case '\r':
		case '\n':
			s = 's';
			break;
		case '[':
		case ']':
		case '=':
			s = c;
			break;
		default:
			if (c >= '0' && c <= '9')
			{
				s = 'w';
			}
			else if (c >= 'a' && c <= 'z')
			{
				s = 'w';
			}
			else if (c >= 'A' && c <= 'Z')
			{
				s = 'w';
			}
			break;
		}
		return s;
	}

	template <class T>
	void serialize(T &ob)
	{
		Sections::iterator it = mSec.begin();
		for (; it != mSec.end(); ++it)
		{
			ob.push_back("[", 1);
			const char *sec = it->first.c_str();
			ob.push_back(sec, strlen(sec));
			ob.push_back("]\n", strlen("]\n"));

			Records &records = it->second;
			Records::iterator itr = records.begin(); 
			for (; itr != records.end(); ++itr)
			{
				const char *key = itr->first.c_str();
				const char *val = itr->second.c_str();
				ob.push_back(key, strlen(key));
				ob.push_back("=", 1);
				ob.push_back(val, strlen(val));
				ob.push_back("\n", 1);
			}
		}
		ob.offset(1);
		*(ob.current()) = '\0';
	}

	int getInt(const char* section, const char* key, int def = 0) const;
	bool setInt(const char* section, const char* key, int val);

	std::string getString(const char* section, const char* key, const char* def = 0) const;
	ulong getString(const char* section, const char* key, char* retStr, size_t size, const char* def = 0) const;
	bool setString(const char* section, const char* key, const char* val);

	void save();

	inline void setDirty(bool bDirty = true)
	{
		mbDirty = bDirty;
	}
private:
	enum EFSMState
	{
		FSMState_Start = 0,
		FSMState_Error,

		FSMState_Sec_1 = 10,
		FSMState_Sec_2,
		FSMState_Sec_3,
		FSMState_Sec_4,

		FSMState_KeyVal_1 = 100,
		FSMState_KeyVal_2,
		FSMState_KeyVal_3,
		FSMState_KeyVal_4,
	};

	typedef std::map<std::string, std::string> Records;
	typedef std::map<std::string, Records> Sections;

	char mPath[MAX_PATH];
	Sections mSec;
	bool mbDirty;
};

#endif
