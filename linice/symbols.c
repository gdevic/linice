/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       10/21/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

        This module contains code for symbols access from the symbol table

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/21/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include <asm/uaccess.h>                // User space memory access functions
#include <linux/string.h>

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

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
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType);

/******************************************************************************
*                                                                             *
*   BOOL cmdSymbol(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display global symbols from the current symbol table
*
******************************************************************************/
BOOL cmdSymbol(char *args, int subClass)
{
    TSYMGLOBAL *pGlobals;
    int nLine = 1;
    int i;

    if( pIce->pSymTabCur )
    {
        if( *args )
        {
            ;
        }
        else
        {
            // If no arguments, list all symbols:
            // Symbols are: Globals + static symbols from all files

            pGlobals = (TSYMGLOBAL *) SymTabFindSection(pIce->pSymTabCur, HTYPE_GLOBALS);
            if( pGlobals )
            {
                for(i=0; i<pGlobals->nGlobals; i++ )
                {
                    if(dprinth(nLine++, " %08X %c %s",
                        pGlobals->global[i].dwStartAddress,
                        (pGlobals->global[i].bFlags & 1)==0x01 ? 'D' : 'C',
                        pIce->pSymTabCur->pPriv->pStrings + pGlobals->global[i].dName)==FALSE)
                    return( TRUE );
                }
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTypes(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display type information from the current symbol table
*
******************************************************************************/
BOOL cmdTypes(char *args, int subClass)
{
    char buf[MAX_STRING+1];             // Temp string for output
    TSYMTYPEDEF *pTypes;
    int nLine = 1;
    int i;

    if( pIce->pSymTabCur )
    {
        if( *args )
        {
            ;
        }
        else
        {
            // If no arguments, list all types:

            pTypes = (TSYMTYPEDEF *) SymTabFindSection(pIce->pSymTabCur, HTYPE_TYPEDEF);
            if( pTypes )
            {
                for(i=0; i<pTypes->nTypedefs; i++ )
                {
                    // Since type definitions are really long strings, we would
                    // fault if they overflow our buffer, so we copy them in a temp
                    // buffer before printing

                    strncpy(buf, pIce->pSymTabCur->pPriv->pStrings + pTypes->list[i].dName, 32);
                    strcat(buf, " = ");
                    strncat(buf, pIce->pSymTabCur->pPriv->pStrings + pTypes->list[i].dDef, MAX_STRING-32);

                    if(dprinth(nLine++, "%s", buf)==FALSE)
                        break;
                }
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdFile(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Change or display the current source file.
*
*   Symtax:
*       FILE *          - lists all symbol files in the current symbol table
*       FILE file-name  - switches to a given source file
*       FILE            - displays the name of the current source file
*
******************************************************************************/
BOOL cmdFile(char *args, int subClass)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMSOURCE *pSrc;                   // Source file header
//  WORD fileID;
    int nLine = 1;

    if( pIce->pSymTabCur )
    {
        if( *args )
        {
            if( *args=='*' )
            {
                // Print the current source file name

                pHead = pIce->pSymTabCur->header;

                while( pHead->hType != HTYPE__END )
                {
                    if( pHead->hType == HTYPE_SOURCE )
                    {
                        pSrc = (TSYMSOURCE *)pHead;

                        // Print the source file path/name
                        if( (dprinth(nLine++, "%s", pIce->pSymTabCur->pPriv->pStrings + pSrc->dSourcePath))==FALSE )
                            break;
                    }

                    pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
                }
            }
            else
            {
                // Switch to a different source file

                // Loop over all the source files and compare to what we typed
                pHead = pIce->pSymTabCur->header;

                while( pHead->hType != HTYPE__END )
                {
                    if( pHead->hType == HTYPE_SOURCE )
                    {
                        pSrc = (TSYMSOURCE *)pHead;

                        if( !strcmp(args, pIce->pSymTabCur->pPriv->pStrings + pSrc->dSourceName) )
                        {
                            deb.pSource = pSrc;             // New source descriptor
                            deb.codeFileTopLine = 1;        // Display at the first source line
                            deb.codeFileXoffset = 0;        // Reset the X offset

                            RecalculateDrawWindows();

                            return( TRUE );
                        }
                    }

                    pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
                }

                dprinth(nLine++, "Source file '%s' not found", args);
            }
        }
        else
        {
            // No arguments given - display the current source file name

            if( deb.pSource )
            {
                dprinth(nLine++, "%s", pIce->pSymTabCur->pPriv->pStrings + deb.pSource->dSourcePath);
            }
        }
    }
    else
        dprinth(0, "No symbol table loaded.");

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL SymbolName2Value(TSYMTAB *pSymTab, DWORD *pValue, char *name)        *
*                                                                             *
*******************************************************************************
*
*   Translates symbol name to its value from a given symbol table
*
*   Where:
*       pSymTab is the symbol table to look
*       pValue is the address of the variable to receive the value DWORD
*       name is the symbol name
*   Returns:
*       FALSE - symbol name not found
*       TRUE - *pValue is set with the symbol value
*
******************************************************************************/
BOOL SymbolName2Value(TSYMTAB *pSymTab, DWORD *pValue, char *name)
{
//    TSYMHEADER *pHead;                  // Generic section header
    DWORD count;

    TSYMGLOBAL  *pGlobals;              // Globals section pointer
    TSYMGLOBAL1 *pGlobal;               // Single global item

//    TSYMFNLIN   *pFnLin;                // Function lines section pointer
//    TSYMFNSCOPE *pFnScope;              // Function scope section pointer
//    TSYMSTATIC  *pStatic;               // Static symbols section pointer
//    TSYMSTATIC1 *pStatic1;              // Single static item

    // Search in this order:
    //  * Local scope variables
    //  * Current file static variables
    //  * Global variables
    //  * Function names

    if( pSymTab )
    {



        // Global symbols

        pGlobals = (TSYMGLOBAL *) SymTabFindSection(pSymTab, HTYPE_GLOBALS);
        if( pGlobals )
        {
            // Search global symbols for a specified symbol name

            pGlobal  = &pGlobals->global[0];

            for(count=0; count<pGlobals->nGlobals; count++, pGlobal++ )
            {
                if( !strcmp(pSymTab->pPriv->pStrings + pGlobal->dName, name) )
                {
                    *pValue = pGlobal->dwStartAddress;

                    return( TRUE );
                }
            }
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   char *SymAddress2Name(WORD wSel, DWORD dwOffset)                          *
*                                                                             *
*******************************************************************************
*
*   Translates address to a symbol name
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL if there are no symbols at that address
*       Address of the symbol name string
*
******************************************************************************/
char *SymAddress2Name(WORD wSel, DWORD dwOffset)
{
    return( "---symbol---" );
}

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

    if( pIce->pSymTabCur )
    {
        pHead = pIce->pSymTabCur->header;

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == HTYPE_FUNCTION_LINES )
            {
                // Check if the address is inclusive
                pFnLin = (TSYMFNLIN *)pHead;
                if( pFnLin->dwStartAddress<=dwOffset && pFnLin->dwEndAddress>=dwOffset )
                    return( pFnLin );
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
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
*   as a part of the given function line descriptor
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
        if( pFnLin->dwStartAddress<=dwAddress && pFnLin->dwEndAddress>=dwAddress )
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
                    pSrc = SymTabFindSource(pIce->pSymTabCur, fileID);
                    if( pSrc )
                    {
                        nLine = pFnLin->list[i].line;

                        // Found it.. Store the optional line number
                        if( pLineNumber!=NULL )
                            *pLineNumber = nLine;

                        // Final check that the line number is not too large
                        if( nLine <= pSrc->nLines )
                        {
                            return( pIce->pSymTabCur->pPriv->pStrings + pSrc->dLineArray[nLine] );
                        }
                    }
                }
            }
        }
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset)                  *
*                                                                             *
*******************************************************************************
*
*   Returns the ASCIIZ name of the function at the given address
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL is no function can be found at that address
*       Function name string
*
******************************************************************************/
char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset)
{
    TSYMGLOBAL *pGlobals;
//  TSYMFNSCOPE *pFnScope;
    int i;

    if( pIce->pSymTabCur )
    {
        // Search global functions
        pGlobals = (TSYMGLOBAL *)SymTabFindSection(pIce->pSymTabCur, HTYPE_GLOBALS);

        for(i=0; i<pGlobals->nGlobals; i++ )
        {
            if( pGlobals->global[i].dwStartAddress==dwOffset )
                return( pIce->pSymTabCur->pPriv->pStrings + pGlobals->global[i].dName );
        }

#if 0
        // Search the function scopes next
        pFnScope = SymAddress2FnScope(wSel, dwOffset);
        if( pFnScope )
        {
            return( pIce->pSymTabCur->pPriv->pStrings + pFnScope->dName );
        }
#endif
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
    if( pIce->pSymTabCur )
    {
        pHead = pIce->pSymTabCur->header;

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
*   char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset)            *
*                                                                             *
*******************************************************************************
*
*   Returns the name of the local variable or argument addressed using the
*   EBP register within a given function scope.
*
*   Note: Since this function is used only to get the variable name, and not
*         the type information, all type info will be stripped from the name
*
*   Where:
*       pFnScope is the function scope
*       ebpOffset is the offset used in the addressing
*   Returns:
*       Variable name
*       NULL
*
******************************************************************************/
char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset)
{
    int i;
    char *pVar, *pType;
    static char sVar[MAX_SYMBOL_LEN+1];

    if( pFnScope )
    {
        // Traverse the function scope tokens and search for the ebp offset
        for(i=0; i<pFnScope->nTokens; i++ )
        {
            if( pFnScope->list[i].TokType==TOKTYPE_PARAM || pFnScope->list[i].TokType==TOKTYPE_LSYM )
            {
                if( pFnScope->list[i].p1==ebpOffset )
                {
                    pVar = pIce->pSymTabCur->pPriv->pStrings + pFnScope->list[i].p2;

                    // Since that variable name contains the type description, we
                    // need to copy only the name away

                    pType = strchr(pVar, ':');

                    if( pType )
                        i = MIN(pType-pVar, MAX_SYMBOL_LEN-1);
                    else
                        // This is just in case that we did not find the type delimiter ':'
                        i = MIN(strlen(pVar), MAX_SYMBOL_LEN-1);

                    // Copy the variable name portion of its definition
                    strncpy(sVar, pVar, i);
                    sVar[i] = 0;

                    return( sVar );
                }
            }
        }
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   void SetCurrentSymbolContext()                                            *
*                                                                             *
*******************************************************************************
*
*   This function sets some of the major pointers and values for dealing
*   with the symbol files upon entering the debugger.
*
******************************************************************************/
void SetCurrentSymbolContext()
{
    WORD wLine;

    // Adjust the code window's machine code top address:
    // We let the cs:eip free flow within the code window that is bound by
    // codeTopAddr and codeBottomAddr, and adjust the first variable when
    // the cs:eip is out of bounds

    if( deb.codeBottomAddr.sel==0 )     // Shortcut for non-initialized bounds
    {
        // If the bounds had not been initialized, reset the current to the top line
        deb.codeTopAddr.sel = deb.r->cs;
        deb.codeTopAddr.offset = deb.r->eip;
    }
    else
    {
        // Check that the cs:eip is still within the bounds
        if( !(deb.r->cs==deb.codeTopAddr.sel && deb.r->eip>=deb.codeTopAddr.offset && deb.r->eip<=deb.codeBottomAddr.offset) )
        {
            // Again reset the top if the new cs:eip is out of bounds
            deb.codeTopAddr.sel = deb.r->cs;
            deb.codeTopAddr.offset = deb.r->eip;
        }
    }

    // Depending on the current process context, set the appropriate pIce->pSymTabCur



    if( pIce->pSymTabCur )
    {
        // Set the current function scope based on the CS:EIP
        deb.pFnScope = SymAddress2FnScope(deb.r->cs, deb.r->eip);

        if( deb.pFnScope )
        {
            // Find the primary file ID for that function
            deb.pSource = SymTabFindSource(pIce->pSymTabCur, deb.pFnScope->file_id);

            // Set the current function line descriptor based on the current CS:EIP
            deb.pFnLin = SymAddress2FnLin(deb.r->cs, deb.r->eip);

            if( deb.pFnLin )
            {
                // See if we need to adjust the top of window line number - if the
                // current eip line is not within a window
                SymFnLin2Line(&wLine, deb.pFnLin, deb.r->eip);

                deb.codeFileEipLine = wLine;    // Store the EIP line number, 1-based

                // Adjust so the EIP line is visible
                if( (wLine < deb.codeFileTopLine) || (deb.codeFileTopLine+pWin->c.nLines-1 < wLine ) )
                {
                    deb.codeFileTopLine = wLine;

                    // If we positioned EIP line on the top, it looks better if we display
                    // one extra line before it
                    if( pWin->c.nLines>2 && deb.codeFileEipLine==deb.codeFileTopLine && deb.codeFileTopLine>1)
                        deb.codeFileTopLine -= 1;
                }

                return;
            }
        }
    }

    // Something failed.. Reset all variables and default to machine code disassembly

    deb.pFnScope = NULL;
    deb.pFnLin = NULL;
    deb.pSource = NULL;
    deb.codeFileTopLine = 0;
    deb.codeFileXoffset = 0;
}
