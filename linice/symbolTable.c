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

extern BYTE *ice_malloc(DWORD size);
extern void ice_free(BYTE *p);
extern void ice_free_heap(BYTE *pHeap);


BOOL SymbolTableRemove(char *pTableName, TSYMTAB *pRemove);

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

// TODO: Need a smarter way to check the address sanity...
#define CHKADDR(address) (((DWORD)(address)>0xC0000000)&&((DWORD)(address)<0xD0000000))

int CheckSymtab(TSYMTAB *pSymtab)
{

    return(0);
}


/******************************************************************************
*                                                                             *
*   int UserAddSymbolTable(void *pSymtab)                                     *
*                                                                             *
*******************************************************************************
*
*   Adds a symbol table from the header.
*
*   Where:
*       pSymtab is the address of the symbol table header in the user space
*
*   Returns:
*       0 init ok
*       -EINVAL Bad symbol table
*       -EFAULT general failure
*       -ENOMEM not enough memory
*
******************************************************************************/
int UserAddSymbolTable(void *pSymtab)
{
    int retval = -EINVAL;
    TSYMTAB SymHeader;
    TSYMTAB *pSym;
    TSYMPRIV *pPriv;

    // Copy the header of the symbol table into the local structur to examine it
    if( copy_from_user(&SymHeader, pSymtab, sizeof(TSYMTAB))==0 )
    {
        // Check if we should allocate memory for the symbol table from the allowed pool
        if( pIce->nSymbolBufferAvail >= SymHeader.dwSize )
        {
            // Allocate memory for complete symbol table from the private pool
            pSym = (TSYMTAB *) ice_malloc((unsigned int)SymHeader.dwSize);
            if( pSym )
            {
                INFO(("Allocated %d bytes at %X for symbol table\n", (int) SymHeader.dwSize, (int) pSym));

                // Allocate memory for the private section of the symbol table
                pPriv = (TSYMPRIV *) ice_malloc(sizeof(TSYMPRIV));
                if( pPriv )
                {
                    INFO(("Allocated %d bytes for private symbol table structure\n", sizeof(TSYMPRIV) ));

                    // Copy the complete symbol table from the use space
                    if( copy_from_user(pSym, pSymtab, SymHeader.dwSize)==0 )
                    {
                        // Make sure we are really loading a symbol table
                        if( !strcmp(pSym->sSig, SYMSIG) )
                        {
                            // Compare the symbol table version - major number has to match
                            if( (pSym->Version>>8)==(SYMVER>>8) )
                            {
                                // Call the remove function that will remove this particular
                                // symbol table if we are reloading it
                                SymbolTableRemove(pSym->sTableName, NULL);

                                dprinth(1, "Loaded symbols for module '%s' size %d (ver %d.%d)",
                                    pSym->sTableName, pSym->dwSize, pSym->Version>>8, pSym->Version&0xFF);

                                // Link the private data structure to the symbol table
                                pSym->pPriv = pPriv;

                                pIce->nSymbolBufferAvail -= SymHeader.dwSize;

                                // Link this symbol table in the linked list
                                pSym->pPriv->next = (struct TSYMTAB *) pIce->pSymTab;

                                pIce->pSymTab = pSym;
                                pIce->pSymTabCur = pIce->pSymTab;

                                // Initialize elements of the private symbol table structure

                                pPriv->pStrings = (char*)((DWORD) pSym + pSym->dStrings);

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
                ice_free_heap((void *) pSym);
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
    int retval;
    char sName[MAX_MODULE_NAME];

    // Copy the symbol table name from the user address space
    if( copy_from_user(sName, pSymtab, MAX_MODULE_NAME)==0 )
    {
        sName[MAX_MODULE_NAME-1] = 0;

        // Search for the symbol table with that specific table name
        if( SymbolTableRemove(sName, NULL)==FALSE )
            retval = -EINVAL;
        retval = -4;
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
    pSym = pIce->pSymTab;
    while( pSym )
    {
        if( pTableName!=NULL )
        {
            if( !strcmp(pSym->sTableName, pTableName) )
                fMatch = TRUE;
        }
        else
            if( pRemove==pSym )
                fMatch = TRUE;

        if( fMatch )
        {
            dprinth(1, "Removed symbol table %s", pSym->sTableName);

            // Found it - unlink, free and return success
            if( pSym==pIce->pSymTab )
                pIce->pSymTab = (TSYMTAB *) pSym->pPriv->next;     // First in the linked list
            else
                pPrev->pPriv->next = pSym->pPriv->next;       // Not the first in the linked list

            // Add the memory block to the free pool
            pIce->nSymbolBufferAvail += pSym->dwSize;

            // Release the private symbol table data
            ice_free((BYTE *)pSym->pPriv);

            // Release the symbol table itself
            ice_free((BYTE *)pSym);

            // Leave no dangling pointers...
            pIce->pSymTabCur = pIce->pSymTab;

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
    TSYMTAB *pSymTab = pIce->pSymTab;
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

                    while( pIce->pSymTab )
                    {
                        SymbolTableRemove(NULL, pIce->pSymTab);
                    }
                }
                else
                {
                    // Remove a particular symbol table

                    if( SymbolTableRemove(args, NULL)==FALSE )
                        dprinth(1, "Nonexisting symbol table %s", args);

                    return(TRUE);
                }
            }
            else
            {
                // Set current symbol table

                while( pSymTab )
                {
                    // Compare internal table name to our argument string
                    if( !strcmp(pSymTab->sTableName, args) )
                    {
                        pIce->pSymTabCur = pSymTab;

                        return(TRUE);
                    }

                    pSymTab = (TSYMTAB *) pSymTab->pPriv->next;
                }

                dprinth(1, "Nonexisting symbol table %s", args);

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
                    DP_SETCOLINDEX, pSymTab==pIce->pSymTabCur? COL_BOLD:COL_NORMAL,
                    pSymTab->dwSize, pSymTab->sTableName);
                pSymTab = (TSYMTAB *) pSymTab->pPriv->next;
            }
        }
    }

    dprinth(nLine, "%d bytes of symbol table memory available", pIce->nSymbolBufferAvail);

    return( TRUE );
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

