/******************************************************************************
*                                                                             *
*   Module:     ctype.c                                                       *
*                                                                             *
*   Date:       04/13/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2004 Goran Devic                                       *
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

        ANSI C / POSIC ctype library implementation

    Note: This file is taken from Yaos project, 2.0 string C library and
          slightly trimmed down

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/13/96   Original                                             Goran Devic *
* 08/14/02   Trimmed down for Linice project                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "ctype.h"                      // Include its own header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/
char _ctype_[257] = {
0,
_CONTROL,                      /*     0x 0  */
_CONTROL,                      /*     0x 1  */
_CONTROL,                      /*     0x 2  */
_CONTROL,                      /*     0x 3  */
_CONTROL,                      /*     0x 4  */
_CONTROL,                      /*     0x 5  */
_CONTROL,                      /*     0x 6  */
_CONTROL,                      /*     0x 7  */
_CONTROL,                      /*     0x 8  */
_CONTROL | _LOWCT,             /*     0x 9  */
_CONTROL | _LOWCT,             /*     0x A  */
_CONTROL | _LOWCT,             /*     0x B  */
_CONTROL | _LOWCT,             /*     0x C  */
_CONTROL | _LOWCT,             /*     0x D  */
_CONTROL,                      /*     0x E  */
_CONTROL,                      /*     0x F  */
_CONTROL,                      /*     0x10  */
_CONTROL,                      /*     0x11  */
_CONTROL,                      /*     0x12  */
_CONTROL,                      /*     0x13  */
_CONTROL,                      /*     0x14  */
_CONTROL,                      /*     0x15  */
_CONTROL,                      /*     0x16  */
_CONTROL,                      /*     0x17  */
_CONTROL,                      /*     0x18  */
_CONTROL,                      /*     0x19  */
_CONTROL,                      /*     0x1A  */
_CONTROL,                      /*     0x1B  */
_CONTROL,                      /*     0x1C  */
_CONTROL,                      /*     0x1D  */
_CONTROL,                      /*     0x1E  */
_CONTROL,                      /*     0x1F  */

_SPACE,                        /*    0x20  */
_PUNCT,                        /* !  0x21  */
_PUNCT,                        /* "  0x22  */
_PUNCT,                        /* #  0x23  */
_PUNCT,                        /* $  0x24  */
_PUNCT,                        /* %  0x25  */
_PUNCT,                        /* &  0x26  */
_PUNCT,                        /* '  0x27  */
_PUNCT,                        /* (  0x28  */
_PUNCT,                        /* )  0x29  */
_PUNCT,                        /* *  0x2A  */
_PUNCT,                        /* +  0x2B  */
_PUNCT,                        /* ,  0x2C  */
_PUNCT,                        /* -  0x2D  */
_PUNCT,                        /* .  0x2E  */
_PUNCT,                        /* /  0x2F  */
_DIGIT | _XDIGIT,              /* 0  0x30  */
_DIGIT | _XDIGIT,              /* 1  0x31  */
_DIGIT | _XDIGIT,              /* 2  0x32  */
_DIGIT | _XDIGIT,              /* 3  0x33  */
_DIGIT | _XDIGIT,              /* 4  0x34  */
_DIGIT | _XDIGIT,              /* 5  0x35  */
_DIGIT | _XDIGIT,              /* 6  0x36  */
_DIGIT | _XDIGIT,              /* 7  0x37  */
_DIGIT | _XDIGIT,              /* 8  0x38  */
_DIGIT | _XDIGIT,              /* 9  0x39  */
_PUNCT,                        /* :  0x3A  */
_PUNCT,                        /* ;  0x3B  */
_PUNCT,                        /* <  0x3C  */
_PUNCT,                        /* =  0x3D  */
_PUNCT,                        /* >  0x3E  */
_PUNCT,                        /* ?  0x3F  */
_PUNCT,                        /* @  0x40  */
_UPPER | _XDIGIT,              /* A  0x41  */
_UPPER | _XDIGIT,              /* B  0x42  */
_UPPER | _XDIGIT,              /* C  0x43  */
_UPPER | _XDIGIT,              /* D  0x44  */
_UPPER | _XDIGIT,              /* E  0x45  */
_UPPER | _XDIGIT,              /* F  0x46  */
_UPPER,                        /* G  0x47  */
_UPPER,                        /* H  0x48  */
_UPPER,                        /* I  0x49  */
_UPPER,                        /* J  0x4A  */
_UPPER,                        /* K  0x4B  */
_UPPER,                        /* L  0x4C  */
_UPPER,                        /* M  0x4D  */
_UPPER,                        /* N  0x4E  */
_UPPER,                        /* O  0x4F  */
_UPPER,                        /* P  0x50  */
_UPPER,                        /* Q  0x51  */
_UPPER,                        /* R  0x52  */
_UPPER,                        /* S  0x53  */
_UPPER,                        /* T  0x54  */
_UPPER,                        /* U  0x55  */
_UPPER,                        /* V  0x56  */
_UPPER,                        /* W  0x57  */
_UPPER,                        /* X  0x58  */
_UPPER,                        /* Y  0x59  */
_UPPER,                        /* Z  0x5A  */
_PUNCT,                        /* [  0x5B  */
_PUNCT,                        /* \  0x5C  */
_PUNCT,                        /* ]  0x5D  */
_PUNCT,                        /* ^  0x5E  */
_PUNCT,                        /* _  0x5F  */
_PUNCT,                        /* `  0x60  */
_LOWER | _XDIGIT,              /* a  0x61  */
_LOWER | _XDIGIT,              /* b  0x62  */
_LOWER | _XDIGIT,              /* c  0x63  */
_LOWER | _XDIGIT,              /* d  0x64  */
_LOWER | _XDIGIT,              /* e  0x65  */
_LOWER | _XDIGIT,              /* f  0x66  */
_LOWER,                        /* g  0x67  */
_LOWER,                        /* h  0x68  */
_LOWER,                        /* i  0x69  */
_LOWER,                        /* j  0x6A  */
_LOWER,                        /* k  0x6B  */
_LOWER,                        /* l  0x6C  */
_LOWER,                        /* m  0x6D  */
_LOWER,                        /* n  0x6E  */
_LOWER,                        /* o  0x6F  */
_LOWER,                        /* p  0x70  */
_LOWER,                        /* q  0x71  */
_LOWER,                        /* r  0x72  */
_LOWER,                        /* s  0x73  */
_LOWER,                        /* t  0x74  */
_LOWER,                        /* u  0x75  */
_LOWER,                        /* v  0x76  */
_LOWER,                        /* w  0x77  */
_LOWER,                        /* x  0x78  */
_LOWER,                        /* y  0x79  */
_LOWER,                        /* z  0x7A  */
_PUNCT,                        /* {  0x7B  */
_PUNCT,                        /* |  0x7C  */
_PUNCT,                        /* }  0x7D  */
_PUNCT,                        /* ~  0x7E  */
_CONTROL,                      /*   0x7F  */

                               /* 0x80 - 0xff */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

