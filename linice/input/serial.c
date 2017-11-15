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
    ;
}
