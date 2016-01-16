#include "PacketReader.h"
#include "Channel.h"
#include "MessageHandler.h"
#include "NetworkStats.h"

PacketReader::PacketReader(Channel* pChannel)
		:mCurrMsgID(0)
		,mCurrMsgLen(0)
		,mpFragmentStream(NULL)
		,mFragmentDatasFlag(FragmentData_Unknown)
		,mpFragmentDatas(NULL)
		,mFragmentDatasWpos(0)
		,mFragmentDatasRemain(0)		
		,mpChannel(pChannel)
{
}

PacketReader::~PacketReader()
{
	reset();
	mpChannel = NULL;
}

void PacketReader::reset()
{
	mFragmentDatasFlag = FragmentData_Unknown;
	mFragmentDatasWpos = 0;
	mFragmentDatasRemain = 0;
	mCurrMsgID = 0;
	mCurrMsgLen = 0;
	
	SafeDeleteArray(mpFragmentDatas);
	MemoryStream::ObjPool().reclaimObject(mpFragmentStream);
	mpFragmentStream = NULL;
}

void PacketReader::processMessages(MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	while(pPacket->length() > 0 || mpFragmentStream != NULL)
	{
		if(mFragmentDatasFlag == FragmentData_Unknown)
		{
			if(mCurrMsgID == 0)
			{
				if(pPacket->length() < sizeof(MessageID))
				{
					writeFragmentMessage(FragmentData_MessageID, pPacket, sizeof(MessageID)); // 消息ID都不完整
					break;
				}

				(*pPacket) >> mCurrMsgID;
				pPacket->setMessageID(mCurrMsgID);
			}

			MessageHandler *pMsgHandler = pMsgHandlers->find(mCurrMsgID);

			if(pMsgHandler == NULL)
			{
				// 				ERROR_MSG(fmt::format("PacketReader::processMessages: not found msgID={}, msglen={}, from {}.\n",
				// 					mCurrMsgID, pPacket1->length(), mpChannel->c_str()));

				mCurrMsgID = 0;
				mCurrMsgLen = 0;
				mpChannel->condemn();
				break;
			}
			
			if(mCurrMsgLen == 0)
			{
				if(pMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE)
					// 如果长度信息是可变的或者配置了永远包含长度信息选项时，从流中分析长度数据
				{
					if(pPacket->length() < sizeof(MessageLength))
					{
						writeFragmentMessage(FragmentData_MessageLength, pPacket, sizeof(MessageLength));
						break;
					}
					else
					{
						MessageLength currlen;
						(*pPacket) >> currlen;
						mCurrMsgLen = currlen;

						NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Recv, *pMsgHandler, 
																  mCurrMsgLen + sizeof(MessageID) + sizeof(MessageLength));

						if(mCurrMsgLen == NETWORK_MESSAGE_MAX_SIZE) // 消息包长度超过了65535
						{
							if(pPacket->length() < sizeof(MessageLength1))
							{
								writeFragmentMessage(FragmentData_MessageLength1, pPacket, sizeof(MessageLength1));
								break;
							}
							else
							{
								(*pPacket) >> mCurrMsgLen;

								NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Recv, *pMsgHandler, 
																		  mCurrMsgLen + sizeof(MessageID) + sizeof(MessageLength1));
							}
						}
					}
				}
				else
				{
					mCurrMsgLen = pMsgHandler->msgLen;

					NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Recv, *pMsgHandler, 
															  mCurrMsgLen + sizeof(MessageLength));
				}
			}

			if(this->mpChannel->isExternal() && mCurrMsgLen > NETWORK_MESSAGE_MAX_SIZE) // 消息包过大，断掉连接！
			{
				// 				WARNING_MSG(fmt::format("PacketReader::processMessages({0}): msglen exceeds the limit! msgID={1}, msglen=({2}:{3}), maxlen={5}, from {4}.\n", 
				// 					pMsgHandler->name.c_str(), mCurrMsgID, mCurrMsgLen, pPacket1->length(), mpChannel->c_str(), NETWORK_MESSAGE_MAX_SIZE));

				mCurrMsgLen = 0;
				mpChannel->condemn();
				break;
			}

			if(mpFragmentStream != NULL)
			{
				pMsgHandler->handle(mpChannel, *mpFragmentStream);

				MemoryStream::ObjPool().reclaimObject(mpFragmentStream);
				mpFragmentStream = NULL;
			}
			else
			{
				if(pPacket->length() < mCurrMsgLen)
				{
					writeFragmentMessage(FragmentData_MessageBody, pPacket, mCurrMsgLen);
					break;
				}

				// 临时设置有效读取位， 防止接口中溢出操作
				size_t wpos = pPacket->wpos();
				size_t frpos = pPacket->rpos() + mCurrMsgLen;
				pPacket->wpos(frpos);

				pMsgHandler->handle(mpChannel, *pPacket);

				// 如果handler没有处理完数据则输出一个警告
				if(mCurrMsgLen > 0)
				{
					if(frpos != pPacket->rpos())
					{
						// 						WARNING_MSG(fmt::format("PacketReader::processMessages({}): rpos({}) invalid, expect={}. msgID={}, msglen={}.\n",
						// 							pMsgHandler->name.c_str(), pPacket->rpos(), frpos, mCurrMsgID, mCurrMsgLen));

						pPacket->rpos(frpos);
					}
				}

				pPacket->wpos(wpos);
			}

			mCurrMsgID = 0;
			mCurrMsgLen = 0;
		}
		else
		{
			mergeFragmentMessage(pPacket);
		}
	}
}

void PacketReader::writeFragmentMessage(EFragmentDataTypes fragmentDatasFlag, Packet* pPacket, uint32 datasize)
{
	Assert(mpFragmentDatas == NULL);

	size_t opsize = pPacket->length();
	mFragmentDatasRemain = datasize - opsize;
	mpFragmentDatas = new uint8[opsize + mFragmentDatasRemain + 1];

	mFragmentDatasFlag = fragmentDatasFlag;
	mFragmentDatasWpos = opsize;

	if(pPacket->length() > 0)
	{
		memcpy(mpFragmentDatas, pPacket->data() + pPacket->rpos(), opsize);
		pPacket->done();
	}
}

void PacketReader::mergeFragmentMessage(Packet* pPacket)
{
	size_t opsize = pPacket->length();
	if(opsize == 0)
		return;

	if(opsize >= mFragmentDatasRemain)
	{
		memcpy(mpFragmentDatas + mFragmentDatasWpos, pPacket->data() + pPacket->rpos(), mFragmentDatasRemain);
		pPacket->rpos(pPacket->rpos() + mFragmentDatasRemain);

		Assert(mpFragmentStream == NULL);

		switch(mFragmentDatasFlag)
		{
		case FragmentData_MessageID: // 消息ID信息不全
			memcpy(&mCurrMsgID, mpFragmentDatas, sizeof(MessageID));
			break;
		case FragmentData_MessageLength: // 消息长度信息不全
			memcpy(&mCurrMsgLen, mpFragmentDatas, sizeof(MessageLength));
			break;
		case FragmentData_MessageLength1: // 消息长度信息不全
			memcpy(&mCurrMsgLen, mpFragmentDatas, sizeof(MessageLength1));
			break;
		case FragmentData_MessageBody: // 消息内容信息不全
			mpFragmentStream = MemoryStream::ObjPool().createObject();
			mpFragmentStream->append(mpFragmentDatas, mCurrMsgLen);
			break;
		default:
			break;
		};

		mFragmentDatasFlag = FragmentData_Unknown;
		mFragmentDatasRemain = 0;
		SafeDeleteArray(mpFragmentDatas);
	}
	else
	{
		memcpy(mpFragmentDatas + mFragmentDatasWpos, pPacket->data() + pPacket->rpos(), opsize);
		mFragmentDatasRemain -= opsize;
		mFragmentDatasWpos += opsize;
		pPacket->rpos(pPacket->rpos() + opsize);
	}	
}
