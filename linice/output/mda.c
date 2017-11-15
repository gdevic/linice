/*****************************************************************************
*                                                                             *
*   Module:     mda.c                                                         *
*                                                                             *
*   Date:       04/22/02                                                      *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

        Monochrome video output module. This module is similar to VGA driver,
        but we dont need to keep any video state since MDA will be used
        for debugging exclusively.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/22/02   Original                                             Goran Devic *
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

TOUT outMda;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define LINUX_MDA_TEXT  (PAGE_OFFSET + 0xB0000)

#define MAX_SIZEX       80
#define MAX_SIZEY       25

//---------------------------------------------------
// MDA registers content for text mode 80x25
//---------------------------------------------------

#define MDA_INDEX       0x3B4           // Index Register
#define MDA_DATA        0x3B5           // Data Register
#define MDA_MODE_CTRL   0x3B8           // Mode Control Register
#define MDA_CONFIG      0x3BF           // Configuration Register

#define MAX_MDA_CRTC    15              // Number of CRTC registers to program

static BYTE MdaText[MAX_MDA_CRTC] =
{
    97,                                 // 0  Horizontal Total
    80,                                 // 1  Horizontal Displayed
    82,                                 // 2  HSYNC Position
    15,                                 // 3  SYNC Width
    25,                                 // 4  Vertical Total
    6,                                  // 5  Vertical Total Adjust
    25,                                 // 6  Vertical Displayed
    25,                                 // 7  VSYNC Position
    2,                                  // 8  Interlaced Mode
    13,                                 // 9  Max Scanline
    11,                                 // 10 Cursor Start
    12,                                 // 11 Cursor End
    0,                                  // 12 Start Address High
    0,                                  // 13 Start Address Low
    0,                                  // 14 Cursor Address High
    0                                   // 14 Cursor Address Low
};

static BYTE MdaGraph[MAX_MDA_CRTC] =
{
    53,                                 // 0  Horizontal Total
    45,                                 // 1  Horizontal Displayed
    46,                                 // 2  HSYNC Position
    7,                                  // 3  SYNC Width
    91,                                 // 4  Vertical Total
    2,                                  // 5  Vertical Total Adjust
    87,                                 // 6  Vertical Displayed
    87,                                 // 7  VSYNC Position
    2,                                  // 8  Interlaced Mode
    3,                                  // 9  Max Scanline
    0,                                  // 10 Cursor Start
    0,                                  // 11 Cursor End
    0,                                  // 12 Start Address High
    0,                                  // 13 Start Address Low
    0,                                  // 14 Cursor Address High
    0                                   // 14 Cursor Address Low
};


// MDA has the constant "color" scheme, can't be altered by the "color" command:
static const MdaColor[6] = 
{
    0,
    2,                                  // COL_NORMAL     (1)
    10,                                 // COL_BOLD       (2)
    10,                                 // COL_REVERSE    (3)
    2,                                  // COL_HELP       (4)
    2                                   // COL_LINE       (5)
};


//---------------------------------------------------
// Helper variables for display output
//---------------------------------------------------

typedef struct
{
    int col;                            // Current line's color index
    BYTE *pText;                        // Address of the MDA text buffer
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates
    BYTE fEnabled;                      // Output is enabled

} TMda;

static TMda mda;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void MdaSprint(char *s);
static void MdaMouse(int x, int y);

/******************************************************************************
*                                                                             *
*   void MdaInit(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Initializes MDA output driver
*
******************************************************************************/
void MdaInit(void)
{
    int index;

    memset(&mda, 0, sizeof(mda));

    // Set default parameters

    outMda.x = 0;
    outMda.y = 0;
    outMda.sizeX = 80;
    outMda.sizeY = 25;
    outMda.sprint = MdaSprint;
    outMda.mouse = MdaMouse;

    mda.scrollTop = 0;
    mda.scrollBottom = MAX_SIZEY - 1;
    mda.pText = (BYTE *) LINUX_MDA_TEXT;
    mda.col = COL_NORMAL;
//  mda.fEnabled = TRUE;

#if 1
    // Disable video signal
    outp(MDA_MODE_CTRL, 0x00);

    // The Configuration Register:
    //  Bit 0 = 0: Disallow setting graphics mode using the Mode Control Register
    //  Bit 1 = 0: Disallow setting page 1 using the Mode Control Register
    //
    outp(MDA_CONFIG, 0);

    // Mode Control Register:
    //  Bit 1 = 0: text mode; 1: graphics mode
    //  Bit 3 = 0: disable video signal; 1: enable video signal
    //  Bit 5 = 0: disable blinking; 1: enable blinking
    //  Bit 7 = 0: page 0 at B0000; 1: page 1 at B8000
    //
    outp(MDA_MODE_CTRL, 0);

    // Program CRTC values
    for(index=0; index<MAX_MDA_CRTC; index++)
    {
        WriteMdaCRTC(index, MdaText[index]);
    }

    // Enable video signal
    outp(MDA_MODE_CTRL, 0x08);
#else
    // Disable video signal
    outp(MDA_MODE_CTRL, 0x00);

    // The Configuration Register:
    //  Bit 0 = 0: Disallow setting graphics mode using the Mode Control Register
    //  Bit 1 = 0: Disallow setting page 1 using the Mode Control Register
    //
    outp(MDA_CONFIG, 0x01);

    // Mode Control Register:
    //  Bit 1 = 0: text mode; 1: graphics mode
    //  Bit 3 = 0: disable video signal; 1: enable video signal
    //  Bit 5 = 0: disable blinking; 1: enable blinking
    //  Bit 7 = 0: page 0 at B0000; 1: page 1 at B8000
    //
    outp(MDA_MODE_CTRL, 0x2);

    // Program CRTC values
    for(index=0; index<MAX_MDA_CRTC; index++)
    {
        WriteMdaCRTC(index, MdaGraph[index]);
    }

    // Enable video signal
    outp(MDA_MODE_CTRL, 0x0A);
#endif
}


/******************************************************************************
*                                                                             *
*   static void SaveBackground(void)                                          *
*                                                                             *
*******************************************************************************
*
*   Saves the MDA text screen background and prepares driver for displaying
*
******************************************************************************/
static void SaveBackground(void)
{
    // Show the cursor with the desired shape

    ShowCursorPos();
}


/******************************************************************************
*                                                                             *
*   static void RestoreBackground(void)                                       *
*                                                                             *
*******************************************************************************
*
*   Restores MDA text screen
*
******************************************************************************/
static void RestoreBackground(void)
{
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

    // Set the cursor on the MDA screen

    wOffset = outMda.y * 80 + outMda.x;

    WriteMdaCRTC(14, wOffset >> 8);
    WriteMdaCRTC(15, wOffset & 0xFF);

    // Set the cursor shape depending on the Insert/Overtype mode

    if( deb.fOvertype )
    {
        // Overtype mode - full cursor block
        WriteMdaCRTC(10, 0);          // Cursor Start Register
        WriteMdaCRTC(11, 12);         // Cursor End Register
    }
    else
    {
        // Insert mode - line cursor shape
        WriteMdaCRTC(10, 11);         // Cursor Start Register
        WriteMdaCRTC(11, 12);         // Cursor End Register
    }
}


/******************************************************************************
*                                                                             *
*   static void MdaMouse(int x, int y)                                        *
*                                                                             *
*******************************************************************************
*
*   Mouse display function
*
******************************************************************************/
static void MdaMouse(int x, int y)
{
    WORD wOffset;

    // Set the mouse cursor on the MDA screen

    wOffset = y * 80 + x;

    WriteMdaCRTC(14, wOffset >> 8);
    WriteMdaCRTC(15, wOffset & 0xFF);

    // Since we use hardware MDA cursor to show the mouse location, set
    // its shape to full block

    WriteMdaCRTC(10, 0);                // Cursor Start Register
    WriteMdaCRTC(11, 12);               // Cursor End Register
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
    if( (mda.scrollTop < mda.scrollBottom) && (mda.scrollBottom < outMda.sizeY) )
    {
        // Scroll up all requested lines
        memmove(mda.pText + (mda.scrollTop * outMda.sizeX) * 2,
                mda.pText + ((mda.scrollTop + 1) * outMda.sizeX) * 2,
                outMda.sizeX * 2 * (mda.scrollBottom - mda.scrollTop));

        // Clear the last line
        memset_w(mda.pText + (mda.scrollBottom * outMda.sizeX) * 2,
               pIce->col[COL_NORMAL] * 256 + ' ',
               outMda.sizeX );
    }
}


/******************************************************************************
*                                                                             *
*   MdaSprint(char *s)                                                        *
*                                                                             *
*******************************************************************************
*
*   String output to a MDA text buffer.
*
******************************************************************************/
void MdaSprint(char *s)
{
    BYTE c;
    DWORD dwTabs;

    while( (c = *s++) != 0 )
    {
        if( mda.fEnabled )
        {
            if( c==DP_TAB )
            {
                for(dwTabs=deb.dwTabs; dwTabs; dwTabs--)
                    MdaSprint(" ");
            }
            else
            switch( c )
            {
                case DP_DISABLE_OUTPUT:
                    // MDA output can't be disabled!
//                        mda.fEnabled = FALSE;
                    break;

                case DP_SAVEBACKGROUND:
                        SaveBackground();
                    break;

                case DP_RESTOREBACKGROUND:
                        RestoreBackground();
                    break;

                case DP_CLS:
                        // Clear the screen and reset the cursor coordinates
                        memset_w(mda.pText,
                            pIce->col[COL_NORMAL] * 256 + ' ',
                            outMda.sizeY * outMda.sizeX);
                        outMda.x = 0;
                        outMda.y = 0;
                    break;

                case DP_SETCURSORXY:
                        outMda.x = (*s++)-1;
                        outMda.y = (*s++)-1;
                    break;

                case DP_SETCURSORSHAPE:
                        deb.fOvertype = (*s++)-1;
                    break;

                case DP_SAVEXY:
                        mda.savedX = outMda.x;
                        mda.savedY = outMda.y;
                    break;

                case DP_RESTOREXY:
                        outMda.x = mda.savedX;
                        outMda.y = mda.savedY;
                    break;

                case DP_SETSCROLLREGIONYY:
                        mda.scrollTop = (*s++)-1;
                        mda.scrollBottom = (*s++)-1;
                    break;

                case DP_SCROLLUP:
                        // Scroll a portion of the screen up and clear the bottom line
                        ScrollUp();
                    break;

                case DP_SCROLLDOWN:
                        // Scroll a portion of the screen down and clear the top line
                        if( (mda.scrollTop < mda.scrollBottom) && (mda.scrollBottom < outMda.sizeY) )
                        {
                            // Scroll down all requested lines
                            memmove(mda.pText + ((mda.scrollTop + 1) * outMda.sizeX) * 2,
                                    mda.pText + (mda.scrollTop * outMda.sizeX) * 2,
                                    outMda.sizeX * 2 * (mda.scrollBottom - mda.scrollTop));

                            // Clear the first line
                            memset_w(mda.pText + (mda.scrollTop * outMda.sizeX) * 2,
                                   pIce->col[COL_NORMAL] * 256 + ' ',
                                   outMda.sizeX);
                        }
                    break;

                case DP_SETCOLINDEX:
                        mda.col = *s++;
                    break;

                case '\r':
                        // Erase all characters to the right of the cursor pos and move cursor back
                        memset_w(mda.pText + (outMda.x +  outMda.y * outMda.sizeX) * 2,
                                pIce->col[mda.col] * 256 + ' ',
                                outMda.sizeX - outMda.x);
                        outMda.x = 0;
                        mda.col = COL_NORMAL;
                    break;

                case '\n':
                        // Go to a new line, possible autoscroll
                        outMda.x = 0;
                        mda.col = COL_NORMAL;

                        // Check if we are on the last line of autoscroll
                        if( mda.scrollBottom==outMda.y )
                            ScrollUp();
                        else
                            outMda.y++;
                    break;

                case DP_RIGHTALIGN:
                        // Right align the rest of the text
                        outMda.x = outMda.sizeX - strlen(s);
                    break;

                default:
                        // All printable characters
                        *(WORD *)(mda.pText + (outMda.x +  outMda.y * outMda.sizeX) * 2)
                            = (WORD) c + MdaColor[mda.col] * 256;

                        // Advance the print position
                        if( outMda.x < outMda.sizeX )
                            outMda.x++;
                    break;
            }
    
            ShowCursorPos();
        }
        else
        {
            if( c==DP_ENABLE_OUTPUT )
                mda.fEnabled = TRUE;
        }
    }
}

