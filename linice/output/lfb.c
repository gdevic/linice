/******************************************************************************
*                                                                             *
*   Module:     lfb.c                                                         *
*                                                                             *
*   Date:       05/18/01                                                      *
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

        This module implements output functionality for linear frame buffer
		mapped bitmapped graphics modes (SVGA, X)

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/18/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

#include "font.h"                       // Include font declarations

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern TOUT outVga;                     // VGA output device
extern TOUT outMda;                     // MDA output device

TOUT outDga = {0};

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

//  R     G     B
static const DWORD vga_fore[16][3] = {
{ 0x00, 0x00, 0x00 },                   // Color 0  BLACK
{ 0x00, 0x00, 0xFF },                   // Color 1  BLUE
{ 0x00, 0xFF, 0x00 },                   // Color 2  GREEN
{ 0x00, 0xFF, 0xFF },                   // Color 3  CYAN
{ 0xFF, 0x00, 0x00 },                   // Color 4  RED
{ 0x80, 0x00, 0x80 },                   // Color 5  MAGENTA
{ 0xC0, 0xC0, 0xC0 },                   // Color 6  BROWN
{ 0xFF, 0xFF, 0xFF },                   // Color 7  GREY
{ 0x40, 0x40, 0x40 },                   // Color 8  DARK GREY
{ 0x00, 0x00, 0x80 },                   // Color 9  LIGHT BLUE
{ 0x00, 0x80, 0x00 },                   // Color 10 LIGHT GREEN
{ 0x00, 0x80, 0x80 },                   // Color 11 LIGHT CYAN
{ 0x80, 0x00, 0x00 },                   // Color 12 LIGHT RED
{ 0x40, 0x00, 0x40 },                   // Color 13 LIGHT MAGENTA
{ 0xFF, 0xFF, 0x00 },                   // Color 14 YELLOW
{ 0xFF, 0xFF, 0xFF }                    // Color 15 WHITE
};

static const DWORD vga_back[16][3] = {
{ 0x00, 0x00, 0x00 },                   // Color 0  BLACK
{ 0x00, 0x00, 0xFF },                   // Color 1  BLUE
{ 0x00, 0xFF, 0x00 },                   // Color 2  GREEN
{ 0x00, 0xFF, 0xFF },                   // Color 3  CYAN
{ 0xFF, 0x00, 0x00 },                   // Color 4  RED
{ 0x80, 0x00, 0x80 },                   // Color 5  MAGENTA
{ 0xC0, 0xC0, 0xC0 },                   // Color 6  BROWN
{ 0x80, 0x80, 0x80 },                   // Color 7  GREY
};

//---------------------------------------------------
// Helper variables for display output
//---------------------------------------------------

typedef struct
{
    int col;                            // Current line's color index
    DWORD pixelFore[16];                // Pixel foreground colors
    DWORD pixelBack[16];                // Pixel background colors
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates
    BYTE fEnabled;                      // Output is enabled

    DWORD dwFrameOffset;                // Offset to our effective frame
    TFont *pFont;                       // Pointer to a current font structure
    DWORD stride;                       // Screen stride
    DWORD xres, yres;                   // X, Y resolution in pixels
    DWORD bpp;                          // BYTES per pixel :-)
    void (*PrintChar)(DWORD, BYTE, int);// Raw print char function
    void (*Cls)(void);                  // Raw cls function

} TDGA;

static TDGA dga;

extern BYTE cacheText[MAX_OUTPUT_SIZEY][MAX_OUTPUT_SIZEX];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static void DgaPrintCharacter32(DWORD ptr, BYTE c, int col);
static void DgaPrintCharacter16(DWORD ptr, BYTE c, int col);
static void DgaSprint(char *s);
static void DgaCls32();
static void DgaCls16();

static void DgaCarret(BOOL fOn);
static void DgaMouse(int x, int y);
static BOOL DgaResize(int x, int y, int nFont);

extern void CacheTextScrollUp(DWORD top, DWORD bottom);
extern void CacheTextCls();

extern DWORD UserVirtToPhys(DWORD address);

/******************************************************************************
*                                                                             *
*   int XInitPacket(TXINITPACKET *pXInit)                                     *
*                                                                             *
*******************************************************************************
*
*   Initializes X framebuffer. This call is made by the xice loader when
*   switching/starting on the X.
*
*   Returns:
*       0 on success
*       -1 on failure
*
******************************************************************************/
int XInitPacket(TXINITPACKET *pXInit)
{
    int i;
    DWORD physicalAddress;
    DWORD dwMappingSize;                // How much of frame buffer to map
    DWORD dwSize;                       // Initial backing store buffer size

    // Do all the messages on a VGA output device. That will also be a nice
    // fallback device should something go wrong
    pOut = &outVga;

    // If the buffer size is specified, use it. Otherwise, use what came with main init packet
    if( pXInit->dwDrawSize )
        deb.nXDrawSize = pXInit->dwDrawSize;

    // Catch unreasonable large buffer size requests
    if( deb.nXDrawSize > MAX_XWIN_BUFFER )
    {
        deb.nXDrawSize = MAX_XWIN_BUFFER;
    }

    // Make sure the size is enough for the starting buffer 80x25 and the selected font
    dwSize = (80+2) * 8 * pXInit->bpp * (25+2) * Font[deb.nFont].ysize;

    if( deb.nXDrawSize < dwSize )
    {
        deb.nXDrawSize = dwSize;
    }

    dprinth(1, "XWIN: Received XInitPacket.. Using buffer size of %d (%d Kb)", deb.nXDrawSize, deb.nXDrawSize/1024);

    // If we already allocated buffers etc., need to release before reallocating
    if( deb.pXDrawBuffer != NULL )
    {
        ice_vfree(deb.pXDrawBuffer);

        if( deb.pXFrameBuffer != NULL )
            ice_iounmap(deb.pXFrameBuffer);

        deb.pXDrawBuffer = NULL;
        deb.pXFrameBuffer = NULL;
    }

    // Allocate memory that will be used to save the content of the framebuffer
    // over the area of drawing
    if( (deb.pXDrawBuffer = ice_vmalloc(deb.nXDrawSize)) != NULL)
    {
        INFO("XWIN: Allocated %d for backing store buffer\n", (int)deb.nXDrawSize);

        // Find the physical address of the frame buffer in user address space
        physicalAddress = UserVirtToPhys(pXInit->pFrameBuf);

        // Find the size of the frame buffer that we need to map
        dwMappingSize = pXInit->stride * pXInit->yres;

        // Map the graphics card physical memory into the kernel address space
        deb.pXFrameBuffer = ice_ioremap(physicalAddress, dwMappingSize);

        if( deb.pXFrameBuffer )
        {
            memset(&dga, 0, sizeof(dga));
            memset(&outDga, 0, sizeof(outDga));

            // Set default parameters

            outDga.x = 0;
            outDga.y = 0;
            outDga.sizeX = 80;                  // This is the initial X size of the dga display
            outDga.sizeY = 25;                  // This is the initial Y size of the dga display
            outDga.sprint = DgaSprint;
            outDga.carret = DgaCarret;
            outDga.mouse = DgaMouse;
            outDga.resize = DgaResize;

            dga.scrollTop = 0;
            dga.scrollBottom = outDga.sizeY - 1;
            dga.col = COL_NORMAL;
            dga.pFont = &Font[deb.nFont];
            dga.fEnabled = FALSE;

            dga.stride = pXInit->stride;
            dga.xres   = pXInit->xres;
            dga.yres   = pXInit->yres;
            dga.bpp    = pXInit->bpp;

            deb.FrameX = 0;                     // Start the window at the top-left corner
            deb.FrameY = 0;

            // Calculate frame offset based on the existing origin variables
            dga.dwFrameOffset = deb.FrameY * dga.stride + deb.FrameX * dga.bpp;

            // We need to calculate colors for the given pixel mode
            for(i=0; i<16; i++ )
            {
                dga.pixelFore[i] =
                    ((vga_fore[i][0] >> pXInit->redColAdj) << pXInit->redShift) |
                    ((vga_fore[i][1] >> pXInit->greenColAdj) << pXInit->greenShift) |
                    ((vga_fore[i][2] >> pXInit->blueColAdj) << pXInit->blueShift);
                dga.pixelBack[i] =
                    ((vga_back[i][0] >> pXInit->redColAdj) << pXInit->redShift) |
                    ((vga_back[i][1] >> pXInit->greenColAdj) << pXInit->greenShift) |
                    ((vga_back[i][2] >> pXInit->blueColAdj) << pXInit->blueShift);
            }

            // Depending on the pixel depth, select the raw utility functions
            switch( dga.bpp * 8 )
            {
                case 32:    dga.PrintChar = DgaPrintCharacter32;
                            dga.Cls       = DgaCls32;
                    break;

                case 16:    dga.PrintChar = DgaPrintCharacter16;
                            dga.Cls       = DgaCls16;
                    break;
            }

            // Print stats

            dprinth(1, "XWIN: User virtual address = %08X", pXInit->pFrameBuf);
            dprinth(1, "XWIN: Physical address     = %08X", physicalAddress);
            dprinth(1, "XWIN: Kernel address       = %08X (%08X)", deb.pXFrameBuffer, dwMappingSize);
            dprinth(1, "XWIN: Desktop %d x %d, stride=%d  bpp=%d", dga.xres, dga.yres, dga.stride, dga.bpp * 8);

            // Switch output to this driver and schedule a clean break into debugger

            pOut = &outDga;

            deb.nScheduleKbdBreakTimeout = 2;

            return( 0 );                // Return success
        }
        else
        {
            // We could not map the frame buffer.. Clean up and switch back to vga driver

            pOut = &outVga;

            dprinth(1, "XWIN: Unable to map frame buffer (size=%d)!", dwMappingSize);

            ice_vfree(deb.pXDrawBuffer);
        }
    }
    else
    {
        // We could not allocate memory.. Clean up and switch back to a vga driver

        pOut = &outVga;

        dprinth(1, "XWIN: Unable to allocate %d for backing store buffer!", deb.nXDrawSize);

    }

    deb.pXDrawBuffer = NULL;
    deb.pXFrameBuffer = NULL;

    return( -1 );                       // Return error!
}


/******************************************************************************
*                                                                             *
*   void DgaPrintCharacter32(DWORD ptr, BYTE c, int col)                      *
*                                                                             *
*******************************************************************************
*
*   A character printing function for 32bpp 888 format
*
******************************************************************************/
static void DgaPrintCharacter32(DWORD ptr, BYTE c, int col)
{
    DWORD *pPixel;
    int x, y;
    BYTE *pChar;
    BYTE line;
    DWORD pixelFore, pixelBack;
    BOOL fCarret = FALSE;               // Special carret character

    // If we are printing a cursor carret, set up the state
    if( col==-1 )
    {
        fCarret = TRUE;                 // Turn on the flag
        col = COL_NORMAL;               // And revert the color index
    }

    // Cache the current colors for foreground and background
    pixelFore = dga.pixelFore[deb.col[col] & 0xF];
    pixelBack = dga.pixelBack[(deb.col[col] >> 4) & 0x7];

    // Get the address of the start of a character
    pChar = (BYTE *)(dga.pFont->Bitmap + c * dga.pFont->ysize);

    for(y=0; y<dga.pFont->ysize; y++)
    {
        // Get one scanline of a character font
        line = *pChar++;
        pPixel = (DWORD *) ptr;

        // If we are printing a cursor carret, set last 2 lines
        if( fCarret && y>=dga.pFont->ysize-2 )
        {
            line = 0xFF;                // OR the full foreground line
        }

        for(x=0; x<8; x++)
        {
            if( line & 0x80 )
                *pPixel = pixelFore;
            else
                *pPixel = pixelBack;

            pPixel++;
            line <<= 1;
        }

        ptr += dga.stride;
    }
}

/******************************************************************************
*                                                                             *
*   void DgaPrintCharacter16(DWORD ptr, BYTE c, int col)                      *
*                                                                             *
*******************************************************************************
*
*   A character printing function for 16bpp
*
******************************************************************************/
static void DgaPrintCharacter16(DWORD ptr, BYTE c, int col)
{
    WORD *pPixel;
    int x, y;
    BYTE *pChar;
    BYTE line;
    WORD pixelFore, pixelBack;
    BOOL fCarret = FALSE;               // Special carret character

    // If we are printing a cursor carret, set up the state
    if( col==-1 )
    {
        fCarret = TRUE;                 // Turn on the flag
        col = COL_NORMAL;               // And revert the color index
    }

    // Cache the current colors for foreground and background
    pixelFore = (WORD)(dga.pixelFore[deb.col[col] & 0xF] & 0xFFFF);
    pixelBack = (WORD)(dga.pixelBack[(deb.col[col] >> 4) & 0x7] & 0xFFFF);

    // Get the address of the start of a character
    pChar = (BYTE *)(dga.pFont->Bitmap + c * dga.pFont->ysize);

    for(y=0; y<dga.pFont->ysize; y++)
    {
        // Get one scanline of a character font
        line = *pChar++;
        pPixel = (WORD *) ptr;

        // If we are printing a cursor carret, set last 2 lines
        if( fCarret && y>=dga.pFont->ysize-2 )
        {
            line = 0xFF;                // OR the full foreground line
        }

        for(x=0; x<8; x++)
        {
            if( line & 0x80 )
                *pPixel = pixelFore;
            else
                *pPixel = pixelBack;

            pPixel++;
            line <<= 1;
        }

        ptr += dga.stride;
    }
}


/******************************************************************************
*                                                                             *
*   static void DgaPrintCharacter(DWORD x, DWORD y, BYTE c, int col)          *
*                                                                             *
*******************************************************************************
*
*   Root character printing function.
*
*   Where:
*       col is the color index to be used, or special -1 for carret invert
*
******************************************************************************/
static void DgaPrintCharacter(DWORD x, DWORD y, BYTE c, int col)
{
    DWORD address;

    // Only print characters that completely fit within the screen bounds
    if( x <= (dga.xres - 8) / 8 )
    {
        if( y <= (dga.yres - dga.pFont->ysize) / dga.pFont->ysize )
        {
            // Calculate the address in the frame buffer to print a character
            address = (DWORD) deb.pXFrameBuffer + dga.dwFrameOffset +
                y * dga.stride * dga.pFont->ysize +
                x * dga.bpp * 8;

            if( dga.PrintChar )
                (dga.PrintChar)(address, c, col);
        }
    }
}


/******************************************************************************
*                                                                             *
*   static void DgaCls16()                                                    *
*                                                                             *
*******************************************************************************
*
*   Clear the framebuffer window in 16bpp.
*
******************************************************************************/
static void DgaCls16()
{
    void *address;
    DWORD y;
    WORD pixelBack;

    pixelBack = (WORD)(dga.pixelBack[(deb.col[dga.col] >> 4) & 0x7] & 0xFFFF);
    address = deb.pXFrameBuffer + dga.dwFrameOffset;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        // Set memory - word sizes
        // We are adding for borders
        memset_w(address, pixelBack, (pOut->sizeX+2) * 8);
        address = (void *)((DWORD) address + dga.stride);
    }
}

/******************************************************************************
*                                                                             *
*   static void DgaCls32()                                                    *
*                                                                             *
*******************************************************************************
*
*   Clear the framebuffer window in 32bpp.
*
******************************************************************************/
static void DgaCls32()
{
    void *address;
    DWORD y;
    DWORD pixelBack;

    pixelBack = dga.pixelBack[(deb.col[dga.col] >> 4) & 0x7];
    address = deb.pXFrameBuffer + dga.dwFrameOffset;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        // Set memory - double word sizes
        // We are adding for borders
        memset_d(address, pixelBack, (pOut->sizeX+2) * 8);
        address = (void *)((DWORD) address + dga.stride);
    }
}


/******************************************************************************
*                                                                             *
*   void MoveBackground(BOOL fSave)                                           *
*                                                                             *
*******************************************************************************
*
*   Saves or restores the DGA display window memory.
*
*   Where: fSave - TRUE for save background
*                  FALSE for restore background
*
******************************************************************************/
static void MoveBackground(BOOL fSave)
{
    DWORD size, y, address;
    BYTE *pBuf;

    // Make sure we have draw buffer allocated and that it is the right size
    if( deb.pXDrawBuffer )
    {
        // Calculate required size in bytes of the window area
        // We are adding border (2 characters)
        size = (outDga.sizeX+2) * 8 * dga.bpp * // X size of the font is always 8 pixels
               (outDga.sizeY+2) * dga.pFont->ysize;
        if( size <= deb.nXDrawSize )
        {
            pBuf = deb.pXDrawBuffer;
            address = (DWORD) deb.pXFrameBuffer + dga.dwFrameOffset;

            // Move the window area, line by line, in the required direction
            for(y=0; y<(outDga.sizeY+2) * dga.pFont->ysize; y++)
            {
                if( fSave )
                    memcpy((void *)pBuf, (void *)address, (outDga.sizeX+2) * 8 * dga.bpp);
                else
                    memcpy((void *)address, (void *)pBuf, (outDga.sizeX+2) * 8 * dga.bpp);

                pBuf += (outDga.sizeX+2) * 8 * dga.bpp;
                address += dga.stride;
            }
        }
        else
            dprinth(1, "XWIN: Backing store buffer too small (need %d, have %d)", size, deb.nXDrawSize);
    }
    else
        dprinth(1, "XWIN: Backing store buffer not allocated");
}


/******************************************************************************
*                                                                             *
*   static void DgaMouse(int x, int y)                                        *
*                                                                             *
*******************************************************************************
*
*   Mouse display function
*
******************************************************************************/
static void DgaMouse(int x, int y)
{
}


/******************************************************************************
*                                                                             *
*   static BOOL DgaResize(int x, int y, int nFont)                            *
*                                                                             *
*******************************************************************************
*
*   Resize DGA display due to few commands:
*       LINES (resize X coordinate)
*       WIDTH (resize Y coordinate)
*       SET FONT (resize Y coordinate/special)
*
*   Returns:
*       TRUE - Accept new parameters
*       FALSE - Reject new parameters
*
******************************************************************************/
static BOOL DgaResize(int x, int y, int nFont)
{
    DWORD dwSize;                       // Backing store buffer requirement
    int fontY;                          // Current or new font Y size

    // Load the font Y size. If the command was not to set different font, we
    // expect the caller to pass us the current font number.
    fontY = Font[nFont].ysize;

    // Check that the number of lines or width do not exceed current display size

    if( x > (dga.xres/8)-2 )
    {
        dprinth(1, "Maximum WIDTH at this screen resolution is %d", (dga.xres/8)-2 );
        return( FALSE );
    }

    if( y > (dga.yres/fontY)-2 )
    {
        dprinth(1, "Maximum LINES at this resolution and font is %d", (dga.yres/fontY)-2);
        return( FALSE );
    }

    // Depending on the amount of backing store buffer memory allocated,
    // we may want to decrease this request to what would fit

    dwSize = (x+2) * 8 * dga.bpp * (y+2) * fontY;

    if( dwSize > deb.nXDrawSize )
    {
        // New window size needs more buffer. Print out the required size
        // and scale down to use what we currently have allocated.

        dprinth(1, "That size would need %d of backing store buffer out of %d allocated.", dwSize, deb.nXDrawSize);

        // We know we are changing only one variable at a time (lines, width or font)
        if( x != outDga.sizeX )
        {
            // Changing X size (command WIDTH)
            x = deb.nXDrawSize / (8 * dga.bpp * (outDga.sizeY+2) * fontY);
            x -= 2;                     // Account for 2 border edges

            dwSize = (x+2) * 8 * dga.bpp * (y+2) * fontY;

            dprinth(1, "Using WIDTH of %d instead.", x);
        }

        if( y != outDga.sizeY )
        {
            // Changing Y size (command LINES)
            y = deb.nXDrawSize / ((outDga.sizeX+2) * 8 * dga.bpp * fontY);
            y -= 2;                     // Account for 2 border edges

            dwSize = (x+2) * 8 * dga.bpp * (y+2) * fontY;

            dprinth(1, "Using LINES of %d instead.", y);
        }

        if( nFont != deb.nFont )
        {
            // Window using a new font size would not fit into the backing store - simply return.
            return( FALSE );
        }
    }

    dputc(DP_RESTOREBACKGROUND);

    // After resizing, we may want to readjust the starting frame X and Y coordinates
    if( deb.FrameX + (x+2) * 8 >= dga.xres )
        deb.FrameX = dga.xres - (x+2) * 8;

    if( deb.FrameY + (y+2) * fontY >= dga.yres )
        deb.FrameY = dga.yres - (y+2) * fontY;

    dga.dwFrameOffset = deb.FrameY * dga.stride + deb.FrameX * dga.bpp;

    // Size is now ok, it will fit. Readjust the variables and exit.
    outDga.sizeX = x;
    outDga.sizeY = y;
    deb.nFont = nFont;                  // Set unchanged or new font index
    dga.pFont = &Font[nFont];           // And the pointer to a new font

    dputc(DP_SAVEBACKGROUND);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   static void DgaCarret(BOOL fOn)                                           *
*                                                                             *
*******************************************************************************
*
*   Draws a cursor carret.
*
******************************************************************************/
static void DgaCarret(BOOL fOn)
{
    // Depending on the off/on message, we redraw cached ASCII code or inverse of it
    // Depending on the insert/overtype more, invert whole character or use a special col code (-1)
    if( fOn )
        DgaPrintCharacter((outDga.x+1), (outDga.y+1), cacheText[outDga.y][outDga.x], deb.fOvertype ? COL_REVERSE : -1);
    else
        DgaPrintCharacter((outDga.x+1), (outDga.y+1), cacheText[outDga.y][outDga.x], dga.col);
}


/******************************************************************************
*                                                                             *
*   static void ScrollUp()                                                    *
*                                                                             *
*******************************************************************************
*
*   Scrolls up a region in a graphics mode. We do little speed trick
*   here - we copy the cacheText buffer, scroll it, and then we actually print
*   out only those characters that changed.
*
******************************************************************************/
static void ScrollUp()
{
    int x, y;
    static BYTE cacheTextPreScroll[MAX_OUTPUT_SIZEY][MAX_OUTPUT_SIZEX];

    // Copy cacheText state before scroll
    memmove(&cacheTextPreScroll[dga.scrollTop][0],
            &cacheText[dga.scrollTop][0],
            MAX_OUTPUT_SIZEX * (dga.scrollBottom-dga.scrollTop+1));

    // We use cacheText buffer that we scroll first, and then read the
    // content to print it, since reading from the linear frame buffer in order
    // to scroll it is painfully slow..
    if( (dga.scrollTop < dga.scrollBottom) && (dga.scrollBottom < outDga.sizeY) )
    {
        CacheTextScrollUp(dga.scrollTop, dga.scrollBottom);

        for(y=dga.scrollTop; y<=dga.scrollBottom; y++ )
        {
            for(x=0; x<outDga.sizeX; x++)
            {
                // Print out only those positions that changed after the scroll
                if( cacheText[y][x] != cacheTextPreScroll[y][x] )
                    DgaPrintCharacter(x+1, y+1, cacheText[y][x], dga.col);
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   static void DgaSprint(char *s)                                            *
*                                                                             *
*******************************************************************************
*
*   String output to a DGA graphics frame buffer.
*
******************************************************************************/
static void DgaSprint(char *s)
{
    BYTE c;
    UINT nTabs;

    // Warning: this function is being reentered
    while( (c = *s++) != 0 )
    {
        if( dga.fEnabled )
        {
            if( c==DP_TAB )
            {
                for(nTabs=deb.nTabs; nTabs; nTabs--)
                    DgaSprint(" ");
            }
            else
            switch( c )
            {
                case DP_ENABLE_OUTPUT:  // Which we ignore
                    break;

                case DP_DISABLE_OUTPUT:
                        dga.fEnabled = FALSE;
                    break;

                case DP_SAVEBACKGROUND:
                        MoveBackground(TRUE);
                    break;

                case DP_RESTOREBACKGROUND:
                        MoveBackground(FALSE);
                    break;

                case DP_CLS:
                        // Clear the cache text
                        CacheTextCls();

                        // Use a depth-dependent function to clear the framebuffer background
                        if( dga.Cls )
                            (dga.Cls)();

                        // Print out window borders
                        // Vertical edges
                        for(outDga.y=1; outDga.y<(outDga.sizeY+1); outDga.y++ )
                        {
                            DgaPrintCharacter(0, outDga.y, 0xBA, COL_LINE);
                            DgaPrintCharacter(outDga.sizeX+1, outDga.y, 0xBA, COL_LINE);
                        }

                        // Horizontal edges
                        for(outDga.x=1; outDga.x<(outDga.sizeX+1); outDga.x++ )
                        {
                            DgaPrintCharacter(outDga.x, 0, 0xCD, COL_LINE);
                            DgaPrintCharacter(outDga.x, outDga.sizeY+1, 0xCD, COL_LINE);
                        }

                        // Four corners
                        DgaPrintCharacter(0, 0,              0xC9, COL_LINE);
                        DgaPrintCharacter(outDga.sizeX+1, 0, 0xBB, COL_LINE);
                        DgaPrintCharacter(0, outDga.sizeY+1, 0xC8, COL_LINE);
                        DgaPrintCharacter(outDga.sizeX+1, outDga.sizeY+1, 0xBC, COL_LINE);

                        // Reset the cursor coordinates
                        outDga.x = 0;
                        outDga.y = 0;
                    break;

                case DP_SETCURSORXY:
                        outDga.x = (*s++)-1;
                        outDga.y = (*s++)-1;
                    break;

                case DP_SETCURSORSHAPE:
                        deb.fOvertype = (*s++)-1;
                    break;

                case DP_SAVEXY:
                        dga.savedX = outDga.x;
                        dga.savedY = outDga.y;
                    break;

                case DP_RESTOREXY:
                        outDga.x = dga.savedX;
                        outDga.y = dga.savedY;
                    break;

                case DP_SETSCROLLREGIONYY:
                        dga.scrollTop = (*s++)-1;
                        dga.scrollBottom = (*s++)-1;
                    break;

                case DP_SCROLLUP:
                        // Scroll a portion of the screen up and clear the bottom line
                        ScrollUp();
                    break;

                case DP_SCROLLDOWN:
                        // Scroll a portion of the screen down and clear the top line
                        // We dont support this type of scroll here
                    break;

                case DP_SETCOLINDEX:
                        dga.col = *s++;
                    break;

                case '\r':
                        // Erase all characters to the right of the cursor pos and move cursor back
                        while( outDga.x < outDga.sizeX )
                        {
                            // Recurse to print spaces
                            DgaSprint(" ");
                        }

                        // Reset cursor coordinates and color attribute
                        outDga.x = 0;
                        dga.col = COL_NORMAL;
                    break;

                case '\n':
                        // Go to a new line, possible autoscroll
                        outDga.x = 0;
                        dga.col = COL_NORMAL;

                        // Check if we are on the last line of autoscroll
                        if( dga.scrollBottom==outDga.y )
                            ScrollUp();
                        else
                            outDga.y++;
                    break;

                case DP_RIGHTALIGN:
                        // Right align the rest of the text
                        outDga.x = outDga.sizeX - strlen(s);
                    break;

                case DP_ESCAPE:
                        // Escape character prints the next code as raw ascii
                        c = *s++;

                        // This case continues into the default...!

                default:
                        // Output a character on the screen
                        if( outDga.x < outDga.sizeX )
                        {
                            DgaPrintCharacter(outDga.x+1, outDga.y+1, c, dga.col);

                            // Store it in the cache
                            cacheText[outDga.y][outDga.x] = c;

                            // Advance the print position
                            outDga.x++;
                        }
                    break;
            }
        }
        else
        {
            if( c==DP_ENABLE_OUTPUT )
                dga.fEnabled = TRUE;
        }
    }
}


/******************************************************************************
*                                                                             *
*   void XWinControl(WCHAR Key)                                               *
*                                                                             *
*******************************************************************************
*
*   This function is called from the keyboard loop for certain hotkeys that
*   are dedicated to moving the window around.
*
*   Where:
*       Key is the key code used to move/center the window
*
******************************************************************************/
void XWinControl(WCHAR Key)
{
    // Ignore this call if the LFB is not the default current output driver

    if( pOut==&outDga )
    {
        // First, send a message to hide our window so we can move it
        dputc(DP_RESTOREBACKGROUND);

        switch( Key )
        {
            case CHAR_CTRL + CHAR_ALT + HOME:
                // Reset the window start coordinates to (0,0)
                deb.FrameX = deb.FrameY = 0;
                break;

            case CHAR_CTRL + CHAR_ALT + 'c':
            case CHAR_CTRL + CHAR_ALT + 'C':
                // Center the window on the screen
                deb.FrameX = dga.xres/2 - ((outDga.sizeX+2) * dga.bpp)/2;
                deb.FrameY = dga.yres/2 - ((outDga.sizeY+2) * dga.pFont->ysize)/2;
                break;

            case CHAR_CTRL + CHAR_ALT + LEFT:
                // Move window to the left or align it with the left edge
                if( deb.FrameX > XWIN_MOVE )
                    deb.FrameX -= XWIN_MOVE;
                else
                    deb.FrameX = 0;
                break;

            case CHAR_CTRL + CHAR_ALT + RIGHT:
                // Move window to the right or align it with the right edge
                // Take into account window borders. Final check is done below..
                deb.FrameX += XWIN_MOVE;
                break;

            case CHAR_CTRL + CHAR_ALT + UP:
                // Move window up or align it with the top line
                if( deb.FrameY > XWIN_MOVE )
                    deb.FrameY -= XWIN_MOVE;
                else
                    deb.FrameY = 0;
                break;

            case CHAR_CTRL + CHAR_ALT + DOWN:
                // Move window down or align it with the bottom
                // Take into account window borders. Final check is done below..
                deb.FrameY += XWIN_MOVE;
                break;
        }

        // Moving window to the right, down or centering may have pushed it
        // over the right or bottom edge, and that could be fatal. Readjust if necessary.
        if( deb.FrameX + (outDga.sizeX+2) * 8 >= dga.xres )
            deb.FrameX = dga.xres - (outDga.sizeX+2) * 8;

        if( deb.FrameY + (outDga.sizeY+2) * dga.pFont->ysize >= dga.yres )
            deb.FrameY = dga.yres - (outDga.sizeY+2) * dga.pFont->ysize;

        // Recalculate new address of our window frame
        dga.dwFrameOffset = deb.FrameY * dga.stride + deb.FrameX * dga.bpp;

        // Repaint the window completely
        dputc(DP_SAVEBACKGROUND);

        RecalculateDrawWindows();
    }
}

