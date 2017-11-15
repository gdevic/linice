/******************************************************************************
*                                                                             *
*   Module:     errors.c                                                      *
*                                                                             *
*   Date:       03/09/02                                                      *
*                                                                             *
*   Copyright (c) 2000 - 2002 Goran Devic                                     *
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

        This module contains code and data strings for error declaration.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/06/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

static char *Strings[MSG_LAST] = {
    "NOERROR",                                  // 0 - No error
    "Syntax error",                             // ERR_SYNTAX_ERROR                 1
    "Unknown command or macro",                 // ERR_COMMAND                      2
    "Not yet implemented",                      // ERR_NOT_IMPLEMENTED              3
    "Out of memory",                            // ERR_MEMORY                       4

    "Duplicate breakpoint",                     // ERR_BPDUP                        5
    "No more breakpoints available",            // ERR_BP_TOO_MANY                  6
    "Debug register is already being used",     // ERR_DRUSED                    7
    "All debug registers used",                 // ERR_DRUSEDUP                     8

    "Expression?? What expression?",            // ERR_EXP_WHAT                     9

    "Out of memory"                             // ERR_INT_OUTOFMEM                 10
};

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
*   char *Index2String(DWORD index)                                           *
*                                                                             *
*******************************************************************************
*
*   Returns a pointer to a message string indexed by 'index'
*
*   Where:
*       index is the message index
*
*   Returns:
*       String describing the error/message
*       "" if the index is incorrect
*
******************************************************************************/
char *Index2String(DWORD index)
{
    if( index < MSG_LAST )
        return( Strings[index] );

    return( "Internal error - bad error code" );
}

