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
//    BYTE *pFB;                          // Address of the framebuffer top-left corner
//    BYTE *pFrame;                       // Address of the linice frame top-left corner
//    BYTE *pSave;                        // Address of the save buffer

    int col;                            // Current line's color index
    DWORD pixelFore[16];                // Pixel foreground colors
    DWORD pixelBack[16];                // Pixel background colors
    BYTE savedX, savedY;                // Last recently saved cursor coordinates
    BYTE scrollTop, scrollBottom;       // Scroll region top and bottom coordinates

    TFont *pFont;                       // Pointer to a current font structure
    DWORD stride;                       // Screen stride
    DWORD xres, yres;                   // X, Y resolution in pixels
    DWORD bpp;                          // BYTES per pixel :-)
    void (*PrintChar)(DWORD, BYTE);     // Raw print char function
    void (*Cls)(void);                  // Raw cls function

} TDGA;

static TDGA dga;

extern BYTE cacheText[MAX_Y][MAX_X];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static void DgaPrintCharacter32(DWORD ptr, BYTE c);
static void DgaPrintCharacter16(DWORD ptr, BYTE c);
void DgaSprint(char *s);
static void DgaCls32();
static void DgaCls16();

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
    int retval = 0, i;
    DWORD physicalAddress;
    static BOOL fInit = FALSE;          // Has not been initialized

    dprinth(1, "XInitPacket");

    if( fInit )
    {
        // This is not a first call to init function. We need to reset the
        // output subsystem so we can allocate all fresh.

        if( pIce->pXDrawBuffer != NULL )
            ice_free_heap(pIce->pXDrawBuffer);

        if( pIce->pXFrameBuffer != NULL )
            iounmap(pIce->pXFrameBuffer);

        pIce->pXDrawBuffer = NULL;
    }
    else
        fInit = TRUE;

    // Allocate memory that will be used to save the content of the framebuffer
    // over the area of drawing
    if( (pIce->pXDrawBuffer = ice_malloc(pIce->nXDrawSize)) != NULL)
    {
        INFO(("Allocated %d Kb for X Draw buffer\n", (int)pIce->nXDrawSize/1024));

        dprinth(1, "user virtual address = %08X", pXInit->pFrameBuf);

        // Find the physical address of the frame buffer in user address space
        physicalAddress = UserVirtToPhys(pXInit->pFrameBuf);

        dprinth(1, "physicalAddress = %08X", physicalAddress);

        // Map the graphics card physical memory to a kernel address space
        pIce->pXFrameBuffer = ioremap(physicalAddress, pIce->nXDrawSize);

        dprinth(1, "pXFrameBuffer = %08X", pIce->pXFrameBuffer);

        if( pIce->pXFrameBuffer == NULL )
        {
            dprinth(1, "Unable to map frame buffer!\n");
            retval = -1;
        }
    }
    else
    {
        dprinth(1, "Unable to allocate %d Kb for X Draw buffer!\n", pIce->nXDrawSize/1024);
        retval = -1;
    }

    memset(&dga, 0, sizeof(dga));

    // Set default parameters

    outDga.x = 0;
    outDga.y = 0;
    outDga.sizeX = 80;
    outDga.sizeY = 25;
    outDga.sprint = DgaSprint;
    outDga.mouse = DgaMouse;
    outDga.resize = DgaResize;

    dga.scrollTop = 0;
    dga.scrollBottom = outDga.sizeY - 1;
    dga.col = COL_NORMAL;
    dga.pFont = &Font[0];               // Default first font

    dga.stride = pXInit->stride;
    dga.xres   = pXInit->xres;
    dga.yres   = pXInit->yres;
    dga.bpp    = pXInit->bpp;

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
            dprinth(1, "16 BPP NOT TESTED!");     break;
        default:
            dprinth(1, "ERROR: Not supported pixel depth of %d", dga.bpp * 8);
    }

    dprinth(1, "Desktop %d x %d, stride=%d  bpp=%d", dga.xres, dga.yres, dga.stride, dga.bpp * 8);

    return(retval);
}


/******************************************************************************
*                                                                             *
*   void DgaPrintCharacter32(DWORD ptr, BYTE c)                               *
*                                                                             *
*******************************************************************************
*
*   A character printing function for 32bpp 888 format
*
******************************************************************************/
static void DgaPrintCharacter32(DWORD ptr, BYTE c)
{
    DWORD *pPixel;
    int x, y;
    BYTE *pChar;
    BYTE line;
    DWORD pixelFore, pixelBack;

    // Cache the current colors for foreground and background
    pixelFore = dga.pixelFore[pIce->col[dga.col] & 0xF];
    pixelBack = dga.pixelBack[(pIce->col[dga.col] >> 4) & 0x7];

    // Get the address of the start of a character
    pChar = (BYTE *)(dga.pFont->Bitmap + c * dga.pFont->ysize);

    for(y=0; y<dga.pFont->ysize; y++)
    {
        // Get one scanline of a character font
        line = *pChar++;
        pPixel = (DWORD *) ptr;

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
*   void DgaPrintCharacter16(DWORD ptr, BYTE c)                               *
*                                                                             *
*******************************************************************************
*
*   A character printing function for 16bpp
*
******************************************************************************/
static void DgaPrintCharacter16(DWORD ptr, BYTE c)
{
    WORD *pPixel;
    int x, y;
    BYTE *pChar;
    BYTE line;
    WORD pixelFore, pixelBack;

    // Cache the current colors for foreground and background
    pixelFore = (WORD)(dga.pixelFore[pIce->col[dga.col] & 0xF] & 0xFFFF);
    pixelBack = (WORD)(dga.pixelBack[(pIce->col[dga.col] >> 4) & 0x7] & 0xFFFF);

    // Get the address of the start of a character
    pChar = (BYTE *)(dga.pFont->Bitmap + c * dga.pFont->ysize);

    for(y=0; y<dga.pFont->ysize; y++)
    {
        // Get one scanline of a character font
        line = *pChar++;
        pPixel = (WORD *) ptr;

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
*   void DgaPrintCharacter(DWORD x, DWORD y, BYTE c)                          *
*                                                                             *
*******************************************************************************
*
*   Root character printing function.
*
******************************************************************************/
void DgaPrintCharacter(DWORD x, DWORD y, BYTE c)
{
    DWORD address;

    // Do not print character if it is out-of screen or would go there
    if( x < (dga.xres - 8) / 8 )
    {
        if( y < (dga.yres - dga.pFont->ysize) / dga.pFont->ysize )
        {
            // Calculate the address in the frame buffer to print a character
            address = (DWORD) pIce->pXFrameBuffer +
                y * dga.stride * dga.pFont->ysize +
                x * dga.bpp * 8;

            if( dga.PrintChar )
                (dga.PrintChar)(address, c);
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
    address = pIce->pXFrameBuffer;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        memset_w(address, pixelBack, pOut->sizeX * 8);
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
    address = pIce->pXFrameBuffer;

    for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
    {
        memset_d(address, pixelBack, pOut->sizeX * 8);
        address += dga.stride;
    }
}


/******************************************************************************
*                                                                             *
*   void SaveBackground(BOOL fSave)                                           *
*                                                                             *
*******************************************************************************
*
*   Saves or restores the DGA display window memory.
*
*   Where: fSave - TRUE for save background
*                  FALSE for restore background
*
******************************************************************************/
static void SaveBackground(BOOL fSave)
{
    DWORD size, y, address;
    BYTE *pBuf;

    // Make sure we have draw buffer allocated and that it is the right size
    if( pIce->pXDrawBuffer )
    {
        // Calculate required size in bytes of the window area
        size = pOut->sizeX * 8 * dga.bpp * // X size of the font is always 8 pixels
               pOut->sizeY * dga.pFont->ysize;
        if( size <= pIce->nXDrawSize )
        {
            pBuf = pIce->pXDrawBuffer;
            address = (DWORD) pIce->pXFrameBuffer;

            // Move the window area, line by line, in the required direction
            for(y=0; y<pOut->sizeY * dga.pFont->ysize; y++)
            {
                if( fSave )
                    memcpy((void *)pBuf, (void *)address, pOut->sizeX * 8 * dga.bpp);
                else
                    memcpy((void *)address, (void *)pBuf, pOut->sizeX * 8 * dga.bpp);

                pBuf += pOut->sizeX * 8 * dga.bpp;
                address += dga.stride;
            }
        }
        else
            dprinth(1, "XDrawBuffer too small (%08X of %08X)", size, pIce->nXDrawSize);
    }
    else
        dprinth(1, "XDrawBuffer not allocated");
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
    return( TRUE );
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
    int x, y;

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
                DgaPrintCharacter(x, y, cacheText[y][x]);
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   DgaSprint(char *s)                                                        *
*                                                                             *
*******************************************************************************
*
*   String output to a DGA graphics frame buffer.
*
******************************************************************************/
void DgaSprint(char *s)
{
    BYTE c;
    DWORD dwTabs;

    // Warning: this function is being reentered
    while( (c = *s++) != 0 )
    {
        if( c==DP_TAB )
        {
            for(dwTabs=deb.dwTabs; dwTabs; dwTabs--)
                DgaSprint(" ");
        }
        else
        switch( c )
        {
            case DP_SAVEBACKGROUND:
                    SaveBackground(TRUE);
                break;

            case DP_RESTOREBACKGROUND:
                    SaveBackground(FALSE);
                break;

            case DP_CLS:
                    // Clear the cache text
                    CacheTextCls();

                    // Use a depth-dependent function to clear the framebuffer background
                    if( dga.Cls )
                        (dga.Cls)();

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
                    DgaPrintCharacter(outDga.x, outDga.y, c);

                    // Store it in the cache
                    cacheText[outDga.y][outDga.x] = c;

                    // Advance the print position
                    if( outDga.x < outDga.sizeX )
                        outDga.x++;
                break;
        }

        ShowCursorPos();
    }
}

