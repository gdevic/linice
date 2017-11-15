/******************************************************************************
*                                                                             *
*   Module:     watch.c                                                       *
*                                                                             *
*   Date:       09/11/2002                                                    *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
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

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL Evaluate(TExItem *pItem, char *pExpr, char **ppNext, BOOL fSymbolsValueOf);
extern void PrettyPrintVariableName(char *pString, char *pName, TSYMTYPEDEF1 *pType1);
extern void MakeSelectedVisible(TLIST *pList, int nLines);
extern BOOL ExItemCompare(TExItem *pItem1, TExItem *pItem2);


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdWatch(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Adds a new expression / variable watch to the list of watches
*
******************************************************************************/
BOOL cmdWatch(char *args, int subClass)
{
    TLISTITEM *pItem;                   // Item to add to watch list
    TExItem ExItem;                     // Expression item evaluated


    // Evaluate a new watch expression and get its expression node result
    if( Evaluate(&ExItem, args, NULL, TRUE) )
    {
        // Check that the given watch item is not a duplicate
        if( !ListFindItem(&deb.Watch, &ExItem) )
        {
            // Add a new item to the end of the list
            // New item will also be marked as selected
            if((pItem = ListAdd(&deb.Watch)))
            {
                // Copy the expression node
                memcpy(&pItem->Item, &ExItem, sizeof(TExItem));

                // We can delete root watch variables
                pItem->fCanDelete = TRUE;

                // Copy the expression string into the list item
                strcpy(pItem->Name, args);

                PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);

                // When we are adding a watch expression, make sure it is visible
                MakeSelectedVisible(&deb.Watch, pWin->w.nLines );

                // Successfully added a watch item, redraw the window
                WatchDraw(TRUE);
            }
            else
                dprinth(1, "Unable to add a watch.");
        }
        else
            dprinth(1, "Duplicate watch expression!");
    }

    return( TRUE );
}


void WatchDraw(BOOL fForce)
{
    ListDraw(&deb.Watch, &pWin->w, fForce);
}

/******************************************************************************
*                                                                             *
*   void RecalculateWatch()                                                   *
*                                                                             *
*******************************************************************************
*
*   Recalculates all watch expressions. This is called after a context change
*   since the watches may go in and out of context, so here we re-evaluate
*   each of them and set up new result string. More detailed behavior is
*   explained in the code.
*
*   Watch needs to be redrawn after this change - here we only update internal
*   lists and buffers.
*
******************************************************************************/
void RecalculateWatch()
{
    TLISTITEM *pItem = NULL;            // Current item
    TExItem ExItem;                     // Expression item to be evaluated

    // Walk the list of root items and check that the expression is still valid
    while( (pItem = ListGetNext(&deb.Watch, pItem)) )
    {
        // Check the expression
        if( Evaluate(&ExItem, pItem->Name, NULL, TRUE) )
        {
            // Expression evaluated correctly, but we dont know if it is the same variable
            // so compare the ExItem data structures
            if( ExItemCompare(&ExItem, &pItem->Item) )
            {
                // Values are identical, no need to do anything here
                ;
            }
            else
            {
                // Values are not identical - it is a different variable

                // Assign the new item data descriptor
                memcpy(&pItem->Item, &ExItem, sizeof(TExItem));

                // Need to collapse item and reassign a new value
                ListDel(&deb.Watch, pItem, FALSE);

                PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);
            }
        }
        else
        {
            // Watch item did not evaluate correctly, so the expression is not
            // valid in this context. Collapse it and mark it invalid
            ListDel(&deb.Watch, pItem, FALSE);

            // Mark the type invalid
            pItem->Item.bType = 0;

            memset(&pItem->Item, 0, sizeof(TExItem));
        }
    }

    // Clear any logged error during this operation
    deb.errorCode = NOERROR;
}
