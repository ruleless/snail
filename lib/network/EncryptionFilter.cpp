#include "EncryptionFilter.h"
#include "network/TCPPacket.h"
#include "network/UDPPacket.h"
#include "network/Channel.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"
#include "network/PacketSender.h"

#ifdef USE_OPENSSL
BlowfishFilter::BlowfishFilter(const Key & key):
KBEBlowfish(key),
pPacket_(NULL),
packetLen_(0),
padSize_(0)
{
}

BlowfishFilter::BlowfishFilter():
KBEBlowfish(),
pPacket_(NULL),
packetLen_(0),
padSize_(0)
{
}

//-------------------------------------------------------------------------------------
BlowfishFilter::~BlowfishFilter()
{
	if(pPacket_)
	{
		reclaimPacket(pPacket_->isTCPPacket(), pPacket_);
		pPacket_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
EReason BlowfishFilter::send(Channel * pChannel, PacketSender& sender, Packet * pPacket)
{
	if(!pPacket->encrypted())
	{
		AUTO_SCOPED_PROFILE("encryptSend")
		
		if (!isGood_)
		{
// 			WARNING_MSG(fmt::format("BlowfishFilter::send: "
// 				"Dropping packet to {} due to invalid filter\n",
// 				pChannel->addr().c_str()));

			return Reason_GeneralNetwork;
		}

		Packet * pOutPacket = NULL;
		pOutPacket = mallocPacket(pPacket->isTCPPacket());

		PacketLength oldlen = pPacket->length();
		pOutPacket->wpos(PACKET_LENGTH_SIZE + 1);
		encrypt(pPacket, pOutPacket);

		PacketLength packetLen = pPacket->length() + 1;
		uint8 padSize = pPacket->length() - oldlen;
		size_t oldwpos =  pOutPacket->wpos();
		pOutPacket->wpos(0);

		(*pOutPacket) << packetLen;
		(*pOutPacket) << padSize;

		pOutPacket->wpos(oldwpos);
		pPacket->swap(*(static_cast<MemoryStream*>(pOutPacket)));
		reclaimPacket(pPacket->isTCPPacket(), pOutPacket);
	}

	return sender.processFilterPacket(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
EReason BlowfishFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	while(pPacket || pPacket_)
	{
		// AUTO_SCOPED_PROFILE("encryptRecv")

		if (!isGood_)
		{
// 			WARNING_MSG(fmt::format("BlowfishFilter::recv: "
// 				"Dropping packet to {} due to invalid filter\n",
// 				pChannel->addr().c_str()));

			return Reason_GeneralNetwork;
		}

		if(pPacket_)
		{
			if(pPacket)
			{
				pPacket_->append(pPacket->data() + pPacket->rpos(), pPacket->length());
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
			}

			pPacket = pPacket_;
		}

		if(packetLen_ <= 0)
		{
			// �������һ����С�����Խ��, ���򻺴������������һ�����ϲ�Ȼ����
			if(pPacket->length() >= (PACKET_LENGTH_SIZE + 1 + BLOCK_SIZE))
			{
				(*pPacket) >> packetLen_;
				(*pPacket) >> padSize_;
				
				packetLen_ -= 1;

				// �����������������������ܣ� ����ж����������Ҫ������ó���������һ�����ϲ�
				if(pPacket->length() > packetLen_)
				{
					pPacket_ = mallocPacket(pPacket->isTCPPacket());
					pPacket_->append(pPacket->data() + pPacket->rpos() + packetLen_, pPacket->wpos() - (packetLen_ + pPacket->rpos()));
					pPacket->wpos(pPacket->rpos() + packetLen_);
				}
				else if(pPacket->length() == packetLen_)
				{
					if(pPacket_ != NULL)
						pPacket_ = NULL;
				}
				else
				{
					if(pPacket_ == NULL)
						pPacket_ = pPacket;

					return receiver.processFilteredPacket(pChannel, NULL);
				}
			}
			else
			{
				if(pPacket_ == NULL)
					pPacket_ = pPacket;

				return receiver.processFilteredPacket(pChannel, NULL);
			}
		}
		else
		{
			// �����һ�������������Ϊ������û���������������
			if(pPacket->length() > packetLen_)
			{
				pPacket_ = mallocPacket(pPacket->isTCPPacket());
				pPacket_->append(pPacket->data() + pPacket->rpos() + packetLen_, pPacket->wpos() - (packetLen_ + pPacket->rpos()));
				pPacket->wpos(pPacket->rpos() + packetLen_);
			}
			else if(pPacket->length() == packetLen_)
			{
				if(pPacket_ != NULL)
					pPacket_ = NULL;
			}
			else
			{
				if(pPacket_ == NULL)
					pPacket_ = pPacket;

				return receiver.processFilteredPacket(pChannel, NULL);
			}
		}

		decrypt(pPacket, pPacket);

		pPacket->wpos(pPacket->wpos() - padSize_);

		packetLen_ = 0;
		padSize_ = 0;

		EReason ret = receiver.processFilteredPacket(pChannel, pPacket);
		if(ret != Reason_Success)
		{
			if(pPacket_)
			{
				reclaimPacket(pPacket_->isTCPPacket(), pPacket);
				pPacket_ = NULL;
			}
			return ret;
		}

		pPacket = NULL;
	}

	return Reason_Success;
}

//-------------------------------------------------------------------------------------
void BlowfishFilter::encrypt(Packet * pInPacket, Packet * pOutPacket)
{
	// BlowFish ÿ��ֻ�ܼ��ܺͽ���8�ֽ�����
	// ����8�ֽ������0
	uint8 padSize = 0;

	if (pInPacket->length() % BLOCK_SIZE != 0)
	{
		// �õ������С
		padSize = BLOCK_SIZE - (pInPacket->length() % BLOCK_SIZE);

		// ��pPacket�������ô��
		pInPacket->data_resize(pInPacket->size() + padSize);

		// ���0
		memset(pInPacket->data() + pInPacket->wpos(), 0, padSize);

		pInPacket->wpos(pInPacket->wpos() + padSize);
	}
	
	if(pInPacket != pOutPacket)
	{
		pOutPacket->data_resize(pInPacket->size() + pOutPacket->wpos());
		int size = KBEBlowfish::encrypt(pInPacket->data(), pOutPacket->data() + pOutPacket->wpos(),  pInPacket->wpos());
		pOutPacket->wpos(size + pOutPacket->wpos());
	}
	else
	{
		if(pInPacket->isTCPPacket())
			pOutPacket = TCPPacket::ObjPool().createObject();
		else
			pOutPacket = UDPPacket::ObjPool().createObject();

		pOutPacket->data_resize(pInPacket->size() + 1);

		int size = KBEBlowfish::encrypt(pInPacket->data(), pOutPacket->data() + pOutPacket->wpos(),  pInPacket->wpos());
		pOutPacket->wpos(size);

		pInPacket->swap(*(static_cast<MemoryStream*>(pOutPacket)));
		reclaimPacket(pInPacket->isTCPPacket(), pOutPacket);
	}

	pInPacket->encrypted(true);
}

//-------------------------------------------------------------------------------------
void BlowfishFilter::decrypt(Packet * pInPacket, Packet * pOutPacket)
{
	if(pInPacket != pOutPacket)
	{
		pOutPacket->data_resize(pInPacket->size());

		int size = KBEBlowfish::decrypt(pInPacket->data() + pInPacket->rpos(), 
			pOutPacket->data() + pOutPacket->rpos(),  
			pInPacket->wpos() - pInPacket->rpos());

		pOutPacket->wpos(size + pOutPacket->wpos());
	}
	else
	{
		KBEBlowfish::decrypt(pInPacket->data() + pInPacket->rpos(), pInPacket->data(),  
			pInPacket->wpos() - pInPacket->rpos());

		pInPacket->wpos(pInPacket->wpos() - pInPacket->rpos());
		pInPacket->rpos(0);
	}
}
#endif