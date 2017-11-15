/******************************************************************************
*                                                                             *
*   Module:     vga.c                                                         *
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

        VGA text buffer output driver

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/01/00   Original                                             Goran Devic *
* 09/10/00   Second revision                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TOUT outVga = {0};

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define LINUX_VGA_TEXT  (ice_page_offset() + 0xB8000)

#define MAX_VGA_SIZEX       80
#define MAX_VGA_SIZEY       60

//---------------------------------------------------
// VGA registers and memory to save
//---------------------------------------------------

typedef struct
{
    BYTE CRTC[0x19];                    // CRTC Registers (3D4/3D5)
    BYTE SR[5];                         // Sequencer Registers (3C4/3C5)
    WORD textbuf[ MAX_VGA_SIZEX * MAX_VGA_SIZEY ];

} TVgaState;

static TVgaState vgaState;

static const int crtc_enable[0x19] = {     // 0x0A Cursor Start Register
    0, 0, 0, 0, 0, 0, 0, 0, // 0 - 7       // 0x0B Cursor End Register
    0, 0, 1, 1, 1, 1, 1, 1, // 8 - Fh      // 0x0C Start Address High
    0, 0, 0, 0, 0, 0, 0, 0, // 10h - 17h   // 0x0D Start Address Low
    1                       // 18h         // 0x0E Cursor Location High
};                                         // 0x0F Cursor Location Low
                                           // 0x18 Line Compare Register

//---------------------------------------------------
// Helper variables for display output
//---------------------------------------------------

typedef struct
{
    int col;                            // Current line's color index
    BYTE *pText;                        // Address of the VGA text buffer
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates
    BYTE fEnabled;                      // Output is enabled

} TVga;

static TVga vga;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void VgaSprint(char *s);
static void VgaCarret(BOOL fOn);
static void VgaMouse(int x, int y);
static BOOL VgaResize(int x, int y, int nFont);

/******************************************************************************
*                                                                             *
*   void VgaInit(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Initializes VGA output driver
*
******************************************************************************/
void VgaInit(void)
{
    memset(&vga, 0, sizeof(vga));
    memset(&outVga, 0, sizeof(outVga));

    // Set default parameters

    outVga.x = 0;
    outVga.y = 0;
    outVga.sizeX = 80;
    outVga.sizeY = 25;
    outVga.sprint = VgaSprint;
    outVga.carret = VgaCarret;
    outVga.mouse = VgaMouse;
    outVga.resize = VgaResize;

    vga.scrollTop = 0;
    vga.scrollBottom = MAX_VGA_SIZEY - 1;
    vga.pText = (BYTE *) LINUX_VGA_TEXT;
    vga.col = COL_NORMAL;
    vga.fEnabled = FALSE;
}


/******************************************************************************
*                                                                             *
*   static void SaveBackground(void)                                          *
*                                                                             *
*******************************************************************************
*
*   Saves the VGA text screen background and prepares driver for displaying
*
******************************************************************************/
static void SaveBackground(void)
{
    int index;

    // Store selected CRTC registers and program them with our values

    for( index=0; index<0x19; index++)
    {
//        if( crtc_enable[index] )
        {
            vgaState.CRTC[index] = ReadCRTC(index);
        }
    }

    // Set up VGA to something we can handle

    // TODO: Here we may want to set up standard VGA timing registers
    //       but for now we assume we are already in text mode

    WriteCRTC(0x18, 0xFF);

    // Save SR3 (Character map select) since we need to program both maps to the same value
    // 2.4.18 on Toshiba is using 2 different maps
    vgaState.SR[3] = ReadSR(3);
    // Bits are allocated this weird way:  - - A2 B2 A1 A0 B1 B0
    // We dont complicate for now and just write 0 selecting both sets 0
    WriteSR(3, 0 );

    // TODO: We will reuse the text buffer at the current address

    vga.pText = (BYTE *) LINUX_VGA_TEXT + ((vgaState.CRTC[0x0C] << 8) + vgaState.CRTC[0x0D]) * 2;

    // Store away what was in the text buffer on the screen

    memcpy(vgaState.textbuf, vga.pText, sizeof(vgaState.textbuf));
}


/******************************************************************************
*                                                                             *
*   static void RestoreBackground(void)                                       *
*                                                                             *
*******************************************************************************
*
*   Restores VGA text screen
*
******************************************************************************/
static void RestoreBackground(void)
{
    BYTE index;

    // Restore selected CRTC registers

    for( index=0; index<0x19; index++)
    {
//        if( crtc_enable[index] )
        {
            WriteCRTC(index, vgaState.CRTC[index]);
        }
    }

    // Save SR3 (Character map select) since we need to program both maps to the same value
    // 2.4.18 on Toshiba is using 2 different maps
    WriteSR(3, vgaState.SR[3]);

    // Restore the frame buffer content that was there before we stepped in

    memcpy(vga.pText, vgaState.textbuf, sizeof(vgaState.textbuf));
}


/******************************************************************************
*                                                                             *
*   static void VgaCarret(BOOL fOn)                                           *
*                                                                             *
*******************************************************************************
*
*   Shows the cursor at the current position
*
******************************************************************************/
static void VgaCarret(BOOL fOn)
{
    WORD wOffset;

    // Set the cursor on the VGA screen. Offset it by the display start address.
    // We ignore on/off message not to interfere with the "natural" blink rate

    wOffset = outVga.y * 80 + outVga.x;
    wOffset += (vgaState.CRTC[0x0C] << 8) | vgaState.CRTC[0x0D];

    WriteCRTC(0x0E, wOffset >> 8);
    WriteCRTC(0x0F, wOffset & 0xFF);

    // Set the cursor shape depending on the Insert/Overtype mode

    if( deb.fOvertype )
    {
        // Overtype mode - full cursor block
        WriteCRTC(0x0A, 0);                 // Cursor Start/End Registers
        WriteCRTC(0x0B, 14);
    }
    else
    {
        // Insert mode - line cursor shape
        WriteCRTC(0x0A, 13);
        WriteCRTC(0x0B, 14);
    }
}


/******************************************************************************
*                                                                             *
*   static void VgaMouse(int x, int y)                                        *
*                                                                             *
*******************************************************************************
*
*   Mouse display function
*
******************************************************************************/
static void VgaMouse(int x, int y)
{
}


/******************************************************************************
*                                                                             *
*   static BOOL VgaResize(int x, int y, int nFont)                            *
*                                                                             *
*******************************************************************************
*
*   Resize VGA text display
*
******************************************************************************/
static BOOL VgaResize(int x, int y, int nFont)
{
    // Limit sizes to 80 in width and 25-60 in the number of lines
    if( x == 80 )
    {
        if( y>=25 && y<=MAX_VGA_SIZEY )
        {
            // Assign new number of lines and repaint
            dputc(DP_RESTOREBACKGROUND);
            outVga.sizeY = y;
            dputc(DP_SAVEBACKGROUND);

            return( TRUE );
        }
        else
        {
            dprinth(1, "Lines should be between 25 and %d on a text VGA display!", MAX_VGA_SIZEY);
        }
    }
    else
    {
        dprinth(1, "Only WIDTH=80 supported on a text VGA display!");
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   static void ScrollUp()                                                    *
*                                                                             *
*******************************************************************************
*
*   Scrolls up a region
*
******************************************************************************/
static void ScrollUp()
{
    if( (vga.scrollTop < vga.scrollBottom) && (vga.scrollBottom < outVga.sizeY) )
    {
        // Scroll up all requested lines
        memmove(vga.pText + (vga.scrollTop * outVga.sizeX) * 2,
                vga.pText + ((vga.scrollTop + 1) * outVga.sizeX) * 2,
                outVga.sizeX * 2 * (vga.scrollBottom - vga.scrollTop));

        // Clear the last line
        memset_w(vga.pText + (vga.scrollBottom * outVga.sizeX) * 2,
               deb.col[COL_NORMAL] * 256 + ' ',
               outVga.sizeX );
    }
}


/******************************************************************************
*                                                                             *
*   VgaSprint(char *s)                                                        *
*                                                                             *
*******************************************************************************
*
*   String output to a VGA text buffer.
*
******************************************************************************/
void VgaSprint(char *s)
{
    BYTE c;
    UINT nTabs;

    while( (c = *s++) != 0 )
    {
        if( vga.fEnabled )
        {
            if( c==DP_TAB )
            {
                for(nTabs=deb.nTabs; nTabs; nTabs--)
                    VgaSprint(" ");
            }
            else
            switch( c )
            {
                case DP_ENABLE_OUTPUT:  // Which we ignore
                    break;

                case DP_DISABLE_OUTPUT:
                        vga.fEnabled = FALSE;
                    break;

                case DP_SAVEBACKGROUND:
                        SaveBackground();
                    break;

                case DP_RESTOREBACKGROUND:
                        RestoreBackground();
                    break;

                case DP_CLS:
                        // Clear the screen and reset the cursor coordinates
                        memset_w(vga.pText,
                            deb.col[COL_NORMAL] * 256 + ' ',
                            outVga.sizeY * outVga.sizeX);
                        outVga.x = 0;
                        outVga.y = 0;
                    break;

                case DP_SETCURSORXY:
                        outVga.x = (*s++)-1;
                        outVga.y = (*s++)-1;
                    break;

                case DP_SETCURSORSHAPE:
                        deb.fOvertype = (*s++)-1;
                    break;

                case DP_SAVEXY:
                        vga.savedX = outVga.x;
                        vga.savedY = outVga.y;
                    break;

                case DP_RESTOREXY:
                        outVga.x = vga.savedX;
                        outVga.y = vga.savedY;
                    break;

                case DP_SETSCROLLREGIONYY:
                        vga.scrollTop = (*s++)-1;
                        vga.scrollBottom = (*s++)-1;
                    break;

                case DP_SCROLLUP:
                        // Scroll a portion of the screen up and clear the bottom line
                        ScrollUp();
                    break;

                case DP_SCROLLDOWN:
                        // Scroll a portion of the screen down and clear the top line
                        if( (vga.scrollTop < vga.scrollBottom) && (vga.scrollBottom < outVga.sizeY) )
                        {
                            // Scroll down all requested lines
                            memmove(vga.pText + ((vga.scrollTop + 1) * outVga.sizeX) * 2,
                                    vga.pText + (vga.scrollTop * outVga.sizeX) * 2,
                                    outVga.sizeX * 2 * (vga.scrollBottom - vga.scrollTop));

                            // Clear the first line
                            memset_w(vga.pText + (vga.scrollTop * outVga.sizeX) * 2,
                                   deb.col[COL_NORMAL] * 256 + ' ',
                                   outVga.sizeX);
                        }
                    break;

                case DP_SETCOLINDEX:
                        vga.col = *s++;
                    break;

                case '\r':
                        // Erase all characters to the right of the cursor pos and move cursor back
                        memset_w(vga.pText + (outVga.x +  outVga.y * outVga.sizeX) * 2,
                                deb.col[vga.col] * 256 + ' ',
                                outVga.sizeX - outVga.x);
                        outVga.x = 0;
                        vga.col = COL_NORMAL;
                    break;

                case '\n':
                        // Go to a new line, possible autoscroll
                        outVga.x = 0;
                        vga.col = COL_NORMAL;

                        // Check if we are on the last line of autoscroll
                        if( vga.scrollBottom==outVga.y )
                            ScrollUp();
                        else
                            outVga.y++;
                    break;

                case DP_RIGHTALIGN:
                        // Right align the rest of the text
                        outVga.x = outVga.sizeX - strlen(s);
                    break;

                case DP_ESCAPE:
                        // Escape character prints the next code as raw ascii
                        c = *s++;

                        // This case continues into the default...!

                default:
                        // All printable characters
                        if( outVga.x < outVga.sizeX )
                        {
                            *(WORD *)(vga.pText + (outVga.x +  outVga.y * outVga.sizeX) * 2)
                                = (WORD) c + deb.col[vga.col] * 256;

                            // Advance the print position
                            outVga.x++;
                        }
                    break;
            }
        }
        else
        {
            if( c==DP_ENABLE_OUTPUT )
                vga.fEnabled = TRUE;
        }
    }
}

