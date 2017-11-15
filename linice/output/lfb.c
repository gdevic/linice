/******************************************************************************
*                                                                             *
*   Module:     lfb.c                                                         *
*                                                                             *
*   Date:       05/18/01                                                      *
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

#include "module-header.h"              // Versatile module header file

#include <asm/page.h>
//#include <asm/pgtable.h>
#include <linux/mm.h>                   // Include memory management protos
#include <asm/io.h>

#include "clib.h"                       // Include C library header file
#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

#include "font.h"                       // Include one-time font definition

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern TOUT outVga;                     // VGA output device
extern TOUT outMda;                     // MDA output device

TOUT outDga;

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

typedef struct
{
    BYTE *Bitmap;                       // Address of the font bitmap
    int ysize;                          // Font height in pixels
} TFont;

#define MAX_FONTS           2           // Number of graphics fonts available

static TFont Font[MAX_FONTS] = {
{
    (BYTE *)&font8x8,                   // 8x8 font
    8                                   // 8 pixels high
},
{
    (BYTE *)&font8x16,                  // 8x16 font
    16                                  // 16 pixels high
}
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
static BOOL DgaResize(int x, int y);

extern void CacheTextScrollUp(DWORD top, DWORD bottom);
extern void CacheTextCls();

extern DWORD UserVirtToPhys(DWORD address);

extern void ice_free_heap(BYTE *pHeap);
extern BYTE *ice_malloc(DWORD size);

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
        pIce->nXDrawSize = pXInit->dwDrawSize;

    // Catch unreasonable large buffer size requests
    if( pIce->nXDrawSize > MAX_XWIN_BUFFER )
    {
        pIce->nXDrawSize = MAX_XWIN_BUFFER;
    }

    // Make sure the size is enough for the starting buffer 80x25 and default first font
    dwSize = (80+2) * 8 * pXInit->bpp * (25+2) * Font[0].ysize;

    if( pIce->nXDrawSize < dwSize )
    {
        pIce->nXDrawSize = dwSize;
    }

    dprinth(1, "XWIN: Received XInitPacket.. Using buffer size of %d (%d Kb)", pIce->nXDrawSize, pIce->nXDrawSize/1024);

    // If we already allocated buffers etc., need to release before reallocating
    if( pIce->pXDrawBuffer != NULL )
    {
        ice_free_heap(pIce->pXDrawBuffer);

        if( pIce->pXFrameBuffer != NULL )
            iounmap(pIce->pXFrameBuffer);

        pIce->pXDrawBuffer = NULL;
        pIce->pXFrameBuffer = NULL;
    }

    // Allocate memory that will be used to save the content of the framebuffer
    // over the area of drawing
    if( (pIce->pXDrawBuffer = ice_malloc(pIce->nXDrawSize)) != NULL)
    {
        INFO(("XWIN: Allocated %d for backing store buffer\n", (int)pIce->nXDrawSize));

        // Find the physical address of the frame buffer in user address space
        physicalAddress = UserVirtToPhys(pXInit->pFrameBuf);

        // Find the size of the frame buffer that we need to map
        dwMappingSize = pXInit->stride * pXInit->yres;

        // Map the graphics card physical memory into the kernel address space
        pIce->pXFrameBuffer = ioremap(physicalAddress, dwMappingSize);

        if( pIce->pXFrameBuffer )
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
            dga.pFont = &Font[0];               // Default first font
            dga.fEnabled = FALSE;
        
            dga.stride = pXInit->stride;
            dga.xres   = pXInit->xres;
            dga.yres   = pXInit->yres;
            dga.bpp    = pXInit->bpp;

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
            dprinth(1, "XWIN: Kernel address       = %08X (%08X)", pIce->pXFrameBuffer, dwMappingSize);
            dprinth(1, "XWIN: Desktop %d x %d, stride=%d  bpp=%d", dga.xres, dga.yres, dga.stride, dga.bpp * 8);

            // Switch output to this driver and schedule a clean break into debugger

            pOut = &outDga;

            pIce->fKbdBreak = TRUE;

            return( 0 );                // Return success
        }
        else
        {
            // We could not map the frame buffer.. Clean up and switch back to vga driver

            pOut = &outVga;
    
            dprinth(1, "XWIN: Unable to map frame buffer (size=%d)!", dwMappingSize);

            ice_free_heap(pIce->pXDrawBuffer);
        }
    }
    else
    {
        // We could not allocate memory.. Clean up and switch back to a vga driver

        pOut = &outVga;

        dprinth(1, "XWIN: Unable to allocate %d for backing store buffer!", pIce->nXDrawSize);

    }

    pIce->pXDrawBuffer = NULL;
    pIce->pXFrameBuffer = NULL;

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
    pixelFore = dga.pixelFore[pIce->col[col] & 0xF];
    pixelBack = dga.pixelBack[(pIce->col[col] >> 4) & 0x7];

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
    pixelFore = (WORD)(dga.pixelFore[pIce->col[col] & 0xF] & 0xFFFF);
    pixelBack = (WORD)(dga.pixelBack[(pIce->col[col] >> 4) & 0x7] & 0xFFFF);

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

    // Do not print character if it is out-of screen or would go there
    if( x < (dga.xres - 8) / 8 )
    {
        if( y < (dga.yres - dga.pFont->ysize) / dga.pFont->ysize )
        {
            // Calculate the address in the frame buffer to print a character
            address = (DWORD) pIce->pXFrameBuffer + dga.dwFrameOffset +
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

    pixelBack = (WORD)(dga.pixelBack[(pIce->col[dga.col] >> 4) & 0x7] & 0xFFFF);
    address = pIce->pXFrameBuffer + dga.dwFrameOffset;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        // Set memory - word sizes
        // We are adding for borders
        memset_w(address, pixelBack, (pOut->sizeX+2) * 8);
        address += dga.stride;
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

    pixelBack = dga.pixelBack[(pIce->col[dga.col] >> 4) & 0x7];
    address = pIce->pXFrameBuffer + dga.dwFrameOffset;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        // Set memory - double word sizes
        // We are adding for borders
        memset_d(address, pixelBack, (pOut->sizeX+2) * 8);
        address += dga.stride;
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
    if( pIce->pXDrawBuffer )
    {
        // Calculate required size in bytes of the window area
        // We are adding border (2 characters)
        size = (outDga.sizeX+2) * 8 * dga.bpp * // X size of the font is always 8 pixels
               (outDga.sizeY+2) * dga.pFont->ysize;
        if( size <= pIce->nXDrawSize )
        {
            pBuf = pIce->pXDrawBuffer;
            address = (DWORD) pIce->pXFrameBuffer + dga.dwFrameOffset;

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
            dprinth(1, "XWIN: Backing store buffer too small (need %d, have %d)", size, pIce->nXDrawSize);
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
*   static BOOL DgaResize(int x, int y)                                       *
*                                                                             *
*******************************************************************************
*
*   Resize DGA display
*
******************************************************************************/
static BOOL DgaResize(int x, int y)
{
    DWORD dwSize;                       // Backing store buffer requirement

    // Depending on the amount of backing store buffer memory allocated,
    // we may want to decrease this request to what would fit

    dwSize = (x+2) * 8 * dga.bpp * (y+2) * dga.pFont->ysize;

    if( dwSize > pIce->nXDrawSize )
    {
        // New window size needs more buffer. Print out the required size 
        // and scale down to use what we currently have allocated.

        dprinth(1, "That size would need %d of backing store buffer out of %d allocated.", dwSize, pIce->nXDrawSize);

        // We know we are changing only one variable at a time (either command LINES or WIDTH)
        if( x != outDga.sizeX )
        {
            // Changing X size (command WIDTH)
            x = pIce->nXDrawSize / (8 * dga.bpp * (outDga.sizeY+2) * dga.pFont->ysize);
            x -= 2;                     // Account for 2 border edges

            dwSize = (x+2) * 8 * dga.bpp * (y+2) * dga.pFont->ysize;

            dprinth(1, "Using WIDTH of %d with %d bytes of backing store buffer needed.", x, dwSize);
        }
        else
        {
            // Changing Y size (command LINES)
            y = pIce->nXDrawSize / ((outDga.sizeX+2) * 8 * dga.bpp * dga.pFont->ysize);
            y -= 2;                     // Account for 2 border edges

            dwSize = (x+2) * 8 * dga.bpp * (y+2) * dga.pFont->ysize;

            dprinth(1, "Using LINES of %d with %d bytes of backing store buffer needed.", y, dwSize);
        }
    }

    dputc(DP_RESTOREBACKGROUND);

    // After resizing, we may want to readjust the starting frame X and Y coordinates

    if( deb.FrameX + (x+2) * dga.bpp >= dga.xres )
        deb.FrameX = dga.xres - (x+2) * dga.bpp;

    if( deb.FrameY + (y+2) * dga.pFont->ysize >= dga.yres )
        deb.FrameY = dga.yres - (y+2) * dga.pFont->ysize;

    dga.dwFrameOffset = deb.FrameY * dga.stride + deb.FrameX * dga.bpp;

    // Size is now ok, it will fit. Readjust the variables and exit.
    outDga.sizeX = x;
    outDga.sizeY = y;

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
    DWORD dwTabs;

    // Warning: this function is being reentered
    while( (c = *s++) != 0 )
    {
        if( dga.fEnabled )
        {
            if( c==DP_TAB )
            {
                for(dwTabs=deb.dwTabs; dwTabs; dwTabs--)
                    DgaSprint(" ");
            }
            else
            switch( c )
            {
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

                default:
                        // Output a character on the screen
                        DgaPrintCharacter(outDga.x+1, outDga.y+1, c, dga.col);

                        // Store it in the cache
                        cacheText[outDga.y][outDga.x] = c;

                        // Advance the print position
                        if( outDga.x < outDga.sizeX )
                            outDga.x++;
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
*   void XWinControl(CHAR Key)                                                *
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
void XWinControl(CHAR Key)
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
        if( deb.FrameX + (outDga.sizeX+2) * dga.bpp >= dga.xres )
            deb.FrameX = dga.xres - (outDga.sizeX+2) * dga.bpp;
    
        if( deb.FrameY + (outDga.sizeY+2) * dga.pFont->ysize >= dga.yres )
            deb.FrameY = dga.yres - (outDga.sizeY+2) * dga.pFont->ysize;

        // Recalculate new address of our window frame
        dga.dwFrameOffset = deb.FrameY * dga.stride + deb.FrameX * dga.bpp;

        // Repaint the window completely
        dputc(DP_SAVEBACKGROUND);

        RecalculateDrawWindows();
    }
}

