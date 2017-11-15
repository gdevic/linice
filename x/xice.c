/******************************************************************************
*                                                                             *
*   Module:     xice.c                                                        *
*                                                                             *
*   Date:       05/20/01                                                      *
*                                                                             *
*   Copyright (c) 2001-2005 Goran Devic                                       *
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

        This module contains the code for X-Windows debugger setup

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/20/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

/* Xlib include files */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <X11/extensions/xf86dga.h>

#include <stdio.h>                      // Include standard C headers
#include <stdlib.h>
#include <string.h>
#include <asm/ioctl.h>                  // Include IO control macros

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include io control codes
#include "loader.h"                     // Include loader global protos

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

DWORD dwBufSize = 0;                    // Buffer size parameter, default value

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct
{
    char *ptr;                          // Pointer to the DGA framebuffer
    int  width;                         // Width of the screen
    int  banksize;                      //
    int  memsize;                       //

} TMYWIN;


/******************************************************************************
*                                                                             *
*   int GetShiftAdj(int mask, int *pWidth)                                    *
*                                                                             *
******************************************************************************/
int GetShiftAdj(int mask, int *pWidth)
{
    int count;
    int width = 0;

    // Find how many bits the mask is shifted by
    for(count=0; count<32; count++)
    {
        if( mask & 1 )
            break;
        mask >>= 1;
    }

    // Continue finding the width of the mask in bits
    while( mask & 1 )
    {
        width++;
        mask >>= 1;
    }

    // Store the adjustment value based on the width value
    *pWidth = 8 - width;

    // Return with the mask shift offset
    return(count);
}


/******************************************************************************
*                                                                             *
*   TXINITPACKET *CollectDisplayInfo(char *sDisplay)                          *
*                                                                             *
*******************************************************************************
*
*   Query given display and fill in IOCTL packet.
*
*   Where:
*       sDisplay is the string name of the display to query (or NULL for default)
*       fQuery if TRUE, will display some additional information
*
*   Returns:
*       Address of the IOCTL packet that is ready to be sent
*       NULL if error
*
******************************************************************************/
TXINITPACKET *CollectDisplayInfo(char *sDisplay, BOOL fQuery)
{
    static TXINITPACKET Init;           // Init packet sent to the debugger

    Display *display;                   // Display name to connect to
    XWindowAttributes windowattr;       // Window attributes structure
    int screen_num;                     // Screen number
    Screen *screen_ptr;                 // Pointer to screen structure
    XVisualInfo vinfo_template;
    XVisualInfo *pVisual;
    TMYWIN mywin;                       // Structure holding window information

    int event_base, error_base;
    long vinfo_mask;
    int nitems_return;

    unsigned int display_width, display_height, depth;


    display = XOpenDisplay(sDisplay);
    if( display )
    {
        // Get the screen number and pointers
        screen_num = DefaultScreen(display);
        screen_ptr = DefaultScreenOfDisplay(display);

        // Get the root window attributes
        if( XGetWindowAttributes(display, RootWindow(display, screen_num), &windowattr) )
        {
            display_width  = windowattr.width;
            display_height = windowattr.height;

            // Query the DGA extension - this is specific to XFree86
            if( XF86DGAQueryExtension(display, &event_base, &error_base) )
            {
                if( XF86DGADirectVideo(display, screen_num, XF86DGADirectGraphics) )
                {
                    if( XF86DGAGetVideo(display, screen_num, &mywin.ptr, &mywin.width, &mywin.banksize, &mywin.memsize) )
                    {
                        // Set the viewport to the (0,0)
                        XF86DGASetViewPort(display, screen_num, 0, 0);
                        depth = DefaultDepth(display, screen_num);

                        vinfo_mask = VisualScreenMask;
                        vinfo_template.screen = screen_num;
                        pVisual = XGetVisualInfo(display, vinfo_mask, &vinfo_template, &nitems_return);

                        if( pVisual )
                        {
                            // Display information that we gathered so far about the screen
                            printf("Root window:\n");
                            printf("   dimensions %d x %d x %d\n", display_width, display_height, depth );
                            printf("   address    %08X\n", mywin.ptr);

                            // TODO: I dont understand - bank size and mem size appear to be swapped... ?
                            printf("   memsize    %08X (%d Kb)\n", mywin.banksize, mywin.banksize/1024);
                            printf("   banksize   %08X (%d Kb)\n", mywin.memsize, mywin.memsize/1024);

                            switch( depth )
                            {
                                case 16:            // 16 bpp - 5-6-5 color components
                                    Init.redColAdj   = 3;
                                    Init.greenColAdj = 2;
                                    Init.blueColAdj  = 3;
                                break;

                                case 24:            // 24 bpp
                                    fprintf(stderr, "Pixel depth is 24 bpp.. We will assume 32 bpp !\n");
                                    depth=32;
                                    // ... continue into 32 bpp...

                                case 32:            // 32 bpp
                                    Init.redColAdj   = 0;
                                    Init.greenColAdj = 0;
                                    Init.blueColAdj  = 0;
                                break;

                                // Unsupported pixel format.. Can't use!
                                default:
                                    fprintf(stderr, "FAILED: Unsupported pixel depth of %d !\n", depth);

                                    return( NULL );
                                break;
                            }

                            // Fill up the Linice IOCTL info packet with the collected data
                            Init.pFrameBuf = (DWORD) mywin.ptr;
                            Init.dwDrawSize= dwBufSize * 1024;          // Buffer size actually in bytes
                            Init.xres      = display_width;
                            Init.yres      = display_height;
                            Init.bpp       = depth / 8;
                            Init.stride    = Init.bpp * mywin.width;
                            Init.redShift  = GetShiftAdj(pVisual->red_mask, &Init.redColAdj);
                            Init.greenShift= GetShiftAdj(pVisual->green_mask, &Init.greenColAdj);
                            Init.blueShift = GetShiftAdj(pVisual->blue_mask, &Init.blueColAdj);
                            Init.redMask   = pVisual->red_mask;
                            Init.greenMask = pVisual->green_mask;
                            Init.blueMask  = pVisual->blue_mask;

                            // Some extra output if we issued a query command parameter
                            if( fQuery )
                            {
                                printf("Query display:\n");
                                printf("   Init.pFrameBuf  = %08X\n", Init.pFrameBuf);
                                printf("   Init.dwDrawSize = %08X\n", Init.dwDrawSize);
                                printf("   Init.xres       = %08X\n", Init.xres);
                                printf("   Init.yres       = %08X\n", Init.yres);
                                printf("   Init.bpp        = %08X\n", Init.bpp);
                                printf("   Init.stride     = %08X\n", Init.stride);
                                printf("   Init.redShift   = %08X\n", Init.redShift);
                                printf("   Init.greenShift = %08X\n", Init.greenShift);
                                printf("   Init.blueShift  = %08X\n", Init.blueShift);
                                printf("   Init.redMask    = %08X\n", Init.redMask);
                                printf("   Init.greenMask  = %08X\n", Init.greenMask);
                                printf("   Init.blueMask   = %08X\n", Init.blueMask);
                            }

                            // Use a simple heuristic to prognose if we would be able to use this display:
                            // We assume that shift counts should be different and that
                            // RGB-color masks should not overlap
                            //
                            if( ((Init.redShift==Init.greenShift) && (Init.redShift==Init.blueShift))
                            ||  (Init.redMask & Init.greenMask & Init.blueMask) )
                            {
                                printf("It appears that your current X Window display device is not suitable to\n");
                                printf("be used by a Linice driver. It is either memory banked or indexed, not true\n");
                                printf("RGB-component, linearly mapped.\n");
                                printf("Try different graphics mode, resolution or card/driver.\n");
                            }

                            // Return the address of the init packet
                            return( &Init );
                        }
                        else
                        {
                            fprintf(stderr, "FAILED: Could not get visual info\n");
                        }
                    }
                    else
                    {
                        fprintf(stderr, "FAILED in XF86DGAGetVideo\n");
                    }
                }
                else
                {
                    fprintf(stderr, "FAILED in XF86DGADirectVideo\n");
                }
            }
            else
            {
                fprintf(stderr, "FAILED: Unable to query the DGA extension\n");
                fprintf(stderr, "Your display does not support DGA extensions!\n");
            }
        }
        else
        {
            fprintf(stderr, "FAILED: Unable to get root window attributes\n");
        }
    }
    else
    {
        fprintf(stderr, "FAILED: Cannot connect to server %s\n", XDisplayName(sDisplay));
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void Help()                                                               *
*                                                                             *
******************************************************************************/
void Help()
{
    printf("Usage: xice [-b buf_size] [-q] [-h] [server[:display]]\n\n");
    printf(" -b or --buffer  Set a backing store buffer size\n");
    printf(" -q or --query   Query the availability of a suitable DGA local display\n");
    printf(" -h or --help    Display help\n");
    printf(" server:display  Select non-default local server and display\n");
}

/******************************************************************************
*                                                                             *
*   int main(int argc, char **argp)                                           *
*                                                                             *
*******************************************************************************
*
*   Command line options:
*
*       xice [-b <buf_size Kb>] [-q] [-h] <server:display>
*
*           -b <buf_size Kb>   or --buffer <buf_size>
*               Allocate specified amount of memory for the screen buffer (in Kb)
*
*           -q  or  --query
*               Only query display and return 1 if present or 0 if not present
*
*           -h  or  --help
*               Display help and program usage
*
*           Connect to a specific X Windows server and display. Useful if you
*           have multiple displays and want to select on which one Linice will
*           pop up. The display still has to be local with DGA support.
*
*           If no display supplied, default local will be used.
*
*   Returns:
*       0  ok
*       1  if Query option and did find suitable display
*       -1 if Query option and did not find suitable display
*
******************************************************************************/
int main(int argc, char **argp)
{
    TXINITPACKET *pInit;
    char *sDisplay = NULL;              // Display string, init to default
    char *sBufSize = NULL;              // String specifying buffer size
    int hIce;
    int status, i;
    int opt = 0;                        // Detected command line options:
#define OPT_QUERY       0x0001          // Query active X Window display

    // Traverse command line options
    for(i=1; i<argc; i++ )
    {
        if( !strcmp(argp[i], "-b") || !strcmp(argp[i], "--buffer"))
        {
            sBufSize = argp[i+1];
            i++;
        }
        else
        if( !strcmp(argp[i], "-q") || !strcmp(argp[i], "--query"))
            opt |= OPT_QUERY;
        else
        if( !strcmp(argp[i], "-h") || !strcmp(argp[i], "--help"))
        {
            Help();                     // Display help and return
            return( 0 );
        }
        else
        {
            // It is a display specification
            sDisplay = argp[i];
            printf("Using display ""%s""\n", sDisplay);
        }
    }

    // Decode the buffer size string
    if( sBufSize )
    {
        if( sscanf(sBufSize, "%d", &dwBufSize)==1 )
        {
            printf("Using buffer size of %d Kb\n", dwBufSize);
        }
        else
        {
            fprintf(stderr, "Error reading buffer size parameter '%s'\n", sBufSize);
            exit(-1);
        }
    }

    pInit = CollectDisplayInfo(sDisplay, opt & OPT_QUERY );

    if( opt & OPT_QUERY )               // Only query display?
    {
        if( pInit )
        {
            printf("Found a suitable display.\n");
            return( 1 );
        }
        else
        {
            printf("Did not find any suitable displays!\n");
            return( -1 );
        }
    }

    if( pInit )
    {
        // Send the info packet down to the Linice module

        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            status = ioctl(hIce, ICE_IOCTL_XDGA, pInit);

            if( status==0 )
            {
                // Message was passed to linice. Since it will schedule a break, delay
                // little bit to give chance display to readjust and repaint
                sleep(1);

                printf("Information sent to linice..\n");
            }
            else
                fprintf(stderr, "Linice refused XInitPacket (status=%d)\n", status);

            close(hIce);
        }
        else
        {
            fprintf(stderr, "Error opening debugger device! Is Linice loaded?\n");
        }
    }

    return( 0 );
}

