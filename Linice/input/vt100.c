/******************************************************************************
*                                                                             *
*   Module:     vt100.c                                                       *
*                                                                             *
*   Date:       05/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
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

        This module contains code for input from the VT100 terminal via
        serial port.

        Some translation is performed and that's it.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/01/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include hardware defines


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void VT100Input(BYTE data)                                                *
*                                                                             *
*******************************************************************************
*
*   Receives a byte from the VT100 terminal, translates it into internal
*   code and queues it in the input buffer.
*
******************************************************************************/
void VT100Input(BYTE data)
{
    static BOOL fESC = FALSE;
    WCHAR key = 0;

    // Are we are expecting another byte following the ESC code?
    if( fESC )
    {
        // Skip the leading [
        if( data=='[' )
            return;

        switch( data )
        {
            case 'A':       // Cursor UP
                key = UP;
                break;

            case 'B':       // Cursor DOWN
                key = DOWN;
                break;

            case 'C':       // Cursor RIGHT
                key = RIGHT;
                break;

            case 'D':       // Cursor LEFT
                key = LEFT;
                break;

            default:
                key = data;
                break;
        }

        if( key )
            PutKey(key);

        fESC = FALSE;
    }
    else
    {
        // New clean character
        switch( data )
        {
            case 0x1B:      // Escape code.. followed by the additional code(s)
                fESC = TRUE;
                return;

            case '\r':
                key = ENTER;
                break;

            case '\n':
                key = ENTER;
                break;

            default:
                key = data;
                break;
        }

        PutKey(key);
    }
}

