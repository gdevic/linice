/******************************************************************************
*                                                                             *
*   Module:     ice-keycode.h                                                 *
*                                                                             *
*   Date:       08/26/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   *
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

