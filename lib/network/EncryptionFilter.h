#ifndef __ENCRYPTIONFILTER_H__
#define __ENCRYPTIONFILTER_H__

#include "network/PacketFilter.h"

#ifdef USE_OPENSSL
#include "common/blowfish.h"
#endif

class EncryptionFilter : public PacketFilter
{
public:
	virtual ~EncryptionFilter() {}

	virtual void encrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
	virtual void decrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
};

#ifdef USE_OPENSSL
class BlowfishFilter : public EncryptionFilter, public Blowfish
{
public:
	virtual ~BlowfishFilter();
	BlowfishFilter(const Key & key);
	BlowfishFilter();

	virtual EReason send(Channel * pChannel, PacketSender& sender, Packet * pPacket);

	virtual EReason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

	void encrypt(Packet * pInPacket, Packet * pOutPacket);
	void decrypt(Packet * pInPacket, Packet * pOutPacket);
private:
	Packet *pPacket_;
	PacketLength packetLen_;
	uint8 padSize_;
};
typedef SmartPointer<BlowfishFilter> BlowfishFilterPtr;
#endif

inline EncryptionFilter* createEncryptionFilter(int8 type, const std::string& datas)
{
	EncryptionFilter* pEncryptionFilter = NULL;
	switch(type)
	{
#ifdef USE_OPENSSL
	case 1:
		pEncryptionFilter = new BlowfishFilter(datas);
		break;
#endif
	default:
		break;
	}

	return pEncryptionFilter;
}

#endif // __ENCRYPTIONFILTER_H__