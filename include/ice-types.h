/******************************************************************************
*                                                                             *
*   Module:     ice-types.h                                                   *
*                                                                             *
*   Date:       09/09/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This source code and produced executable is copyrighted by Goran Devic.   *
*   This source, portions or complete, and its derivatives can not be given,  *
*   copied, or distributed by any means without explicit written permission   *
*   of the copyright owner. All other rights, including intellectual          *
*   property rights, are implicitly reserved. There is no guarantee of any    *
*   kind that this software would perform, and nobody is liable for the       *
*   consequences of running it. Use at your own risk.                         *
*                                                                             *
*******************************************************************************

    Module Description:

        Define some extended data types

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/28/00   Original                                             Goran Devic *
* 09/09/00   Modified for Linice                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_TYPES_H_
#define _ICE_TYPES_H_


#ifndef offsetof
#define offsetof(s,m) (int)&(((s*)0)->m)
#endif

typedef unsigned char  BYTE;            // 8  bits
typedef unsigned short WORD;            // 16 bits
typedef signed short   SWORD;           // 16 bits
typedef unsigned int   DWORD;           // 32 bits
typedef unsigned int   BOOL;            // - not relevant -
typedef unsigned int   UINT;            // - not relevant -
typedef int            ENUM;            // - not relevant -
typedef char *         PSTR;            // - not relevant - (pointer size)

#ifndef TRUE
#define TRUE    (1==1)
#endif

#ifndef FALSE
#define FALSE   (1==0)
#endif

#ifndef MIN
#define MIN(a,b)        ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b)        ((a)<(b)?(b):(a))
#endif

// We define character type as a 16-bit unsigned int so we can use
// top several bits for the state of the key modifiers (control/alt)
typedef unsigned short int WCHAR;

// When we need it, character type is unsigned
typedef unsigned char UCHAR;

///////////////////////////////////////////////////////////////////////////////
// Define packed structure for MSVC and gcc
///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32

#define PACKED
#pragma pack(1)

#else // !WINDOWS

#define PACKED __attribute__((packed))

#endif // WINDOWS


#endif //  _ICE_TYPES_H_

