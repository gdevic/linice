/******************************************************************************
*                                                                             *
*   Module:     memaccess.c                                                   *
*                                                                             *
*   Date:       09/30/00                                                      *
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

        This module contains code for memory access

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/30/00   Original                                             Goran Devic *
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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL AddrIsPresent(PTADDRDESC pAddr)
{
    deb.memaccess = GetByte(pAddr->sel, pAddr->offset);

    return( (deb.memaccess <= 0xFF)? TRUE : FALSE );
}

BYTE AddrGetByte(PTADDRDESC pAddr)
{
    deb.memaccess = GetByte(pAddr->sel, pAddr->offset);

    return( deb.memaccess & 0xFF );
}

DWORD AddrSetByte(PTADDRDESC pAddr, BYTE value, BOOL fForce)
{
    TADDRDESC Addr;
    DWORD Access;
    TGDT_Gate *pGdt;

    deb.memaccess = SetByte(pAddr->sel, pAddr->offset, value);

    // If the set memory failed, and we really wanted to override
    // protection, use kernel DS selector to set the value, since that is
    // always a writable selector
    if( deb.memaccess>0xFF && fForce )
    {
        Access = SelLAR(pAddr->sel);
        if( Access )
        {
            // If the source selector was ok, get its base address
            pGdt = (TGDT_Gate *) (deb.gdt.base + pAddr->sel);

            Addr.sel    = __KERNEL_DS;
            Addr.offset = GET_GDT_BASE(pGdt) + pAddr->offset;

            deb.memaccess = SetByte(Addr.sel, Addr.offset, value);
        }
    }

    return( deb.memaccess );
}


/******************************************************************************
*                                                                             *
*   BOOL VerifySelector(WORD Sel)                                             *
*                                                                             *
*******************************************************************************
*
*   Verifies a given selector value and prints error message if the selector
*   is invalid.
*
******************************************************************************/
BOOL VerifySelector(WORD Sel)
{
    if( SelLAR(Sel)==0 )
    {
        dprinth(1, "Invalid selector 0x%04X", Sel);

        return( FALSE );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL VerifyRange(PTADDRDESC pAddr, DWORD dwSize)                          *
*                                                                             *
*******************************************************************************
*
*   Verified a set of memory locations for access
*
******************************************************************************/
BOOL VerifyRange(PTADDRDESC pAddr, DWORD dwSize)
{
    int pages;                          // Page counter
    TADDRDESC Addr;                     // Local copy of address descriptor

    Addr = *pAddr;                      // that we can modify

    // Both starting and ending address needs to be accessible,
    // as well as every page in between

    pages = (dwSize / 4096) + 1;

    do
    {
        if( !AddrIsPresent(&Addr) )     // If we can't access it, return failure
            return( FALSE );

        Addr.offset += 4096;            // Advance to the next page
        
    } while( --pages );

    // Verify the last byte of the region
    Addr.offset = pAddr->offset + dwSize - 1;

    if( !AddrIsPresent(&Addr) )         // If we can't access it, return failure
        return( FALSE );

    return( TRUE );                     // The whole range is ok
}
