/*****************************************************************************
*                                                                             *
*   Module:     vga.c                                                         *
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

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

#include <asm/page.h>                   // We need page offset

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TOUT outVga;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define LINUX_VGA_TEXT  (PAGE_OFFSET + 0xB8000)

#define MAX_SIZEX       80
#define MAX_SIZEY       60

//---------------------------------------------------
// VGA registers and memory to save
//---------------------------------------------------

typedef struct
{
    BYTE CRTC[0x19];                    // CRTC Registers
    WORD textbuf[ MAX_SIZEX * MAX_SIZEY ];

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

} TVga;

static TVga vga;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void VgaSprint(char *s);
static void VgaMouse(int x, int y);

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

    // Set default parameters

    outVga.x = 0;
    outVga.y = 0;
    outVga.sizeX = 80;
    outVga.sizeY = 25;
    outVga.sprint = VgaSprint;
    outVga.mouse = VgaMouse;

    vga.scrollTop = 0;
    vga.scrollBottom = MAX_SIZEY - 1;
    vga.pText = (BYTE *) LINUX_VGA_TEXT;
    vga.col = COL_NORMAL;
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
        if( crtc_enable[index] )
        {
            vgaState.CRTC[index] = ReadCRTC(index);
        }
    }

    // Store away what was in the text buffer that we will use

    memcpy(vgaState.textbuf, vga.pText, outVga.sizeX * outVga.sizeY * 2);

    // Set up VGA to something we can handle

    WriteCRTC(0x18, 0xFF);              // Line Compare Register

    // Adjust the effective text display start address, since Linux
    // console may have scrolled down

    vga.pText = (BYTE *) LINUX_VGA_TEXT + ((vgaState.CRTC[0x0C] << 8) + vgaState.CRTC[0x0D]) * 2;
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
        if( crtc_enable[index] )
        {
            WriteCRTC(index, vgaState.CRTC[index]);
        }
    }

    // Restore the frame buffer content that was there before we stepped in

    memcpy(vga.pText, vgaState.textbuf, outVga.sizeX * outVga.sizeY * 2);
}


/******************************************************************************
*                                                                             *
*   static void ShowCursorPos(void)                                           *
*                                                                             *
*******************************************************************************
*
*   Shows the cursor at the current position
*
******************************************************************************/
static void ShowCursorPos(void)
{
    WORD wOffset;

    // Set the cursor on the VGA screen. Offset it by the display start address.

    wOffset = outVga.y * 80 + outVga.x;
    wOffset += (vgaState.CRTC[0x0C] << 8) | vgaState.CRTC[0x0D];

    WriteCRTC(0x0E, wOffset >> 8);
    WriteCRTC(0x0F, wOffset & 0xFF);

    // Set the cursor shape depending on the Insert/Overtype mode

    if( outVga.fOvertype )
    {
        // Overtype mode - full cursor block
        WriteCRTC(0x0A, 0);                 // Cursor Start Register
        WriteCRTC(0x0B, 7);                 // Cursor End Register
    }
    else
    {
        // Insert mode - line cursor shape
        WriteCRTC(0x0A, 6);                 // Cursor Start Register
        WriteCRTC(0x0B, 7);                 // Cursor End Register
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
    WORD wOffset;

    // Set the mouse cursor on the VGA screen

    wOffset = y * 80 + x;
    wOffset += (vgaState.CRTC[0x0C] << 8) | vgaState.CRTC[0x0D];

    WriteCRTC(0x0E, wOffset >> 8);
    WriteCRTC(0x0F, wOffset & 0xFF);

    // Since we use hardware VGA cursor to show the mouse location, set
    // its shape to block

    WriteCRTC(0x0A, 0);                 // Cursor Start Register
    WriteCRTC(0x0B, 7);                 // Cursor End Register
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
               pIce->col[COL_NORMAL] * 256 + ' ',
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

    while( (c = *s++) != 0 )
    {
        switch( c )
        {
            case DP_SAVEBACKGROUND:
                    SaveBackground();
                break;

            case DP_RESTOREBACKGROUND:
                    RestoreBackground();
                break;

            case DP_CLS:
                    // Clear the screen and reset the cursor coordinates
                    memset_w(vga.pText,
                        pIce->col[COL_NORMAL] * 256 + ' ',
                        outVga.sizeY * outVga.sizeX);
                    outVga.x = 0;
                    outVga.y = 0;
                break;

            case DP_SETCURSORXY:
                    outVga.x = (*s++)-1;
                    outVga.y = (*s++)-1;
                break;

            case DP_SETCURSORSHAPE:
                    outVga.fOvertype = (*s++)-1;
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
                               pIce->col[COL_NORMAL] * 256 + ' ',
                               outVga.sizeX);
                    }
                break;

            case DP_SETCOLINDEX:
                    vga.col = *s++;
                break;

            case '\r':
                    // Erase all characters to the right of the cursor pos and move cursor back
                    memset_w(vga.pText + (outVga.x +  outVga.y * outVga.sizeX) * 2,
                            pIce->col[vga.col] * 256 + ' ',
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

            default:
                    // All printable characters
                    *(WORD *)(vga.pText + (outVga.x +  outVga.y * outVga.sizeX) * 2)
                        = (WORD) c + pIce->col[vga.col] * 256;

                    // Advance the print position
                    if( outVga.x < outVga.sizeX )
                        outVga.x++;
                break;
        }

        ShowCursorPos();
    }
}


