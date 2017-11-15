/******************************************************************************
*                                                                             *
*   Module:     font.h                                                        *
*                                                                             *
*   Date:       06/13/02                                                      *
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

        This is a header file describing font definition.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 06/13/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _FONT_H_
#define _FONT_H_

#include "ice-limits.h"                 // Include our limits

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

typedef struct
{
    BYTE *Bitmap;                       // Address of the font bitmap
    int ysize;                          // Font height in pixels
} TFont;

extern TFont Font[MAX_FONTS];

// Define the ASCII code for various characters, note that this may change
// depending on the font loaded. In particular, the horizontal line may be
// 0xCA or 0xC4 ! (That's why it is abstracted here for now)

#define FONT_HLINE      0xCA            // Horizontal line

// TODO: Investigate why the current console font does not have these in my simulation box?
// TODO: Should we load our own font?
#define FONT_LEFT       24              // We stopped using these since on one font
#define FONT_RIGHT      25              // they dont exist
#define FONT_UP         26
#define FONT_DOWN       27


#endif // _FONT_H_
