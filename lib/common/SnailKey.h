#ifndef __SNAILKEY_H__
#define __SNAILKEY_H__

#include "rsa.h"
#include "common/Singleton.h"

class SnailKey : public SNAIL_RSA, public Singleton<SnailKey>
{
public:
	SnailKey(const std::string& pubkeyname, const std::string& prikeyname);
	SnailKey();

	virtual ~SnailKey();

	virtual bool isGood() const;
};

#endif // __SNAILKEY_H__
