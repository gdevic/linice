/******************************************************************************
*                                                                             *
*   Module:     context.c                                                     *
*                                                                             *
*   Date:       02/07/04                                                      *
*                                                                             *
*   Copyright (c) 2004 Goran Devic                                            *
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

        This module contains code for setting the context of the state.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 02/07/04   Largely copied from the scope.c                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern BOOL FillLocalScope(TSYMFNSCOPE *pFnScope, DWORD dwEIP);
extern void RecalculateWatch();


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   TSYMFNLIN *SymAddress2FnLin(WORD wSel, DWORD dwOffset)                    *
*                                                                             *
*******************************************************************************
*
*   Locates function line descriptor that contains the given address
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL if there are no functions containing that address
*       Pointer to a function line descriptor
*
******************************************************************************/
TSYMFNLIN *SymAddress2FnLin(WORD wSel, DWORD dwOffset)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMFNLIN *pFnLin;

    if( deb.pSymTabCur )
    {
        pHead = deb.pSymTabCur->header;

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == HTYPE_FUNCTION_LINES )
            {
                // Check if the address is inclusive
                pFnLin = (TSYMFNLIN *)pHead;
                if( dwOffset>=pFnLin->dwStartAddress && dwOffset<=pFnLin->dwEndAddress )
                    return( pFnLin );
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   char *SymFnLin2LineExact(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress)*
*                                                                             *
*******************************************************************************
*
*   Returns the the ASCIIZ source code line at that given address
*   as a part of the given function line descriptor. The address must exactly
*   match the requested address.
*
*   Where:
*       pLineNumber, if not NULL, will store line number there
*                    if NULL, ignored
*       pFnLin is the function line descriptor to use
*       dwAddress is the address to look up
*   Returns:
*       NULL if no line found at that location
*       Source code string
*
******************************************************************************/
char *SymFnLin2LineExact(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress)
{
    int i;
    WORD fileID;
    WORD nLine;
    WORD wOffset;
    TSYMSOURCE *pSrc;

    if( pFnLin )
    {
        // More checks that the address is right
        if( dwAddress>=pFnLin->dwStartAddress && dwAddress<=pFnLin->dwEndAddress )
        {
            wOffset = (WORD)((dwAddress - pFnLin->dwStartAddress) & 0xFFFF);

            // Traverse the function line array and find the right offset
            for(i=0; i<pFnLin->nLines; i++ )
            {
                if( pFnLin->list[i].offset==wOffset )
                {
                    // Get the file ID of the file that contains that source line
                    fileID = pFnLin->list[i].file_id;

                    // Find that file
                    pSrc = SymTabFindSource(deb.pSymTabCur, fileID);
                    if( pSrc )
                    {
                        nLine = pFnLin->list[i].line;

                        // Found it.. Store the optional line number
                        if( pLineNumber!=NULL )
                            *pLineNumber = nLine;

                        // Final check that the line number is not too large
                        if( nLine <= pSrc->nLines )
                        {
                            return( pSrc->pLineArray[nLine-1] );
                        }
                    }

                    return( NULL );
                }
            }
        }
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   char *SymFnLin2Line(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress)*
*                                                                             *
*******************************************************************************
*
*   Returns the the ASCIIZ source code line at that given address
*   as a part of the given function line descriptor. The address does not have
*   to exactly match the requested address, but has to be within the code
*   block that is defined by a source code line.
*
*   Where:
*       pLineNumber, if not NULL, will store line number there
*                    if NULL, ignored
*       pFnLin is the function line descriptor to use
*       dwAddress is the address to look up
*   Returns:
*       NULL if no line found at that location
*       Source code string
*
******************************************************************************/
char *SymFnLin2Line(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress)
{
    int i;
    WORD fileID;
    WORD nLine;
    WORD wOffset;
    TSYMSOURCE *pSrc;

    if( pFnLin )
    {
        // More checks that the address is right
        if( dwAddress>=pFnLin->dwStartAddress && dwAddress<=pFnLin->dwEndAddress )
        {
            wOffset = (WORD)((dwAddress - pFnLin->dwStartAddress) & 0xFFFF);

            // Traverse the function line array and find the right offset
            for(i=0; i<pFnLin->nLines-1; i++ )
            {
                // We use the fact that the offsets in the function line descriptor
                // array are always in the ascending order so we dont need to check
                // for the upper bound. Also, we are poised to find the line since
                // the offset is already checked to be within a function bounds
                if( pFnLin->list[i].offset==wOffset )
                {
                    break;
                }
                else
                if( pFnLin->list[i].offset > wOffset )
                {
                    if(i) i--;          // Adjust - we overshoot a line
                    break;              // (If we were still at 0, stay there)
                }
            }

            // Get the file ID of the file that contains that source line
            fileID = pFnLin->list[i].file_id;

            // Find that file
            pSrc = SymTabFindSource(deb.pSymTabCur, fileID);
            if( pSrc )
            {
                nLine = pFnLin->list[i].line;

                // Found it.. Store the optional line number
                if( pLineNumber!=NULL )
                    *pLineNumber = nLine;

                // Final check that the line number is not too large
                if( nLine <= pSrc->nLines )
                {
                    return( pSrc->pLineArray[nLine-1] );
                }
            }
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   TSYMTAB *Address2SymbolTable(WORD wSel, DWORD dwOffset)                   *
*                                                                             *
*******************************************************************************
*
*   Finds the symbol table with the function at the given address. This call
*   is used to auto-setup the symbol table context.
*
*   NOTE: This function modifies deb.pSymTabCur !
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL - function is not found at that address
*       address of the symbol table that owns this address
*
******************************************************************************/
TSYMTAB *Address2SymbolTable(WORD wSel, DWORD dwOffset)
{
    char *pComm;                        // Command line of the current process

    // Find the symbol table that contains a function from this offset,
    // loop for each symbol table in the list
    deb.pSymTabCur = deb.pSymTab;

    // We recognize two cases - kernel mode code and the user mode code
    if( wSel==GetKernelCS() )
    {
        // ------------------------------------------------------------------
        // This is a kernel code selector. The symbol table should describe
        // a module or a Linux kernel.
        // ------------------------------------------------------------------
        while( deb.pSymTabCur )
        {
            if( SymAddress2FnScope(wSel, dwOffset) )
            {
                // We found a function that is described in this symbol table
                // Return the address of the symbol table (already stored in deb)
                return( deb.pSymTabCur );
            }

            // Get to the next symbol table in the linked list of symbol tables
            deb.pSymTabCur = (TSYMTAB*) deb.pSymTabCur->next;
        };
    }
    else
    {
        // ------------------------------------------------------------------
        // This is a user level code. The symbol table should describe
        // the user application code.
        // ------------------------------------------------------------------
        // In addition to the function address, the process name has to match
        pComm = ice_get_current_comm();

        while( deb.pSymTabCur )
        {
            if( !stricmp(pComm, deb.pSymTabCur->sTableName) )
            {
                if( SymAddress2FnScope(wSel, dwOffset) )
                {
                    // We found a function that is described in this symbol table
                    // Return the address of the symbol table (already stored in deb)
                    return( deb.pSymTabCur );
                }
            }

            // Get to the next symbol table in the linked list of symbol tables
            deb.pSymTabCur = (TSYMTAB*) deb.pSymTabCur->next;
        };
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   TSYMFNSCOPE *SymAddress2FnScope(WORD wSel, DWORD dwOffset)                *
*                                                                             *
*******************************************************************************
*
*   Returns the function scope descriptor for a given address
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL - function is not found at that address
*       address of the function scope descriptor
*
******************************************************************************/
TSYMFNSCOPE *SymAddress2FnScope(WORD wSel, DWORD dwOffset)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMFNSCOPE *pFnScope;              // Function scope header type

    // Find the function scope descriptor that contains the given address
    if( deb.pSymTabCur )
    {
        pHead = deb.pSymTabCur->header;

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == HTYPE_FUNCTION_SCOPE )
            {
                // Check if the address is inclusive
                pFnScope = (TSYMFNSCOPE *)pHead;
                if( pFnScope->dwStartAddress<=dwOffset && pFnScope->dwEndAddress>=dwOffset )
                    return( pFnScope );
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void SetSymbolContext(WORD wSel, DWORD dwOffset)                          *
*                                                                             *
*******************************************************************************
*
*   This function sets some of the major pointers and values for dealing
*   with the symbol files and their context for a given address.
*
******************************************************************************/
void SetSymbolContext(WORD wSel, DWORD dwOffset)
{
    WORD wLine;

    // Reset all context state variables so we start from the clean state
    // and build upon this for whatever context extent we can

    deb.pFnScope = NULL;
    deb.pFnLin = NULL;
    deb.pSource = NULL;
    deb.pSrcEipLine = NULL;
    deb.codeFileTopLine = 0;
    deb.codeFileXoffset = 0;

    // Adjust the code window's machine code top address:
    // We let the cs:eip free flow within the code window that is bound by
    // codeTopAddr and codeBottomAddr, and adjust the first variable when
    // the cs:eip is out of bounds

    if( deb.codeBottomAddr.sel==0 )     // Shortcut for non-initialized bounds
    {
        // If the bounds had not been initialized, reset the current to the top line
        deb.codeTopAddr.sel = wSel;
        deb.codeTopAddr.offset = dwOffset;
    }
    else
    {
        // Check that the cs:eip is still within the bounds
        if( !(wSel==deb.codeTopAddr.sel && dwOffset>=deb.codeTopAddr.offset && dwOffset<=deb.codeBottomAddr.offset) )
        {
            // Again reset the top if the new cs:eip is out of bounds
            deb.codeTopAddr.sel = wSel;
            deb.codeTopAddr.offset = dwOffset;
        }
    }

    // Depending on the current process context, set the appropriate deb.pSymTabCur
    // if the user requested table autoon feature (which is on by default)

    if( deb.fTableAutoOn )
        Address2SymbolTable(wSel, dwOffset);

    if( deb.pSymTabCur )
    {
        // Set the current function scope based on the CS:EIP
        deb.pFnScope = SymAddress2FnScope(wSel, dwOffset);

        if( deb.pFnScope )
        {
            // Find the primary file ID for that function
            deb.pSource = SymTabFindSource(deb.pSymTabCur, deb.pFnScope->file_id);

            // Set the current function line descriptor based on the current CS:EIP
            deb.pFnLin = SymAddress2FnLin(wSel, dwOffset);

            // Set the array of local scope variables (only if the function scope is valid)
            FillLocalScope(deb.pFnScope, deb.r->eip);

            if( deb.pFnLin )
            {
                // See if we need to adjust the top of window line number - if the
                // current eip line is not within a window; this line does not have to be exact -
                // any line that covers a block of code within a function will do
                deb.pSrcEipLine = SymFnLin2Line(&wLine, deb.pFnLin, dwOffset);

                deb.codeFileEipLine = wLine;    // Store the EIP line number, 1-based

                // Adjust so the EIP line is visible
                if( (wLine < deb.codeFileTopLine) || (deb.codeFileTopLine+pWin->c.nLines-2 < wLine ) || (deb.codeFileTopLine==0) )
                {
                    deb.codeFileTopLine = wLine;

                    // If we positioned EIP line on the top, it looks better if we display
                    // one extra line before it
                    if( pWin->c.nLines>2 && deb.codeFileEipLine==deb.codeFileTopLine && deb.codeFileTopLine>1)
                        deb.codeFileTopLine -= 1;
                }
            }
        }
    }

    // Recalculate watch expressions that depend on context

    RecalculateWatch();
}
