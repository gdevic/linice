/******************************************************************************
*                                                                             *
*   Module:     stack.c                                                       *
*                                                                             *
*   Date:       01/14/2002                                                    *
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

        This module contans code to display stack window and stack data.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 01/14/02   Original                                             Goran Devic *
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

static char buf[MAX_STRING];            // Buffer to store lines to be written out

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void StackDraw(BOOL fForce)                                               *
*                                                                             *
*******************************************************************************
*
*   Draws stack frame
*
******************************************************************************/
void StackDraw(BOOL fForce)
{
    ListDraw(&deb.Stack, &pWin->s, fForce);
}

/******************************************************************************
*                                                                             *
*   BOOL FillStackList(DWORD dwEBP, DWORD dwEIP, BOOL fLocals)                *
*                                                                             *
*******************************************************************************
*
*   Rebuilds the stack list based on the given stack frame and code context.
*
*   Where:
*       dwEBP is the pointer to the stack frame
*       dwEIP is the code context
*       fLocals if TRUE, will list local symbols as well (NOT IMPLEMENTED YET)
*   Returns:
*       TRUE Stack list has been rebuilt
*
******************************************************************************/
BOOL FillStackList(DWORD dwEBP, DWORD dwEIP, BOOL fLocals)
{
    TLISTITEM *pItem;                   // List item that we are adding
    TADDRDESC Addr;                     // Address descriptor to use when fetching values
    UINT level = 0;                     // Stack frame level count
    int range;                          // Symbol offset range variable
    char *pBuf, *pName;                 // Temp pointer to the write out buffer and the name

    Addr.sel = deb.r->ss;
    Addr.offset = dwEBP;

    // Start at the current frame and dump all frames walking back (up the stack).
    //   | return address |  EBP + 4
    //   | saved EBP      |  EBP

    // We walk the number of levels until we hit an invalid pointer
    while( level++ < MAX_STACK_LEVELS )
    {
        // Get the original value of the EBP on the stack and the return address
        if( !VerifyRange(&Addr, 2 * sizeof(DWORD)) )
            break;

        // Print the current stack frame address of the RET pointer and where it points to
        pBuf = buf;                     // Reset the write pointer

        // Get the previous EBP from the current stack frame
        dwEBP = AddrGetDword(&Addr);

        // Get the return value from the current stack frame
        Addr.offset += 4;
        dwEIP = AddrGetDword(&Addr);
        Addr.offset -= 4;

        // If the EIP is not valid, stop the traversal because the stack frame
        // if probably not valid any more
        if( GlobalReadBYTE((BYTE *)&range, dwEIP)==FALSE )
            break;

        // Print the TOS context: return address and the function it points to
        pBuf += sprintf(pBuf, "%08X %08X", Addr.offset, dwEIP);

        // Find the closest symbol to the EIP that we found on the stack
        pName = SymAddress2Name(dwEIP, &range);

        // Print it if we found any
        if( pName )
            pBuf += sprintf(pBuf, "   %s+%X", pName, range);

        // Finally, add the string to the stack list
        if((pItem = ListAdd(&deb.Stack))==NULL)
            break;

        sprintf(pItem->String, "%s", buf);

        // Set the frame pointer to the next frame in the chain
        Addr.offset = dwEBP;
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdStack(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Stack command. It will always use DS register and assume SS==DS since that
*   is the case in Linux in kernel mode as well as user code.
*
******************************************************************************/
BOOL cmdStack(char *args, int subClass)
{
    DWORD dwEBP, dwEIP;                 // Values that we read from a stack frame
    BOOL fLocals = FALSE;               // By default, dont display locals

    dwEBP = deb.r->ebp;                 // Default EBP is the current one
    dwEIP = deb.r->eip;                 // And the EIP the same

    if( *args )
    {
        // Option -v will cause all locals to be displayed as well
        if( !strnicmp(args, "-v", 2) )
        {
            args += 2;
            fLocals = TRUE;

            // Skip spaces that may be after this option and before the next one
            while( *args==' ' ) args++;
        }
        // Optional address of a valid stack frame
        if( *args )
        {
            // Evaluate expression for the address portion after which should be nothing
            if( Expression(&dwEBP, args, &args) && !*args )
            {
                // We have a new starting EBP frame pointer, but since it has changed,
                // there is no way to know what EIP context we are in to be able to
                // find the information about locals.
                ;
            }
            else
            {
                PostError(ERR_SYNTAX, 0);   // Syntax error evaluating
                return( TRUE );
            }
        }
    }

    // Build the list of stack information

    ListDelAll(&deb.Stack);

    FillStackList(dwEBP, dwEIP, fLocals);

    StackDraw(TRUE);

    return( TRUE );
}

