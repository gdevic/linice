/******************************************************************************
*                                                                             *
*   Module:     clib.h                                                        *
*                                                                             *
*   Date:       03/11/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file includes most of C-functions available as macros
        to a Linux module.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/11/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _CLIB_H_
#define _CLIB_H_

#include <fs.h>                         // Include file operations file
#include <ctype.h>                      // Include character types definition
#include <string.h>                     // Include macros for string/memory
#include <stdarg.h>                     // Include variable argument header

#include "ice-types.h"                  // Include exended data types

///////////////////////////////////////////////////////////////////////////////
//
// Key-codes
//
///////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT      0x0100          // <key> + SHIFT
#define CHAR_ALT        0x0200          // <key> + ALT
#define CHAR_CTRL       0x0400          // <key> + CTRL

#define F1            0x80
#define F2            0x81
#define F3            0x82
#define F4            0x83
#define F5            0x84
#define F6            0x85
#define F7            0x86
#define F8            0x87
#define F9            0x88
#define F10           0x89
#define F11           0x8A
#define F12           0x8B

#define SF1           (F1 + 1 * 12)
#define SF2           (F2 + 1 * 12)
#define SF3           (F3 + 1 * 12)
#define SF4           (F4 + 1 * 12)
#define SF5           (F5 + 1 * 12)
#define SF6           (F6 + 1 * 12)
#define SF7           (F7 + 1 * 12)
#define SF8           (F8 + 1 * 12)
#define SF9           (F9 + 1 * 12)
#define SF10          (F10+ 1 * 12)
#define SF11          (F11+ 1 * 12)
#define SF12          (F12+ 1 * 12)

#define AF1           (F1 + 2 * 12)
#define AF2           (F2 + 2 * 12)
#define AF3           (F3 + 2 * 12)
#define AF4           (F4 + 2 * 12)
#define AF5           (F5 + 2 * 12)
#define AF6           (F6 + 2 * 12)
#define AF7           (F7 + 2 * 12)
#define AF8           (F8 + 2 * 12)
#define AF9           (F9 + 2 * 12)
#define AF10          (F10+ 2 * 12)
#define AF11          (F11+ 2 * 12)
#define AF12          (F12+ 2 * 12)

#define CF1           (F1 + 3 * 12)
#define CF2           (F2 + 3 * 12)
#define CF3           (F3 + 3 * 12)
#define CF4           (F4 + 3 * 12)
#define CF5           (F5 + 3 * 12)
#define CF6           (F6 + 3 * 12)
#define CF7           (F7 + 3 * 12)
#define CF8           (F8 + 3 * 12)
#define CF9           (F9 + 3 * 12)
#define CF10          (F10+ 3 * 12)
#define CF11          (F11+ 3 * 12)
#define CF12          (F12+ 3 * 12)


#define BACKSPACE     '\b'
#define TAB           '\t'
#define ENTER         '\n'
#define ESC           27
#define NUMLOCK       18
#define SCROLL        19
#define HOME          20
#define UP            21
#define PGUP          22
#define LEFT          23
#define RIGHT         24
#define END           25
#define DOWN          26
#define PGDN          28
#define INS           29
#define DEL           30


extern BYTE *ice_init_heap( size_t size );
extern void * _kMalloc( BYTE *pHeap, DWORD size );
extern void _kFree( BYTE *pHeap, void *mPtr );
extern void strtolower(char *str);

#endif //  _CLIB_H_
