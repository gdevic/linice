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

        This module contains pretty much 2 different drivers - one for the 25
        lines MDA mode, and the other for the 43 lines (Hercules) mode. The
        latter is using the graphics mode to fit that many lines.

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

#include "font.h"       // TODO - merge 8x8 fonts

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
#define MAX_SIZEY       25              // 25 or 43 !!!

//---------------------------------------------------
// MDA registers content for text mode 80x25
//---------------------------------------------------

#define MDA_INDEX       0x3B4           // Index Register
#define MDA_DATA        0x3B5           // Data Register
#define MDA_MODE_CTRL   0x3B8           // Mode Control Register
#define MDA_CONFIG      0x3BF           // Configuration Register

#define MAX_MDA_CRTC    16              // Number of CRTC registers to program

// Text mode parameter table - 25 lines screen
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
    0                                   // 15 Cursor Address Low
};

// Graphics mode parameter table - 43 lines screen
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
static const BYTE MdaColor[6] = 
{
    0,
    0x07,                               // COL_NORMAL     (1)
    0x0F,                               // COL_BOLD       (2)
    0x70,                               // COL_REVERSE    (3)
    0x07,                               // COL_HELP       (4)
    0x07                                // COL_LINE       (5)
};


//---------------------------------------------------
// Helper variables for display output
//---------------------------------------------------

typedef struct
{
    int lines;                          // Current number of lines (25 or 43), statically init.

    int col;                            // Current line's color index
    BYTE *pText;                        // Address of the MDA text buffer
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates

} TMda;

static TMda mda = { 25, };              // 25 lines by default

extern BYTE cacheText[MAX_Y][MAX_X];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void CacheTextScrollUp(DWORD top, DWORD bottom);
extern void CacheTextCls();

static void MdaSprint(char *s);
static void MdaMouse(int x, int y);
static BOOL MdaResize(int x, int y);

static void HercSprint(char *s);
static void HercMouse(int x, int y);

/******************************************************************************
*                                                                             *
*   static void InitMdaHercVideoMode(int lines)                               *
*                                                                             *
*******************************************************************************
*
*   Programs MDA/Hercules video registers for a given mode
*
*   Where:
*       lines is 25 or 43 lines
*
******************************************************************************/
static void InitMdaHercVideoMode(int lines)
{
    int index;

    outMda.x = 0;
    outMda.y = 0;
    outMda.sizeX = 80;
    outMda.sizeY = lines;
    outMda.resize = MdaResize;

    mda.lines = lines;
    mda.scrollTop = 0;
    mda.scrollBottom = outMda.sizeY - 1;
    mda.pText = (BYTE *) LINUX_MDA_TEXT;
    mda.col = COL_NORMAL;

    //-----------------------------------------------------------------------
    //          43 lines - use MDA graphics mode
    //-----------------------------------------------------------------------
    if( lines==43 )
    {
        // Set 43-line pointers
        outMda.sprint = HercSprint;
        outMda.mouse = HercMouse;

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
    }
    else
    //-----------------------------------------------------------------------
    //          25 lines - use MDA text mode
    //-----------------------------------------------------------------------
    {
        // Set 25-line pointers
        outMda.sprint = MdaSprint;
        outMda.mouse = MdaMouse;

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
    }
}


/******************************************************************************
*                                                                             *
*   void MdaInit(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Initializes MDA output driver for 25 lines mode.
*
******************************************************************************/
void MdaInit(void)
{
    // Set mode with default parameters

    InitMdaHercVideoMode(mda.lines);
}


/******************************************************************************
*                                                                             *
*   static BOOL MdaResize(int x, int y)                                       *
*                                                                             *
*******************************************************************************
*
*   Resize MDA text display
*
******************************************************************************/
static BOOL MdaResize(int x, int y)
{
    // Only 25 or 43 lines are supported, also only 80 WIDTH

    if( x == 80 )
    {
        if( y==25 || y==43 )
        {
            // Reset video mode depending on the number of lines

            InitMdaHercVideoMode(y);

            return( TRUE );
        }
        else
            dprinth(1, "Only 25 and 43 lines supported on MDA display");
    }
    else
        dprinth(1, "Only WIDTH=80 supported");

    return( FALSE );
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
    // We dont care for the background on MDA/Hercules card
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
    // We dont care for the background on MDA/Hercules card
}


/******************************************************************************
*                                                                             *
*   static void MdaShowCursorPos(void)                                        *
*                                                                             *
*******************************************************************************
*
*   Shows the cursor at the current position
*
******************************************************************************/
static void MdaShowCursorPos(void)
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
}


/******************************************************************************
*                                                                             *
*   static void MdaScrollUp()                                                 *
*                                                                             *
*******************************************************************************
*
*   Scrolls up a region in 25-line mode
*
******************************************************************************/
static void MdaScrollUp()
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
*   String output to a 25-line MDA text buffer.
*
******************************************************************************/
void MdaSprint(char *s)
{
    BYTE c;
    DWORD dwTabs;

    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(dwTabs=deb.dwTabs; dwTabs; dwTabs--)
                MdaSprint(" ");
        }
        else
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
                    MdaScrollUp();
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
                        MdaScrollUp();
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

        MdaShowCursorPos();
    }
}


/******************************************************************************
*                                                                             *
*   static void HercPrintCharacter(DWORD x, DWORD y, BYTE c)                  *
*                                                                             *
*******************************************************************************
*
*   Prints a single character onto the Hercules graphics display buffer
*
******************************************************************************/
static void HercPrintCharacter(DWORD x, DWORD y, BYTE c)
{
    BYTE *dest, *src;
    BYTE *pStart;

    dest = mda.pText + y * 90 * 2 + x;
    src = (BYTE *) &font8x8[c];
    pStart = dest;

    switch( mda.col )
    {
        case COL_BOLD:
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
    
            dest = pStart + 90;
    
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1); src++; dest+=0x2000;
            *dest = *src | (*src >> 1);

            break;

        case COL_REVERSE:
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
    
            dest = pStart + 90;
    
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF;

            break;

        default:
            *dest = *src++; dest+=0x2000;
            *dest = *src++; dest+=0x2000;
            *dest = *src++; dest+=0x2000;
            *dest = *src++; dest+=0x2000;
    
            dest = pStart + 90;
    
            *dest = *src++; dest+=0x2000;
            *dest = *src++; dest+=0x2000;
            *dest = *src++; dest+=0x2000;
            *dest = *src++;
            
            break;
    }
}


/******************************************************************************
*                                                                             *
*   static void HercScrollUp()                                                *
*                                                                             *
*******************************************************************************
*
*   Scrolls up a region in 43-line graphics mode. We do little speed trick
*   here - we copy the cacheText buffer, scroll it, and then we actually print
*   out only those characters that changed.
*
******************************************************************************/
static void HercScrollUp()
{
    int x, y;
    static BYTE cacheTextPreScroll[MAX_Y][MAX_X];

    // Copy cacheText state before scroll
    memmove(&cacheTextPreScroll[mda.scrollTop][0], 
            &cacheText[mda.scrollTop][0], 
            MAX_X * (mda.scrollBottom-mda.scrollTop+1));

    // We use cacheText buffer that we scroll first, and then read the
    // content to print it, since reading from the graphics buffer in order
    // to scroll it is painfully slow..
    if( (mda.scrollTop < mda.scrollBottom) && (mda.scrollBottom < outMda.sizeY) )
    {
        CacheTextScrollUp(mda.scrollTop, mda.scrollBottom);

        for(y=mda.scrollTop; y<=mda.scrollBottom; y++ )
        {
            for(x=0; x<outMda.sizeX; x++)
            {
                // Print out only those positions that changed after the scroll
                if( cacheText[y][x] != cacheTextPreScroll[y][x] )
                    HercPrintCharacter(x, y, cacheText[y][x]);
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   static void HercShowCursorPos(void)                                       *
*                                                                             *
*******************************************************************************
*
*   Shows the cursor at the current position
*
******************************************************************************/
static void HercShowCursorPos(void)
{
}


/******************************************************************************
*                                                                             *
*   static void HercMouse(int x, int y)                                       *
*                                                                             *
*******************************************************************************
*
*   Mouse display function
*
******************************************************************************/
static void HercMouse(int x, int y)
{
}


/******************************************************************************
*                                                                             *
*   HercSprint(char *s)                                                       *
*                                                                             *
*******************************************************************************
*
*   String output to a Hercules graphics screen buffer.
*
******************************************************************************/
void HercSprint(char *s)
{
    BYTE c;
    DWORD dwTabs;

    // Warning: this function is being reentered
    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(dwTabs=deb.dwTabs; dwTabs; dwTabs--)
                HercSprint(" ");
        }
        else
        switch( c )
        {
            case DP_SAVEBACKGROUND:
                    SaveBackground();
                break;

            case DP_RESTOREBACKGROUND:
                    RestoreBackground();
                break;

            case DP_CLS:
                    // Clear the cache text
                    CacheTextCls();

                    // Clear the screen and reset the cursor coordinates
                    memset_w(mda.pText, 0, 32 * 1024);

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
                    HercScrollUp();
                break;

            case DP_SCROLLDOWN:
                    // Scroll a portion of the screen down and clear the top line
                    // We dont support this type of scroll here
                break;

            case DP_SETCOLINDEX:
                    mda.col = *s++;
                break;

            case '\r':
                    // Erase all characters to the right of the cursor pos and move cursor back
                    while( outMda.x < outMda.sizeX )
                    {
                        // Recurse to print spaces
                        HercSprint(" ");
                    }

                    // Reset cursor coordinates and color attribute
                    outMda.x = 0;
                    mda.col = COL_NORMAL;
                break;

            case '\n':
                    // Go to a new line, possible autoscroll
                    outMda.x = 0;
                    mda.col = COL_NORMAL;

                    // Check if we are on the last line of autoscroll
                    if( mda.scrollBottom==outMda.y )
                        HercScrollUp();
                    else
                        outMda.y++;
                break;

            case DP_RIGHTALIGN:
                    // Right align the rest of the text
                    outMda.x = outMda.sizeX - strlen(s);
                break;

            default:
                    // All printable characters
                    HercPrintCharacter(outMda.x, outMda.y, c);

                    // Store it in the cache
                    cacheText[outMda.y][outMda.x] = c;

                    // Advance the print position
                    if( outMda.x < outMda.sizeX )
                        outMda.x++;
                break;
        }

        HercShowCursorPos();
    }
}

