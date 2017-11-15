/******************************************************************************
*                                                                             *
*   Module:     output.c                                                      *
*                                                                             *
*   Date:       03/11/01                                                      *
*                                                                             *
*   Copyright (c) 1997, 2001 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

    This module contains the printing code that is independent on the
    type of output. All printing goes through this module.

    If the first character of a string is '@', a line will be stored in the
    history buffer as well.
    

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 03/11/01         Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void dputc(char c)                                                        *
*                                                                             *
*******************************************************************************
*
*   A simple counterpart of the dprint().  This one sends a single character
*   to an output device.
*
******************************************************************************/
void dputc(char c)
{
    // This works fine with little-endian...
    WORD printBuf = c;

    pOut->sprint((char *) printBuf);
}    

    
/******************************************************************************
*                                                                             *
*   int dprint( char *format, ... )                                           *
*                                                                             *
*******************************************************************************
*
*   This is the main print function for debugger output.
*
*   Where:
*       format is the standard printf() format string
*       ... standard printf() list of arguments.
*
*   Returns:
*       number of characters actually printed.
*
******************************************************************************/
int dprint( char *format, ... )
{
    char printBuf[256], *pBuf = printBuf;
    int written;
    va_list arg;

    // Print the line into a string
    va_start( arg, format );
    written = vsprintf(pBuf, format, arg);
    va_end(arg);

    // If we need to strore the line into the history buffer, do it first
    if( pBuf[0]=='@' )
    {
        pBuf++;
        HistoryAdd(pBuf);
    }

    // Send the string to a current output device driver
    pOut->sprint(pBuf);

    return( written );
}


/******************************************************************************
*                                                                             *
*   BOOL dprinth( int nLineCount, char *format, ... )                         *
*                                                                             *
*******************************************************************************
*
*   This print function should be used by all commands. If a command prints
*   also in a window, line
*
*   Where:
*       nLineCount is the line count for printing in the history buffer
*       format is the standard printf() format string
*       ... standard printf() list of arguments.
*
*   Returns:
*       TRUE if continue printing
*       FALSE if abort further printing
*
******************************************************************************/
BOOL dprinth( int nLineCount, char *format, ... )
{
    CHAR Key;
    char printBuf[256], *pBuf = printBuf;
    va_list arg;

    // Print the line into a string
    va_start( arg, format );
    vsprintf(pBuf, format, arg);
    va_end(arg);

    // If we are printing in the history buffer, store it there as well
    if( pOut->y > pWin->h.Top )
    {
        HistoryAdd(pBuf);
        pOut->sprint(pBuf);

        // If we are printing to a history buffer, and the line count is reached,
        // print the help line and wait for a keypress
        if( (nLineCount % pWin->h.nLines)==0 )
        {
            dprint("%c%c%c    Press any key to continue; Esc to cancel\r", DP_SETCURSORXY, 1+0, 1+pOut->height-1);
            Key = GetKey(TRUE);
            if( Key==ESC )
                return( FALSE );
        }
    }
    else
    {
        // We are printing to a window
        pOut->sprint(pBuf);
    }

    return( TRUE );
}


