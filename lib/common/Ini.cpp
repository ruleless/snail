#include "Ini.h"

Ini::Ini(const char *pathname)
{

}

Ini::~Ini()
{
}

void Ini::_parse()
{
	FILE *pFile = fopen(mPath, "r");
	if (NULL == pFile)
	{
		return;
	}

	EFSMState q = FSMState_Start;
	char c, s;
	while (fscanf(pFile, "%c", &c))
	{
		s = _toSymbol(c);
		switch (q)
		{
		case FSMState_Start:
			if (s == 's')
			{
				q = FSMState_Start;
			}
			else if (s == '[')
			{
				q = FSMState_Sec_1;
			}
			else if (s == 'w')
			{
				q = FSMState_KeyVal_1;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_Sec_1:
			if (s == 's')
			{
				q = FSMState_Sec_1;
			}
			else if (s == 'w')
			{
				q = FSMState_Sec_2;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_Sec_2:
			if (s == 's')
			{
				q = FSMState_Sec_3;
			}
			else if (s == 'w')
			{
				q = FSMState_Sec_2;
			}
			else if(s == ']')
			{
				q = FSMState_Sec_4;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_Sec_3:
			if (s == 's')
			{
				q = FSMState_Sec_3;
			}
			else if(s == ']')
			{
				q = FSMState_Sec_4;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_Sec_4:
			if (s == 's')
			{
				q = FSMState_Start;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_KeyVal_1:
			if (s == 'w')
			{
				q = FSMState_KeyVal_1;
			}
			else if (s == 's')
			{
				q = FSMState_KeyVal_2;
			}
			else if (s == '=')
			{
				q = FSMState_KeyVal_3;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_KeyVal_2:
			if (s == 's')
			{
				q = FSMState_KeyVal_2;
			}
			else if (s == '=')
			{
				q = FSMState_KeyVal_3;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_KeyVal_3:
			if (s == 's')
			{
				q = FSMState_KeyVal_3;
			}
			else if (s == 'w')
			{
				q = FSMState_KeyVal_4;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		case FSMState_KeyVal_4:
			if (s == 'w')
			{
				q = FSMState_KeyVal_4;
			}
			else if (s == 's')
			{
				q = FSMState_Start;
			}
			else
			{
				q = FSMState_Error;
			}
			break;
		default:
			q = FSMState_Error;
			break;
		}
	}
}