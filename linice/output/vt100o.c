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

        This module contains code for output to the VT100 terminal via
        serial port.

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

TOUT outVT100 = {0};

#define FONT_HLINE      0xCA            // Horizontal line (defined in font.h)

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static UINT VT100_INIT_X = 80;          // Initial width
static UINT VT100_INIT_Y = 24;          // Initial number of lines

typedef struct                          // Define VT100 terminal structure
{
    int col;                            // Current line's color index
    int lastColor;
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates

} PACKED TVT100;

static TVT100 TVT;                      // VT100 terminal structure

static char sBuf[32] = { 0x1B, };       // Terminal out ESC string

// Init string for the VT100 terminal:
// 1. Reset the VT100 terminal
// 2. Set no wrapping
// 3. Cursor home
// 4. Erase screen
// 5  Reset all attributes
//                        1         2       3      4       5
static char *sInitVT00 = "\0x1B\0x66\0x1B[7l\0x1B[H\0x1B[2J\0x1B[0m";

// VGA -> VT100 color translation table
// Since VGA text mode colors can define up to 16 colors, and VT100 terminal
// can use only 7, we need to do some translation and a BOLD attribute
static BYTE colorTab[16] =
{
    0, 4, 2, 6, 1, 5, 7, 7,
    0, 4, 2, 6, 1, 5, 3, 7
};

static BYTE boldTab[16] =
{
    0, 0, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1
};


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void SerialOut(BYTE data);
extern void SerialOutString(char *str);
extern void SerialMouse(int x, int y);

static BOOL SerialResize(int x, int y, int nFont);
static void SerialSprint(char *s);


/******************************************************************************
*                                                                             *
*   int InitVT100(void)                                                       *
*                                                                             *
*******************************************************************************
*
*   Initializes VT100 terminal
*
******************************************************************************/
int InitVT100(void)
{
    memset(&TVT, 0, sizeof(TVT));
    TVT.scrollBottom = 0xFF;            // They need to be out of range
    TVT.scrollTop    = 0xFF;

    //========================================================================
    // Initialize global output structure
    //========================================================================

    memset(&outVT100, 0, sizeof(outVT100));

    // Set default parameters

    outVT100.x = 0;
    outVT100.y = 0;
    outVT100.sizeX = VT100_INIT_X;
    outVT100.sizeY = VT100_INIT_Y;
    outVT100.sprint = SerialSprint;
    outVT100.mouse = SerialMouse;
    outVT100.resize = SerialResize;

    // Send the init string to the VT100 terminal
    SerialOutString(sInitVT00);

    return(0);
}


/******************************************************************************
*                                                                             *
*   static BOOL SerialResize(int x, int y, int nFont)                         *
*                                                                             *
*******************************************************************************
*
*   Resize on a serial terminal is not supported
*
******************************************************************************/
static BOOL SerialResize(int x, int y, int nFont)
{
    if( x!=80 && x!=132 )
    {
        dprinth(1, "Serial terminal width has to be 80 or 132");
        return( FALSE );
    }

    if( y!=24 && y!=25 && y!=48 && y!=50 )
    {
        dprinth(1, "Serial terminal number of lines has to be 24, 25, 48 or 50");
        return( FALSE );
    }

    dputc(DP_RESTOREBACKGROUND);

    // Set the new serial terminal size, and store that size as the default for
    // the next time if we are switching to a terminal from some other device
    outVT100.sizeX = VT100_INIT_X = x;
    outVT100.sizeY = VT100_INIT_Y = y;

    dputc(DP_SAVEBACKGROUND);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   static void SetCursorPos(void)                                            *
*                                                                             *
*******************************************************************************
*
*   Set the cursor position and colors
*
******************************************************************************/
static void SetCursorPos(void)
{
    sprintf(sBuf+1, "[%d;%dH", outVT100.y+1, outVT100.x+1);
    SerialOutString(sBuf);
}


/******************************************************************************
*                                                                             *
*   SerialSprint(char *s)                                                     *
*                                                                             *
*******************************************************************************
*
*   String output through the serial port.
*
******************************************************************************/
static void SerialSprint(char *s)
{
    BYTE c;
    UINT nTabs;

    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(nTabs=deb.nTabs; nTabs; nTabs--)
                SerialSprint(" ");
        }
        else
        switch( c )
        {
            case DP_ENABLE_OUTPUT:
            case DP_DISABLE_OUTPUT:
                    // Enable and disable output are ignored on a terminal device
                break;

            case DP_SAVEBACKGROUND:
            case DP_RESTOREBACKGROUND:
                    // Save and restore background are ignored on a terminal device
                break;

            case DP_CLS:
                    // Clear the screen and reset the cursor coordinates
                    sprintf(sBuf+1, "[2J");
                    SerialOutString(sBuf);

                    outVT100.x = 0;
                    outVT100.y = 0;
                break;

            case DP_SETCURSORXY:
                    outVT100.x = (*s++)-1;
                    outVT100.y = (*s++)-1;

                    SetCursorPos();
                break;

            case DP_SAVEXY:
                    sprintf(sBuf+1, "[s");
                    SerialOutString(sBuf);

                    TVT.savedX = outVT100.x;
                    TVT.savedY = outVT100.y;
                break;

            case DP_RESTOREXY:
                    sprintf(sBuf+1, "[u");
                    SerialOutString(sBuf);

                    outVT100.x = TVT.savedX;
                    outVT100.y = TVT.savedY;
                break;

            case DP_SETSCROLLREGIONYY:
                    TVT.scrollTop = (*s++)-1;
                    TVT.scrollBottom = (*s++)-1;

                    sprintf(sBuf+1, "[%d;%dr", TVT.scrollTop + 1, TVT.scrollBottom + 1);
                    SerialOutString(sBuf);
                break;

            case DP_SCROLLUP:
                    // Scroll a portion of the screen up and clear the bottom line
                    sprintf(sBuf+1, "M");
                    SerialOutString(sBuf);
                break;

            case DP_SCROLLDOWN:
                    // Scroll a portion of the screen down and clear the top line
                    sprintf(sBuf+1, "D");
                    SerialOutString(sBuf);
                break;

            case DP_SETCOLINDEX:
                    TVT.col = *s++;
                break;

            case '\r':
                    // Erase all characters to the right of the cursor pos and move cursor back
                    sprintf(sBuf+1, "[K");
                    SerialOutString(sBuf);

                    outVT100.x = 0;
                    TVT.col = COL_NORMAL;

                    SetCursorPos();
                break;

            case '\n':
                    // Go to a new line, possible autoscroll
                    SerialOut('\r');
                    SerialOut('\n');

                    outVT100.x = 0;
                    TVT.col = COL_NORMAL;

                    // Check if we are on the last line of autoscroll
                    if( TVT.scrollBottom==outVT100.y )
                    {
                        ;
                    }
                    else
                    {
                        outVT100.y++;
                    }
                break;

            case DP_RIGHTALIGN:
                    // Right align the rest of the text - this is ignored in serial out;
                    // Instead, we print a space and continue into a default statement
                    c = ' ';

            case DP_ESCAPE:
                    // Escape character prints the next code as raw ascii
                    // We ignore all codes less than the space
                    if( c==DP_ESCAPE )
                    {
                        c = *s++;
                    }

            default:
                    // All printable characters with few exceptions:
                    if( c==FONT_HLINE )     // Horizontal line graphics character
                        c = '-';
                    if( c>127 || c<32 )     // Non-ANSI characters
                        c = '.';

                    // Update color attribute on color change
                    if( TVT.lastColor != TVT.col )
                    {
                        sprintf(sBuf+1, "[%d;%d;%dm",
                            boldTab[deb.col[TVT.col] & 0x0F],
                            colorTab[deb.col[TVT.col] >> 4] + 40,
                            colorTab[deb.col[TVT.col] & 0x0F] + 30);
                        SerialOutString(sBuf);
                        TVT.lastColor = TVT.col;
                    }

                    if( outVT100.x < outVT100.sizeX )
                    {
                        SerialOut(c);

                        // Advance the print position
                        outVT100.x++;
                    }
                break;
        }
    }
}


