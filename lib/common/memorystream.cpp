#include "MemoryStream.h"

static ObjectPool<MemoryStream> s_ObjPool("MemoryStream");

ObjectPool<MemoryStream>& MemoryStream::ObjPool()
{
	return s_ObjPool;
}

void MemoryStream::destroyObjPool()
{
// 	DEBUG_MSG(fmt::format("MemoryStream::destroyObjPool(): size {}.\n", 
// 		s_ObjPool.size()));

	s_ObjPool.destroy();
}

size_t MemoryStream::getPoolObjectBytes()
{
	size_t bytes = sizeof(rpos_) + sizeof(wpos_) + data_.capacity();
	return bytes;
}

void MemoryStream::onReclaimObject()
{
	if(data_.capacity() > DEFAULT_SIZE * 2)
		data_.reserve(DEFAULT_SIZE);

	clear(false);
}
