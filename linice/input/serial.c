/******************************************************************************
*                                                                             *
*   Module:     serial.c                                                      *
*                                                                             *
*   Date:       03/11/01                                                      *
*                                                                             *
*   Copyright (c) 1996-2001 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

          This module contains the low-level serial input handler code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/11/01   Original                                             Goran Devic *
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
*   Functions
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void SerialHandler(int port)                                              *
*                                                                             *
*******************************************************************************
*
*   This handler is used when the debugger has control.
*
*   This is a low-level serial port handler.
*
******************************************************************************/
void SerialHandler(int port)
{
    // If mouse has a control over the serial port, send it there

    // Queue mouse packets from the serial port:
    //
    //  D7   D6   D5   D4   D3   D2   D1   D0
    //  X    1    LB   RB   Y7   Y6   X7   X6
    //  X    0    X5   X4   X3   X2   X1   X0
    //  X    0    Y5   Y4   Y3   Y2   Y1   Y0
    //


    // Otherwise, it is a remote terminal

    // Or just a junk
}

