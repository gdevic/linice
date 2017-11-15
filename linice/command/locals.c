/******************************************************************************
*                                                                             *
*   Module:     locals.c                                                      *
*                                                                             *
*   Date:       01/18/2002                                                    *
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

static char *RegisterVariables[8] =
{
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"
};

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void ParseLocalSymbol(TExItem *item, TSYMFNSCOPE1 *pLocal);
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

    // Find where is our current EIP within the current function, then backtrack to the
    // beginning of the scope array for the bracket start "{"
    // Any closing brackets "}" will increment the scope indent counter so the
    // following variables will not be visible, until the scope indent count comes back
    // to the base level

    if( pFnScope && pFnScope->dwStartAddress<=dwEIP && pFnScope->dwEndAddress>=dwEIP )
    {
        // Scan forward for the matching code offset
        dwFnOffset = dwEIP - pFnScope->dwStartAddress;

        for(i=0; i<pFnScope->nTokens; i++ )
        {
            // Only LBRAC and RBRAC contain the function offset value in param, so we can compare to where we are
            if( pFnScope->list[i].TokType==TOKTYPE_LBRAC || pFnScope->list[i].TokType==TOKTYPE_RBRAC )
            {
                // Since the offsets are in increasing order, if our offset is already less than the
                // function scope offset, we assume we found (or overshoot) our place
                if( pFnScope->list[i].param < dwFnOffset )
                    continue;
                else
                {
                    if( pFnScope->list[i].param > dwFnOffset )
                        i--;

                    break;
                }
            }
        }

        // We have now index 'i' specifying where in the function scope array are we located
        // Need to start filling in the scope array by traversing backwards
        index  = 0;

        for(; i>=0 && index<MAX_LOCALS_QUEUE; i--)
        {
            // Take a variable only if the current block is visible; Add in only local variables
            if( pFnScope->list[i].TokType==TOKTYPE_PARAM || pFnScope->list[i].TokType==TOKTYPE_RSYM || pFnScope->list[i].TokType==TOKTYPE_LSYM )
            {
                // Store a local variable that is now visible
                LocalsQueue[index] = &pFnScope->list[i];
                index++;            // Increment the index into the locals queue
            }
            else
            {
                // If there is a scope nested block starting "}", find the end of the nesting thus prohibiting the
                // variables of that scope level(s) to be stored
                if( pFnScope->list[i].TokType==TOKTYPE_RBRAC )          // }
                {
                    nBlock = 0;

                    do
                    {
                        if( pFnScope->list[i].TokType==TOKTYPE_RBRAC )  // }
                            nBlock++;
                        else
                        if( pFnScope->list[i].TokType==TOKTYPE_LBRAC )  // {
                            nBlock--;
                        i--;

                    } while( i>0 && nBlock );

                    // Find the next { so we can start taking variables
                    while( i>=0 && pFnScope->list[i].TokType!=TOKTYPE_LBRAC )  i--;
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
    TLISTITEM *pItem;                   // List item that we are adding
    int index;                          // Running indicex into queue

    index = MAX_LOCALS_QUEUE;           // We will walk the list from back to front

    while( --index>=0 )
    {
        // Take only non-NULL items
        if( LocalsQueue[index] )
        {
            // Found an item in the queue, add it up to the list of locals

            if((pItem = ListAdd(&deb.Local)))
            {
                // Get the type descriptor into the new list node
                ParseLocalSymbol(
                    &pItem->Item,
                    LocalsQueue[index]);

                // Copy the variable name into the Name field
                strccpy(pItem->Name, LocalsQueue[index]->pName, ':');

                // Print the variable header, the offset from the EBP or the register variable

                if( LocalsQueue[index]->TokType==TOKTYPE_RSYM )
                {
                    sprintf(pItem->String, "[%s] ", RegisterVariables[LocalsQueue[index]->param & 7]);
                }
                else
                {
                    // It is either a parameter or a local variable - both are off the EBP

                    if( (signed)LocalsQueue[index]->param < 0 )
                        sprintf(pItem->String, "[EBP-%X] ", -(signed)LocalsQueue[index]->param);
                    else
                        sprintf(pItem->String, "[EBP+%X] ", LocalsQueue[index]->param);
                }

                PrettyPrintVariableName(pItem->String+strlen(pItem->String), pItem->Name, &pItem->Item.Type);
            }
        }
    }
}

/******************************************************************************
*                                                                             *
*   BOOL FillLocalScope(TLIST *List, TSYMFNSCOPE *pFnScope, DWORD dwEIP)      *
*                                                                             *
*******************************************************************************
*
*   Rebuilds the list of local variables.
*
*   Where:
*       List is the reference to a list to change
*       pFnScope is the function scope to search
*       dwEIP is the address within that function scope
*   Returns:
*       TRUE List of locals has been rebuilt
*       FALSE Invalid scope or offset
*
******************************************************************************/
BOOL FillLocalScope(TLIST *List, TSYMFNSCOPE *pFnScope, DWORD dwEIP)
{
    memset(&LocalsQueue, 0, sizeof(LocalsQueue));

    // Build the scope only if the function scope is valid
    if( pFnScope )
    {
        if( LocalBuildQueue(pFnScope, dwEIP) )
        {
            // Rebuild the list of locals based on the queue

            LocalBuildList();
        }
    }

    return( TRUE );
}

