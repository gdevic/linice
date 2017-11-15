/* Xlib include files */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <X11/extensions/xf86dga.h>

#include <stdio.h>

Display *display;
int screen_num;
XWindowAttributes windowattr;

static char *progname;

typedef struct
{
    char **ptr;
    int  width;
    int  banksize;
    int  memsize;
    
} TMYWIN, *PTMYWIN;

void main(int argc, char **argv)
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
    int *nitems_return;
    volatile int i, j;

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
        fprintf(stderr, "Your display does not support DGA extension.\n");
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

    if( XF86DGADirectVideo(display, screen_num, XF86DGADirectGraphics)==0 )
    {
        fprintf(stderr, "Failed in XF86DGADirectVideo\n");
        exit(-1);
    }

    for( i=10; i<50; i++)
    {
        long *p = mywin.ptr;

        for( j=64; j<128; j++)
        {
            *(p + i * 1024 + j) = 0x00FF0000;
        }
    }

    for( i=0; i<10000; i++)
    {
        for( j=0; j<1000; j++ )
        {
            ;
        }
    }

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
        fprintf(stderr, "Could not get visual info.\n");

    fprintf(stderr, "Root window is %d x %d\n", display_width, display_height );
    exit(-1);
}    
