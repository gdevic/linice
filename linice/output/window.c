/******************************************************************************
*                                                                             *
*   Module:     window.c                                                      *
*                                                                             *
*   Date:       03/11/01                                                      *
*                                                                             *
*   Copyright (c) 1997, 2001 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 03/11/01         Original                                       Goran Devic *
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
    int active = 0;

    if( pWin->r.fVisible )  active++;
    if( pWin->d.fVisible )  active++;
    if( pWin->c.fVisible )  active++;

    newSize = (pOut->sizeY - 5) / (active + 1);

    if( pWin->d.fVisible )  pWin->d.nLines = newSize;
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
    int less = pOut->sizeY - 3;         // Add 3 lines for history and help

    if( pWin->r.fVisible )  less -= pWin->r.nLines;
    if( pWin->d.fVisible )  less -= pWin->d.nLines;
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
*   window configuration can fit on the current screen.
*
******************************************************************************/
void RecalculateDrawWindows()
{
    int excess;

    avail = pOut->sizeY;
    total = 0;

    if( (excess = WindowIsSizeValid()) < 0 )
        AdjustToFit( -excess );

    AdjustTopBottom(&pWin->r);
    AdjustTopBottom(&pWin->d);
    AdjustTopBottom(&pWin->c);

    // The last one is the history window, and we will let it fill in the rest
    pWin->h.nLines = avail - 1;         // Save one extra for the help line
    AdjustTopBottom(&pWin->h);

    // Draw the screen only if linice display enabled
    if( pWin->fEnable )
    {
        // Draw the screen
        dputc(DP_CLS);

        // Set the new history frame scroll region
        // Add one to the top Y to skip the header line
        dprint("%c%c%c", DP_SETSCROLLREGIONYY, pWin->h.Top+1+1, pWin->h.Bottom+1);

        if( pWin->r.fVisible )  (pWin->r.draw)();
        if( pWin->d.fVisible )  (pWin->d.draw)();
        if( pWin->c.fVisible )  (pWin->c.draw)();
        (pWin->h.draw)();
    }
}
