/******************************************************************************
*                                                                             *
*   Module:     watch.c                                                       *
*                                                                             *
*   Date:       09/11/2002                                                    *
*                                                                             *
*   Copyright (c) 2000 - 2002 Goran Devic                                     *
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

        This module contans code to display and manage watch window and
        associated operations.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
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

#define MAX_LOCALS_QUEUE    32          // Max number of local variables to display

static TSYMFNSCOPE1 *LocalsQueue[MAX_LOCALS_QUEUE + 1];

static char buf[MAX_STRING];


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL SymEvalFnScope1(char *pBuf, TSYMFNSCOPE1 *Local);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL FillLocalScopeX(TSYMFNSCOPE1 *LocalsQueue[], TSYMFNSCOPE *pFnScope, DWORD dwOffset);


/******************************************************************************
*                                                                             *
*   BOOL cmdWatch(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Adds a new expression / variable watch to the list of watches
*
******************************************************************************/

extern TITEM *ListAdd(TQueue *pQueue);
extern void ListDelCur(TQueue *pQueue);
extern void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce);


BOOL cmdWatch(char *args, int subClass)
{
    TITEM *pItem;                       // Item to add to watch list

    // Add a watch expression to the root watch list

    if(pItem = ListAdd(&deb.Watch.Item))
    {
        // Fill in the item values

        pItem->pExp = strdup(args);

        if( pItem->pExp )
        {
            // Successfully added a watch item

            // Draw the watch
            ListDraw(&deb.Watch, &pWin->w, TRUE);
            
            return( TRUE );
        }

        // Failure, delete the item
        ListDelCur(&deb.Watch.Item);
    }

    dprinth(1, "Unable to add a watch.");

    return( TRUE );
}


void WatchDraw(BOOL fForce)
{
    ListDraw(&deb.Watch, &pWin->w, fForce);
}


/******************************************************************************
*                                                                             *
*   BOOL FillLocalScope(TSYMFNSCOPE1 *LocalsQueue[], TSYMFNSCOPE *pFnScope, DWORD dwOffset)
*                                                                             *
*******************************************************************************
*
*   Fills in the array of pointers to local variables at the given context
*   of pFnScope and the eip address.
*
*   Where:
*       LocalsQueue[] is the array of locals to be filled in (up to a max
*           number of elements MAX_LOCALS_QUEUE, NULL-terminated.
*       pFnScope is the function scope to search
*       dwOffset is the offset address within that function scope
*   Returns:
*       TRUE array filled up
*       FALSE LocalsQueue is NULL
*
******************************************************************************/
BOOL FillLocalScopeX(TSYMFNSCOPE1 *LocalsQueue[], TSYMFNSCOPE *pFnScope, DWORD dwOffset)
{
    DWORD dwFnOffset;
    int i, position, index, xNested;

    if( LocalsQueue )
    {
        index = 0;                      // Index into the writing locals queue
        
        // Make sure our code offset is within this function scope descriptor
        if( pFnScope && pFnScope->dwStartAddress<=dwOffset && pFnScope->dwEndAddress>=dwOffset )
        {
            // Scan forward for the right scope brackets depending on the code offset
            dwFnOffset = dwOffset - pFnScope->dwStartAddress;
            position = -1;                  // -1 means we did not find any locals or out of scope

            for(i=0; i<pFnScope->nTokens; i++ )
            {
                if( pFnScope->list[i].TokType==TOKTYPE_LBRAC || pFnScope->list[i].TokType==TOKTYPE_RBRAC )
                {
                    if( dwFnOffset >= pFnScope->list[i].p1 )
                        position = i;
                    else
                    if( dwFnOffset < pFnScope->list[i].p1 )
                        break;
                }
            }

            i = position;               // We will use 'i' from now on...

            // Fill in the queue by traversing back the function scope list
            if( i >= 0 )
            {
                index = 0;              // Index into the locals queue
                xNested = 1;            // Make the current nesting level normal

                while( index<MAX_LOCALS_QUEUE && i>=0 )
                {
                    if( pFnScope->list[i].TokType==TOKTYPE_RBRAC )
                        xNested++;      // Increase nesting level - hide scope variables of that level
                    else
                    if( pFnScope->list[i].TokType==TOKTYPE_LBRAC )
                    {
                        if( xNested>0 ) // Decrease nesting level only if we have a extra scope to avoid
                            xNested--;
                    }
                    else
                    {
                        if( xNested==0 )
                        {
                            // Store a local variable that is now visible
                            LocalsQueue[index] = &pFnScope->list[i];
                            index++;            // Increment the index into the locals queue
                        }
                    }
                    i--;                // Decrement the index within the function scope list
                }
            }
        }
        LocalsQueue[index] = NULL;      // Always terminate the queue

        return( TRUE );
    }

    return( FALSE );
}

