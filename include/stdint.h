#ifndef STDINT_H
#define STDINT_H

#include <climits>

// Definition of 8 bit types
#if UCHAR_MAX >= 255
typedef unsigned char uint8_t;
typedef signed char int8_t;
#elif USHRT_MAX >= 255
typedef unsigned short uint8_t;
typedef signed short int8_t;
#elif UINT_MAX >= 255
typedef unsigned int uint8_t;
typedef signed int int8_t;
#elif ULONG_MAX >= 255
typedef unsigned long uint8_t;
typedef signed long int8_t;
#endif

// Definition of 16 bit types
#if UCHAR_MAX >= 65535
typedef unsigned char uint16_t;
typedef signed char int16_t;
#elif USHRT_MAX >= 65535
typedef unsigned short uint16_t;
typedef signed short int16_t;
#elif UINT_MAX >= 65535
typedef unsigned int uint16_t;
typedef signed int int16_t;
#elif ULONG_MAX >= 65535
typedef unsigned long uint16_t;
typedef signed long int16_t;
#endif

// Definition of 32 bit types
#if UCHAR_MAX >= 4294967295
typedef unsigned char uint32_t;
typedef signed char int32_t;
#elif USHRT_MAX >= 4294967295
typedef unsigned short uint32_t;
typedef signed short int32_t;
#elif UINT_MAX >= 4294967295
typedef unsigned int uint32_t;
typedef signed int int32_t;
#elif ULONG_MAX >= 4294967295
typedef unsigned long uint32_t;
typedef signed long int32_t;
#endif

#endif STDINT_H