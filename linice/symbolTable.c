/******************************************************************************
*                                                                             *
*   Module:     symbolTable.c                                                 *
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

        This module contains code for symbol table management

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

#include "module-header.h"              // Include types commonly defined for a module

#include "ice-version.h"                // Include version file

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "errno.h"                      // Include kernel error numbers
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

extern BYTE *ice_malloc(DWORD size);
extern void ice_free(BYTE *p);
extern void ice_free_heap(BYTE *pHeap);
extern BOOL FindModule(TMODULE *pMod, char *pName, int nNameLen);

BOOL SymbolTableRemove(char *pTableName, TSYMTAB *pRemove);
void SymTabRelocate(TSYMTAB *pSymTab, int pReloc[], int factor);
void SymTabMakePointers(TSYMTAB *pSymTab, DWORD dStrings);


/******************************************************************************
*                                                                             *
*   int CheckSymtab(TSYMTAB *pSymtab)                                         *
*                                                                             *
*******************************************************************************
*
*   Checks the consistency of the symbol table structrure
*
*   Returns: Numerical value of erroreous field (look source) or 0 if ok
*
******************************************************************************/

int CheckSymtab(TSYMTAB *pSymtab)
{

    return(0);
}


/******************************************************************************
*                                                                             *
*   int UserAddSymbolTable(void *pSymUser)                                    *
*                                                                             *
*******************************************************************************
*
*   Adds a symbol table from the header.
*
*   Where:
*       pSymUser is the address of the symbol table header in the user space
*
*   Returns:
*       0 init ok
*       -EINVAL Bad symbol table
*       -EFAULT general failure
*       -ENOMEM not enough memory
*
******************************************************************************/
int UserAddSymbolTable(void *pSymUser)
{
    int retval = -EINVAL;
    TSYMTAB SymHeader;
    TSYMTAB *pSymTab;                   // Symbol table in debugger buffer
    TSYMPRIV *pPriv;
    DWORD dStrings;                     // Offset to strings (to adjust)
    BYTE *pInitFunctionOffset;
    DWORD dwInitFunctionSymbol;
    DWORD dwDataReloc;
    TSYMRELOC  *pReloc;                 // Symbol table relocation header
    TMODULE Mod;                        // Module information structure


    // Copy the header of the symbol table into the local structur to examine it
    if( ice_copy_from_user(&SymHeader, pSymUser, sizeof(TSYMTAB))==0 )
    {
        // Check if we should allocate memory for the symbol table from the allowed pool
        if( deb.nSymbolBufferAvail >= SymHeader.dwSize )
        {
            // Allocate memory for complete symbol table from the private pool
            pSymTab = (TSYMTAB *) ice_malloc((unsigned int)SymHeader.dwSize);
            if( pSymTab )
            {
                INFO(("Allocated %d bytes at %X for symbol table\n", (int) SymHeader.dwSize, (int) pSymTab));

                // Allocate memory for the private section of the symbol table
                pPriv = (TSYMPRIV *) ice_malloc(sizeof(TSYMPRIV));
                if( pPriv )
                {
                    INFO(("Allocated %d bytes for private symbol table structure\n", sizeof(TSYMPRIV) ));
                    memset(pPriv, 0, sizeof(TSYMPRIV));

                    // Copy the complete symbol table from the use space
                    if( ice_copy_from_user(pSymTab, pSymUser, SymHeader.dwSize)==0 )
                    {
                        // Make sure we are really loading a symbol table

                        // TODO: Here we also want to check the CRC or something like that for a table being loaded

                        if( !strcmp(pSymTab->sSig, SYMSIG) )
                        {
                            // Compare the symbol table version - major number has to match
                            if( (pSymTab->Version>>8)==(SYMVER>>8) )
                            {
                                // Call the remove function that will remove this particular
                                // symbol table if we are reloading it
                                SymbolTableRemove(pSymTab->sTableName, NULL);

                                dprinth(1, "Loaded symbols for module `%s' size %d (ver %d.%d)",
                                    pSymTab->sTableName, pSymTab->dwSize, pSymTab->Version>>8, pSymTab->Version&0xFF);

                                // Link the private data structure to the symbol table
                                pSymTab->pPriv = pPriv;

                                deb.nSymbolBufferAvail -= SymHeader.dwSize;

                                // Link this symbol table in the linked list and also make it current
                                pSymTab->pPriv->next = (struct TSYMTAB *) deb.pSymTab;

                                deb.pSymTab = pSymTab;
                                deb.pSymTabCur = deb.pSymTab;

                                // Initialize elements of the private symbol table structure

                                dStrings = (DWORD) pSymTab + pSymTab->dStrings;

                                // Relocate all strings within this symbol table to pointers (from offsets)

                                SymTabMakePointers(deb.pSymTabCur, dStrings);

                                // If the symbol table being loaded describes a kernel module, we need to
                                // see if that module is already loaded, and if so, relocate its symbols

                                // Try to find if a module with that symbol name has already been loaded
                                if( FindModule(&Mod, pSymTab->sTableName, strlen(pSymTab->sTableName)) )
                                {
                                    // Module is already loaded - we need to relocate symbol table based on it

                                    // Get the offset of the init_module global symbol from this symbol table
                                    if( SymbolName2Value(pSymTab, &dwInitFunctionSymbol, "init_module", 11) )
                                    {
                                        dprinth(1, "Relocating symbols for `%s'", pSymTab->sTableName);

                                        // Details of relocation scheme are explained in ParseReloc.c file

                                        // --- relocating code section ---

                                        // Get the real kernel address of the init_module function after relocation
                                        pInitFunctionOffset = (BYTE *) Mod.init;

                                        // Store private reloc adjustment value for code section
                                        pSymTab->pPriv->reloc[0] = (int)(pInitFunctionOffset - dwInitFunctionSymbol);

                                        // --- relocating data section ---

                                        // Find the symbol table relocation block
                                        pReloc = (TSYMRELOC *) SymTabFindSection(pSymTab, HTYPE_RELOC);
                                        if( pReloc )
                                        {
                                            int i;

                                            for(i=1; i<MAX_SYMRELOC; i++)
                                            {
                                                // Find the address within the code segment from which we will read the offset to
                                                // our data. Relocation block contains the relative offset from the init_module function
                                                // to our dword sample that we need to take.
                                                if( pReloc->list[i].refFixup )
                                                {
                                                    dwDataReloc = *(DWORD *) ((DWORD) Mod.init + pReloc->list[i].refFixup);
                                                    pSymTab->pPriv->reloc[i] = dwDataReloc - pReloc->list[i].refOffset;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // There was not a single global variable to use for relocation. Odd, but
                                            // possible... In that case it does not really matter not to relocate data...

                                            dprinth(1, "Symbol table missing HTYPE_RELOC");
                                        }

                                        // Relocate symbol table by the required offset
                                        SymTabRelocate(pSymTab, pSymTab->pPriv->reloc, 1);
                                    }
                                }

                                // Return OK
                                return( 0 );
                            }
                            else
                            {
                                dprinth(1, "Error: Symbol table has incompatible version number!");
                            }
                        }
                        else
                        {
                            dprinth(1, "Invalid symbol table signature!");
                        }
                    }
                    else
                    {
                        ERROR(("Error copying symbol table"));
                        retval = -EFAULT;
                    }

                    // Deallocate memory for the private symbol table structure
                    ice_free_heap((void *) pPriv);
                }
                else
                {
                    ERROR(("Unable to allocate %d for private symbol table structure!\n", sizeof(TSYMPRIV)));
                    retval = -ENOMEM;
                }

                // Deallocate memory for symbol table
                ice_free_heap((void *) pSymTab);
            }
            else
            {
                ERROR(("Unable to allocate %d for symbol table!\n", (int) SymHeader.dwSize));
                retval = -ENOMEM;
            }
        }
        else
        {
            dprinth(1, "Symbol table memory pool too small to load this table!");
            retval = -ENOMEM;
        }
    }
    else
    {
        ERROR(("Invalid IOCTL packet address\n"));
        retval = -EFAULT;
    }

    return( retval );
}


/******************************************************************************
*                                                                             *
*   int UserRemoveSymbolTable(void *pSymtab)                                  *
*                                                                             *
*******************************************************************************
*
*   Removes a symbol table from the debugger.
*
*   Where:
*       pSymtab is the address of the symbol table name string in the user space
*
*   Returns:
*       0 remove ok
*       -EINVAL Invalid symbol table name; nonexistent table
*       -EFAULT general failure
*
******************************************************************************/
int UserRemoveSymbolTable(void *pSymtab)
{
    int retval = 0;                     // Assume no error
    char sName[MAX_MODULE_NAME];

    // Copy the symbol table name from the user address space
    if( ice_copy_from_user(sName, pSymtab, MAX_MODULE_NAME)==0 )
    {
        // Make sure the string name is zero-terminated
        sName[MAX_MODULE_NAME-1] = 0;

        // Search for the symbol table with that specific table name and remove it
        if( SymbolTableRemove(sName, NULL)==FALSE )
            retval = -EINVAL;
    }
    else
    {
        ERROR(("Invalid IOCTL packet address\n"));
        retval = -EFAULT;
    }

    return(retval);
}

/******************************************************************************
*                                                                             *
*   BOOL SymbolTableRemove(char *pTableName, TSYMTAB *pRemove)                *
*                                                                             *
*******************************************************************************
*
*   Low level function to remove a symbol table from the debugger.
*
*   Where:
*       pTable name is the name string. It can be NULL, in which case...
*       pRemove address of the table to be removed
*
*   Returns:
*       TRUE - table found & removed
*       FALSE - table not found
*
******************************************************************************/
BOOL SymbolTableRemove(char *pTableName, TSYMTAB *pRemove)
{
    TSYMTAB *pSym, *pPrev = NULL;
    BOOL fMatch = FALSE;

    // Traverse the linked list of symbol tables and find which one matched
    pSym = deb.pSymTab;
    while( pSym )
    {
        if( pTableName!=NULL )
        {
            // Compare the name of the symbol table to be removed (case insensitive)
            if( !stricmp(pSym->sTableName, pTableName) )
                fMatch = TRUE;
        }
        else
            if( pRemove==pSym )
                fMatch = TRUE;

        if( fMatch )
        {
            dprinth(1, "Removing symbol table `%s'", pSym->sTableName);

            // Found it - unlink, free and return success
            if( pSym==deb.pSymTab )
                deb.pSymTab = (TSYMTAB *) pSym->pPriv->next;     // First in the linked list
            else
                pPrev->pPriv->next = pSym->pPriv->next;       // Not the first in the linked list

            // Add the memory block to the free pool
            deb.nSymbolBufferAvail += pSym->dwSize;

            // Release the private symbol table data
            ice_free((BYTE *)pSym->pPriv);

            // Release the symbol table itself
            ice_free((BYTE *)pSym);

            // Leave no dangling pointers...
            deb.pSymTabCur = deb.pSymTab;

            //------------------------------------------------------------------
            // Symbol table dependency:
            // Find all the pointers that were addressing elements within this
            // table and zero them so they are not dangling
            //------------------------------------------------------------------
            // The best way to do that is to recalculate context and repaint
            SetSymbolContext(deb.r->cs, deb.r->eip);

            deb.fRedraw = TRUE;

            return(TRUE);
        }

        pPrev = pSym;
        pSym = (TSYMTAB *) pSym->pPriv->next;
    }

    return(FALSE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTable(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display or set current symbol table. Table name may be specified --
*   partial name would suffice.
*
*   Optional argument 'R' removes a specified table (or all tables if 'R *')
*
******************************************************************************/
BOOL cmdTable(char *args, int subClass)
{
    TSYMTAB *pSymTab = deb.pSymTab;
    int nLine = 2;

    if( pSymTab==NULL )
    {
        dprinth(1, "No symbol table loaded.");
    }
    else
    {
        // If we specified a table name, find that table and set it as current
        if( *args )
        {
            // Remove a symbol table?
            if( toupper(*args)=='R' && *(args+1)==' ' )
            {
                // Remove a symbol table whose name is given (or '*' for all)
                args += 2;
                while(*args==' ') args++;

                if( *args=='*' )
                {
                    // Remove all symbol tables

                    dprinth(nLine++, "Removing all symbol tables...");

                    while( deb.pSymTab )
                    {
                        SymbolTableRemove(NULL, deb.pSymTab);
                    }
                }
                else
                {
                    // Remove a particular symbol table given by the name

                    if( SymbolTableRemove(args, NULL)==FALSE )
                        dprinth(1, "Nonexisting symbol table `%s'", args);

                    return(TRUE);
                }
            }
            else
            {
                // Set current symbol table

                while( pSymTab )
                {
                    // Compare internal table name to our argument string
                    if( !stricmp(pSymTab->sTableName, args) )
                    {
                        deb.pSymTabCur = pSymTab;

                        return(TRUE);
                    }

                    pSymTab = (TSYMTAB *) pSymTab->pPriv->next;
                }

                dprinth(1, "Nonexisting symbol table `%s'", args);

                return(TRUE);
            }
        }
        else
        {
            // No arguments - List all symbol tables that are loaded

            dprinth(1, "Symbol tables:");
            while( pSymTab )
            {
                // Print a symbol table name. If a table is current, highlight it.
                dprinth(nLine++, " %c%c%6d  %s",
                    DP_SETCOLINDEX, pSymTab==deb.pSymTabCur? COL_BOLD:COL_NORMAL,
                    pSymTab->dwSize, pSymTab->sTableName);
                pSymTab = (TSYMTAB *) pSymTab->pPriv->next;
            }
        }
    }

    dprinth(nLine, "%d bytes of symbol table memory available", deb.nSymbolBufferAvail);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   TSYMTAB *SymTabFind(char *name)                                           *
*                                                                             *
*******************************************************************************
*
*   Searches for the named symbol table.
*
*   Where:
*       name is the internal symbol table name
*
*   Returns:
*       Pointer to a symbol table
*       NULL if the symbol table of that name is not loaded
*
******************************************************************************/
TSYMTAB *SymTabFind(char *name)
{
    TSYMTAB *pSymTab = deb.pSymTab;   // Pointer to all symbol tables

    // Make sure name is given as a valid pointer
    if( name!=NULL )
    {
        while( pSymTab )
        {
            if( !strcmp(pSymTab->sTableName, name) )
                return( pSymTab );

            pSymTab = (TSYMTAB *) pSymTab->pPriv->next;
        }
    }

    return(NULL);
}


/******************************************************************************
*                                                                             *
*   void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType)                     *
*                                                                             *
*******************************************************************************
*
*   Searches for the named section within the given symbol table.
*
*   Where:
*       pSymTab is the address of the symbol table to search
*       hType is the section number
*
*   Returns:
*       Pointer to a section header
*       NULL if the section can not be located
*
******************************************************************************/
void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType)
{
    TSYMHEADER *pHead;                  // Generic section header

    if( pSymTab )
    {
        pHead = pSymTab->header;

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == hType )
                return( (void *)pHead );

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return(NULL);
}


/******************************************************************************
*                                                                             *
*   TSYMSOURCE *SymTabFindSource(TSYMTAB *pSymTab, WORD fileID)               *
*                                                                             *
*******************************************************************************
*
*   Searches for the fileID descriptor of the given file id number
*
*   Where:
*       pSymTab is the address of the symbol table to search
*       fileID is the file ID to search for
*
*   Returns:
*       Pointer to a source descriptor
*       NULL if the file id descriptor can not be located
*
******************************************************************************/
TSYMSOURCE *SymTabFindSource(TSYMTAB *pSymTab, WORD fileID)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMSOURCE *pSource;

    if( pSymTab )
    {
        pHead = pSymTab->header;

        while( pHead->hType != HTYPE__END )
        {
            if( (pHead->hType == HTYPE_SOURCE) )
            {
                pSource = (TSYMSOURCE *)pHead;

                if( pSource->file_id==fileID )
                    return( pSource );
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void SymTabRelocate(TSYMTAB *pSymTab, int pReloc[], int factor            *
*                                                                             *
*******************************************************************************
*
*   Relocates all address references within a symbol table by a set of
*   code/data offsets.
*
*   Where:
*       pSymTab is the pointer to a symbol table to relocate
*       pReloc is address of array of relocation values for each symbol type
*       factor is 1/-1 for relocation direction
*
******************************************************************************/
void SymTabRelocate(TSYMTAB *pSymTab, int pReloc[], int factor)
{
    TSYMHEADER *pHead;                  // Generic section header
    DWORD count;

    TSYMGLOBAL  *pGlobals;              // Globals section pointer
    TSYMGLOBAL1 *pGlobal;               // Single global item
    TSYMFNLIN   *pFnLin;                // Function lines section pointer
    TSYMFNSCOPE *pFnScope;              // Function scope section pointer
    TSYMSTATIC  *pStatic;               // Static symbols section pointer
    TSYMSTATIC1 *pStatic1;              // Single static item

    if( pSymTab )
    {
        pHead = pSymTab->header;

        // Loop over the complete symbol table and relocate each appropriate section
        while( pHead->hType != HTYPE__END )
        {
            switch( pHead->hType )
            {
                case HTYPE_GLOBALS:

                    pGlobals = (TSYMGLOBAL *) pHead;
                    pGlobal  = &pGlobals->list[0];

                    for(count=0; count<pGlobals->nGlobals; count++, pGlobal++ )
                    {
                        // pGlobal->bFlags contains the symbol segment that is index
                        // into the relocation array for that segment type
                        if( pGlobal->bFlags==0x00 )
                        {
                            // [0] is .text
                            pGlobal->dwStartAddress += pReloc[0] * factor;
                            pGlobal->dwEndAddress   += pReloc[0] * factor;
                        }
                        else
                        {
                            // [1] is .data
                            pGlobal->dwStartAddress += pReloc[1] * factor;
                            pGlobal->dwEndAddress   += pReloc[1] * factor;
                        }

                    }
                    break;

                case HTYPE_SOURCE:
                    // This section does not need relocation
                    break;

                case HTYPE_FUNCTION_LINES:

                    pFnLin = (TSYMFNLIN *) pHead;
                    pFnLin->dwStartAddress += pReloc[0] * factor;        // [0] is .text
                    pFnLin->dwEndAddress   += pReloc[0] * factor;
                    break;

                case HTYPE_FUNCTION_SCOPE:

                    pFnScope = (TSYMFNSCOPE *) pHead;
                    pFnScope->dwStartAddress += pReloc[0] * factor;
                    pFnScope->dwEndAddress   += pReloc[0] * factor;
                    break;

                case HTYPE_STATIC:

                    pStatic  = (TSYMSTATIC *) pHead;
                    pStatic1 = &pStatic->list[0];

                    for(count=0; count<pStatic->nStatics; count++, pStatic1++ )
                    {
                        pStatic1->dwAddress += pReloc[1] * factor;
                    }
                    break;

                case HTYPE_TYPEDEF:
                    // This section does not need relocation
                    break;

                case HTYPE_IGNORE:
                    // This section does not need relocation
                    break;

                default:
                    // We could catch a corrupted symbols error here if we want to...
                    break;
            }

            // Next section
            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }
}


/******************************************************************************
*                                                                             *
*   void SymTabMakePointers(TSYMTAB *pSymTab, DWORD dStrings)                 *
*                                                                             *
*******************************************************************************
*
*   This function is called only once upon symbol table load. It traverses
*   over all elements of a given table and adjusts pointers to strings since
*   they are only relative offsets in the raw symbol table file.
*
*   Where:
*       pSymTab is the pointer to a symbol table to adjust
*       dStrings is the offset adjustment
*
******************************************************************************/
static void SymTabMakePointers(TSYMTAB *pSymTab, DWORD dStrings)
{
    TSYMHEADER *pHead;                  // Generic section header
    UINT count;                         // Generic counter

    TSYMFNSCOPE  *pFnScope;             // Function scope section pointer
    TSYMFNSCOPE1 *pFnScope1;            // Single function scope item
    TSYMGLOBAL   *pGlobals;             // Globals section pointer
    TSYMGLOBAL1  *pGlobal;              // Single global item
    TSYMSTATIC   *pStatic;              // Static symbols section pointer
    TSYMSTATIC1  *pStatic1;             // Single static item
    TSYMSOURCE   *pSource;              // Source section header
    TSYMFNLIN    *pFnLin;               // Function lines section pointer
    TSYMFNLIN1   *pFnLin1;              // Single function line item
    TSYMTYPEDEF  *pType;                // Type section pointer
    TSYMTYPEDEF1 *pType1;               // Single type item

    if( pSymTab )
    {
        pHead = pSymTab->header;

        // Loop over the complete symbol table and adjust all pointers as appropriate
        while( pHead->hType != HTYPE__END )
        {
            switch( pHead->hType )
            {
                case HTYPE_FUNCTION_SCOPE:

                    pFnScope  = (TSYMFNSCOPE *) pHead;
                    pFnScope1 = &pFnScope->list[0];

                    for(count=0; count<pFnScope->nTokens; count++, pFnScope1++)
                    {
                        pFnScope1->pName += dStrings;
                    }

                    break;

                case HTYPE_GLOBALS:

                    pGlobals = (TSYMGLOBAL *) pHead;
                    pGlobal  = &pGlobals->list[0];

                    for(count=0; count<pGlobals->nGlobals; count++, pGlobal++ )
                    {
                        pGlobal->pName += dStrings;
                        pGlobal->pDef  += dStrings;
                    }
                    break;

                case HTYPE_STATIC:

                    pStatic  = (TSYMSTATIC *) pHead;
                    pStatic1 = &pStatic->list[0];

                    for(count=0; count<pStatic->nStatics; count++, pStatic1++ )
                    {
                        pStatic1->pName += dStrings;
                        pStatic1->pDef  += dStrings;
                    }
                    break;

                case HTYPE_SOURCE:

                    pSource = (TSYMSOURCE *) pHead;

                    pSource->pSourcePath += dStrings;
                    pSource->pSourceName += dStrings;

                    for(count=0; count<pSource->nLines; count++)
                    {
                        pSource->pLineArray[count] += dStrings;
                    }

                    break;

                case HTYPE_FUNCTION_LINES:
                    break;

                case HTYPE_TYPEDEF:

                    pType  = (TSYMTYPEDEF *) pHead;
                    pType1 = &pType->list[0];

                    for(count=0; count<pType->nTypedefs; count++, pType1++)
                    {
                        pType1->pName += dStrings;
                        pType1->pDef  += dStrings;
                    }

                    break;

                case HTYPE_IGNORE:
                    break;

                default:
                    // We could catch a corrupted symbols error here if we want to...
                    break;
            }

            // Next section
            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }
}
