/******************************************************************************
*                                                                             *
*   Module:     xice.c                                                        *
*                                                                             *
*   Date:       05/20/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include io control codes
#include "loader.h"                     // Include loader global protos

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
*
*   Returns:
*       Address of the IOCTL packet that is ready to be sent
*       NULL if error
*
******************************************************************************/
TXINITPACKET *CollectDisplayInfo(char *sDisplay)
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
                        XF86DGASetViewPort(display, screen_num, 0, 0);
                        depth = DefaultDepth(display, screen_num);

                        vinfo_mask = VisualScreenMask;
                        vinfo_template.screen = screen_num;
                        pVisual = XGetVisualInfo(display, vinfo_mask, &vinfo_template, &nitems_return);

                        if( pVisual )
                        {
                            // Display information that we gathered so far about the screen
                            fprintf(stderr, "Root window: %d x %d x %d\n", display_width, display_height, depth );
                            fprintf(stderr, "             address %08X\n", mywin.ptr);
                            fprintf(stderr, "             banksize %X (%d Kb)\n", mywin.banksize, mywin.banksize/1024);
                            fprintf(stderr, "             memsize %08X (%d Kb)\n", mywin.memsize, mywin.memsize/1024);

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


void WriteConfig(TXINITPACKET *pInit, char *sFile)
{
    FILE *fp;

    // Open a configuration file
    fp = fopen(sFile, "w+b");
    if( fp )
    {
        fprintf(fp, "pFrameBuf   = %X\n", pInit->pFrameBuf   );
        fprintf(fp, "xres        = %X\n", pInit->xres        );
        fprintf(fp, "yres        = %X\n", pInit->yres        );
        fprintf(fp, "bpp         = %X\n", pInit->bpp         );
        fprintf(fp, "stride      = %X\n", pInit->stride      );
        fprintf(fp, "redShift    = %X\n", pInit->redShift    );
        fprintf(fp, "greenShift  = %X\n", pInit->greenShift  );
        fprintf(fp, "blueShift   = %X\n", pInit->blueShift   );
        fprintf(fp, "redMask     = %X\n", pInit->redMask     );
        fprintf(fp, "greenMask   = %X\n", pInit->greenMask   );
        fprintf(fp, "blueMask    = %X\n", pInit->blueMask    );
        fprintf(fp, "redColAdj   = %X\n", pInit->redColAdj   );
        fprintf(fp, "greenColAdj = %X\n", pInit->greenColAdj );
        fprintf(fp, "blueColAdj  = %X\n", pInit->blueColAdj  );
        fprintf(fp, "*** Do not modify this file. It has been generated by xice ***\n");

        fclose(fp);
    }
    else
    {
        printf("(Error writing configuration file %s)\n", sFile);
    }
}


/******************************************************************************
*                                                                             *
*   void Help()                                                               *
*                                                                             *
******************************************************************************/
void Help()
{
    printf("Usage: xice [-q] [-h] [server[:display]]\n\n");
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
*       xice [-q] [-h] <server:display>
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
*       Return value depends on a function.
*
******************************************************************************/
int main(int argc, char **argp)
{
    TXINITPACKET *pInit;
    char *sDisplay = NULL;              // Display string, init to default
    int hIce;
    int status, i;
    int opt = 0;                        // Detected command line options:
#define OPT_QUERY       0x0001          // Query active X Window display
#define OPT_HELP        0x0002          // Display help information

    // Traverse command line options
    for(i=1; i<argc; i++ )
    {
        if( !strcmp(argp[i], "-q") || !strcmp(argp[i], "--query"))
            opt |= OPT_QUERY;
        else
        if( !strcmp(argp[i], "-h") || !strcmp(argp[i], "--help"))
            opt |= OPT_HELP;
        else
        {
            // It is a display specification
            sDisplay = argp[i];
            printf("Using display ""%s""\n", sDisplay);
        }
    }

    if( opt & OPT_HELP )                // Help option?
    {
        Help();
    }

    if( opt & OPT_QUERY )               // Only query display?
    {
        pInit = CollectDisplayInfo(sDisplay);

        if( pInit )
        {
            printf("Found a suitable display.\n");

            // At this point we dump relevant information about the display
            // to a configuration file, that is read by the linice
            WriteConfig(pInit, XICE_CONFIG_FILE);

            return( 1 );
        }
        else
        {
            printf("Did not find any suitable displays!\n");
            return( 0 );
        }
    }
    
    pInit = CollectDisplayInfo(sDisplay);
    if( pInit )
    {
        // Send the info packet down to the Linice module

        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            fprintf(stderr, "IOCTL");
            status = ioctl(hIce, ICE_IOCTL_XDGA, pInit);
            fprintf(stderr, " status=%d\n", status);
            close(hIce);
        }
        else
        {
            fprintf(stderr, "Error opening debugger device!\n");
            fprintf(stderr, "Is Linice loaded?\n");
        }
    }

    return( 0 );
}

