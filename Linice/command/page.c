/******************************************************************************
*                                                                             *
*   Module:     page.c                                                        *
*                                                                             *
*   Date:       10/25/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        Commands that deal with page tables.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/25/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "intel.h"
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
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void GetSysreg( TSysreg * pSys );

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdPage(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display page table information.  There are two functions in one - when
*   we specify a vitual address as a parameter and when we issue command
*   without parameters.
*
******************************************************************************/
BOOL cmdPage(char *args, int subClass)
{
    TPage *pPD, *pPTE;                  // Pointers to PG and PTE
    DWORD address, pg, pt;              // Linear address and its pd, pte
    int pages = 1;                      // Number of pages to display
    int nLine = 1;                      // Standard dprinth line counter

    // Get the page directory virtual address
    pPD = (TPage *) (ice_page_offset() + deb.sysReg.cr3);

    if( *args )
    {
        // Parameters are specified

        // Read the address parameter
        if( Expression(&address, args, &args) )
        {
            // Next parameter is optional: L <length>
            if( *args=='L' || *args=='l' )
            {
                args++;

                // Read optional length parameter
                if( Expression((DWORD *)&pages, args, &args) )
                {
                    goto Proceed;
                }
            }
            else
                goto Proceed;
        }

        dprinth(1, "Syntax error");
        return(TRUE);
Proceed:

        dprinth(nLine++, "Linear    Physical  Attributes");

        // Make linear address page aligned
        address &= ~0xFFF;

        while( pages > 0 )
        {
            pg = address >> 22;
            pt = (address >> 12) & 1023;

            // If it is a 4Mb page, do it differently
            if( (deb.sysReg.cr4 & BITMASK(PSE_BIT)) && pPD[pg].fPS )
            {
                // 4Mb page

                if(dprinth(nLine++, "%08X  %08X  %s %c %c %c %s 4M %c %s",
                        address,
                        (pPD[pg].Index << 12) | (address & 0x003FF000),
                        pPD[pg].fPresent? "P ":"NP",
                        pPD[pg].fDirty? 'D':' ',
                        pPD[pg].fAccessed? 'A':' ',
                        pPD[pg].fUser? 'U':'S',
                        pPD[pg].fWrite? "RW":"R ",
                        pPD[pg].fGlobal? 'G':' ',
                        pPD[pg].fWriteThr? "WT":"  ")==FALSE)
                    break;

                address += 1024 * 1024 * 4;
                pages -= 1024;
            }
            else
            {
                // Regular 4Kb page - use second level page table
                if( pPD[pg].fPresent )
                {
                    // PD marks pte present, so we can examine it

                    pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

                    if( pPTE[pt].fPresent )
                    {
                        // Target page is present in the memory

                        if(dprinth(nLine++, "%08X  %08X  P  %c %c %c %s   %c %s",
                                address,
                                pPTE[pt].Index << 12,
                                pPTE[pt].fDirty? 'D':' ',
                                pPTE[pt].fAccessed? 'A':' ',
                                (pPTE[pt].fUser & pPD[pg].fUser)? 'U':'S',
                                (pPTE[pt].fWrite & pPD[pg].fWrite)? "RW":"R ",
                                pPTE[pt].fGlobal? 'G':' ',
                                pPTE[pt].fWriteThr? "WT":"  ")==FALSE)
                            break;
                    }
                    else
                    {
                        // Second level page table marks target page not present

                        if(dprinth(nLine++, "%08X  00000000  NP",
                                address )==FALSE)
                            break;
                    }
                }
                else
                {
                    // PD marks pte not present, we are pretty much done

                    if(dprinth(nLine++, "%08X  00000000  NP",
                            address )==FALSE)
                        break;
                }

                address += 1024 * 4;
                pages -= 1;
            }
        }
    }
    else
    {
        // No parameters - list all pages with somewhat different output

        // Print the first line with the page directory info
        dprinth(nLine++, "Page Directory Physical=%08X", deb.sysReg.cr3 );

        dprinth(nLine++, "Physical   Attributes          Linear Address Range");

        address = 0;                    // Start at address 0
        pages = 1024 * 1024;            // All memory

        while( pages > 0 )
        {
            pg = address >> 22;
            pt = (address >> 12) & 1023;

            // If it is a 4Mb page, do it differently
            if( (deb.sysReg.cr4 & BITMASK(PSE_BIT)) && pPD[pg].fPS )
            {
                // 4Mb page

                // Print only present pages
                if( pPD[pg].fPresent )
                {
                    if(dprinth(nLine++, "%08X   P  %c %c %c %s 4M %c %s %08X - %08X",
                            pPD[pg].Index << 12,
                            pPD[pg].fDirty? 'D':' ',
                            pPD[pg].fAccessed? 'A':' ',
                            pPD[pg].fUser? 'U':'S',
                            pPD[pg].fWrite? "RW":"R ",
                            pPD[pg].fGlobal? 'G':' ',
                            pPD[pg].fWriteThr? "WT":"  ",
                            address,
                            address + 1024 * 1024 * 4 - 1 )==FALSE)
                        break;
                }

                address += 1024 * 1024 * 4;
                pages -= 1024;
            }
            else
            {
                // Regular 4Kb page - use second level page table

                // Print only present pages
                if( pPD[pg].fPresent )
                {
                    pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

                    if( pPTE[pt].fPresent )
                    {
                        // Target page is present in the memory

                        if(dprinth(nLine++, "%08X   P  %c %c %c %s   %c %s  %08X - %08X",
                                pPTE[pt].Index << 12,
                                pPTE[pt].fDirty? 'D':' ',
                                pPTE[pt].fAccessed? 'A':' ',
                                (pPTE[pt].fUser & pPD[pg].fUser)? 'U':'S',
                                (pPTE[pt].fWrite & pPD[pg].fWrite)? "RW":"R ",
                                pPTE[pt].fGlobal? 'G':' ',
                                pPTE[pt].fWriteThr? "WT":"  ",
                                address,
                                address + 1024 * 4 - 1 )==FALSE)
                            break;
                    }
                }

                address += 1024 * 4;
                pages -= 1;
            }
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdPhys(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display all virtual addresses that correspond to a physical address.
*
******************************************************************************/
BOOL cmdPhys(char *args, int subClass)
{
    TPage *pPD, *pPTE;                  // Pointers to PG and PTE
    DWORD address, address_index;       // Physical address to look up
    int pg, pt;
    int nLine = 1;

    if( *args )
    {
        // Read the physical address parameter
        if( Expression(&address, args, &args) )
        {
            address_index = address >> 12;

            // Get the page directory virtual address
            pPD = (TPage *) (ice_page_offset() + deb.sysReg.cr3);

            // Search through the complete Intel page table structure
            for(pg=0; pg<1024; pg++)
            {
                // Page directory have to mark it present
                if( pPD[pg].fPresent )
                {
                    // If it is a 4Mb page, do it differently
                    if( (deb.sysReg.cr4 & BITMASK(PSE_BIT)) && pPD[pg].fPS )
                    {
                        // 4Mb page - maps 4Mb physical range
                        if( (address>>22)==(pPD[pg].Index>>10) )
                            if( dprinth(nLine++, "%08X", (pg << 22) | (address & 0x3FFFFF) )==FALSE )
                                break;
                    }
                    else
                    {
                        pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

                        // 4Kb page - loop through the page tables
                        for(pt=0; pt<1024; pt++)
                        {
                            if( pPTE[pt].fPresent )
                            {
                                if( address_index==pPTE[pt].Index )
                                    if( dprinth(nLine++, "%08X", (pg<<22) | (pt<<12) | (address & 0xFFF))==FALSE )
                                        break;
                            }
                        }
                    }
                }
            }
        }
        else
            dprinth(1, "Syntax error");
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdPeek(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Read from physical memory. There are 3 subclasses of this function:
*   byte (default), word and dword.
*
******************************************************************************/
BOOL cmdPeek(char *args, int subClass)
{
    DWORD address;                      // Physical address
    BYTE *ptr;                          // Virtual address

    // Read the physical address parameter
    if( Expression(&address, args, &args) )
    {
        // Map the physical memory into the virtual address space so we can access it
        ptr = ice_ioremap(address, 4);
        if( ptr )
        {
            // Read the value from the memory page
            switch(subClass)
            {
                case 0:                 // BYTE access
                    dprinth(1, "%02X", *ptr);
                    break;

                case 1:                 // WORD access
                    dprinth(1, "%04X", *(WORD *)ptr);
                    break;

                case 2:                 // DWORD access
                    dprinth(1, "%08X", *(DWORD *)ptr);
                    break;
            }

            ice_iounmap(ptr);
        }
        else
            dprinth(1, "Unable to map physical %08X", address);
    }
    else
        dprinth(1, "Syntax error");

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdPoke(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Write to physical memory. There are 3 subclasses of this function:
*   byte (default), word and dword.
*
******************************************************************************/
BOOL cmdPoke(char *args, int subClass)
{
    DWORD address, value;               // Physical address and value
    BYTE *ptr;                          // Virtual address

    // Read the physical address parameter
    if( Expression(&address, args, &args) )
    {
        // Read the value to store
        if( Expression(&value, args, &args) )
        {
            // Map the physical memory into the virtual address space so we can access it
            ptr = ice_ioremap(address, 4);
            if( ptr )
            {
                // Write a value to the memory page
                switch(subClass)
                {
                    case 0:                 // BYTE access
                        if( value <= 0xFF )
                        {
                            *(BYTE *)ptr = (BYTE) value;
                        }
                        break;

                    case 1:                 // WORD access
                        if( value <= 0xFFFF )
                        {
                            *(WORD *)ptr = (WORD) value;
                        }
                        break;

                    case 2:                 // DWORD access
                            *(DWORD *)ptr = value;
                        break;
                }

                ice_iounmap(ptr);
            }
            else
                dprinth(1, "Unable to map physical %08X", address);
        }
        else
            dprinth(1, "Parameters required");
    }
    else
        dprinth(1, "Parameters required");


    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   DWORD UserVirtToPhys(DWORD address)                                       *
*                                                                             *
*******************************************************************************
*
*   Returns physical address of the current application (user) virtual address.
*
*   Where:
*       virt - current user virtual address
*
*   Returns:
*       physical address
*       NULL if the page is not present
*
******************************************************************************/
DWORD UserVirtToPhys(DWORD address)
{
    TPage *pPD, *pPTE;                  // Pointers to PG and PTE
    DWORD phys = 0;                     // Assume NULL for the return value
    DWORD pg, pt;                       // Directory and page table index

    GetSysreg(&deb.sysReg);

    // Get the page directory virtual address
    pPD = (TPage *) (ice_page_offset() + deb.sysReg.cr3);

    // Find the page directory and page table index for the given virtual address
    pg = address >> 22;
    pt = (address >> 12) & 1023;

    // Is it a 4Mb page?
    if( (deb.sysReg.cr4 & BITMASK(PSE_BIT)) && pPD[pg].fPS )
    {
        // 4Mb page; present?
        if( pPD[pg].fPresent )
        {
            phys = (pPD[pg].Index << 12) | (pt << 12);
        }
    }
    else
    {
        // Regular 4Kb page - use second level page table
        if( pPD[pg].fPresent )
        {
            // PD marks pte present, so we can examine it
            pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

            if( pPTE[pt].fPresent )
            {
                // Target page is present in the memory
                phys = pPTE[pt].Index << 12;
            }
        }
    }

    return(phys);
}

