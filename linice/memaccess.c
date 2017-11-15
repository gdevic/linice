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

// Define structure that holds our checksums for verious regions

typedef struct
{
    BYTE hSymbolBufferHeapChecksum;
    BYTE hHistoryBufferHeapChecksum;
    BYTE hHeapChecksum;
    BYTE debChecksum;
    BYTE WinChecksum;

} TSTAMP;

// Define that structure in the global uninitialized section (.BSS)

static TSTAMP Stamp;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*   Returns TRUE if the address descriptor can be use to read data
******************************************************************************/
BOOL AddrIsPresent(PTADDRDESC pAddr)
{
    deb.memaccess = GetByte(pAddr->sel, pAddr->offset);

    return( (deb.memaccess <= 0xFF)? TRUE : FALSE );
}

/******************************************************************************
*   Returns a BYTE from a memory location
******************************************************************************/
BYTE AddrGetByte(PTADDRDESC pAddr)
{
    deb.memaccess = GetByte(pAddr->sel, pAddr->offset);

    return( deb.memaccess & 0xFF );
}

/******************************************************************************
*   Returns a DWORD from a memory location
******************************************************************************/
DWORD AddrGetDword(PTADDRDESC pAddr)
{
    DWORD dwValue;

    // Do the access check and leave the result in the deb.memaccess
    deb.memaccess = GetByte(pAddr->sel, pAddr->offset);

    // Do the actual memory fetch if we could access it
    if( (deb.memaccess & 0x100)==0 )
        dwValue = GetDWORD(pAddr->sel, pAddr->offset);
    else
        dwValue = 0xFFFFFFFF;

    return( dwValue );
}

/******************************************************************************
*   Another way to read a DWORD with a return value if succeeded
******************************************************************************/
BOOL GlobalReadDword(DWORD *pDword, DWORD dwAddress)
{
    TADDRDESC Addr;                     // Address access descriptor
    DWORD dwData;                       // Data read

    Addr.sel = GetKernelDS();
    Addr.offset = dwAddress;
    dwData = AddrGetDword(&Addr);       // Get the value from that address

    if( deb.memaccess & 0x100 )
        return( FALSE );

    *pDword = dwData;

    return( TRUE );
}

/******************************************************************************
*   Another way to read a BYTE with a return value if succeeded
******************************************************************************/
BOOL GlobalReadBYTE(BYTE *pByte, DWORD dwAddress)
{
    TADDRDESC Addr;                     // Address access descriptor
    BYTE bData;                         // Data read

    Addr.sel = GetKernelDS();
    Addr.offset = dwAddress;
    bData = AddrGetByte(&Addr);         // Get the value from that address

    if( deb.memaccess & 0x100 )
        return( FALSE );

    *pByte = bData;

    return( TRUE );
}

/******************************************************************************
*   Read a number of bytes from a memory location
******************************************************************************/
BOOL GlobalReadMem(BYTE *pBuf, DWORD dwAddress, UINT nLen)
{
    memset(pBuf, 0xFF, nLen);

    while( nLen-- )
    {
        if( !GlobalReadBYTE(pBuf, dwAddress) )
            return( FALSE );

        pBuf++;
        dwAddress++;
    }

    return( TRUE );
}

/******************************************************************************
*   Set a BYTE value
******************************************************************************/
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

            Addr.sel    = GetKernelDS();
            Addr.offset = GET_GDT_BASE(pGdt) + pAddr->offset;

            deb.memaccess = SetByte(Addr.sel, Addr.offset, value);
        }
    }

    return( deb.memaccess );
}


/******************************************************************************
*                                                                             *
*   DWORD fnPtr(DWORD arg)                                                    *
*                                                                             *
*******************************************************************************
*
*   Expression evaluator helper function to return a DWORD from a memory
*   location given a pointer.
*
******************************************************************************/
DWORD fnPtr(DWORD arg)
{
    BYTE value[4];                      // Temporary DWORD store
    TADDRDESC Addr;                     // Address to look up

    // If the selector is not quite right, set it up
    if( SelLAR(evalSel)==0 )
        evalSel = deb.r->ds;

    Addr.sel = evalSel;
    Addr.offset = arg;

    // TODO - rewrite this using getDWORD

    value[0] = AddrGetByte(&Addr); Addr.offset++;
    value[1] = AddrGetByte(&Addr); Addr.offset++;
    value[2] = AddrGetByte(&Addr); Addr.offset++;
    value[3] = AddrGetByte(&Addr);

    return( *(DWORD *)value );
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

// TODO - This function needs to simply set the error number in deb.error and return FALSE
// TODO - deb.error may need another optional argument to the error code, like in this case

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


/******************************************************************************
*
*   Memory stamp (checksum) functions.
*   They are not used at the moment since it turns out it takes too long to
*   compute checksum on all areas that we would be interested in.
*
*   TODO: Find the better use for these checksum functions.
*
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BYTE ComputeChecksum(BYTE *pMem, UINT size)                               *
*                                                                             *
*******************************************************************************
*
*   Computes a BYTE-checksum of a memory region.
*
*   Where:
*       pMem is the start memory address
*       size is the size in bytes of the memory region
*
*   Returns:
*       Checksum
*
******************************************************************************/
BYTE ComputeChecksum(BYTE *pMem, UINT size)
{
    BYTE bChecksum = 0;

    while( size-- )
    {
        bChecksum += *pMem++;
    }

    bChecksum = 0xFF - bChecksum;

    return( bChecksum );
}


/******************************************************************************
*                                                                             *
*   void ComputeStamp(TSTAMP *pStamp)                                         *
*                                                                             *
*******************************************************************************
*
*   Helper function that does the actual filling in of the TSTAMP structure.
*
******************************************************************************/
static void ComputeStamp(TSTAMP *pStamp)
{
    // Set the memory stamp on the following blocks:
    //  Symbol file heap
    //  History buffer heap
    //  Internal heap
    //  deb structure
    //  Win structure

    pStamp->hSymbolBufferHeapChecksum = ComputeChecksum(deb.hSymbolBufferHeap, deb.nSymbolBufferSize);
    pStamp->hHistoryBufferHeapChecksum = ComputeChecksum(deb.hHistoryBufferHeap, deb.nHistorySize);
    pStamp->hHeapChecksum = ComputeChecksum(deb.hHeap, MAX_HEAP);
    pStamp->debChecksum = 0; //ComputeChecksum((BYTE *)&deb, sizeof(TDEB));
    pStamp->WinChecksum = ComputeChecksum((BYTE *)&Win, sizeof(TWINDOWS));
}

/******************************************************************************
*                                                                             *
*   void SetMemoryStamp(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Stamps all the memory regions (areas) with the checksum
*
******************************************************************************/
void SetMemoryStamp(void)
{
    // Set the memory stamp on the following blocks:
    //  Symbol file heap
    //  History buffer heap
    //  Internal heap

    ComputeStamp(&Stamp);
}

/******************************************************************************
*                                                                             *
*   BOOL VerifyMemoryStamp(void)                                              *
*                                                                             *
*******************************************************************************
*
*   Checksums all the memory regions (areas) and makes sure the checksum match
*
*   Returns:
*       TRUE - Checksums are ok
*       FALSE - Memory has been compromised!
*
******************************************************************************/
BOOL VerifyMemoryStamp(void)
{
    TSTAMP CurrentStamp;

    ComputeStamp(&CurrentStamp);

    if( !memcmp(&Stamp, &CurrentStamp, sizeof(TSTAMP)) )
        return( TRUE );

    return( FALSE );
}
