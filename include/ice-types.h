/******************************************************************************
*                                                                             *
*   Module:     ice-types.h                                                   *
*                                                                             *
*   Date:       03/09/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 10/28/00   Original                                             Goran Devic *
* 03/09/01   Modified for LinIce                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_TYPES_H_
#define _ICE_TYPES_H_


#ifndef offsetof
#define offsetof(s,m) (int)&(((s*)0)->m)
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;

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
typedef unsigned short int CHAR;


#endif //  _ICE_TYPES_H_
