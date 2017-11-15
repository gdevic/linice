/******************************************************************************
*                                                                             *
*   Module:     ctype.h                                                       *
*                                                                             *
*   Date:       04/13/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2005 Goran Devic                                       *
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

        ANSI C / POSIX ctype header file

    Note: This file is taken from Yaos project, 2.0 string C library and
          slightly trimmed down

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/13/96   Original                                             Goran Devic *
* 08/14/02   Trimmed down for Linice project                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _CTYPE_H_
#define _CTYPE_H_

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern char _ctype_[257];

#define _LOWER         0x01
#define _UPPER         0x02
#define _DIGIT         0x04
#define _XDIGIT        0x08
#define _CONTROL       0x10
#define _LOWCT         0x20
#define _SPACE         0x40
#define _PUNCT         0x80

#define _CASE          0x20

/* POSIX */

#define isalnum(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER | _DIGIT))
#define isalpha(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER))
#define iscntrl(c)     ((_ctype_[(c)+1])&(_CONTROL))
#define isdigit(c)     ((_ctype_[(c)+1])&(_DIGIT))
#define isgraph(c)     ((_ctype_[(c)+1])&(_PUNCT | _DIGIT | _UPPER | _LOWER))
#define islower(c)     ((_ctype_[(c)+1])&(_LOWER))
#define isprint(c)     ((_ctype_[(c)+1])&(_SPACE | _PUNCT | _DIGIT | _UPPER | _LOWER))
#define ispunct(c)     ((_ctype_[(c)+1])&(_PUNCT))
#define isspace(c)     ((_ctype_[(c)+1])&(_SPACE | _LOWCT))
#define isupper(c)     ((_ctype_[(c)+1])&(_UPPER))
#define isxdigit(c)    ((_ctype_[(c)+1])&(_XDIGIT))

#define tolower(c)     (isupper(c)? (c)|_CASE : (c))
#define toupper(c)     (islower(c)? (c)&~_CASE : (c))


/* Non-POSIX */

#define isascii(c)     ((unsigned)(c) < 128)


#endif // _CTYPE_H_
