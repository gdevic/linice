/******************************************************************************
*                                                                             *
*   Module:     symbolTable.c                                                 *
*                                                                             *
*   Date:       10/21/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

extern BOOL FindModule(TMODULE *pMod, char *pName, int nNameLen);


BOOL SymbolTableRemove(char *pTableName, TSYMTAB *pRemove);
BOOL SymTabSetupRelocOffset(TSYMTAB *pSymTab, DWORD dwInitModule, DWORD dwInitModuleSample);
void SymTabRelocate(TSYMTAB *pSymTab, int factor);
static void SymTabMakePointers(TSYMTAB *pSymTab, DWORD dStrings);


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
    DWORD dStrings;                     // Offset to strings (to adjust)
    TMODULE Mod;                        // Module information structure


    // Copy the header of the symbol table into the local structur to examine it
    if( ice_copy_from_user(&SymHeader, pSymUser, sizeof(TSYMTAB))==0 )
    {
        // Check if we should allocate memory for the symbol table from the allowed pool
        if( deb.nSymbolBufferAvail >= SymHeader.dwSize )
        {
            // Allocate memory for complete symbol table from the private pool
            pSymTab = (TSYMTAB *) mallocHeap(deb.hSymbolBufferHeap, SymHeader.dwSize);
            if( pSymTab )
            {
                INFO("Allocated %d bytes at %X for symbol table\n", (int) SymHeader.dwSize, (int) pSymTab);

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
                            // symbol table if we are reloading it (at this point new table is not linked in yet)
                            // TODO: Remove breakpoints in this function?
                            SymbolTableRemove(pSymTab->sTableName, NULL);

                            dprinth(1, "Loaded symbols for module `%s' size %d (ver %d.%d)",
                                pSymTab->sTableName, pSymTab->dwSize, pSymTab->Version>>8, pSymTab->Version&0xFF);

                            deb.nSymbolBufferAvail -= SymHeader.dwSize;

                            // Link this symbol table in the linked list and also make it current
                            pSymTab->next = (struct TSYMTAB *) deb.pSymTab;

                            deb.pSymTab = pSymTab;
                            deb.pSymTabCur = deb.pSymTab;

                            // Initialize elements of the private symbol table structure

                            dStrings = (DWORD) pSymTab + pSymTab->dStrings;

                            // Relocate all strings within this symbol table to pointers (from offsets)

                            SymTabMakePointers(deb.pSymTabCur, dStrings);

                            // If the symbol table being loaded describes a kernel module, we need to
                            // see if that module is already loaded, and if so, relocate its symbols

                            // If a module with that symbol name has already been loaded dont relocate again
                            if( FindModule(&Mod, pSymTab->sTableName, strlen(pSymTab->sTableName)) )
                            {
                                // Module is already loaded - we need to relocate symbol table based on it

                                // Setup relocation offsets
                                if( SymTabSetupRelocOffset(pSymTab, (DWORD) Mod.init, (DWORD) Mod.init) )
                                {
                                    // Relocate symbol table
                                    SymTabRelocate(pSymTab, 1);

                                    // Make that symbol table the current one
                                    deb.pSymTabCur = pSymTab;
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
                    ERROR("Error copying symbol table");
                    retval = -EFAULT;
                }

                // Deallocate memory for symbol table
                freeHeap(deb.hSymbolBufferHeap, (void *) pSymTab);
            }
            else
            {
                ERROR("Unable to allocate %d for symbol table!\n", (int) SymHeader.dwSize);
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
        ERROR("Invalid IOCTL packet address\n");
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
        ERROR("Invalid IOCTL packet address\n");
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
                deb.pSymTab = (TSYMTAB *) pSym->next;           // First in the linked list
            else
                pPrev->next = pSym->next;                       // Not the first in the linked list

            // Add the memory block to the free pool
            deb.nSymbolBufferAvail += pSym->dwSize;

            // Release the symbol table itself
            freeHeap(deb.hSymbolBufferHeap, (BYTE *)pSym);

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
        pSym = (TSYMTAB *) pSym->next;
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
*   [[r] partial-table-name] | autoon | autoof | $
*
*   Optional argument 'R' removes a specified table (or all tables if 'R *')
*
*   Optional argument "autoon" or "autoof" will cause automatical switch to
*   a table that is detected as current.
*
*   Optional argument "$" will cause to switch to a table where the current
*   instruction pointer is located.
*
******************************************************************************/
BOOL cmdTable(char *args, int subClass)
{
    TSYMTAB *pSymTab = deb.pSymTab;
    int nLine = 2;

    // First check for few special reserver words, keywords

    if( !strcmp(args, "$") )
    {
        // Set the symbol table where the current CS:EIP points to
        SetSymbolContext(deb.r->cs, deb.r->eip);
    }
    else
    if( !stricmp(args, "autoon") )
    {
        // Turn on the automatic symbol table switch
        deb.fTableAutoOn = TRUE;
    }
    else
    if( !stricmp(args, "autooff") )
    {
        // Turn off the automatic symbol table switch
        deb.fTableAutoOn = FALSE;
    }
    else
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

                    pSymTab = (TSYMTAB *) pSymTab->next;
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
                pSymTab = (TSYMTAB *) pSymTab->next;
            }
        }
    }

    dprinth(nLine, "%d bytes of symbol table memory available", deb.nSymbolBufferAvail);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   TSYMTAB *SymTabFind(char *name, BYTE SymTableType)                        *
*                                                                             *
*******************************************************************************
*
*   Searches for the named symbol table of a given type.
*
*   Where:
*       name is the internal symbol table name
*       SymTableType is the type that we are looking for (or 0 for any type)
*
*   Returns:
*       Pointer to a symbol table
*       NULL if the symbol table of that name is not loaded
*
******************************************************************************/
TSYMTAB *SymTabFind(char *name, BYTE SymTableType)
{
    TSYMTAB *pSymTab = deb.pSymTab;   // Pointer to all symbol tables

    // Make sure name is given as a valid pointer
    if( name!=NULL )
    {
        while( pSymTab )
        {
            if( SymTableType==SYMTABLETYPE_UNDEF || pSymTab->SymTableType==SymTableType )
                if( !strcmp(pSymTab->sTableName, name) )
                    return( pSymTab );

            pSymTab = (TSYMTAB *) pSymTab->next;
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

// TODO: Make one function of these two (above, below), abstract search for a given header type

/******************************************************************************
*                                                                             *
*   TSYMTYPEDEF *SymTabFindTypedef(TSYMTAB *pSymTab, WORD fileID)             *
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
*       Pointer to a typedef descriptor
*       NULL if the file id descriptor can not be located
*
******************************************************************************/
TSYMTYPEDEF *SymTabFindTypedef(TSYMTAB *pSymTab, WORD fileID)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pTypedef;

    if( pSymTab )
    {
        pHead = pSymTab->header;

        while( pHead->hType != HTYPE__END )
        {
            if( (pHead->hType == HTYPE_TYPEDEF) )
            {
                pTypedef = (TSYMTYPEDEF *)pHead;

                if( pTypedef->file_id==fileID )
                    return( pTypedef );
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( NULL );
}


/******************************************************************************
*
*   BOOL SymTabSetupRelocOffset(TSYMTAB *pSymTab, DWORD dwInitModule, DWORD dwInitModuleSample)
*
*******************************************************************************
*
*   Prepares the symbol table relocation values based on the init_module symbol address.
*
*   Where:
*       pSymTab is the symbol table to prepare. It has to have HTYPE_RELOC data.
*       dwInitModule is the real address of the init_module
*       dwInitModuleSample is the address to consider when taking data samples
*
*       The 'sample' may not be identical to init_module in the case of sys call
*       where the code has not yet been copied to the final address, so
*       although we use that address to compute the .text section offset,
*       we still need to use the "low" address (which is user space) when
*       reading sample data for variable relocation.
*
*   Returns:
*       TRUE for ok
*       FALSE if the relocation section could not be found
*
******************************************************************************/
BOOL SymTabSetupRelocOffset(TSYMTAB *pSymTab, DWORD dwInitModule, DWORD dwInitModuleSample)
{
    DWORD dwSampleOffset;               // Real offset of that item
    TSYMRELOC  *pReloc;                 // Symbol table relocation header
    int i;                              // Relocation section

    // Find the symbol table relocation block
    pReloc = (TSYMRELOC *) SymTabFindSection(pSymTab, HTYPE_RELOC);
    if( pReloc )
    {
        // Store private reloc adjustment value for code section (.text)
        pReloc->list[0].reloc = (int)(dwInitModule - pReloc->list[0].refOffset);

        // Compute the relocation values for other segments
        for(i=1; i<pReloc->nReloc; i++)
        {
            // We can never have fixup from 0 for the sample, so skip those entries.
            // This can happen if there is no .rodata, for example and we always
            // store all 4 basic segments (.text, .data, .rodata and .bss)
            if( pReloc->list[i].refFixup )
            {
                // Find the address within the code segment from which we will read the offset to
                // our data. Relocation block contains the relative offset from the init_module function
                // to our dword sample that we need to take.
dprinth(1, "[%d] fixup=%08X offset=%08X", i, pReloc->list[i].refFixup, pReloc->list[i].refOffset);
                dwSampleOffset = *(DWORD *)(dwInitModuleSample + pReloc->list[i].refFixup);
dprinth(1, "  %08X %08X = %08X", dwInitModuleSample, dwInitModuleSample + pReloc->list[i].refFixup, dwSampleOffset);
                pReloc->list[i].reloc = dwSampleOffset - pReloc->list[i].refOffset;
dprinth(1, "   reloc[%d] = (%08X,%08X)", i, pReloc->list[i].refFixup, pReloc->list[i].refOffset);
            }
        }
    }
    else
    {
        dprinth(1, "SYSCALL: Symbol table missing HTYPE_RELOC!");

        return( FALSE );
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   void SymTabRelocate(TSYMTAB *pSymTab, int factor                          *
*                                                                             *
*******************************************************************************
*
*   Relocates all address references within a symbol table by a set of
*   code/data offsets.
*
*   Where:
*       pSymTab is the pointer to a symbol table to relocate
*       factor is 1/-1 for relocation direction
*
******************************************************************************/
void SymTabRelocate(TSYMTAB *pSymTab, int factor)
{
    TSYMRELOC  *pReloc;                 // Symbol table relocation header
    TSYMHEADER *pHead;                  // Generic section header

    TSYMGLOBAL  *pGlobals;              // Globals section pointer
    TSYMGLOBAL1 *pGlobal;               // Single global item
    TSYMFNLIN   *pFnLin;                // Function lines section pointer
    TSYMFNSCOPE *pFnScope;              // Function scope section pointer
    TSYMSTATIC  *pStatic;               // Static symbols section pointer
    TSYMSTATIC1 *pStatic1;              // Single static item
    DWORD count;
    int nSegment;                       // Segment number that the variable is in

    if( pSymTab )
    {
        pReloc = (TSYMRELOC *) SymTabFindSection(pSymTab, HTYPE_RELOC);

        if( pReloc )
        {
            if( factor>0 )
                dprinth(1, "SYSCALL: Relocating symbols for `%s' .text=%08X", pSymTab->sTableName, pReloc->list[0].reloc);
            else
                dprinth(1, "SYSCALL: Reverting symbol relocation for `%s'", pSymTab->sTableName);

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
                            // pGlobal->bSegment contains the symbol segment that is index
                            // into the relocation array for that segment type

                            nSegment = pGlobal->bSegment;

                            pGlobal->dwStartAddress += pReloc->list[nSegment].reloc * factor;
                            pGlobal->dwEndAddress   += pReloc->list[nSegment].reloc * factor;
                        }
                        break;

                    case HTYPE_SOURCE:
                        // This section does not need relocation
                        break;

                    case HTYPE_FUNCTION_LINES:

                        pFnLin = (TSYMFNLIN *) pHead;
                        pFnLin->dwStartAddress += pReloc->list[0].reloc * factor;
                        pFnLin->dwEndAddress   += pReloc->list[0].reloc * factor;
                        break;

                    case HTYPE_FUNCTION_SCOPE:

                        pFnScope = (TSYMFNSCOPE *) pHead;
                        pFnScope->dwStartAddress += pReloc->list[0].reloc * factor;
                        pFnScope->dwEndAddress   += pReloc->list[0].reloc * factor;

                        // Loop for all function scope tokens and relocate TOKTYPE_LCSYM as it is
                        // the only one containing the address of the symbol (uninitialized local static)

                        for(count=0; count<pFnScope->nTokens; count++ )
                        {
                            if( pFnScope->list[count].TokType==TOKTYPE_LCSYM )
                            {
                                nSegment = pFnScope->list[count].bSegment;

                                pFnScope->list[count].param += pReloc->list[nSegment].reloc * factor;
                            }
                        }
                        break;

                    case HTYPE_STATIC:

                        pStatic  = (TSYMSTATIC *) pHead;
                        pStatic1 = &pStatic->list[0];

                        for(count=0; count<pStatic->nStatics; count++, pStatic1++ )
                        {
                            nSegment = pStatic1->bSegment;

                            pStatic1->dwAddress += pReloc->list[nSegment].reloc * factor;
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
//  TSYMFNLIN    *pFnLin;               // Function lines section pointer
//  TSYMFNLIN1   *pFnLin1;              // Single function line item
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

                    pFnScope->pName += dStrings;

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
                    pType->pRel = (TSYMADJUST *)((DWORD)pType->pRel + dStrings);

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
