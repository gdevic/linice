/******************************************************************************
*                                                                             *
*   Module:     output.c                                                      *
*                                                                             *
*   Date:       09/11/00                                                      *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

    This module contains the printing code that is independent on the
    type of output. All printing goes through this module.

    If the first character of a string is '@', a line will be stored in the
    history buffer as well.

    The cache output buffer is also here and some utility functions to deal
    with it. Cache output buffer is a simple ASCII text buffer to which
    shadowed print is made. It is used with high-resolution scrolling to
    read characters to scroll and serial line not to send redundant codes.
    (TODO)

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 09/11/00         Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "font.h"                       // Include font header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Cache output buffer
BYTE cacheText[MAX_OUTPUT_SIZEY][MAX_OUTPUT_SIZEX] = {{0}};

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

void CacheTextScrollUp(DWORD top, DWORD bottom)
{
    // Scroll up all requested lines
    memmove(&cacheText[top][0], &cacheText[top+1][0], MAX_OUTPUT_SIZEX * (bottom-top));

    // Clear the last line
    memset(&cacheText[bottom][0], 0, MAX_OUTPUT_SIZEX);
}

void CacheTextCls()
{
    // Clear complete cache to spaces
    memset(cacheText, ' ', MAX_OUTPUT_SIZEX * MAX_OUTPUT_SIZEY);
}

/******************************************************************************
*                                                                             *
*   void dputc(UCHAR c)                                                       *
*                                                                             *
*******************************************************************************
*
*   A simple counterpart of the dprint().  This one sends a single character
*   to an output device.
*
******************************************************************************/
void dputc(UCHAR c)
{
    // This works fine with little-endian...
    WORD printBuf = (WORD) c;

    pOut->sprint((char *) &printBuf);
}


/******************************************************************************
*                                                                             *
*   int PrintLine(char *format,...)                                           *
*                                                                             *
*******************************************************************************
*
*   Prints a loosely formatted header line. By default, the first character
*   starts indented.
*
*   Where:
*       format is the standard printf() format string
*       ... standard printf() list of arguments.
*
*   Returns:
*       number of characters actually printed.
*
******************************************************************************/
int PrintLine(char *format,...)
{
    char printBuf[MAX_OUTPUT_SIZEX+6];
    char *pBuf;
    int written;
    va_list arg;

    // Clear the print buffer with spaces
    memset(printBuf, ' ', sizeof(printBuf));

    // First two characters contain the line color codes
    printBuf[0] = DP_SETCOLINDEX;
    printBuf[1] = COL_LINE;

    // We start printing at the second byte in the buffer
    pBuf = &printBuf[2];

    // Print the line into a buffer starting indented
    va_start( arg, format );
    written = ivsprintf(pBuf + 4, format, arg);
    va_end(arg);

    // Let the line expand into the whole buffer that we spaced
    pBuf[4+written] = ' ';      // This zaps the trailing '0' written by ivsprintf()

    // Terminate the buffer string, add 4 extra chars to accomodate data window
    // line extra color sequences
    pBuf[pOut->sizeX + 4]   = '\n';
    pBuf[pOut->sizeX + 5] = 0;

    // Change spaces into a graphics line character
    while( *pBuf )
    {
        if( *pBuf==' ' )
            *pBuf = FONT_HLINE;         // Horizontal line extended code
        pBuf++;
    }

    // Send the string to a current output device driver
    pOut->sprint(printBuf);

    return( pOut->sizeX );
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
    char printBuf[MAX_STRING+1];
    int written;
    va_list arg;

    // Print the line into a string
    va_start( arg, format );
    written = ivsprintf(printBuf, format, arg);
    va_end(arg);

    // Send the string to a current output device driver
    pOut->sprint(printBuf);

    return( written );
}


/******************************************************************************
*                                                                             *
*   BOOL dprinth( int nLineCount, char *format, ... )                         *
*                                                                             *
*******************************************************************************
*
*   This print function should be used by all commands.
*
*   Where:
*       nLineCount is the line count for printing in the history buffer
*           Start with 1 and increment for every line.
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
    static char printBuf[1024];

    WCHAR Key;
    char *pBuf = printBuf;
    int written;
    va_list arg;

    // Print the line into a string
    va_start( arg, format );
    written = ivsprintf(pBuf, format, arg);
    va_end(arg);

    // If we are printing to the history buffer, store it there as well
    if( pOut->y > pWin->h.Top )
    {
        HistoryAdd(pBuf);
        pOut->sprint(pBuf);

        // If we are printing to a history buffer, and the line count is reached,
        // print the help line and wait for a keypress, unless the PAUSE is set to OFF
        if( nLineCount % ((pWin->h.nLines-1) )==0 )
        {
            if( deb.fPause==TRUE )
            {
                // PAUSE is on, so do all that wait for key stuff...

                dprint("%c%c%c%c%c%c    Press any key to continue; Esc to cancel\r%c",
                    DP_SAVEXY,
                    DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
                    DP_SETCOLINDEX, COL_HELP,
                    DP_RESTOREXY);
                Key = GetKey(TRUE);
            }
            else
            {
                // PAUSE is FALSE, but still peek a keyboard so we can abort if we want to
                Key = GetKey(FALSE);
            }

            // Move cursor to a new line
            dprint("\n");

            // If ESC or 'q' was pressed in any case, abort
            if( Key==ESC || Key=='q' || Key=='Q' )
                return(FALSE);
        }
        else
            dprint("\n");
    }
    else
    {
        // We are printing within a window
        // Append new line
        strcat(pBuf, "\n");
        pOut->sprint(pBuf);
    }

    return( TRUE );
}

