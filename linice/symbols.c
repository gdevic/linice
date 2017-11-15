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

                // Copy the complete symbol table from the use space
                if( copy_from_user(pSym, pSymtab, SymHeader.dwSize)==0 )
                {
                    // Make sure we are really loading a symbol table
                    if( !strcmp(pSym->sSig, SYMSIG) )
                    {
                        // Compare the symbol table version - major number has to match
                        if( (pSym->Version>>8)==(SYMVER>>8) )
                        {
                            dprinth(1, "Loaded symbols for module '%s' size %d (ver %d.%d)",
                                pSym->sTableName, pSym->dwSize, pSym->Version>>8, pSym->Version&0xFF);

                            pIce->nSymbolBufferAvail -= SymHeader.dwSize;

                            // Link this symbol table in the linked list
                            pSym->next = (struct _SYMTAB *) pIce->pSymTab;
                            pIce->pSymTab = pSym;
                            pIce->pSymTabCur = pIce->pSymTab;

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
*       pSymtab is the address of the symbol table name in the user space
*
*   Returns:
*       0 remove ok
*       -EINVAL Invalid symbol table name
*       -EFAULT general failure
*
******************************************************************************/
int UserRemoveSymbolTable(void *pSymtab)
{
    int retval = -EINVAL;
    char sName[MAX_MODULE_NAME];
    TSYMTAB *pSym, *pPrev = NULL;

    // Copy the symbol table name from the user address space
    if( copy_from_user(sName, pSymtab, MAX_MODULE_NAME)==0 )
    {
        sName[MAX_MODULE_NAME-1] = 0;

        // Search for the symbol table with that specific table name
        pSym = pIce->pSymTab;
        while( pSym )
        {
            if( !strcmp(pSym->sTableName, sName) )
            {
                dprinth(1, "Removed symbol table %s", pSym->sTableName);

                // Found it - unlink, free and return success
                if( pSym==pIce->pSymTab )
                    pIce->pSymTab = (TSYMTAB *) pSym->next;     // First in the linked list
                else
                    pPrev->next = pSym->next;       // Not the first in the linked list

                // Add the memory block to the free pool
                pIce->nSymbolBufferAvail += pSym->dwSize;

                ice_free((BYTE *)pSym);

                // Leave no dangling pointers...
                pIce->pSymTabCur = pIce->pSymTab;

                return(0);
            }

            pPrev = pSym;
            pSym = (TSYMTAB *) pSym->next;
        }
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
*   BOOL cmdTable(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display or set current symbol table
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
        dprinth(1, "Symbol tables:");
        while( pSymTab )
        {
            dprinth(nLine++, " %6d  %s", pSymTab->dwSize, pSymTab->sTableName);
            pSymTab = (TSYMTAB *) pSymTab->next;
        }
    }

    dprinth(nLine, "%d bytes of symbol table memory available", pIce->nSymbolBufferAvail);

    return( TRUE );
}


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

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL SymbolName2Value(DWORD *pValue, char *name)                          *
*                                                                             *
*******************************************************************************
*
*   Translates symbol name to its value
*
*   Where:
*       pValue is the address of the variable to receive the value DWORD
*       name is the symbol name
*   Returns:
*       FALSE - symbol name not found
*       TRUE - *pValue is set with the symbol value
*
******************************************************************************/
BOOL SymbolName2Value(DWORD *pValue, char *name)
{

    return( FALSE );
}

