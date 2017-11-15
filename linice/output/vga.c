/******************************************************************************
*                                                                             *
*   Module:     vga.c                                                         *
*                                                                             *
*   Date:       11/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        VGA text output driver

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/01/00   Original                                             Goran Devic *
* 03/10/01   Second revision                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include <asm/page_offset.h>            // We need page offset

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

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

#define LINUX_VGA_TEXT  (PAGE_OFFSET_RAW + 0xB8000)

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

    vga.pText = (BYTE *) LINUX_VGA_TEXT;
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

    WriteCRTC(0x0A, 0);                 // Cursor Start Register
    WriteCRTC(0x0B, 7);                 // Cursor End Register
    WriteCRTC(0x0C, 0);                 // Start Address High
    WriteCRTC(0x0D, 0);                 // Start Address Low
    WriteCRTC(0x18, 0x3FF);             // Line Compare Register
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

    // Set the cursor on the VGA screen

    wOffset = outVga.y * 40 * 2 + outVga.x * 2;

    WriteCRTC(0x0E, wOffset >> 8);
    WriteCRTC(0x0F, wOffset & 0xFF);
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
        memset(vga.pText + (vga.scrollBottom * outVga.sizeY) * 2,
               0,
               outVga.sizeX * 2 );
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
                    memset(vga.pText, 0, outVga.sizeY * outVga.sizeX * 2);
                    outVga.x = 0;
                    outVga.y = 0;
                break;

            case DP_SETCURSORXY:
                    outVga.x = (*s++)-1;
                    outVga.y = (*s++)-1;
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
                        memset(vga.pText + (vga.scrollTop * outVga.sizeX) * 2,
                               0,
                               outVga.sizeX * 2 );
                    }
                break;

            case DP_SETWRITEATTR:
                    s++;            // NOT IMPLEMENTED YET
                break;

            case '\n':
                    // Go to a new line, possible autoscroll
                    outVga.x = 0;

                    // Check if we are on the last line of autoscroll
                    if( vga.scrollBottom==outVga.y )
                        ScrollUp();
                    else
                        outVga.y++;
                break;

            default:
                    // All printable characters
                    *(WORD *)(vga.pText + (outVga.x +  outVga.y * outVga.sizeX) * 2) = (WORD) c + 0x0700;

                    // Advance the print position
                    if( outVga.x < outVga.sizeX )
                        outVga.x++;
                break;
        }

        ShowCursorPos();
    }
}

