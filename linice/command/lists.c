/******************************************************************************
*                                                                             *
*   Module:     lists.c                                                       *
*                                                                             *
*   Date:       02/03/2003                                                    *
*                                                                             *
*   Copyright (c) 2000 - 2003 Goran Devic                                     *
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

        This module contains code for variable lists, that is, a set
        of expressions or variables. It supports functionality of the
        stack window, locals window and watch window

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 02/03/03   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "lists.h"                      // Include lists support header file

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

static char buf[MAX_STRING];

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

TITEM *ListAdd(TQueue *pQueue)
{
    TITEM *pItem;                       // New item

    // Allocate memory for the new item
    pItem = (TITEM *)_kMallocHeap(sizeof(TITEM));

    if( pItem )
    {
        // Zero it out so to set initial states
        memset(pItem, 0, sizeof(TITEM));

        // Add new item into the queue
        if( QAdd(pIce->hHeap, pQueue, pItem) != 0 )
        {
            return( pItem );
        }

        // Operation failed.. Free what was allocated.
        _kFreeHeap(pItem);
    }

    return( NULL );
}


void ListDelCur(TQueue *pQueue)
{
    TITEM *pItem;                       // Current item to dispose of

    // Delete the item and dispose the queue node

    pItem = QCurrent(pQueue);

    if( pItem )
    {
        // Free all allocations of an item:
        // (our "free" functions can handle NULL pointer by ignoring :)

        _kFreeHeap(pItem->pExp);
        _kFreeHeap(pItem->pType);

        // If a sublist contains any item, recursively free them
        while( QIsEmpty(&pItem->SubList) )
        {
            ListDelCur(&pItem->SubList);
        }

        _kFreeHeap(pItem);
        
        QDelete(pIce->hHeap, pQueue);
    }
}


/******************************************************************************
*                                                                             *
*   void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce)                  *
*                                                                             *
*******************************************************************************
*
*   Draws list variables. If a cursor selects an item, that line will be drawn
*   inverted (selected).
*
******************************************************************************/
void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce)
{
    TITEM *pItem;
    int maxLines = 9999;
    int nLine = 1;
    int i;
    char col;                           // Current line color

    if( pFrame->fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pFrame->Top+1);
        PrintLine(" Watch");

        maxLines = pFrame->nLines;
    }
    else
        if( fForce==FALSE )
            return;

    // List all items of a specific list

    pItem = QFirst(&pList->Item);

    while( pItem && nLine<maxLines )
    {
        // If the current item is the one selected, invert the line color
        if( pItem==pList->pSelected )
            col = COL_REVERSE;
        else
            col = COL_NORMAL;

        sprintf(buf, "%s", pItem->pExp);

        if( dprinth(nLine++, "%c%c%s", DP_SETCOLINDEX, col, buf)==FALSE )
            break;

        pItem = QNext(&pList->Item);
    }

    if( pFrame->fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}

