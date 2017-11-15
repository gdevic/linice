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
    TGLOBAL *pGlobal;
    TSYM_PUB *pSym;
    int i;

    if( pSymtab==NULL )
        return(100);

    if( !CHKADDR(pSymtab) )
        return(200);

    if( !CHKADDR(pSymtab->next) && pSymtab->next!=NULL )
        return(201);

    if( !CHKADDR(pSymtab->pStrings) )
        return(300);

    if( !CHKADDR(pSymtab->pGlobals) )
        return(301);

    // Check the GLOBAL structure
    pGlobal = pSymtab->pGlobals;

    if( !CHKADDR(pGlobal->pSym) )
        return(500);
    if( !CHKADDR(pGlobal->pHash) )
        return(501);
    if( pGlobal->nHash > 100000 )
        return(502);
    if( pGlobal->nSyms > 10000 )
        return(503);

    pSym = pGlobal->pSym;

    for(i=0; i<pGlobal->nSyms; i++ )
    {
        if( !CHKADDR(pSym[i].pName) )
            return(10000+i);
    }

    return(0);
}


/******************************************************************************
*                                                                             *
*   static DWORD str2key(char *str)                                           *
*                                                                             *
*******************************************************************************
*
*   Creates a hash key for the given string.
*
******************************************************************************/
static DWORD str2key(char *str)
{
    int len = strlen(str);
    DWORD key = 0;

    while( len-- )
    {
        key += *str++ << len;
    }

    return( key );
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
*       -EINVAL general failure
*       -ENOMEM not enough memory
*
******************************************************************************/
int UserAddSymbolTable(void *pSymtab)
{
    int retval = -EINVAL, i;
    TSYMTAB SymHeader;
    TSYMTAB *pSym;

    // Copy the header of the symbol table into the kernel space
    if( copy_from_user(&SymHeader, pSymtab, sizeof(TSYMTAB))==0 )
    {
        // Allocate memory for complete symbol table from the private pool
        pSym = (TSYMTAB *) ice_malloc((unsigned int)SymHeader.size);
        if( pSym )
        {
            INFO(("Allocated %d bytes at %X for symbol table\n", (int) SymHeader.size, (int) pSym));
            // Copy the complete symbol table from the use space
            if( copy_from_user(pSym, pSymtab, SymHeader.size)==0 )
            {
                dprinth(1, "Loaded symbols for module '%s' size %d", pSym->name, pSym->size);

                pIce->nSymbolBufferAvail -= SymHeader.size;

                //============================================================
                // Fix up all the relative indices to addresses (pointers)
                //============================================================
                // 1) general header pointers

                pSym->pStrings = (char*)((DWORD)pSym + (DWORD)pSym->pStrings);

                pSym->pGlobals = (TGLOBAL *)((DWORD)pSym + (DWORD)pSym->pGlobals);
                pSym->pGlobals->pSym = (TSYM_PUB *)((DWORD)pSym + (DWORD)pSym->pGlobals->pSym);
                pSym->pGlobals->pHash = (WORD *)((DWORD)pSym + (DWORD)pSym->pGlobals->pHash);

                // 2) Global symbol table pointers to names

                for(i=0; i<pSym->pGlobals->nSyms; i++ )
                {
                    pSym->pGlobals->pSym[i].pName += (DWORD)pSym;
                }

                // Link this symbol table in the linked list
                pSym->next = (struct TSYMTAB *) pIce->pSymTab;
                pIce->pSymTab = pSym;
                pIce->pSymTabCur = pIce->pSymTab;

                // Return OK
                return( 0 );
            }
            else
            {
                ERROR(("Error copying symbol table"));
            }
        }
        else
        {
            ERROR(("Unable to allocate %d for symbol table!\n", (int) SymHeader.size));
            retval = -ENOMEM;
        }

        // Deallocate memory for symbol table
        ice_free_heap((void *) pSym);
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
*
******************************************************************************/
int UserRemoveSymbolTable(void *pSymtab)
{
    int retval = -EINVAL;
    char sName[MAX_MODULE_NAME+1];
    TSYMTAB *pSym, *pPrev = NULL;

    // Copy the symbol table name from the user address space
    if( copy_from_user(sName, pSymtab, MAX_MODULE_NAME)==0 )
    {
        sName[MAX_MODULE_NAME] = 0;

        // Search for the named symbol structure
        pSym = pIce->pSymTab;
        while( pSym )
        {
            if( !strcmp(pSym->name, sName) )
            {
                // Found it, unlink it, free it and return success
                if( pSym==pIce->pSymTab )
                    pIce->pSymTab = (TSYMTAB *) pSym->next;     // First in the linked list
                else
                    pPrev->next = pSym->next;       // Not the first in the linked list

                // Add the bytes to the free pool
                pIce->nSymbolBufferAvail += pSym->size;

                ice_free((BYTE *)pSym);

                // Make no dangling pointers..
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
    TSYMTAB *pSymTab = (TSYMTAB *) pIce->pSymTab;
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
            dprinth(nLine++, " %6d  %s   (%08X - %08X)",
                pSymTab->size, pSymTab->name, pSymTab->pGlobals->dwAddrStart, pSymTab->pGlobals->dwAddrEnd);
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
    TGLOBAL *pGlobals;
    BOOL fCont = TRUE;
    int nLine = 1, index;

    if( pIce->pSymTabCur==NULL )
    {
        dprinth(1, "No symbol table loaded.");
    }
    else
    {
        pGlobals = pIce->pSymTabCur->pGlobals;
        for( index=0; index<pGlobals->nSyms && fCont; index++ )
        {
            fCont = dprinth(nLine++,  "  %08X  %s",
                pGlobals->pSym[index].dwAddress,
                pGlobals->pSym[index].pName);
        }
    }

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
    TGLOBAL *pGlobals;
    TSYM_PUB *pSympub;
    DWORD key, hash, index;
    int len;

    // Exit early if no symbol table available
    if( pIce->pSymTabCur==NULL )
        return( FALSE );

    pGlobals = pIce->pSymTabCur->pGlobals;
    pSympub  = pGlobals->pSym;

    len = strlen(name);

    // Find the key of the symbol name
    key = str2key(name);
    hash = key % pGlobals->nHash;

    // Do a hash search for the symbol name
    while( (index = pGlobals->pHash[hash]) != 0 )
    {
        // Compare strings and return if the names match
        if( strnicmp(name, pSympub[index-1].pName, len)==0 )
        {
            *pValue = pSympub[index-1].dwAddress;
            return( TRUE );
        }

        // Otherwise, keep linear search...
        if( ++hash==pGlobals->nHash )
            hash = 0;
    }

    // If we are here, the kay was not there
    return( FALSE );
}

