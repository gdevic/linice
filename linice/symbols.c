/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       04/21/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 04/21/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include <asm/uaccess.h>                // User space memory access functions

#include "ice-ioctl.h"                  // Include our own IOCTL numbers
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
extern void ice_free_heap(BYTE *pHeap);

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
    int retval = -EINVAL;
    TSYMTAB Sym;
    PTSYMTAB pSym;

    // Copy the header of the symbol table into the kernel space
    if( copy_from_user(&Sym, pSymtab, sizeof(TSYMTAB))==0 )
    {
        // Check the magic number
        if( Sym.magic==MAGIC_SYMTAB )
        {
            // Allocate memory for the complete symbol table from the private pool
            pSym = (PTSYMTAB) ice_malloc((unsigned int)Sym.size);
            if( pSym )
            {
                INFO(("Allocated %d bytes at %X for symbol table\n", (int) Sym.size, (int) pSym));
                // Copy the complete symbol table from the use space
                if( copy_from_user(pSym, pSymtab, Sym.size)==0 )
                {
                    INFO(("Loaded symbols for module '%s'\n", pSym->name));

                    // Link this symbol table with the list of them
                    pSym->next = (PTSYMTAB) pIce->pSymTab;
                    pIce->pSymTab = (struct TSYMTAB *) pSym;

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
                ERROR(("Unable to allocate %d for symbol table!\n", (int) Sym.size));
                retval = -ENOMEM;
            }

            // Deallocate memory for symbol table
            ice_free_heap((void *) pSym);
        }
        else
        {
            ERROR(("Invalid signature\n"));
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
*   BOOL cmdTable(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display current symbol table
*
******************************************************************************/
BOOL cmdTable(char *args, int subClass)
{
    PTSYMTAB pSym = (PTSYMTAB) pIce->pSymTab;
    int nLine = 2;

    if( pSym==NULL )
    {
        dprinth(1, "No symbol table loaded.\n");
    }
    else
    {
        dprinth(1, "Symbol tables:\n");
        while( pSym )
        {
            dprinth(nLine++, " %6d  %s\n", pSym->size, pSym->name);
            pSym = pSym->next;
        }
    }

    return( TRUE );
}

