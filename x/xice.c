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

#include <stdio.h>

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

Display *display;
int screen_num;
XWindowAttributes windowattr;

static char *progname;

typedef struct
{
    char *ptr;
    int  width;
    int  banksize;
    int  memsize;

} TMYWIN, *PTMYWIN;


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

#define RAISE_INTERRUPT(_x)  __asm__ __volatile__("int %0" :: "g" (_x))

/******************************************************************************
*                                                                             *

*                                                                             *
*******************************************************************************
*
*
******************************************************************************/
int main(int argc, char **argv)
{
    Window win;
    unsigned int width, height;         // Window size
    int x = 0, y = 0;                   // Window position
    unsigned int border_width = 4;      // Border size
    unsigned int display_width, display_height;
    char *window_name = "pIce window";
    char *icon_name = "basicwin";
    Pixmap icon_pixmap;
    XSizeHints size_hints;
    XEvent report;
    GC gc;
    XFontStruct *font_info;
    char *display_name = NULL;          // Server to connect to
    Screen *screen_ptr;
    int event_base, error_base;
    int flags;
    TMYWIN mywin;
    int depth;
    XVisualInfo *pVisual;
    long vinfo_mask;
    XVisualInfo vinfo_template;
    int nitems_return;
    volatile int i, j;
    int redShift, greenShift, blueShift;

    int hIce;
    int status;
    TXINITPACKET Init;                  // Init packet to the debugger

    // Connect to the server

    progname = argv[0];
    if( (display=XOpenDisplay(display_name))==NULL )
    {
        fprintf(stderr, "%s cannot connect to X server %s\n",
            progname, XDisplayName(display_name));
        exit(-1);
    }

    screen_num = DefaultScreen(display);
    screen_ptr = DefaultScreenOfDisplay(display);

    // Get the root window attributes

    if( XGetWindowAttributes(display, RootWindow(display, screen_num), &windowattr)==0 )
    {
        fprintf(stderr, "%s failed to get root window attributes.\n", progname);
        exit(-1);
    }

    display_width  = windowattr.width;
    display_height = windowattr.height;

    // Query the DGA extension - this is specific to XFree86

    if( XF86DGAQueryExtension(display, &event_base, &error_base)==0 )
    {
        fprintf(stderr, "%s failed to query the DGA extension.\n", progname);
        fprintf(stderr, "Your display does not support DGA extensions.\n");
        exit(-1);
    }

    if( XF86DGADirectVideo(display, screen_num, XF86DGADirectGraphics)==0 )
    {
        fprintf(stderr, "Failed in XF86DGADirectVideo\n");
        exit(-1);
    }

    if( XF86DGAGetVideo(display, screen_num, &mywin.ptr, &mywin.width, &mywin.banksize, &mywin.memsize)==0 )
    {
        fprintf(stderr, "Failed in XF86DGAGetVideo\n");
        exit(-1);
    }

    fprintf(stderr, "Root window: address %08X\n", mywin.ptr);
    fprintf(stderr, "             width %d\n", mywin.width);
    fprintf(stderr, "             banksize %X (%d Kb)\n", mywin.banksize, mywin.banksize/1024);
    fprintf(stderr, "             memsize %08X (%d Kb)\n", mywin.memsize, mywin.memsize/1024);

    XF86DGASetViewPort(display, screen_num, 0, 0);

    depth = DefaultDepth(display, screen_num);
    fprintf(stderr, "DefaultDepth is %d\n", depth);

    vinfo_mask = VisualScreenMask;
    vinfo_template.screen = screen_num;
    pVisual = XGetVisualInfo(display, vinfo_mask, &vinfo_template, &nitems_return);

    if( pVisual )
    {
        fprintf(stderr, "Visual: depth = %d\n", pVisual->depth);
        fprintf(stderr, "Visual: red_mask = %08X\n", pVisual->red_mask);
        fprintf(stderr, "Visual: green_mask = %08X\n", pVisual->green_mask);
        fprintf(stderr, "Visual: blue_mask = %08X\n", pVisual->blue_mask);
        fprintf(stderr, "Visual: bits_per_rgb = %d\n", pVisual->bits_per_rgb);

        XFree(pVisual);
    }
    else
    {
        fprintf(stderr, "Could not get visual info.\n");
        exit(-1);
    }

    fprintf(stderr, "Root window is %d x %d\n", display_width, display_height );

    // If the pixel depth is 24, assume that is 32
    if( depth==24 )
        depth=32;

    // Send a IOCTL packet to a linice to start using the framebuffer
    //====================================================================
    // Send the init packet down to the module
    //====================================================================
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

    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        status = ioctl(hIce, ICE_IOCTL_XDGA, &Init);
        close(hIce);

        printf("IOCTL=%d\n", status);
    }
    else
    {
        printf("Error opening debugger device!\n");
        exit(-1);
    }

    return(0);
}

