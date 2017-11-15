/******************************************************************************
*                                                                             *
*   Module:     flow.c                                                        *
*                                                                             *
*   Date:       04/16/01                                                      *
*                                                                             *
*   Copyright (c) 2001 - 2001 Goran Devic                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        Flow control commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/16/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

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
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdXit(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Returns to the debugee
*
******************************************************************************/
BOOL cmdXit(char *args, int subClass)
{
    // If we entered debugger via INT1 or INT3, we need to advance eip first
    // to skip those 1-byte codes

    // TODO: How about 2-byte counterparts????

    if( deb.nInterrupt==1 || deb.nInterrupt==3 )
    {
//        deb.r->eip += 1;
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdGo(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   Runs the interrupted program
*
******************************************************************************/
BOOL cmdGo(char *args, int subClass)
{
    // If we entered debugger via INT1 or INT3, we need to advance eip first
    // to skip those 1-byte codes

    // TODO: How about 2-byte counterparts????

    if( deb.nInterrupt==1 || deb.nInterrupt==3 )
    {
        deb.r->eip += 1;
    }

    return( FALSE );
}

