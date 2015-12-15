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

	char mPath[MAX_PATH];
	int mState;
};

#endif
