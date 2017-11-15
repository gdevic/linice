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

#include <asm-i386/page_offset.h>       // We need page offset

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
    BYTE misc_output;
    DWORD baseCRTC;
    BYTE CRTC[0x19];                    // CRTC Registers
    
    WORD textbuf[ MAX_SIZEX * MAX_SIZEY ];

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
    ice_memset(&vga, 0, sizeof(vga));

    // Set default parameters

    outVga.x = 0;
    outVga.y = 0;
    outVga.width = 80;
    outVga.height = 25;

    vga.pText = (BYTE *) LINUX_VGA_TEXT;
    vga.sprint = VgaSprint;
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
*   static void ScrollUp()                                                    *
*                                                                             *
*******************************************************************************
*
*   Scrolls up a region
*
******************************************************************************/
static void ScrollUp()
{
    if( (vga.scrollTop < vga.scrollBottom) && (vga.scrollBottom < outVga.height) )
    {
        // Scroll up all requested lines
        memmove(vga.pText + (vga.scrollTop * vga.width) * 2,
                vga.pText + ((vga.scrollTop + 1) * vga.width) * 2,
                vga.width * 2 * (vga.scrollBottom - vga.scrollTop));

        // Clear the last line
        memset(vga.pText + (vga.scrollBottom * outVga.height) * 2,
               0,
               outVga.width * 2 );
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
    while( *s )
    {
        switch( *s++ )
        {
            case DP_SAVEBACKGROUND:
                    SaveBackground();
                break;

            case DP_RESTOREBACKGROUND:
                    RestoreBackground();
                break;

            case DP_CLS:
                    // Clear the screen and reset the cursor coordinates
                    memset(vga.pText, 0, outVga.height * outVga.width * 2);
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
                    if( (vga.scrollTop < vga.scrollBottom) && (vga.scrollBottom < outVga.height) )
                    {
                        // Scroll down all requested lines
                        memmove(vga.pText + ((vga.scrollTop + 1) * outVga.width) * 2,
                                vga.pText + (vga.scrollTop * outVga.width) * 2,
                                outVga.width * 2 * (vga.scrollBottom - vga.scrollTop));

                        // Clear the first line
                        memset(vga.pText + (vga.scrollTop * outVga.width) * 2,
                               0,
                               outVga.width * 2 );
                    }
                break;

            case DP_SETWRITEATTR:
                    s++;            // NOT IMPLEMENTED YET
                break;

            case '\n':
                    // Go to a new line, possible autoscroll
                    outVga.x = 0;
    
                    // Check if we are on the last line of autoscroll
                    if( vga.scrollBottom==vga.y )
                        ScrollUp();
                    else
                        outVga.y++;
                break;

            default:
                    // All printable characters
                    *(WORD *)(vga.pText + (outVga.x +  outVga.y * outVga.width) * 2) = (WORD) c + 0x0700;

                    // Advance the print position
                    if( outVga.x < outVga.width )
                        outVga.x++;
                break;
        }
    }
}

