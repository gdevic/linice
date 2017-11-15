/******************************************************************************
*                                                                             *
*   Module:     window.c                                                      *
*                                                                             *
*   Date:       09/11/00                                                      *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

        This module contains functions for windowing.  All command
        windows are abstracted here.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 09/11/00         Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

static int avail;                       // Counting lines: available left
static int total;                       // Counting lines: total count

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static void AdjustTopBottom(PTFRAME pFrame)
{
    if( pFrame->fVisible )
    {
        pFrame->Top = total;
        pFrame->Bottom = pFrame->Top + pFrame->nLines - 1;
        total += pFrame->nLines;
        avail -= pFrame->nLines;
    }
}

static void AdjustToFit(int excess)
{
    int newSize;
    int window;                         // Data window number
    int active = 0;

    // Count how many windows are visible, so we can decide on the size(s)
    if( pWin->r.fVisible )  active++;
    if( pWin->l.fVisible )  active++;
    if( pWin->w.fVisible )  active++;
    if( pWin->s.fVisible )  active++;
    for(window=0; window<MAX_DATA; window++)
        if( pWin->data[window].fVisible )active++;
    if( pWin->c.fVisible )  active++;

    newSize = (pOut->sizeY - 5) / (active + 1);

    // Evenly distribute windows across the sceeen
    if( pWin->l.fVisible )  pWin->l.nLines = newSize;
    if( pWin->w.fVisible )  pWin->w.nLines = newSize;
    if( pWin->s.fVisible )  pWin->s.nLines = newSize;
    for(window=0; window<MAX_DATA; window++)
        if( pWin->data[window].fVisible )
            pWin->data[window].nLines = newSize;
    if( pWin->c.fVisible )  pWin->c.nLines = newSize;

    pWin->h.nLines = 0;         // This one will be adjusted later
}

/******************************************************************************
*                                                                             *
*   int WindowIsSizeValid()                                                   *
*                                                                             *
*******************************************************************************
*
*   Calculates if the current window size assignment is valid for the current
*   window height. It ignores the size of the history frame as this one can
*   be adjusted based on the findings of this function.
*
*   Returns:
*       >=0 if all windows can fit on the screen
*       < 0 with the number of lines not fitting current window height
*
******************************************************************************/
int WindowIsSizeValid()
{
    int window;                         // Data window number

    // Add 4 lines for help, history (2) and history header line
    int less = pOut->sizeY - 4;

    if( pWin->r.fVisible )  less -= pWin->r.nLines;
    if( pWin->l.fVisible )  less -= pWin->l.nLines;
    if( pWin->w.fVisible )  less -= pWin->w.nLines;
    if( pWin->s.fVisible )  less -= pWin->s.nLines;
    for(window=0; window<MAX_DATA; window++)
        if( pWin->data[window].fVisible )
            less -= pWin->data[window].nLines;
    if( pWin->c.fVisible )  less -= pWin->c.nLines;

    return( less );
}

/******************************************************************************
*                                                                             *
*   void RecalculateDrawWindows()                                             *
*                                                                             *
*******************************************************************************
*
*   Recalculates windowing parameters and redraws the complete screen.
*   If the initial window sizes are not valid, it scales them down so the
*   window configuration can fit on within the current screen size.
*
******************************************************************************/
void RecalculateDrawWindows()
{
    int excess;                         // Available number of lines
    int window;                         // Data window number
    UINT current;                       // Current data window store

    avail = pOut->sizeY;
    total = 0;

    if( (excess = WindowIsSizeValid()) < 0 )
        AdjustToFit( -excess );

    AdjustTopBottom(&pWin->r);
    AdjustTopBottom(&pWin->l);
    AdjustTopBottom(&pWin->w);
    AdjustTopBottom(&pWin->s);
    for(window=0; window<MAX_DATA; window++)
        AdjustTopBottom(&pWin->data[window]);
    AdjustTopBottom(&pWin->c);

    // The last one is the history window, and we will let it fill in the rest
    pWin->h.nLines = avail - 1;         // Save one extra for the help line
    AdjustTopBottom(&pWin->h);

    // This prevent from us from calling all these drawing functions when the
    // debugger is not active, in which case we would not have the right memory
    // access set up
    if( deb.fRunningIce==TRUE )
    {
        // Draw the screen
        dputc(DP_CLS);

        // Set the new history frame scroll region
        // Add one to the top Y to skip the header line
        dprint("%c%c%c", DP_SETSCROLLREGIONYY, pWin->h.Top+1+1, pWin->h.Bottom+1);

        RegDraw(FALSE);
        LocalsDraw(FALSE);
        WatchDraw(FALSE);
        StackDraw(FALSE);

        current = deb.nData;
        for(deb.nData=0; deb.nData<MAX_DATA; deb.nData++)
            DataDraw(FALSE, deb.dataAddr[deb.nData].offset, (deb.nData==current));
        deb.nData = current;

        CodeDraw(FALSE);
        HistoryDraw();

        // HistoryDraw leaves the cursor coordinates at the proper Y-coordinate
    }
}

