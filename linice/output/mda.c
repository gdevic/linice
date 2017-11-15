/******************************************************************************
*                                                                             *
*   Module:     mda.c                                                         *
*                                                                             *
*   Date:       04/22/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
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

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

#include "font.h"                       // Include font declarations

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TOUT outMda = {0};

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define LINUX_MDA_TEXT  (ice_page_offset() + 0xB0000)

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

extern BYTE cacheText[MAX_OUTPUT_SIZEY][MAX_OUTPUT_SIZEX];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void CacheTextScrollUp(DWORD top, DWORD bottom);
extern void CacheTextCls();

static void MdaSprint(char *s);
static void MdaMouse(int x, int y);
static BOOL MdaResize(int x, int y, int nFont);
static void MdaCarret(BOOL fOn);

static void HercSprint(char *s);
static void HercMouse(int x, int y);
static void HercCarret(BOOL fOn);

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

    memset(&outMda, 0, sizeof(outMda));

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
        outMda.carret = HercCarret;
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
        outMda.carret = MdaCarret;
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
*   static BOOL MdaResize(int x, int y, int nFont)                            *
*                                                                             *
*******************************************************************************
*
*   Resize MDA text display
*
******************************************************************************/
static BOOL MdaResize(int x, int y, int nFont)
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
        dprinth(1, "Only WIDTH=80 supported on MDA display");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   static void MdaCarret(BOOL fOn)                                           *
*                                                                             *
*******************************************************************************
*
*   Shows the cursor at the current position
*
******************************************************************************/
static void MdaCarret(BOOL fOn)
{
    WORD wOffset;

    // Set the cursor on the MDA screen
    // We ignore on/off message not to interfere with the "natural" blink rate

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
               deb.col[COL_NORMAL] * 256 + ' ',
               outMda.sizeX );
    }
}


/******************************************************************************
*                                                                             *
*   static void MdaSprint(char *s)                                            *
*                                                                             *
*******************************************************************************
*
*   String output to a 25-line MDA text buffer.
*
******************************************************************************/
static void MdaSprint(char *s)
{
    BYTE c;
    UINT nTabs;

    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(nTabs=deb.nTabs; nTabs; nTabs--)
                MdaSprint(" ");
        }
        else
        switch( c )
        {
            case DP_ENABLE_OUTPUT:
            case DP_DISABLE_OUTPUT:
                    // Enable and disable output are ignored on mda device
                break;

            case DP_SAVEBACKGROUND:
            case DP_RESTOREBACKGROUND:
                    // Save and restore background are ignored on mda device
                break;

            case DP_CLS:
                    // Clear the screen and reset the cursor coordinates
                    memset_w(mda.pText,
                        deb.col[COL_NORMAL] * 256 + ' ',
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
                               deb.col[COL_NORMAL] * 256 + ' ',
                               outMda.sizeX);
                    }
                break;

            case DP_SETCOLINDEX:
                    mda.col = *s++;
                break;

            case '\r':
                    // Erase all characters to the right of the cursor pos and move cursor back
                    memset_w(mda.pText + (outMda.x +  outMda.y * outMda.sizeX) * 2,
                            deb.col[mda.col] * 256 + ' ',
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

            case DP_ESCAPE:
                    // Escape character prints the next code as raw ascii
                    c = *s++;

                    // This case continues into the default...!

            default:
                    // All printable characters
                    *(WORD *)(mda.pText + (outMda.x +  outMda.y * outMda.sizeX) * 2)
                        = (WORD) c + MdaColor[mda.col] * 256;

                    // Advance the print position
                    if( outMda.x < outMda.sizeX )
                        outMda.x++;
                break;
        }
    }
}


/******************************************************************************
*                                                                             *
*   static void HercPrintCharacter(DWORD x, DWORD y, BYTE c, int col)         *
*                                                                             *
*******************************************************************************
*
*   Prints a single character onto the Hercules graphics display buffer
*
*   Where:
*       col is the color index to be used, or special -1 for carret invert
*
******************************************************************************/
static void HercPrintCharacter(DWORD x, DWORD y, BYTE c, int col)
{
    BYTE *dest, *src;
    BYTE *pStart;

    dest = mda.pText + y * 90 * 2 + x;
    src = &Font[0].Bitmap[c * 8];           // Always use font 0 for Hercules!
    pStart = dest;

    switch( col )
    {
        case -1:    // Carret invert code, last 2 lines
            src += 6;
            dest = pStart + 90 + 2 * 0x2000;
            *dest = (*src++) ^ 0xFF; dest+=0x2000;
            *dest = (*src++) ^ 0xFF;

            break;

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
*   static void HercCarret(BOOL fOn)                                          *
*                                                                             *
*******************************************************************************
*
*   Draws a cursor carret.
*
******************************************************************************/
static void HercCarret(BOOL fOn)
{
    // Depending on the off/on message, we redraw cached ASCII code or inverse of it
    // Depending on the insert/overtype more, invert whole character or use a special col code (-1)
    if( fOn )
        HercPrintCharacter(outMda.x, outMda.y, cacheText[outMda.y][outMda.x], deb.fOvertype ? COL_REVERSE : -1);
    else
        HercPrintCharacter(outMda.x, outMda.y, cacheText[outMda.y][outMda.x], mda.col);
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
    static BYTE cacheTextPreScroll[MAX_OUTPUT_SIZEY][MAX_OUTPUT_SIZEX];

    // Copy cacheText state before scroll
    memmove(&cacheTextPreScroll[mda.scrollTop][0],
            &cacheText[mda.scrollTop][0],
            MAX_OUTPUT_SIZEX * (mda.scrollBottom-mda.scrollTop+1));

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
                    HercPrintCharacter(x, y, cacheText[y][x], mda.col);
            }
        }
    }
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
*   static void HercSprint(char *s)                                           *
*                                                                             *
*******************************************************************************
*
*   String output to a Hercules graphics screen buffer.
*
******************************************************************************/
static void HercSprint(char *s)
{
    BYTE c;
    UINT nTabs;

    // Warning: this function is being reentered
    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(nTabs=deb.nTabs; nTabs; nTabs--)
                HercSprint(" ");
        }
        else
        switch( c )
        {
            case DP_ENABLE_OUTPUT:
            case DP_DISABLE_OUTPUT:
                    // Enable and disable output are ignored on mda device
                break;

            case DP_SAVEBACKGROUND:
            case DP_RESTOREBACKGROUND:
                    // Save and restore background are ignored on mda device
                break;

            case DP_CLS:
                    // Clear the cache text
                    CacheTextCls();

                    // Clear the screen and reset the cursor coordinates (in dwords)
                    memset_d(mda.pText, 0, 32 * 1024 / 4);

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

            case DP_ESCAPE:
                    // Escape character prints the next code as raw ascii
                    c = *s++;

                    // This case continues into the default...!

            default:
                    // All printable characters
                    if( outMda.x < outMda.sizeX )
                    {
                        HercPrintCharacter(outMda.x, outMda.y, c, mda.col);

                        // Store it in the cache
                        cacheText[outMda.y][outMda.x] = c;

                        // Advance the print position
                        outMda.x++;
                    }
                break;
        }
    }
}

