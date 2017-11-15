/******************************************************************************
*                                                                             *
*   Module:     font.h                                                        *
*                                                                             *
*   Date:       06/13/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2004 Goran Devic                                       *
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
