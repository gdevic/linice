/******************************************************************************
*                                                                             *
*   Module:     text.c                                                        *
*                                                                             *
*   Date:       11/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements output functionality for VGA text modes
		(mapped at B800:0000) and monochrome buffer (mapped at B000:0000).

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/01/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define LINUX_VGA_TEXT  0xC00B8000

#define SIZEX           80
#define SIZEY           25

//---------------------------------------------------
// VGA registers and memory to save
//---------------------------------------------------

typedef struct
{
    BYTE misc_output;
    DWORD baseCRTC;
    BYTE CRTC[0x19];                    // CRTC Registers
    
    WORD textbuf[ SIZEX * SIZEY ];

} TVgaState;

static TVgaState vgaState;

static const int crtc_enable[0x19] = {
    1, 1, 1, 1, 1, 1, 1, 0, 
    0, 0, 1, 1, 1, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0, 
    0
};


//---------------------------------------------------
// Helper variables for display output
//---------------------------------------------------

typedef struct
{
    DWORD   sizeX, sizeY;               // Screen size
    DWORD   x, y;                       // X, Y printing location
    WORD    attr;                       // Current write attribute
    BYTE *  pText;                      // Address of the VGA text buffer

    DWORD   ScrollTop;                  // Y top for autoscroll
    DWORD   ScrollBottom;               // Y bottom for autoscroll
    
} TVga;

static TVga vga;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

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
    memset(&vgaState, 0, sizeof(vgaState));
    memset(&vga, 0, sizeof(vga));

    // Set default parameters

    vga.sizeY = SIZEY;
    vga.sizeX = SIZEX;
    vga.attr  = 0x0700;
    vga.pText = (BYTE *) LINUX_VGA_TEXT;

    // Set text VGA to be the default output

    pfnPutChar = vga_putchar;
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
    BYTE index;

    vgaState.misc_output = inp(0x3CC);

    if( vgaState.misc_output & 1 )
        vgaState.baseCRTC = 0x3D4;
    else
        vgaState.baseCRTC = 0x3B4;

    // Store CRTC registers and program them with our values

    for( index=0; index<0x19; index++)
    {
        if( crtc_enable[index] )
        {
            vgaState.CRTC[index] = ReadCRTC(vgaState.baseCRTC, index);
        }
    }

    // Set up VGA to something we can handle

    WriteCRTC(vgaState.baseCRTC, 0xC, 0);
    WriteCRTC(vgaState.baseCRTC, 0xD, 0);

    // Store away what was in the text buffer that we will use

    memcpy(vgaState.textbuf, vga.pText, vga.sizeX * vga.sizeY * 2);
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

    // Restore CRTC registers

    for( index=0; index<0x19; index++)
    {
        if( crtc_enable[index] )
        {
            WriteCRTC(vgaState.baseCRTC, index, vgaState.CRTC[index]);
        }
    }

    // Restore the frame buffer content that was there before we stepped in

    memcpy(vga.pText, vgaState.textbuf, vga.sizeX * vga.sizeY * 2);
}   


/******************************************************************************
*                                                                             *
*   vga_putchar( char c )                                                     *
*                                                                             *
*******************************************************************************
*
*   Output a character on a VGA text mode screen.  Take care of other
*   special control codes.
*
******************************************************************************/
void vga_putchar( char c )
{
    static int xSave[4], ySave[4];
    static int iSave = 0;
    static int nRetainCount = 0;
    static BYTE bRetain[5];
    DWORD yTop, yBottom;


    // If the retained count is not zero, decrement it, and if it reached
    // zero now, we can execute special code command

    if( nRetainCount )
    {
        // Store additional character in a buffer

        bRetain[ nRetainCount-- ] = c;

        if( nRetainCount==0 )
        {
            switch( bRetain[0] )
            {
                case DP_SETWRITEATTR:

                    // Set the attribute for writing text out

                    vga.attr = (WORD) bRetain[1] << 8;

                    break;

                case DP_SETLOCATTR:

                    // Set attribute patch at the given coordinate
                    // bRetain[0]  - command code
                    // bRetain[1]  - length of a patch in characters
                    // bRetain[2]  - attribute value
                    // bRetain[3]  - y coordinate
                    // bRetain[4]  - x coordinate

                    if( bRetain[3]==0xFF )
                        bRetain[3] = vga.y;

                    if( bRetain[4]==0xFF )
                        bRetain[4] = vga.x;

                    while( bRetain[1]-- )
                    {
                        *(BYTE *)(vga.pText + (bRetain[1] + bRetain[4] + bRetain[3] * vga.sizeX) * 2 + 1) = bRetain[2];
                    }

                    break;

                case DP_SETCURSOR:

                    // Set the write cursor location
                    // bRetain[0]  - command code
                    // bRetain[1]  - y coordinate
                    // bRetain[2]  - x coordinate

                    if( bRetain[1]==0xFF )
                        bRetain[1] = vga.y;

                    if( bRetain[2]==0xFF )
                        bRetain[2] = vga.x;

                    vga.x = bRetain[2];
                    vga.y = bRetain[1];

                    break;

                case DP_SETLINES:

                    // Set the number of lines of a text display
                    // bRetain[0]  - command code
                    // bRetain[1]  - Number of lines

                    vga.sizeY = bRetain[1];

                    break;

                case DP_SCROLLUP:
     
                    // Scroll a portion of the screen up and clear the bottom line
                    // bRetain[0]  - command code
                    // bRetain[1]  - Y bottom coordinate
                    // bRetain[2]  - Y top coordinate

                    yTop = bRetain[2];
                    yBottom = bRetain[1];

                    if( (yTop < yBottom) && (yBottom < vga.sizeY) )
                    {
                        // Scroll up all requested lines
                        memmove(vga.pText + (yTop * vga.sizeX) * 2,
                                vga.pText + ((yTop + 1) * vga.sizeX) * 2,
                                vga.sizeX * 2 * (yBottom - yTop));
    
                        // Clear the last line
                        memset_w(vga.pText + (yBottom * vga.sizeX) * 2,
                                 vga.attr,
                                 vga.sizeX );
                    }

                    break;

                case DP_SCROLLDOWN:
     
                    // Scroll a portion of the screen down and clear the top line
                    // bRetain[0]  - command code
                    // bRetain[1]  - Y bottom coordinate
                    // bRetain[2]  - Y top coordinate

                    yTop = bRetain[2];
                    yBottom = bRetain[1];

                    if( (yTop < yBottom) && (yBottom < vga.sizeY) )
                    {
                        // Scroll down all requested lines
                        memmove(vga.pText + ((yTop + 1) * vga.sizeX) * 2,
                                vga.pText + (yTop * vga.sizeX) * 2,
                                vga.sizeX * 2 * (yBottom - yTop));
    
                        // Clear the first line
                        memset_w(vga.pText + (yTop * vga.sizeX) * 2,
                                 vga.attr,
                                 vga.sizeX );
                    }

                    break;

                case DP_SETSCROLLREGION:

                    // Set an autoscroll region for the print output.  Whenever
                    // the last character is on the bScrollBottom line and it
                    // causes the next char to slide to a new line, this
                    // region is scrolled up by one line.
                    // bRetain[0]  - command code
                    // bRetain[1]  - Y bottom coordinate
                    // bRetain[2]  - Y top coordinate

                    vga.ScrollTop = bRetain[2];
                    vga.ScrollBottom = bRetain[1];

                    break;
            }
        }

        return;
    }

    // Evaluate any character that may be a special sequence or code

    switch( c )
    {
        case DP_SETWRITEATTR:
        case DP_SETLINES:

            // We need an additional byte to set the working attribute (a)
            // Set the number of lines - need additional byte

            nRetainCount = 1;
            bRetain[0] = c;

            break;

        case DP_SETCURSOR:
        case DP_SCROLLUP:
        case DP_SCROLLDOWN:
        case DP_SETSCROLLREGION:

            // Additional 2 bytes are needed for cursor coordinates (x,y)
            // Additional 2 bytes are needed for scroll up/down (yTop, yBottom)
            // Additional 2 bytes are needed to set the autoscroll region (yTop, yBottom)

            nRetainCount = 2;
            bRetain[0] = c;

            break;

        case DP_SETLOCATTR:

            // We need 4 additional bytes to set arbitrary attribute patch (x,y,a,len)

            nRetainCount = 4;
            bRetain[0] = c;

            break;

        case DP_CLS:

            // Clear the screen and reser the cursor coordinates

            memset_w(vga.pText, vga.attr, vga.sizeX * vga.sizeY);
            vga.x = 0;
            vga.y = 0;

            break;

        case DP_SAVEBACKGROUND:

            // Save content of the background

            SaveBackground();

            break;

        case DP_RESTOREBACKGROUND:

            // Restore saved content of the background

            RestoreBackground();

            break;

        case DP_SAVEXY:

            // Save cursor location

            xSave[iSave] = vga.x;
            ySave[iSave] = vga.y;
            iSave = (iSave + 1) & 3;

            break;

        case DP_RESTOREXY:

            // Restore saved cursor location

            iSave = (iSave - 1) & 3;
            vga.x = xSave[iSave];
            vga.y = ySave[iSave];

            break;

        case '\n':

            // Go to a new line, possible autoscroll. Clear to the end of the current line

            if( vga.x < vga.sizeX )
                memset_w(vga.pText + (vga.x + vga.y * vga.sizeX) * 2,
                         vga.attr,
                         vga.sizeX - vga.x );
            vga.x = 0;

            // Check if we are on the last line of autoscroll
            if( (vga.ScrollTop < vga.ScrollBottom) && (vga.ScrollBottom==vga.y) )
            {
                // Scroll up all austoscroll lines
                memmove(vga.pText + (vga.ScrollTop * vga.sizeX) * 2,
                        vga.pText + ((vga.ScrollTop + 1) * vga.sizeX) * 2,
                        vga.sizeX * 2 * (vga.ScrollBottom - vga.ScrollTop));

                // Clear the last line
                memset_w(vga.pText + (vga.ScrollBottom * vga.sizeX) * 2,
                         vga.attr,
                         vga.sizeX );
            }
            else
                vga.y++;

            nCharsWritten++;

            break;

        case '\r':

            // Clear to the end of the current line and reset X

            if( vga.x < vga.sizeX )
                memset_w(vga.pText + (vga.x + vga.y * vga.sizeX) * 2,
                         vga.attr,
                         vga.sizeX - vga.x );
            vga.x = 0;
            nCharsWritten++;

            break;

        default:

            // All printable characters

            *(WORD *)(vga.pText + (vga.x +  vga.y * vga.sizeX) * 2) = (WORD) c + vga.attr;

            // Advance the print position

            if( vga.x < vga.sizeX )
                vga.x++;

            nCharsWritten++;

            break;
    }
}


