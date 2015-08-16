Bundle & Bundle::operator<<(uint8 value)
{
	onPacketAppend(sizeof(uint8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint16 value)
{
	onPacketAppend(sizeof(uint16));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint32 value)
{
	onPacketAppend(sizeof(uint32));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint64 value)
{
	onPacketAppend(sizeof(uint64));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int8 value)
{
	onPacketAppend(sizeof(int8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int16 value)
{
	onPacketAppend(sizeof(int16));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int32 value)
{
	onPacketAppend(sizeof(int32));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int64 value)
{
	onPacketAppend(sizeof(int64));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(float value)
{
	onPacketAppend(sizeof(float));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(double value)
{
	onPacketAppend(sizeof(double));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(bool value)
{
	onPacketAppend(sizeof(int8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(const std::string &value)
{
	int32 len = (int32)value.size() + 1; // +1为字符串尾部的0位置
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append(value.c_str() + addtotalsize, ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::operator<<(const char *str)
{
	int32 len = (int32)strlen(str) + 1;  // +1为字符串尾部的0位置
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append(str + addtotalsize, ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::append(Bundle* pBundle)
{
	Assert(pBundle != NULL);
	return append(*pBundle);
}

Bundle & Bundle::append(Bundle& bundle)
{
	Packets::iterator iter = bundle.mPackets.begin();
	for(; iter!=bundle.mPackets.end(); ++iter)
	{
		append((*iter)->data(), (*iter)->length());
	}

	if(bundle.mpCurrPacket == NULL)
		return *this;

	return append(bundle.mpCurrPacket->data(), bundle.mpCurrPacket->length());
}

Bundle & Bundle::append(MemoryStream* s)
{
	Assert(s != NULL);
	return append(*s);
}

Bundle & Bundle::append(MemoryStream& s)
{
	if(s.length() > 0)
		return append(s.data() + s.rpos(), s.length());

	return *this;
}

Bundle & Bundle::append(const uint8 *str, int n)
{
	return assign((char*)str, n);
}

Bundle & Bundle::append(const char *str, int n)
{
	return assign(str, n);
}

Bundle & Bundle::assign(const char *str, int n)
{
	int32 len = (int32)n;
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append((uint8*)(str + addtotalsize), ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::appendBlob(const std::string& str)
{
	return appendBlob((const uint8 *)str.data(), str.size());
}

Bundle & Bundle::appendBlob(const char* str, ArraySize n)
{
	return appendBlob((const uint8 *)str, n);
}

Bundle & Bundle::appendBlob(const uint8 *str, ArraySize n)
{
	(*this) << n;
	return assign((char*)str, n);
}

Bundle & Bundle::operator>>(bool &value)
{
	return outputValue<bool>(value);
}

Bundle & Bundle::operator>>(uint8 &value)
{
	return outputValue<uint8>(value);
}

Bundle & Bundle::operator>>(uint16 &value)
{
	return outputValue<uint16>(value);
}

Bundle & Bundle::operator>>(uint32 &value)
{
	return outputValue<uint32>(value);
}

Bundle & Bundle::operator>>(uint64 &value)
{
	return outputValue<uint64>(value);
}

Bundle & Bundle::operator>>(int8 &value)
{
	return outputValue<int8>(value);
}

Bundle & Bundle::operator>>(int16 &value)
{
	return outputValue<int16>(value);
}

Bundle & Bundle::operator>>(int32 &value)
{
	return outputValue<int32>(value);
}

Bundle & Bundle::operator>>(int64 &value)
{
	return outputValue<int64>(value);
}

Bundle & Bundle::operator>>(float &value)
{
	return outputValue<float>(value);
}

Bundle & Bundle::operator>>(double &value)
{
	return outputValue<double>(value);
}

Bundle & Bundle::operator>>(std::string& value)
{
	Assert(packetsLength() > 0);
	size_t reclaimCount = 0;
	value.clear();

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		Packet* pPacket = (*iter);

		while (pPacket->length() > 0)
		{
			char c = pPacket->read<char>();
			if (c == 0)
				break;

			value += c;
		}

		if(pPacket->data()[pPacket->rpos() - 1] == 0)
		{
			if(pPacket->length() == 0)
			{
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}

			break;
		}
		else
		{
			Assert(pPacket->length() == 0);
			++reclaimCount;
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
		}
	}

	if(reclaimCount > 0)
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);

	return *this;
}

ArraySize Bundle::readBlob(std::string& datas)
{
	datas.clear();

	ArraySize rsize = 0;
	(*this) >> rsize;

	if((int32)rsize > packetsLength())
		return 0;

	size_t reclaimCount = 0;
	datas.reserve(rsize);

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		Packet* pPacket = (*iter);

		if(pPacket->length() > rsize - datas.size())
		{
			datas.append((char*)pPacket->data() + pPacket->rpos(), rsize - datas.size());
			pPacket->rpos(pPacket->rpos() + rsize - datas.size());
			if(pPacket->length() == 0)
			{
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}

			break;
		}
		else
		{
			datas.append((char*)pPacket->data() + pPacket->rpos(), pPacket->length());
			pPacket->done();
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
			++reclaimCount;
		}
	}

	if(reclaimCount > 0)
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);

	return rsize;
}

template<typename T> Bundle & Bundle::outputValue(T &v)
{
	Assert(packetsLength() >= (int32)sizeof(T));

	size_t currSize = 0;
	size_t reclaimCount = 0;
	Packets::iterator iter = mPackets.begin();

	for (; iter != mPackets.end(); ++iter)
	{
		Packet* pPacket = (*iter);
		size_t remainSize = (size_t)sizeof(T) - currSize;
		if(pPacket->length() >= remainSize)
		{
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), remainSize);
			pPacket->rpos(pPacket->rpos() + remainSize);
			if(pPacket->length() == 0)
			{
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}
			break;
		}
		else
		{
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), pPacket->length());
			currSize += pPacket->length();
			pPacket->done();
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
			++reclaimCount;
		}
	}

	if(reclaimCount > 0)
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);
	return *this;
}