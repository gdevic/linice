/******************************************************************************
*                                                                             *
*   Module:     windowcontrol.c                                               *
*                                                                             *
*   Date:       10/18/00                                                      *
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

        Window control commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/15/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

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

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdWd(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WD          - toggle data window on/off
*   WD [n]      - sets the data window size (and opens it)
*
******************************************************************************/
BOOL cmdWd(char *args, int subClass)
{
    int value;

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        value = GetDec(&args);

        // If the number of lines was 0, close window
        if( value==0 )
            pWin->d.fVisible = FALSE;
        else
        {
            pWin->d.fVisible = TRUE;
            pWin->d.nLines = value + 1;
        }
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->d.fVisible = !pWin->d.fVisible;
    }

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWc(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WC          - toggle code window on/off
*   WC [n]      - sets the code window size (and opens it)
*
******************************************************************************/
BOOL cmdWc(char *args, int subClass)
{
    int value;

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        value = GetDec(&args);

        // If the number of lines was 0, close window
        if( value==0 )
            pWin->c.fVisible = FALSE;
        else
        {
            pWin->c.fVisible = TRUE;
            pWin->c.nLines = value + 1;
        }
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->c.fVisible = !pWin->c.fVisible;
    }

    // If we were in the code edit mode, adjust it accordingly
    if( deb.fCodeEdit )
    {
        // If the code window is now closed, deactivate code edit mode
        if( pWin->c.fVisible==FALSE )
        {
            deb.fCodeEdit = FALSE;
        }
        else
        {
            // The code window is still open, but we may want to check the Y size
            if( deb.nCodeEditY > pWin->c.nLines-2 )
                deb.nCodeEditY = pWin->c.nLines-2;
        }
    }

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWl(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WL          - toggle locals window on/off
*   WL [n]      - sets the locals window size (and opens it)
*
******************************************************************************/
BOOL cmdWl(char *args, int subClass)
{
    int value;

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        value = GetDec(&args);

        // If the number of lines was 0, close window
        if( value==0 )
            pWin->l.fVisible = FALSE;
        else
        {
            pWin->l.fVisible = TRUE;
            pWin->l.nLines = value + 1;
        }
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->l.fVisible = !pWin->l.fVisible;
    }

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWw(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WW          - toggle watch window on/off
*   WW [n]      - sets the watch window size (and opens it)
*
******************************************************************************/
BOOL cmdWw(char *args, int subClass)
{
    int value;

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        value = GetDec(&args);

        // If the number of lines was 0, close window
        if( value==0 )
            pWin->w.fVisible = FALSE;
        else
        {
            pWin->w.fVisible = TRUE;
            pWin->w.nLines = value + 1;
        }
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->w.fVisible = !pWin->w.fVisible;
    }

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWr(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WR          - toggle register window on/off
*
******************************************************************************/
BOOL cmdWr(char *args, int subClass)
{
    pWin->r.fVisible = !pWin->r.fVisible;

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdCls(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Clears command window
*
******************************************************************************/
BOOL cmdCls(char *args, int subClass)
{
    ClearHistory();

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdRs(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   Restores client window, waits for a keypress and then pops up debugger back
*
*   The keypress wait that is used is not polling since we dont want to display
*   cursor carret on the screen.
*
******************************************************************************/
BOOL cmdRs(char *args, int subClass)
{
    // Restore client window
    dputc(DP_RESTOREBACKGROUND);

    // Wait for a keypress
    while( GetKey( FALSE )==0 );

    // Back to debugger
    dputc(DP_SAVEBACKGROUND);

    // Repaint all windows
    RecalculateDrawWindows();

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdFlash(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Restores client window during P and T commands
*
******************************************************************************/
BOOL cmdFlash(char *args, int subClass)
{
    switch( GetOnOff(args) )
    {
        case 1:         // On
            deb.fFlash = TRUE;
        break;

        case 2:         // Off
            deb.fFlash = FALSE;
        break;

        case 3:         // Display the state of the FLASH variable
            dprinth(1, "Flash is %s", deb.fFlash? "on":"off");
        break;
    }

    return( TRUE );
}

