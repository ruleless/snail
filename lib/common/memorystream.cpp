/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "memorystream.h"
namespace KBEngine
{
static ObjectPool<MemoryStream> s_ObjPool("MemoryStream");
//-------------------------------------------------------------------------------------
ObjectPool<MemoryStream>& MemoryStream::ObjPool()
{
	return s_ObjPool;
}

//-------------------------------------------------------------------------------------
void MemoryStream::destroyObjPool()
{
	DEBUG_MSG(fmt::format("MemoryStream::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

//-------------------------------------------------------------------------------------
size_t MemoryStream::getPoolObjectBytes()
{
	size_t bytes = sizeof(rpos_) + sizeof(wpos_) + data_.capacity();
	return bytes;
}

//-------------------------------------------------------------------------------------
void MemoryStream::onReclaimObject()
{
	if(data_.capacity() > DEFAULT_SIZE * 2)
		data_.reserve(DEFAULT_SIZE);

	clear(false);
}

//-------------------------------------------------------------------------------------
} 


