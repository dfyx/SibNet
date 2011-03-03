#ifndef NETWORKING_H
#define NETWORKING_H

#ifdef NETWORKING_EXPORTS
    #ifdef _WIN32
    #define DLLDIR  __declspec(dllexport)   // export DLL information
    #else
    #define DLLDIR export
    #endif
#else
    #ifdef _WIN32
    #define DLLDIR  __declspec(dllimport)   // import DLL information
    #else
    #define DLLDIR
    #endif
#endif 

#include <stdint_wrap.h>

// Byte order stuff
inline bool mustswap()
{
	static const uint16_t x = 1;
	return *((const char*) &x) == 1;
}

inline uint8_t swap(uint8_t input)
{
	return input;
}

inline uint16_t swap(uint16_t input)
{
	if(mustswap())
	{
		return ((input >> 8) & 0x00FFU) |
			   ((input << 8) & 0xFF00U);
	}
	else
	{
		return input;
	}
}

inline uint32_t swap(uint32_t input)
{
	if(mustswap())
	{
		return ((input >> 24) & 0x000000FFUL) |
			   ((input >>  8) & 0x0000FF00UL) |
			   ((input <<  8) & 0x00FF0000UL) |
			   ((input << 24) & 0xFF000000UL);
	}
	else
	{
		return input;
	}
}

#ifdef UINT64_MAX
inline uint64_t swap(uint64_t input)
{
	if(mustswap())
	{
		return ((input >> 56) & 0x00000000000000FFULL) |
			   ((input >> 40) & 0x000000000000FF00ULL) |
			   ((input >> 24) & 0x0000000000FF0000ULL) |
			   ((input >>  8) & 0x00000000FF000000ULL) |
			   ((input <<  8) & 0x000000FF00000000ULL) |
			   ((input << 24) & 0x0000FF0000000000ULL) |
			   ((input << 40) & 0x00FF000000000000ULL) |
			   ((input << 56) & 0xFF00000000000000ULL);
	}
	else
	{
		return input;
	}
}
#endif
// End of byte order stuff

#define _DEBUGSIBNET 1
#ifdef _DEBUGSIBNET
#include <string>
#include <iostream>
#include <errno.h>
#include <cstring>
#define DEBUG_ERROR(x)						\
	std::cout << "! " << x << std::endl << std::flush;	/*\
	std::cerr << "  Error code was " << errno << ": " << strerror(errno) << "." << std::endl; /*\
	if(nlGetError() == NL_SYSTEM_ERROR)		\
	std::cerr << "  System error code was " << nlGetSystemError() << ": " << (char*) nlGetSystemErrorStr(nlGetSystemError()) << "." << std::endl;*/

#define DEBUG_NOTICE(x)						\
	std::cout << "? " << x << std::endl;
#else
#define DEBUG_ERROR(x)
#define DEBUG_NOTICE(x)
#endif
#endif // NETWORKING_H