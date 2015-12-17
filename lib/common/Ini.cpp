#include "Ini.h"
#include "Buffer.h"

Ini::Ini(const char *pathname)
:mSec()
,mbDirty(false)
{
	memset(mPath, 0, sizeof(mPath));
	strncpy(mPath, pathname, sizeof(mPath)-1);
	_parse();
}

Ini::~Ini()
{
	save();
}

void Ini::_parse()
{
	mSec.clear();
	FILE *pFile = fopen(mPath, "r");
	if (NULL == pFile)
	{
		return;
	}

	EFSMState q = FSMState_Start;
	char c, s;
	char sec[MAX_BUF] = {0}, key[MAX_BUF] = {0}, val[MAX_BUF] = {0};
	int secindex = 0, keyindex = 0, valindex = 0;
	bool loop = true;
	while (loop)
	{
		int res = fscanf(pFile, "%c", &c);
		if (res == EOF)
		{
			s = 's';
			loop = false;
		}
		else
		{
			s = _toSymbol(c);
		}

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
				sec[secindex=0] = '\0';
			}
			else if (s == 'w')
			{
				q = FSMState_KeyVal_1;
				key[keyindex++] = c;
				key[keyindex] = '\0';
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
				sec[secindex++] = c;
				sec[secindex] = '\0';
			}
			else if (s == ']')
			{
				q = FSMState_Sec_4;
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
				sec[secindex++] = c;
				sec[secindex] = '\0';
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
				key[keyindex++] = c;
				key[keyindex] = '\0';
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
				val[valindex++] = c;
				val[valindex] = '\0';
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
				val[valindex++] = c;
				val[valindex] = '\0';
			}
			else if (s == 's')
			{
				q = FSMState_Start;
				if (keyindex > 0)
				{
					Records &records = mSec[sec];
					records[key] = val;
					key[keyindex=0] = '\0';
					val[valindex=0] = '\0';
				}
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
	fclose(pFile);

	if (q != FSMState_Start)
	{
		mSec.clear();
	}
}

int Ini::getInt(const char* section, const char* key, int def) const
{
	int res = def;
	Sections::const_iterator itSec = mSec.find(section);
	if (itSec != mSec.end())
	{
		const Records &records = itSec->second;
		Records::const_iterator itRec = records.find(key);
		if (itRec != records.end())
		{
			res = atoi(itRec->second.c_str());
		}
	}
	return res;
}

bool Ini::setInt(const char* section, const char* key, int val)
{
	Records &records = mSec[section];
	char strVal[11] = {0};
	__snprintf(strVal, sizeof(strVal), "%d", val);
	records[key] = std::string(strVal);
	setDirty(true);
	return true;
}

std::string Ini::getString(const char* section, const char* key, const char* def) const
{
	std::string res(def);
	Sections::const_iterator itSec = mSec.find(section);
	if (itSec != mSec.end())
	{
		const Records &records = itSec->second;
		Records::const_iterator itRec = records.find(key);
		if (itRec != records.end())
		{
			res = itRec->second;
		}
	}
	return res;
}

ulong Ini::getString(const char* section, const char* key, char* retStr, size_t size, const char* def) const
{
	std::string res = this->getString(section, key, def);
	if (retStr && size > 0)
	{
		__snprintf(retStr, size, "%s", res.c_str());
	}
	return strlen(retStr);
}

bool Ini::setString(const char* section, const char* key, const char* val)
{
	Records &records = mSec[section];
	records[key] = val;
	setDirty(true);
	return true;
}

void Ini::save()
{
	if (!mbDirty)
		return;

	FILE *pFile = fopen(mPath, "w");
	if (pFile)
	{
		obuf ob;
		this->serialize<obuf>(ob);

		size_t sz = ob.size();
		if (*ob.current() == '\0')
		{
			--sz;
		}
		sz = max(sz, 0);
		fwrite(ob.data(), sizeof(char), sz, pFile);
		setDirty(false);
		fclose(pFile);
	}
}