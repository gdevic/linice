/******************************************************************************
*                                                                             *
*   Module:     lists.c                                                       *
*                                                                             *
*   Date:       02/03/2003                                                    *
*                                                                             *
*   Copyright (c) 2003-2005 Goran Devic                                       *
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

        This module contains code for variable lists, that is, a set
        of expressions or variables. It supports functionality of the
        locals, stack and watch windows.

        The list items are stored in the linked list structure, where each node
        may have a child item (element) which contains another list:

pList->pList |
             V
     -----------    -----------    -----------
     |        n|--->|   e    n|--->|        n|--o                                  root variable
     -----------    -----------    -----------
                        |
                        V
                    -----------    -----------    -----------    -----------
                    |        n|--->|   e    n|--->|        n|--->|        n|--o    first level expansion
                    -----------    -----------    -----------    -----------
                                        |
                                        V
                                    -----------    -----------
                                    |        n|--->|        n|--o                  second level expansion
                                    -----------    -----------

        Where 'n' are the next pointers, and 'e' are the element pointers.
        This sample list would losely correspond to something like this:

        struct nonexpanded
        struct debexpandex
          struct r
          struct cpuexpanded
            int eax
            int ebx
          int variable
          int last
        int lastinroot

    The absence of a valid 'element' pointer signifies that a node is not expanded.

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

#define PRINT_ALL_LINES     999         // Magic value to print all lines (not in a window)

static char buf[MAX_STRING];            // Temp buffer to print the final string


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL GlobalReadDword(DWORD *pDword, DWORD dwAddress);
extern BOOL GlobalReadBYTE(BYTE *pByte, DWORD dwAddress);
extern void PrintTypeValue(char *buf, TExItem *pItem, BYTE *pVar, UINT delta, UINT width);
extern void PrintTypeListExpand(TLISTITEM *pListItem);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL ListFindItem(TLIST *pList, TExItem *pExItem)                         *
*                                                                             *
*******************************************************************************
*
*   Looks for the already existing item, only comparing the TExItem data.
*   This is used to find duplicate watch expressions.
*
*   It is searching only the linked list of root nodes.
*
******************************************************************************/
BOOL ListFindItem(TLIST *pList, TExItem *pExItem)
{
    TLISTITEM *pItem;

    pItem = pList->pList;

    while( pItem )
    {
        if( !memcmp(pExItem, &pItem->Item, sizeof(TExItem)) )
            return( TRUE );

        pItem = (TLISTITEM *) pItem->pNext;
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListGetNewItem(void)                                           *
*                                                                             *
*******************************************************************************
*
*   Allocates and returns a new list item.
*
*   Returns:
*       Address of the new item
*       NULL - allocation failed
*
******************************************************************************/
TLISTITEM *ListGetNewItem(void)
{
    TLISTITEM *pItem;                   // New item

    if( (pItem = (TLISTITEM *) mallocHeap(deb.hHeap, sizeof(TLISTITEM))) )
        memset(pItem, 0, sizeof(TLISTITEM));

    return( pItem );
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListAdd(TLIST *pList)                                          *
*                                                                             *
*******************************************************************************
*
*   Adds a new item to the end of the list. Allocates memory needed for the item.
*   New item will be marked as selected.
*
*   If this call succeeds, the caller needs to fill in list data elements.
*
*   Returns:
*       Address of the new item - needs to be filled in
*       NULL if the item cannot be added to the list
*
******************************************************************************/
TLISTITEM *ListAdd(TLIST *pList)
{
    TLISTITEM *pItem;                   // New item
    TLISTITEM *pEnd;                    // Pointer to the end of the list

    // Allocate list item block and add it to the end of the list
    if( (pItem = ListGetNewItem()) )
    {
        // Find the end of the list in order to add the new item

        pEnd = pList->pList;
        if( pEnd )
        {
            while( pEnd->pNext )
                pEnd = (TLISTITEM *) pEnd->pNext;

            pEnd->pNext = (struct TLISTITEM *) pItem;
        }
        else
        {
            // The list is currently empty, so we add to the front
            pList->pList = pItem;

            // If this is the first node added to the list, set the window top
            pList->pWinTop = pItem;
        }

        // Mark this item selected
        pList->pSelected = pItem;
    }

    return( pItem );
}

/******************************************************************************
*
*   This is a helper function to select a previous item in the list from the
*   given one.
*
*   ListFindPrevRecurse() is its helper traversal function.
*
******************************************************************************/
static void ListFindPrevRecurse(TLISTITEM *pTraverse, TLISTITEM *pItem, BOOL *fTakeNext, TLISTITEM **pResultItem)
{
    if( pTraverse )
    {

        if( pTraverse==pItem )
        {
            *fTakeNext = FALSE;
        }

        if( *fTakeNext )
        {
            *pResultItem = pTraverse;
        }

        ListFindPrevRecurse((TLISTITEM *) pTraverse->pElement, pItem, fTakeNext, pResultItem);
        ListFindPrevRecurse((TLISTITEM *) pTraverse->pNext, pItem, fTakeNext, pResultItem);
    }
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListFindPrev(TLISTITEM *pTraverse, TLISTITEM *pItem)           *
*                                                                             *
*******************************************************************************
*
*   This is a helper function to select a previous item in the list from the
*   given one.
*
*   ListFindPrevRecurse() is its helper traversal function.
*
******************************************************************************/
static TLISTITEM *ListFindPrev(TLIST *pList, TLISTITEM *pItem)
{
    BOOL fTakeNext = TRUE;              // Take all of them until the node hits
    TLISTITEM *pResultItem = NULL;

    ListFindPrevRecurse(pList->pList, pItem, &fTakeNext, &pResultItem);

    return( pResultItem );
}

/******************************************************************************
*
*   This is a helper function to select a next item in the list from the
*   given one. This is not as simple as it seems since after we've done with
*   an element list, we may need to backtrack to the next parent item.
*
*   ListFindNextRecurse() is its helper traversal function.
*
******************************************************************************/
static void ListFindNextRecurse(TLISTITEM *pTraverse, TLISTITEM *pItem, BOOL *fTakeNext, TLISTITEM **pResultItem)
{
    if( pTraverse )
    {
        if( *fTakeNext )
        {
            *fTakeNext = FALSE;
            *pResultItem = pTraverse;
            return;
        }

        if( pTraverse==pItem )
        {
            *fTakeNext = TRUE;
        }

        ListFindNextRecurse((TLISTITEM *) pTraverse->pElement, pItem, fTakeNext, pResultItem);
        ListFindNextRecurse((TLISTITEM *) pTraverse->pNext, pItem, fTakeNext, pResultItem);
    }
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListFindNext(TLIST *pList, TLISTITEM *pItem)                   *
*                                                                             *
*******************************************************************************
*
*   This is a helper function to select a next item in the list from the
*   given one. This is not as simple as it seems since after we've done with
*   an element list, we may need to backtrack to the next parent item.
*
*   ListFindNextRecurse() is its helper traversal function.
*
******************************************************************************/
static TLISTITEM *ListFindNext(TLIST *pList, TLISTITEM *pItem)
{
    BOOL fTakeNext = FALSE;             // Dont take any until the node hits
    TLISTITEM *pResultItem = NULL;

    ListFindNextRecurse(pList->pList, pItem, &fTakeNext, &pResultItem);

    return( pResultItem );
}

/******************************************************************************
*
*   Returns the last item in the list.
*
*   ListFindLastRecurse() is its helper traversal function.
*
******************************************************************************/
static void ListFindLastRecurse(TLISTITEM *pTraverse, TLISTITEM **pResultItem)
{
    if( pTraverse )
    {
        *pResultItem = pTraverse;

        ListFindLastRecurse((TLISTITEM *) pTraverse->pElement, pResultItem);
        ListFindLastRecurse((TLISTITEM *) pTraverse->pNext, pResultItem);
    }
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListFindLast(TLIST *pList)                                     *
*                                                                             *
*******************************************************************************
*
*   Returns the last item in the list.
*
*   ListFindLastRecurse() is its helper traversal function.
*
******************************************************************************/
static TLISTITEM *ListFindLast(TLIST *pList)
{
    TLISTITEM *pResultItem = NULL;

    ListFindLastRecurse(pList->pList, &pResultItem);

    return( pResultItem );
}

/******************************************************************************
*                                                                             *
*   void ListDeleteItem(TLISTITEM *pItem)                                     *
*                                                                             *
*******************************************************************************
*
*   Delete only one list item. This function does not unlink it from the list!
*
******************************************************************************/
static void ListDeleteItem(TLISTITEM *pItem)
{
    if( pItem )
        freeHeap(deb.hHeap, pItem);
}

/******************************************************************************
*                                                                             *
*   void ListDelRecurse(TLISTITEM *pItem)                                     *
*                                                                             *
*******************************************************************************
*
*   Recursively delete the whole subtree pointed by pItem.
*
*   Where:
*       pItem is the address of the first item on the list (may be NULL)
*
******************************************************************************/
static void ListDelRecurse(TLISTITEM *pItem)
{
    if( pItem )
    {
        ListDelRecurse((TLISTITEM *) pItem->pElement);
        ListDelRecurse((TLISTITEM *) pItem->pNext);

        ListDeleteItem(pItem);
    }
}

/******************************************************************************
*                                                                             *
*   BOOL ListDel(TLIST *pList, TLISTITEM *pItem, BOOL fDelRoot)               *
*                                                                             *
*******************************************************************************
*
*   Deletes a specified item from the list.
*
*   It sets the next one on the list to be the current (selected); if the
*   item is expanded, deletes all its expanded children
*
*   Returns:
*       TRUE if there are more elements on the root node list
*       FALSE if the root node list is empty
*
******************************************************************************/
BOOL ListDel(TLIST *pList, TLISTITEM *pItem, BOOL fDelRoot)
{
    TLISTITEM *pPrev;                   // Previous node next address
    BOOL retval = TRUE;                 // Assume there are more nodes on the root list

    // Recursively delete all child elements from that list
    ListDelRecurse((TLISTITEM *) pItem->pElement);

    // Depending on if we need to delete the root or not, unlink them
    // We can only delete a root node (variable or expression)
    if( fDelRoot )
    {
        if( pList->pList==pItem )
        {
            // Deleting the first item on the root list
            if( (pList->pList = (TLISTITEM *) pItem->pNext)==NULL)
            {
                // There are no more items on the root list, exit the focus mode
                pList->pSelected = NULL;

                retval = FALSE;         // The list is empty, also
            }
            else
                pList->pSelected = pList->pList;

            pList->pWinTop = pList->pSelected;
        }
        else
        {
            pPrev = pList->pList;

            // Find the node that immediately preceeds our node to be deleted
            while( (TLISTITEM *) pPrev->pNext != pItem )
                pPrev = (TLISTITEM *) pPrev->pNext;

            // Bypass our node that will be deleted
            if( (pPrev->pNext = pItem->pNext)==NULL )
                pList->pSelected = pPrev;
            else
                pList->pSelected = (TLISTITEM *) pPrev->pNext;
        }

        ListDeleteItem(pItem);          // Delete our node
    }
    else
    {
        pItem->pElement = NULL;
        pList->pSelected = pItem;
    }

    return( retval );
}

/******************************************************************************
*                                                                             *
*   void ListDelAll(TLIST *pList)                                             *
*                                                                             *
*******************************************************************************
*
*   Deletes a complete list.
*
******************************************************************************/
void ListDelAll(TLIST *pList)
{
    while( pList->pList )
        ListDel(pList, pList->pList, TRUE);
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListGetNext(TLIST *pList, TLISTITEM *pItem)                    *
*                                                                             *
*******************************************************************************
*
*   Walks the root nodes of a list.
*
*   Where:
*       pList is the list to traverse
*       pItem is: NULL - get the first item in the list
*                 non-NULL - get the next item in the list
*
*   Returns:
*       Address of the item node
*       NULL if there is no more items on the list
*
******************************************************************************/
TLISTITEM *ListGetNext(TLIST *pList, TLISTITEM *pItem)
{
    if( pItem )
    {
        // Return the next item on the list
        return( (TLISTITEM *) pItem->pNext );
    }
    else
    {
        // pItem was NULL, return the address of the first node
        return( pList->pList );
    }
}

/******************************************************************************
*
*   Helper function to ListPrint()
*
******************************************************************************/
static void ListPrintRecurse(TLIST *pList, TLISTITEM *pItem, BOOL *fPrintNext, int *pLineCount)
{
    static char *sIndent = "                    ";      // 20 characters
    char col;                           // Current line color
    char *pLine;                        // Pointer to the line that we are printing

    if( pItem && *pLineCount>0)
    {
        // If we hit the window top node, start printing
        if( pItem==pList->pWinTop )
            *fPrintNext = TRUE;

        if( *fPrintNext )
        {
            (*pLineCount)--;

            // If we have the focus and the current item is the one selected, invert the line color
            if( pList->fInFocus && pList->pSelected==pItem )
                col = COL_REVERSE;
            else
                col = COL_NORMAL;

            // Take into account possible shift in X direction
            // If the list is TYPEDEF or STACK, print only element name, not the value
            switch( pList->ID )
            {
                case LIST_ID_TYPE:
                    sprintf(buf, "%s%s;", sIndent + strlen(sIndent) - pItem->nLevel * 2, pItem->String);
                    break;

                case LIST_ID_STACK:
                    sprintf(buf, "%s%s", sIndent + strlen(sIndent) - pItem->nLevel * 2, pItem->String);
                    break;

                default:
                    sprintf(buf, "%s%s = %s", sIndent + strlen(sIndent) - pItem->nLevel * 2, pItem->String, pItem->Value);
            }

            if( pList->nXOffset > strlen(buf) )
                pLine = "";
            else
                pLine = buf + pList->nXOffset;

            dprinth(1, "%c%c%s", DP_SETCOLINDEX, col, pLine);
        }

        ListPrintRecurse(pList, (TLISTITEM *) pItem->pElement, fPrintNext, pLineCount);
        ListPrintRecurse(pList, (TLISTITEM *) pItem->pNext, fPrintNext, pLineCount);
    }
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListPrint(TLIST *pList, int nLines)                            *
*                                                                             *
*******************************************************************************
*
*   Prints the given subtree including all elements.
*
******************************************************************************/
static TLISTITEM *ListPrint(TLIST *pList, int nLines)
{
    TLISTITEM *pResultItem = NULL;
    BOOL fPrintNext;

    // Print all lines if the number of lines is a magic value, otherwise, trigger off the windows top
    fPrintNext = (nLines==PRINT_ALL_LINES)? TRUE : FALSE;

    ListPrintRecurse(pList, pList->pList, &fPrintNext, &nLines);

    return( pResultItem );
}

/******************************************************************************
*                                                                             *
*   void MakeSelectedVisible(TLIST *pList, int nLines)                        *
*                                                                             *
*******************************************************************************
*
*   Adjusts pWinTop pointer so the selected line is visible. Assume that the
*   selected line had been changed.
*
******************************************************************************/
void MakeSelectedVisible(TLIST *pList, int nLines)
{
    TLISTITEM *pItem;
    BOOL fFoundWinTop = FALSE;

    pItem = pList->pList;
    nLines--;

    while( pItem )
    {
        // If we found the selected node, mark it and start counting lines
        if( pItem==pList->pSelected )
        {
            // If we did not found windows top, set it to the selected line
            // If we already found windows top, just exit
            if( !fFoundWinTop )
                pList->pWinTop = pList->pSelected;

            return;
        }

        // If we found the window top' marked node (we dont expect to find selected yet)
        if( pItem==pList->pWinTop )
            fFoundWinTop = TRUE;

        if( fFoundWinTop )
        {
            // If we run out of window lines, drag the window top with us
            if( --nLines <= 0 )
            {
                pList->pWinTop = ListFindNext(pList, pList->pWinTop);
            }
        }

        pItem = ListFindNext(pList, pItem);
    }
}

/******************************************************************************
*
*   Helper function to ListEvaluate() that evaluates all the nodes recursively.
*
******************************************************************************/
static void ListEvaluateRecurse(TLISTITEM *pItem, BYTE *pVar )
{
    if( pItem )
    {
        PrintTypeValue(pItem->Value, &pItem->Item, pVar+pItem->delta/8, pItem->delta, pItem->width);

        ListEvaluateRecurse((TLISTITEM *) pItem->pElement, pVar+pItem->delta/8);
        ListEvaluateRecurse((TLISTITEM *) pItem->pNext, pVar);
    }
}

/******************************************************************************
*                                                                             *
*   void ListEvaluate(TLIST *pList)                                           *
*                                                                             *
*******************************************************************************
*
*   Evaluates the effective values for all expression items in the given list.
*
*   Where:
*       pList is the list to evaluate
*
******************************************************************************/
void ListEvaluate(TLIST *pList)
{
    TLISTITEM *pItem;

    pItem = pList->pList;

    while( pItem )
    {
        ListEvaluateRecurse(pItem, pItem->Item.pData);

        pItem = (TLISTITEM *) pItem->pNext;
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
    int maxLines = PRINT_ALL_LINES;     // By default print all lines

    if( pFrame->fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pFrame->Top+1);

        switch( pList->ID )
        {
            case LIST_ID_WATCH:
                PrintLine("Watch");
                break;

            case LIST_ID_LOCALS:
                PrintLine("Locals");
                break;

            case LIST_ID_STACK:
                PrintLine("Stack");
                break;
        }

        // Scale down the number of lines to print to the size of the window
        maxLines = pFrame->nLines - 1;
    }
    else
        if( fForce==FALSE )
            return;

    // Evaluate type structure into effective values, except for the type list
    // (command TYPES) where we just print the type definition, and the STACK information
    if( pList->ID!=LIST_ID_TYPE && pList->ID!=LIST_ID_STACK )
    {
        ListEvaluate(pList);
    }

    ListPrint(pList, maxLines);

    if( pFrame->fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}

/******************************************************************************
*                                                                             *
*   void FocusInPlace(TLIST *pList, TFRAME *pFrame)                           *
*                                                                             *
*******************************************************************************
*
*   Enables a list to be managed in-place. This function is called from the
*   command line editing when a key combination is pressed to manage a list:
*
*   Alt-L  - manage locals list
*   Alt-W  - manage watch list
*   Alt-S  - manage stack list
*
******************************************************************************/
void FocusInPlace(TLIST *pList, TFRAME *pFrame)
{
    TLISTITEM *pListItem;               // Temp list item
    BOOL fContinue = TRUE;              // Continuation flag
    WCHAR Key;                          // Current key pressed
    int Y;                              // Window vertical size

    // If there was no items in the list, return since there is nothing to manage
    if( pList->pList==NULL )
        return;

    // If the selected window is not visible, make it visible
    if( pFrame->fVisible==FALSE )
    {
        // Make a window visible and redraw whole screen
        pFrame->fVisible = TRUE;
        RecalculateDrawWindows();
    }

    // Set the signal that we are managing the list window, that will enable
    // the drawing code to highlight the current item
    pList->fInFocus = TRUE;

    do
    {
        // Check if the window top needs to be adjusted
        MakeSelectedVisible(pList, pFrame->nLines);

        // Before every key redraw the screen
        RecalculateDrawWindows();

        // Print the help line for the management of this window
        dprint("%c%c%c%c%c%cValid control keys: %s %s %s %s Home End PgUp PgDn Enter %s\r%c",
        DP_SAVEXY,
        DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
        DP_SETCOLINDEX, COL_HELP,
        "Left", "Right", "Up", "Dn",
        pList->ID==LIST_ID_WATCH? "Del":"",     // Watch list allow deletion
        DP_RESTOREXY);

        // Get the next keypress that should tell us what to do here
        Key = GetKey(TRUE) & 0xFF;

        switch( Key )
        {
            case ENTER:     // Enter key expands a selected item under some conditions

                if( pList->pSelected->pElement )
                    ListDel(pList, pList->pSelected, FALSE);
                else
                    PrintTypeListExpand(pList->pSelected);

                break;

            case DEL:       // Delete key deletes selected item in the list

                // For those lists that allow deletion, only a root item can be deleted
                if( pList->pSelected->fCanDelete )
                    fContinue = ListDel(pList, pList->pSelected, TRUE);

                break;

            case LEFT:      // Left key scrolls the list pane to the right

                if( pList->nXOffset )
                    pList->nXOffset -= 1;

                break;

            case RIGHT:     // Right key scrolls the list to the left

                if( pList->nXOffset < MAX_STRING-80 )
                    pList->nXOffset += 1;

                break;

            case UP:        // Up key selects the previous list item, possibly scrolling

                if( (pList->pSelected = ListFindPrev(pList, pList->pSelected))==NULL )
                    pList->pSelected = pList->pList;


                break;

            case DOWN:      // Down key selects the next list item, possibly scrolling

                if( (pListItem = ListFindNext(pList, pList->pSelected)) )
                    pList->pSelected = pListItem;

                break;

            case PGUP:      // Scrolls a whole window page up, listing items that are above
                            // This simply duplicates the code for UP.
                Y = pFrame->nLines;
                while( Y-- )
                {
                    if( (pList->pSelected = ListFindPrev(pList, pList->pSelected))==NULL )
                        pList->pSelected = pList->pList;
                }
                break;

            case PGDN:      // Scrolls a whole window page down, listing items that are below
                            // This simply duplicates the code for DOWN.
                Y = pFrame->nLines;
                while( Y-- )
                {
                    if( (pListItem = ListFindNext(pList, pList->pSelected)) )
                        pList->pSelected = pListItem;
                }
                break;

            case HOME:      // Position on to the first item

                pList->pSelected = pList->pWinTop = pList->pList;
                pList->nXOffset = 0;                                // Also reset the X offset

                break;

            case END:       // Position on to the last item

                pList->pSelected = ListFindLast(pList);

                break;

            default:        // Anything else implicitly quits this window management mode
                fContinue = FALSE;
        }
    }
    while( fContinue );

    // We are done managing the list window
    pList->fInFocus = FALSE;

    // On the way out we need to reset the X displayed offset
    pList->nXOffset = 0;

    // Redraw the screen to clear the focus line
    RecalculateDrawWindows();
}

