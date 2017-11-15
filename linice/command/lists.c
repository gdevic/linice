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

static char buf[MAX_STRING];            // Temp output line buffer

static BOOL fListInFocus = FALSE;       // Are we currently manage a list window?

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL SymbolFindByName(DWORD **pSym, TSYMTYPEDEF1 **ppType1, char *pName, int nNameLen);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   static void MakeSelectedVisible(TLIST *pList)                             *
*                                                                             *
*******************************************************************************
*
*   Adjusts pWinTop to make selected item visible inside the given window frame.
*
******************************************************************************/
static void MakeSelectedVisible(TLIST *pList, TFRAME *pFrame)
{
    int Y;                              // Window vertical size

    // Starting with the selected item, count back and make sure the
    // pWinTop is set within the frame window' span

    // Set the current of that list to what is selected
    if( QSetCurrent(&pList->Item, pList->pSelected) )
    {
        // Count back a window-worth of lines
        for(Y=pFrame->nLines; Y>1; Y--)
        {
            if( pList->pWinTop==QCurrent(&pList->Item) )
                return;

            // Select the previous item in the list
            QPrev(&pList->Item);
        }

        // If we reach this point, window top was not in the span, so set it
        pList->pWinTop = QNext(&pList->Item);
    }
}

/******************************************************************************
*                                                                             *
*   void MakeSelectedLast(TLIST *pList, TFRAME *pFrame)                       *
*                                                                             *
*******************************************************************************
*
*   Adjust windows top so the selected item appears last in the list of items
*   for as long the windows is.
*
******************************************************************************/
static void MakeSelectedLast(TLIST *pList, TFRAME *pFrame)
{
    // Starting with the selected item, reposition window top so the item
    // will be drawn last for as high the window is

    int Y;                              // Window vertical size

    // Set the current of that list to what is selected
    if( QSetCurrent(&pList->Item, pList->pSelected) )
    {
        // Count back a window-worth of lines
        for(Y=pFrame->nLines - 1; Y>1; Y--)
        {
            // Select the previous item in the list
            QPrev(&pList->Item);
        }

        // Set the new window top either to the first element or last counted
        if( QCurrent(&pList->Item)==NULL )
            pList->pWinTop = QFirst(&pList->Item);
        else
            pList->pWinTop = QCurrent(&pList->Item);
    }
}


/******************************************************************************
*                                                                             *
*   static TLISTITEM *ListAlloc(TQueue *pList)                                *
*                                                                             *
*******************************************************************************
*
*   Allocated memory for the list node structure and adds it to a list queue
*
*   Where:
*       pList is the address of the List queue to add it to
*
*   Returns:
*       Address of the new item
*       NULL if the item cannot be allocated and added
*
******************************************************************************/
static TLISTITEM *ListAlloc(TQueue *pList)
{
    TLISTITEM *pItem;                   // New item

    // Allocate memory for the new item
    pItem = (TLISTITEM *)_kMallocHeap(sizeof(TLISTITEM));

    if( pItem )
    {
        // Zero it out to set initial state
        memset(pItem, 0, sizeof(TLISTITEM));

        // Add new item into the queue
        if( QAdd(pIce->hHeap, pList, pItem) != 0 )
            return( pItem );

        // Operation failed.. Free what was allocated.
        _kFreeHeap(pItem);
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame, char *pVar)              *
*                                                                             *
*******************************************************************************
*
*   Adds a new item to the end of the list. Allocates memory needed for the item.
*   New item will be marked as selected.
*
*   Returns:
*       Address of the new item
*       NULL if the item cannot be added to the list
*
******************************************************************************/
TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame, char *pVar)
{
    TLISTITEM *pItem;                   // New item

    // Make sure it will be added to the end of the list queue
    QLast(&pList->Item);

    // Allocate memory and add new item to the queue
    pItem = ListAlloc(&pList->Item);

    if( pItem )
    {
        // Mark this item selected
        pList->pSelected = pItem;

        // Make the last item appear last in the list of items
        MakeSelectedLast(pList, pFrame);

        // Copy the expression or variable string
        strcpy(pItem->sExp, pVar);

        return( pItem );
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   void ListDelCur(TLIST *pList)                                             *
*                                                                             *
*******************************************************************************
*
*   Deletes a selected item from the list.
*   It sets the next one on the list to be the current (selected); if the
*   item is expanded, deletes all its expanded children.
*
*   Returns:
*       TRUE - there are more items left on the list
*       FALSE - The last item had been deleted - list is empty
*
******************************************************************************/
static BOOL ListDelCur(TLIST *pList)
{
    TLISTITEM *pItem, *pItemNext, *pRootItem;

    // Delete the item and dispose the queue node
    pRootItem = pItem = pList->pSelected;

    // Select our current item for deletion
    QSetCurrent(&pList->Item, pItem);

    do
    {
        // Remember the next item so we can iterate
        pItemNext = QPeekNext(&pList->Item);

        // Delete it from the queue - this will set the current to the next one or previous one
        // if the deleted one was the last item on the list.
        // QDelete returns the address of the item queued, so free that item as well.
        _kFreeHeap( QDelete(pIce->hHeap, &pList->Item) );

        // Get the next current item after deletion
        pItem = QCurrent(&pList->Item);

    }
    while( pItemNext && pItemNext->Parent );

    pList->pSelected = pItem;

    // If the item deleted was the windows top, adjust windows top
    if( pList->pWinTop==pRootItem )
        pList->pWinTop = pItem;

    return( pList->pSelected != NULL );
}

/******************************************************************************
*                                                                             *
*   static BOOL ListDelSubtree(TLIST *pList, TLISTITEM *pRoot, BOOL fDelRoot) *
*                                                                             *
*******************************************************************************
*
*   Deletes all the expanded nodes (including recursively expanded) of a given
*   node and optionally deletes the given root node.
*
*   Where:
*       pList is the list to manage
*       pRoot is the root node from which we are deleting
*       fDelRoot - do we delete root node (along with all its expanded children)?
*   Returns:
*       TRUE - there are more items left on the list
*       FALSE - The last item had been deleted - list is empty
*
******************************************************************************/
static BOOL ListDelSubtree(TLIST *pList, TLISTITEM *pRoot, BOOL fDelRoot)
{
    TLISTITEM *pItem, *pItemNext;

    // Select our current item for deletion
    QSetCurrent(&pList->Item, pRoot);

    // If the root item is expanded, loop for all its children
    if(pRoot->fExpanded)
    {
        // Remember the next item so we can iterate
        pItemNext = QNext(&pList->Item);

        // Loop for all the children of this prent node
        while( pItemNext && pItemNext->Parent==pRoot )
        {
            ListDelSubtree(pList, pItemNext, TRUE);

            // Get the next current item after deletion
            pItemNext = QCurrent(&pList->Item);
        }
    }

    // Delete the root node if required
    if( fDelRoot )
    {
        // Set the root node since the previous iteration might have changed it
        QSetCurrent(&pList->Item, pRoot);

        // Delete it from the queue - this will set the current to the next one or previous one
        // if the deleted one was the last item on the list.
        // QDelete returns the address of the item queued, so free that item as well.
        _kFreeHeap( QDelete(pIce->hHeap, &pList->Item) );

        // Get the next current item after deletion
        pItem = QCurrent(&pList->Item);

        pList->pSelected = pItem;

        // If the item deleted was the windows top, adjust windows top
        if( pList->pWinTop==pRoot )
            pList->pWinTop = pItem;
    }
    else
    {
        // Not deleting given root node, so set it the current

        pRoot->fExpanded = FALSE;       // Now it is definitely not expanded (any more)
        pList->pSelected = pRoot;
    }

    return( pList->pSelected != NULL );
}

/******************************************************************************
*                                                                             *
*   static void ListCollapse(TLIST *pList)                                    *
*                                                                             *
*******************************************************************************
*
*   Collapses a selected item of the list. It leaves that item current.
*
******************************************************************************/
static void ListCollapse(TLIST *pList)
{
    ListDelSubtree(pList, pList->pSelected, FALSE);
}

/******************************************************************************
*                                                                             *
*   static void ListExpand(TLIST *pList)                                      *
*                                                                             *
*******************************************************************************
*
*   Expands selected item of the list.
*
******************************************************************************/
static void ListExpand(TLIST *pList)
{
    TLISTITEM *pItem, *pParent;         // New item and the parent item
    BOOL fAnother;                      // Do we have another expanded element

    pParent = pList->pSelected;

    // Prepare the root item for a certain expansion
    SetupForExpansion(pParent);

    // Item is not expanded, so expand it if it is expandable
    if( pParent->fExpandable )
    {
        do
        {   // Get the new list node and add it after the selected one

            // Allocate memory and add new item to the queue. This may fail if not enough memory!
            if( (pItem = ListAlloc(&pList->Item))==NULL )
                break;

            // Mark this item as child of the base parent
            pItem->Parent = pParent;

            // Fill up the list item elements using the iterator
            fAnother = TypeExpandIterator(pItem);

            // Prepare this list item for a possible expansion
            SetupForExpansion(pItem);
        }
        while( fAnother );

        pParent->fExpanded = TRUE;
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
    TLISTITEM *pItem;
    int maxLines = 9999;
    int nLine = 1;
    char *pLine;                        // Pointer to the output line
    char col;                           // Current line color

    // By default list from the top item (first item on the list)
    pItem = QFirst(&pList->Item);

    if( pFrame->fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pFrame->Top+1);

        switch( pList->ID )
        {
            case LIST_ID_WATCH:
                PrintLine(" Watch");
                break;

            case LIST_ID_LOCALS:
                PrintLine("Locals");
                break;
        }

        maxLines = pFrame->nLines;

        // Use the window top as the first item in the list
        pItem = pList->pWinTop;
        QSetCurrent(&pList->Item, pItem);
    }
    else
        if( fForce==FALSE )
            return;

    // List all items of a specific list
    while( pItem && nLine<maxLines )
    {
        // If the current item is the one selected, invert the line color
        // Do this only if we are currently managing (editing) a list window
        if( pItem==pList->pSelected && fListInFocus )
            col = COL_REVERSE;
        else
            col = COL_NORMAL;

        // We need to find out what we are drawing in this particular line

        // Parent items may or may not be in the scope, if they are not, we collapse children
        // If they are, we traverse into child items

        if( pItem->Parent )
        {
            // Parent link is non-zero, so this is a child item.

            // Children items already have the string ready
            strcpy(buf, pItem->sExp);
        }
        else
        {
            // Parent link is zero, so this is a parent, or root, variable

            // At the drawing time find the symbol and fill in all the data in the list structure
            if( SymbolFindByName(&pItem->pSymbol, &pItem->pType1, pItem->sExp, strlen(pItem->sExp) ))
            {
                // Prepare this list item for a possible expansion
                SetupForExpansion(pItem);

                // Print this root type symbol
                PrintListItem(buf, pItem, pItem->sExp, strlen(pItem->sExp), -1);
            }
            else
            {
                // Error evaluating this expression - we may be out of the context?
                sprintf(buf, "%s = <?>", pItem->sExp);

                // Now we need to collapse all the possible children of this parent symbol
                if( pItem->fExpanded )
                {
                    ListCollapse(pList);
                }
            }
        }

        if( pList->nXOffset > strlen(buf) )
            pLine = "";
        else
            pLine = buf + pList->nXOffset;

        if( dprinth(nLine++, "%c%c%s\r", DP_SETCOLINDEX, col, pLine)==FALSE )
            break;

        pItem = QNext(&pList->Item);
    }

    if( pFrame->fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}


/******************************************************************************
*                                                                             *
*   void FocusInPlace(TLIST *pList, TFRAME *pFrame, BOOL fCanDel)             *
*                                                                             *
*******************************************************************************
*
*   Enables a list to be managed in-place. This function is called from the
*   command line editing when a key combination is pressed to manage a list:
*
*   Alt-L  - manage locals list
*   Alt-W  - manage watch list
*
*   Where:
*       fCanDel - Item can be deleted (only watch item can be deleted)
*
******************************************************************************/
void FocusInPlace(TLIST *pList, TFRAME *pFrame, BOOL fCanDel)
{
    BOOL fContinue = TRUE;              // Continuation flag
    CHAR Key;                           // Current key pressed
    TLISTITEM *pItem;                   // Current list item
    int Y;                              // Window vertical size

    // If the selected window is not visible, make it visible
    if( pFrame->fVisible==FALSE )
    {
        // Make a window visible and redraw whole screen
        pFrame->fVisible = TRUE;
        RecalculateDrawWindows();
    }

    // Set the signal that we are managing the list window, that will enable
    // the drawing code to highlight the current item
    fListInFocus = TRUE;

    MakeSelectedVisible(pList, pFrame);

    do
    {
        // Before every key redraw the screen
        RecalculateDrawWindows();

        // Print the help line for the management of this window
        dprint("%c%c%c%c%c%cValid control keys: %c %c %c %c Home End PgUp PgDn Enter %s\r%c",
        DP_SAVEXY,
        DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
        DP_SETCOLINDEX, COL_HELP,
        24, 25, 26, 27,
        fCanDel? "Del":"",
        DP_RESTOREXY);

        // Get the next keypress that should tell us what to do here
        Key = GetKey(TRUE) & 0xFF;

        // Set the queue-wise current to the selected one
        QSetCurrent(&pList->Item, pList->pSelected);

        switch( Key )
        {
            case ENTER:     // Enter key expands a selected item under some conditions

                if( pList->pSelected->fExpanded )
                    ListCollapse(pList);
                else
                    ListExpand(pList);

                break;

            case DEL:       // Delete key deletes selected item in the list

                // For those lists that allow deletion, only a root item can be deleted
                if( fCanDel && pList->pSelected->Parent==NULL )
                    fContinue = ListDelSubtree(pList, pList->pSelected, TRUE);

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

                pItem = QPrev(&pList->Item);
                if( pItem )
                {
                    pList->pSelected = pItem;

                    // Check if the window top needs to be adjusted
                    if( pList->pWinTop==QNext(&pList->Item) )
                        pList->pWinTop = pList->pSelected;
                }

                break;

            case DOWN:      // Down key selects the next list item, possibly scrolling

                pItem = QNext(&pList->Item);
                if( pItem )
                    pList->pSelected = pItem;
                MakeSelectedVisible(pList, pFrame);

                break;

            case PGUP:      // Scrolls a whole window page up, listing items that are above
                            // This simply duplicates the code for UP. I am not sure if I like this...
                Y = pFrame->nLines;
                while( Y-- )
                {
                    QSetCurrent(&pList->Item, pList->pSelected);
                    pItem = QPrev(&pList->Item);
                    if( pItem )
                    {
                        pList->pSelected = pItem;

                        // Check if the window top needs to be adjusted
                        if( pList->pWinTop==QNext(&pList->Item) )
                            pList->pWinTop = pList->pSelected;
                    }
                }
                break;

            case PGDN:      // Scrolls a whole window page down, listing items that are below
                            // This simply duplicates the code for DOWN. I am not sure if I like this...
                Y = pFrame->nLines;
                while( Y-- )
                {
                    QSetCurrent(&pList->Item, pList->pSelected);
                    pItem = QNext(&pList->Item);
                    if( pItem )
                        pList->pSelected = pItem;
                    MakeSelectedVisible(pList, pFrame);
                }
                break;

            case HOME:      // Position on to the first item

                pList->pSelected = pList->pWinTop = QFirst(&pList->Item);

                break;

            case END:       // Position on to the last item

                pList->pSelected = QLast(&pList->Item);

                // Make the last item appear last in the list of items
                MakeSelectedLast(pList, pFrame);

                break;

            default:        // Anything else implicitly quits this management
                fContinue = FALSE;
        }
    }
    while( fContinue );

    // We are done managing the list window

    fListInFocus = FALSE;

    // On the way out we need to reset the X displayed offset
    pList->nXOffset = 0;

    // Redraw the screen to clear the focus line
    RecalculateDrawWindows();
}

