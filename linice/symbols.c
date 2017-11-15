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
//#include <linux/string.h>

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
extern void RecalculateDrawWindows();

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
                    if(dprinth(nLine++, " %08X %02d %s",
                        pGlobals->global[i].dwStartAddress,
                        pGlobals->global[i].bFlags,
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
    TSYMHEADER *pHead;                  // Generic section header

    // Search the current symbol table first, if loaded
    if( pIce->pSymTabCur )
    {
        ;
    }

    return(NULL);
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
*   DWORD SymLinNum2Address(DWORD line)                                       *
*                                                                             *
*******************************************************************************
*
*   Returns the offset of the code that corresponds to the given source line
*   number in the current source file. (Used with dot-line_number expression).
*
*   Where:
*       line is the line number
*   Returns:
*       address of the code that corresponds to that line number
*       0 if no address found, no source loaded, etc.
*
******************************************************************************/
DWORD SymLinNum2Address(DWORD line)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMFNLIN *pFnLin;
    WORD n, file_id;

    if( pIce->pSymTabCur && deb.pSource )
    {
        pHead = pIce->pSymTabCur->header;
        file_id = deb.pSource->file_id;

        while( pHead->hType != HTYPE__END )
        {
            // Scan all function line records to find our source/line number
            if( pHead->hType == HTYPE_FUNCTION_LINES )
            {
                pFnLin = (TSYMFNLIN *)pHead;

                for(n=0; n<pFnLin->nLines; n++)
                {
                    if( pFnLin->list[n].line==line && pFnLin->list[n].file_id==file_id )
                    {
                        return( pFnLin->dwStartAddress + pFnLin->list[n].offset );
                    }
                }
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   DWORD SymFnLinAddress2NextAddress(TSYMFNLIN *pFnLin, DWORD dwAddress)     *
*                                                                             *
*******************************************************************************
*
*   Given a function line descriptor and an address within that function,
*   return the address of the next line within that function.
*
*   Where:
*       pFnLin is a function line descriptor
*       dwAddress is the offset part of the address
*   Returns:
*       Address of the code from the next line within a function
*       End of function address if function fails in any ways
*
******************************************************************************/
DWORD SymFnLinAddress2NextAddress(TSYMFNLIN *pFnLin, DWORD dwAddress)
{
    int i;
    WORD wOffset;

    if( pFnLin )
    {
        // More checks that the address is right
        if( dwAddress>=pFnLin->dwStartAddress && dwAddress<=pFnLin->dwEndAddress )
        {
            wOffset = (WORD)((dwAddress - pFnLin->dwStartAddress) & 0xFFFF);

            // Traverse the function line array and find the next upper offset
            for(i=0; i<pFnLin->nLines; i++ )
            {
                if( pFnLin->list[i].offset > wOffset )
                {
                    return( pFnLin->list[i].offset + pFnLin->dwStartAddress );
                }
            }
        }
    }

    return( pFnLin->dwEndAddress );
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
                            return( pIce->pSymTabCur->pPriv->pStrings + pSrc->dLineArray[nLine-1] );
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
                    i--;                // Adjust - we overshoot a line
                    break;
                }
            }

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
                    return( pIce->pSymTabCur->pPriv->pStrings + pSrc->dLineArray[nLine-1] );
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

// TODO: Local register variables + dont search all, but only until the current eip fn offset

char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset)
{
    int i;
    char *pVar, *pType;
    static char sVar[MAX_SYMBOL_LEN+1]; // Temp store of the variable full name

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
*   BOOL SymEvalFnScope1(char *pBuf, TSYMFNSCOPE1 *pLocal)                    *
*                                                                             *
*******************************************************************************
*
*   Evaluates a single local variable into a string. The variable will be
*   computed based on the current register state in deb.r structure.
*
*   Where:
*       pBuf is the buffer to store the resulting string
*       pLocal is the address of the local variable descriptor
*   Returns:
*       TRUE string ok
*       FALSE result is not stored due to an error
*
******************************************************************************/
BOOL SymEvalFnScope1(char *pBuf, TSYMFNSCOPE1 *pLocal)
{
    int pos, i;
    char *pVar, *pType;                 // Pointer to a variable name
    static char sVar[MAX_SYMBOL_LEN+1]; // Temp store of the variable full name
    TADDRDESC Addr;                     // Variable address

    if( pBuf && pLocal )
    {
        // Print into a buffer the scope local variable in the proper format
        switch( pLocal->TokType )
        {
            case TOKTYPE_PARAM:
                pos = sprintf(pBuf, "[EBP+%X]", pLocal->p1);
                Addr.sel = deb.r->ss;
                Addr.offset = deb.r->ebp + pLocal->p1;
                break;

            case TOKTYPE_RSYM:
                pos = sprintf(pBuf, "[reg %X]", pLocal->p1);
                // Register variable - find out which register
                Addr.sel = __KERNEL_DS;     // Register file resides in the Ice' address space
                switch( pLocal->p1 )
                {
                    case 0:     Addr.offset = (DWORD) &deb.r->eax;  break;
                    case 1:     Addr.offset = (DWORD) &deb.r->ebx;  break;
                    case 2:     Addr.offset = (DWORD) &deb.r->ecx;  break;
                    case 3:     Addr.offset = (DWORD) &deb.r->edx;  break;
                    case 4:     Addr.offset = (DWORD) &deb.r->esi;  break;
                    case 5:     Addr.offset = (DWORD) &deb.r->edi;  break;
                    default:
                        // TODO: Take care of bad values in locals
                        Addr.offset = (DWORD) &deb.r->eax;          break; // Just something, kind-of...
                }
                break;

            case TOKTYPE_LSYM:
                pos = sprintf(pBuf, "[EBP-%X]", -pLocal->p1);
                Addr.sel = deb.r->ss;
                Addr.offset = deb.r->ebp + pLocal->p1;
                break;

            case TOKTYPE_LCSYM:
                pos = sprintf(pBuf, "[%08X]", pLocal->p1);
                Addr.sel = deb.r->ds;
                Addr.offset = pLocal->p1;
                break;

            case TOKTYPE_LBRAC:
            case TOKTYPE_RBRAC:
            default:
                // TODO: Some different code here??
                pos = sprintf(pBuf, "ERROR");
                return( TRUE );     // ???
        }

        // Print the variable name and the value
        pVar = pIce->pSymTabCur->pPriv->pStrings + pLocal->p2;

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

        // Test the final address of the variable to be valid
        if( AddrIsPresent(&Addr)==FALSE)
        {
            pos += sprintf(pBuf+pos, " %s = <invalid>", sVar);
        }
        else
        {
            pos += sprintf(pBuf+pos, " %s = %08X", sVar, GetDWORD(Addr.sel, Addr.offset));
        }

        return( TRUE );
    }

    return( FALSE );
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

//dprinth(1, "SetSymbolContext");

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

    // Depending on the current process context, set the appropriate pIce->pSymTabCur



    if( pIce->pSymTabCur )
    {
        // Set the current function scope based on the CS:EIP
        deb.pFnScope = SymAddress2FnScope(wSel, dwOffset);
//dprinth(1, "deb.pFnScope = %08X", deb.pFnScope);
        if( deb.pFnScope )
        {
            // Find the primary file ID for that function
            deb.pSource = SymTabFindSource(pIce->pSymTabCur, deb.pFnScope->file_id);
//dprinth(1, "deb.pSource = %08X", deb.pSource);
            // Set the current function line descriptor based on the current CS:EIP
            deb.pFnLin = SymAddress2FnLin(wSel, dwOffset);
//dprinth(1, "deb.pFnLin = %08X", deb.pFnLin);
            if( deb.pFnLin )
            {
                // See if we need to adjust the top of window line number - if the
                // current eip line is not within a window; this line does not have to be exact -
                // any line that covers a block of code within a function will do
                SymFnLin2Line(&wLine, deb.pFnLin, dwOffset);

                deb.codeFileEipLine = wLine;    // Store the EIP line number, 1-based
//dprinth(1, "wLine = %d, deb.codeFileTopLine = %d", wLine, deb.codeFileTopLine);
                // Adjust so the EIP line is visible
                if( (wLine < deb.codeFileTopLine) || (deb.codeFileTopLine+pWin->c.nLines-1 < wLine ) || (deb.codeFileTopLine==0) )
                {
                    deb.codeFileTopLine = wLine;
//dprinth(1, "deb.codeFileTopLine = %d", deb.codeFileTopLine);
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
