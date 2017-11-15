/******************************************************************************
*                                                                             *
*   Module:     watch.c                                                       *
*                                                                             *
*   Date:       09/11/2002                                                    *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame);
extern void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce);
extern BOOL Evaluate(TExItem *pItem, char *pExpr, char **ppNext, BOOL fSymbolsValueOf);
extern void PrettyPrintVariableName(char *pString, char *pName, TSYMTYPEDEF1 *pType1);
extern void MakeSelectedVisible(TLIST *pList, int nLines);
extern BOOL ListFindItem(TLIST *pList, TExItem *pExItem);


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
            if((pItem = ListAdd(&deb.Watch, &pWin->w)))
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

