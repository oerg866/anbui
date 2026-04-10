/* STDINT.H Implementation For Microsoft C 7.00 */

#ifndef _STDINT_H_
#define _STDINT_H_

#include <limits.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;

#define UINT8_MAX UCHAR_MAX
#define INT8_MAX SCHAR_MAX
#define INT8_MIN SCHAR_MIN
#define UINT16_MAX USHRT_MAX
#define INT16_MAX SHRT_MAX
#define INT16_MIN SHRT_MIN
#define UINT32_MAX ULONG_MAX
#define INT32_MIN LONG_MIN
#define INT32_MAX LONG_MAX

#if defined(_MSC_VER) && (MSC_VER > 0x700) && !defined(SUPER_STRICT)
/* This annoying warning pops up all the time */
#pragma warning(disable: 4761) /* MS C: integral size mismatch in argument */
#endif

/* For some reason Microsoft C does not have SIZE_MAX ?? */
#define SIZE_MAX UINT16_MAX

#ifndef inline
#define inline __inline

#define vsnprintf _vsnprintf

#endif

#endif
