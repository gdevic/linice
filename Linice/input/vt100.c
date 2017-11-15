/******************************************************************************
*                                                                             *
*   Module:     vt100.c                                                       *
*                                                                             *
*   Date:       05/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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
    CHAR key = 0;

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

