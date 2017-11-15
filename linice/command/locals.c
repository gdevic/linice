/******************************************************************************
*                                                                             *
*   Module:     locals.c                                                      *
*                                                                             *
*   Date:       01/18/2002                                                    *
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

        This module contans code to display local variables of a function
        scope.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 01/18/02   Original                                             Goran Devic *
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

static TSYMFNSCOPE1 *LocalsQueue[MAX_LOCALS_QUEUE];

static char *RegisterVariables[] =
{
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "eip", "eflags", "cs", "ss", "ds", "es", "fs", "gs",
};

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void ParseLocalSymbol(TExItem *item, TSYMFNSCOPE1 *pLocal);
extern void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce);
extern TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame);
extern void ListDel(TLIST *pList, TLISTITEM *pItem, BOOL fDelRoot);
extern void PrettyPrintVariableName(char *pString, char *pName, TSYMTYPEDEF1 *pType1);


/******************************************************************************
*                                                                             *
*   void LocalsDraw(BOOL fForce)                                              *
*                                                                             *
*******************************************************************************
*
*   Draws local variables
*
******************************************************************************/
void LocalsDraw(BOOL fForce)
{
    ListDraw(&deb.Local, &pWin->l, fForce);
}

/******************************************************************************
*                                                                             *
*   BOOL cmdLocals(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   List local variables from the current stack frame to the command window.
*
******************************************************************************/
BOOL cmdLocals(char *args, int subClass)
{
    LocalsDraw(TRUE);

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL LocalBuildQueue(TSYMFNSCOPE *pFnScope, DWORD dwEIP)                  *
*                                                                             *
*******************************************************************************
*
*   Fills in the array of pointers to local variables at the given context
*   of pFnScope and the eip address.
*
*   Where:
*       pFnScope is the function scope to search
*       dwEIP is the address within that function scope
*   Returns:
*       TRUE array filled up with zero or more values
*       FALSE Invalid scope or offset
*
******************************************************************************/
static BOOL LocalBuildQueue(TSYMFNSCOPE *pFnScope, DWORD dwEIP)
{
    DWORD dwFnOffset;
    int i, index;                   // Running indices to function scope and queue
    int nBlock;                     // Current scope block


    // TODO: FIXME - fix the way locals are queued: Right now we look for the EIP position
    // then walk back and fill up the queue. This is not correct. First, the local vars
    // are defined _before_ an opening { for their scope, not within; second it picks
    // up vars that are not defined yet for that scope.

    // Find where is our current EIP within the current function
    if( pFnScope && pFnScope->dwStartAddress<=dwEIP && pFnScope->dwEndAddress>=dwEIP )
    {
        // Scan forward for the right scope brackets depending on the code offset
        dwFnOffset = dwEIP - pFnScope->dwStartAddress;

        for(i=0; i<pFnScope->nTokens; i++ )
        {
            // Only LBRAC and RBRAC contain the function offset value in param, so we know where we are
            if( pFnScope->list[i].TokType==TOKTYPE_LBRAC || pFnScope->list[i].TokType==TOKTYPE_RBRAC )
            {
                // Since the offsets are in increasing order, if our offset is already less than the
                // function scope offset, we assume we found our place
                if( dwFnOffset <= pFnScope->list[i].param )
                    break;
            }
        }

        // We have now index 'i' specifying where in the function scope array are we located
        // Need to start filling in the scope array by traversing backwards
        nBlock = 0;
        index  = 0;

        for(; i>=0 && index<MAX_LOCALS_QUEUE; i--)
        {
            // If there is a new scope block staring, decrement the block number, but never past 0
            if( pFnScope->list[i].TokType==TOKTYPE_LBRAC )
            {
                if( nBlock>0 )
                    nBlock--;
            }

            // If there is a scope block ending, increment the block number
            if( pFnScope->list[i].TokType==TOKTYPE_RBRAC )
                nBlock++;

            // Take a variable only if the current block is visible
            // Add in only local variables
            if( pFnScope->list[i].TokType==TOKTYPE_PARAM || pFnScope->list[i].TokType==TOKTYPE_RSYM || pFnScope->list[i].TokType==TOKTYPE_LSYM )
            {
                if( nBlock==0 )
                {
                    // Store a local variable that is now visible
                    LocalsQueue[index] = &pFnScope->list[i];
                    index++;            // Increment the index into the locals queue
                }
            }
        }

        return( TRUE );
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   void LocalBuildList()                                                     *
*                                                                             *
*******************************************************************************
*
*   Builds a list of locals based on the locals queue.
*
******************************************************************************/
static void LocalBuildList()
{
    TLISTITEM *pItem;               // List item that we are adding
    int index;                          // Running indicex into queue

    index = MAX_LOCALS_QUEUE;           // We will walk the list from back to front

    while( --index>=0 )
    {
        // Take only non-NULL items
        if( LocalsQueue[index] )
        {
            // Found an item in the queue, add it up to the list of locals

            if(pItem = ListAdd(&deb.Local, &pWin->l))
            {
                // Get the type descriptor into the new list node
                ParseLocalSymbol(
                    &pItem->Item,
                    LocalsQueue[index]);

                // Copy the variable name into the Name field
                strccpy(pItem->Name, LocalsQueue[index]->pName, ':');

                PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);
            }
        }
    }
}

/******************************************************************************
*                                                                             *
*   BOOL FillLocalScope(TSYMFNSCOPE *pFnScope, DWORD dwEIP)                   *
*                                                                             *
*******************************************************************************
*
*   Rebuilds the list of local variables.
*
*   Where:
*       pFnScope is the function scope to search
*       dwEIP is the address within that function scope
*   Returns:
*       TRUE List of locals has been rebuilt
*       FALSE Invalid scope or offset
*
******************************************************************************/
BOOL FillLocalScope(TSYMFNSCOPE *pFnScope, DWORD dwEIP)
{
    // Clear the current list so we can rebuild it or not have it
    while( deb.Local.pList )
        ListDel(&deb.Local, deb.Local.pList, TRUE);

    memset(&LocalsQueue, 0, sizeof(LocalsQueue));

    if( LocalBuildQueue(pFnScope, dwEIP) )
    {
        // Rebuild the list of locals based on the queue

        LocalBuildList();
    }

    return( TRUE );
}































#if 0
    // Make sure our code offset is within this function scope descriptor
    if( pFnScope && pFnScope->dwStartAddress<=dwEIP && pFnScope->dwEndAddress>=dwEIP )
    {
        // Scan forward for the right scope brackets depending on the code offset
        dwFnOffset = dwEIP - pFnScope->dwStartAddress;
        position = -1;                  // -1 means we did not find any locals or out of scope

        for(i=0; i<pFnScope->nTokens; i++ )
        {
            // Only LBRAC and RBRAC contain the function offset value in param, so we know where we are
            if( pFnScope->list[i].TokType==TOKTYPE_LBRAC || pFnScope->list[i].TokType==TOKTYPE_RBRAC )
            {
                if( dwFnOffset >= pFnScope->list[i].param )
                    position = i;
                else
                if( dwFnOffset < pFnScope->list[i].param )
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
                    if( xNested>0 ) // Decrease nesting level only if we have an extra scope to avoid
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
    else
        return( FALSE );


    LocalsQueue[index] = NULL;      // Always terminate the queue

    // Now we have the local scope array filled with the tokens...
    // It is time to set up the master list for local variables

    // Add all the items that we queued up, in reverse order, index still points to the last item
    index--;                        // Index the previous one, not the last one

    while( index>=0 )
    {
#if 0
        if( (pItem = ListAdd(&deb.Local, &pWin->l)) )
#endif
        {
            // A new list node has been added. Populate it with the type data etc.

            // Get the type descriptor into the new list node
            FindLocalSymbol(
                &pItem->Item,
                LocalsQueue[index]->pName,
                strchr(LocalsQueue[index]->pName, ':') - LocalsQueue[index]->pName);

//            TypedefCanonical(&pItem->Item.Type);

            // Print the initial string for the given variable
//            pStr = &pItem->sExp[0];

            if( pItem->Item.bType==EXTYPE_REGISTER )
            {
                pStr += sprintf(pStr, "[%s] ", RegisterVariables[LocalsQueue[index]->param]);
            }
            else
            {
                if( (int)pItem->Item.Data > 0 )
                    pStr += sprintf(pStr, "[EBP+%X] ", pItem->Item.Data);
                else
                    pStr += sprintf(pStr, "[EBP-%X] ", -(int)pItem->Item.Data);
            }

            pStr += PrintTypeName(pStr, &pItem->Item.Type);

            pStr += sprintf(pStr, "%s = ", substr(LocalsQueue[index]->pName, 0, strchr(LocalsQueue[index]->pName, ':') - LocalsQueue[index]->pName));

            pStr += PrintTypedValue(pStr, &pItem->Item.Type, &pItem->Item.pData);

            pStr += PrintTypedValueExpandCanonical(pStr, &pItem->Item.Type, &pItem->Item.pData);


            index--;                // Repeat until our index becomes 0
        }
//        else
            //break;                  // Could not add the list node
    }
#endif
