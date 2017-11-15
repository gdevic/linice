/******************************************************************************
*                                                                             *
*   Module:     ice-keycode.h                                                 *
*                                                                             *
*   Date:       08/26/02                                                      *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

        This header file defines special key codes used by keyboard
        layout routines and linice.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/26/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _KEYCODE_H_
#define _KEYCODE_H_

///////////////////////////////////////////////////////////////////////////////
//
// Key-codes
//
///////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT      0x1000          // <key> + SHIFT
#define CHAR_ALT        0x2000          // <key> + ALT
#define CHAR_CTRL       0x4000          // <key> + CTRL

// Define pseudo-ascii codes for control characters:

#define UP              2
#define DOWN            3
#define PGUP            4
#define PGDN            5
#define LEFT            6
#define RIGHT           7

#define BACKSPACE       8               // ASCII '\b'
#define TAB             9               // ASCII '\t'
#define ENTER           10              // ASCII '\n'

#define F1              11
#define F2              12
#define F3              13
#define F4              14
#define F5              15
#define F6              16
#define F7              17
#define F8              18
#define F9              19
#define F10             20
#define F11             21
#define F12             22

#define NUMLOCK         23
#define SCROLL          24
#define INS             25
#define DEL             26

#define ESC             27

#define HOME            28
#define END             29


#define UNUSED1         1
#define UNUSED2         30
#define UNUSED3         31


#endif //  _KEYCODE_H_

