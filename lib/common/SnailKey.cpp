#include "SnailKey.h"
#include "common.h"

SINGLETON_INIT(SnailKey);

SnailKey::SnailKey(const std::string& pubkeyname, const std::string& prikeyname)
:SNAIL_RSA()
{
	if(pubkeyname.size() > 0 || prikeyname.size() > 0)
	{
		Assert(pubkeyname.size() > 0);
		
#if 0
		if(g_componentType != CLIENT_TYPE)
		{
			Assert(prikeyname.size() > 0);

			bool key1 = loadPrivate(prikeyname);
			bool key2 = loadPublic(pubkeyname);
			Assert(key1 == key2);

			if(!key1 && !key2)
			{
				bool ret = generateKey(pubkeyname, prikeyname);
				Assert(ret);
				key1 = loadPrivate(prikeyname);
				key2 = loadPublic(pubkeyname);
				Assert(key1 && key2);
			}
		}
		else
		{
			bool key = loadPublic(pubkeyname);
			Assert(key);
		}
#endif
	}
}

SnailKey::SnailKey():
SNAIL_RSA()
{
}

SnailKey::~SnailKey()
{
}

bool SnailKey::isGood() const
{
	if(g_componentType == CLIENT_TYPE)
	{
		return rsa_public != NULL;
	}

	return rsa_public != NULL && rsa_private != NULL;
}
