#ifndef __MEMORYSTREAMCONVERTER_H__
#define __MEMORYSTREAMCONVERTER_H__

#include "common/common.h"

template<size_t T>
inline void convert(char *val)
{
	std::swap(*val, *(val + T - 1));
	convert<T - 2>(val + 1);
}

template<> inline void convert<0>(char *) {}
template<> inline void convert<1>(char *) {}

template<typename T> inline void apply(T *val)
{
	convert<sizeof(T)>((char *)(val));
}

inline void convert(char *val, size_t size)
{
	if(size < 2)
		return;

	std::swap(*val, *(val + size - 1));
	convert(val + 1, size - 2);
}

#if ENDIAN == BIG_ENDIAN
template<typename T> inline void EndianConvert(T& val) { apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }
#else
template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { apply<T>(&val); }
#endif

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(uint8&) { }
inline void EndianConvert(int8&) { }
inline void EndianConvertReverse(uint8&) { }
inline void EndianConvertReverse(int8&) { }

#endif // __MEMORYSTREAMCONVERTER_H__
