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
*   BOOL cmdData(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   DATA        - opens next sequential data window, switch to the next data
*                 window if all are open
*   DATA [n]    - opens (if it is not open) and switch to that data window
*
******************************************************************************/
BOOL cmdData(char *args, int subClass)
{
    UINT window;                        // Optional data window number

    if( *args )
    {
        if( GetDecB(&window, &args) && !*args )
        {
            if( window < MAX_DATA )
            {
                deb.nData = window;

                pWin->data[deb.nData].fVisible = TRUE;
            }
            else
                PostError(ERR_DATAWIN, 0);
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - open and switch to the next sequential data window

        if( pWin->data[deb.nData].fVisible==TRUE )
        {
            // Open the next one and switch to it

            deb.nData = (deb.nData + 1) % MAX_DATA;
        }

        pWin->data[deb.nData].fVisible = TRUE;
    }

    deb.fRedraw = TRUE;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWd(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WD          - toggle current data window on/off
*   WD [n]      - sets the data window size (and opens it)
*
*   WD.#        - toggle the state of the data window number #
*   WD.# [n]    - sets the data window number # size (and opens it)
*
******************************************************************************/
BOOL cmdWd(char *args, int subClass)
{
    UINT window;                        // Optional data window number
    int value;

    // Get the optional data window number, otherwise use the current window
    window = deb.nData;

    if( *args && *args=='.' )           // Syntax is WD.#
    {
        args++;                         // Advance to the data window number
        if( GetDecB(&window, &args) )
        {
            if( window < MAX_DATA )
            {
                deb.nData = window;

                while( *args==' ' ) *args++;    // Skip optional spaces
            }
            else
            {
                PostError(ERR_DATAWIN, 0);
                return( TRUE );
            }
        }
        else
        {
            PostError(ERR_SYNTAX, 0);
            return( TRUE );
        }
    }

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        if( GetDecB(&value, &args) && !*args )
        {
            // If the number of lines was 0, close window
            if( value==0 )
                pWin->data[deb.nData].fVisible = FALSE;
            else
            {
                pWin->data[deb.nData].fVisible = TRUE;
                pWin->data[deb.nData].nLines = value + 1;
            }
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->data[deb.nData].fVisible = !pWin->data[deb.nData].fVisible;
    }

    // If the current window was just closed, move the 'current' to the previous one opened
    if( !pWin->data[deb.nData].fVisible )
    {
        for(window=PREV(deb.nData, MAX_DATA); window!=deb.nData; window = PREV(window, MAX_DATA))
        {
            if( pWin->data[window].fVisible==TRUE )
                break;
        }

        // If we did not find any open window, set up the data window number 0
        if( window==deb.nData )
            deb.nData = 0;
        else
            deb.nData = window;
    }

    deb.fRedraw = TRUE;

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
        if( GetDecB(&value, &args) && !*args )
        {
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
            PostError(ERR_SYNTAX, 0);
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
    deb.fRedraw = TRUE;

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
        if( GetDecB(&value, &args) && !*args )
        {
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
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->l.fVisible = !pWin->l.fVisible;
    }
    deb.fRedraw = TRUE;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdWs(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   WS          - toggle stack window on/off
*   WS [n]      - sets the stack window size (and opens it)
*
******************************************************************************/
BOOL cmdWs(char *args, int subClass)
{
    int value;

    if( *args )
    {
        // Argument is present - get the number of lines and activate window
        if( GetDecB(&value, &args) && !*args )
        {
            // If the number of lines was 0, close window
            if( value==0 )
                pWin->s.fVisible = FALSE;
            else
            {
                pWin->s.fVisible = TRUE;
                pWin->s.nLines = value + 1;
            }
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->s.fVisible = !pWin->s.fVisible;
    }
    deb.fRedraw = TRUE;

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
        if( GetDecB(&value, &args) && !*args )
        {
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
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - toggle open/close window

        pWin->w.fVisible = !pWin->w.fVisible;
    }
    deb.fRedraw = TRUE;

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

    deb.fRedraw = TRUE;

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

    deb.fRedraw = TRUE;

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

    deb.fRedraw = TRUE;

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

