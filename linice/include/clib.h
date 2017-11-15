/******************************************************************************
*                                                                             *
*   Module:     clib.h                                                        *
*                                                                             *
*   Date:       09/11/00                                                      *
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

        This header file includes most of C-functions available as macros
        to a Linux module.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _CLIB_H_
#define _CLIB_H_

#include <linux/fs.h>                         // Include file operations file
#include <linux/ctype.h>                      // Include character types definition
#include <linux/string.h>                     // Include macros for string/memory
// #include <stdarg.h>                     // Include variable argument header

#include "ice-types.h"                  // Include exended data types

///////////////////////////////////////////////////////////////////////////////
//
// Key-codes
//
///////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT      0x1000          // <key> + SHIFT
#define CHAR_ALT        0x2000          // <key> + ALT
#define CHAR_CTRL       0x4000          // <key> + CTRL

// F1 Code have to start at 0x80 - code in edlin counts on it!
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

