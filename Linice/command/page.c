/******************************************************************************
*                                                                             *
*   Module:     page.c                                                        *
*                                                                             *
*   Date:       10/25/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
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

        Commands that deal with page tables.

    A physical page allocated to process VM will have at least two virtual mappings:
    one in kernel memory at PAGE_OFFSET+physical_page_address, and one at some
    address less than PAGE_OFFSET in process VM. Such pages may also have additional
    mappings: for example, multiple processes executing the same program share the
    program's executable code by mapping the same physical pages in their respective
    page tables.

    HIGHMEM pages don't have a permanent virtual kernel address, so they are exception.
    Because of them, we need to map PTE ourselves in order to access them since
    we cannot rely any longer on the formula:
         pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096)

    In order to map an arbitrary physical page (but only one at a time!!)
    We modify the page directory page only, since we know its linear address:
        PD[0] -> Maps itself
        PD[1] -> Maps the physical page we want to see

    This effectively maps the page we want to see to the second 4Mb block, or
    at the address of 4Mb, with the page naturally 4K in size.

    The only drawback to it is we now need to assume that the first 2 PD
    entries will be 0, which is safe, I believe.

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

// Place to store the original values of first 2 PD entries during the remap

static DWORD selfMapping[2];

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void GetSysreg( TSysreg * pSys );
extern void FlushTLB(void);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   DWORD mapPhysicalPage(DWORD physAddress)                                  *
*                                                                             *
*******************************************************************************
*
*   Temporary internally map a physical page to a known address.
*   This is used to access pages that can not be viewed by any other mapping,
*   such is Page Tables which do not conform to the kernel 2.2 formula.
*
*   Where:
*       physAddress is the address of the page to map
*
*   Returns:
*       Linear address of the page
*
*   Note: You MUST call unmapPhysicalPage() to release the mapping before
*         going back to the user process.
*
******************************************************************************/
static DWORD mapPhysicalPage(DWORD physAddress)
{
    DWORD *p;                           // Linear address of the PD
    TPage *pPD;                         // Different way to view PD

    // Get the page directory virtual address
    p = (DWORD *) (ice_page_offset() + deb.sysReg.cr3);
    pPD = (TPage *) p;                  // Two ways of looking at the value

    // Store away the original values from the first two entries
    selfMapping[0] = p[0];
    selfMapping[1] = p[1];

    p[0] = p[1] = 0;                    // Clean the first two entries

    // Map itself into the linear address 0
    pPD[0].Index = deb.sysReg.cr3 >> 12;
    pPD[0].fPresent = TRUE;

    // Map our page into the linear address of 4 K
    pPD[1].Index = physAddress >> 12;
    pPD[1].fPresent = TRUE;

    FlushTLB();                         // Flush the TLB

    // Now this page is visible at the address of 4K
    return( 1024 * 4 );
}

/******************************************************************************
*                                                                             *
*   void unmapPhysicalPage(void)                                              *
*                                                                             *
*******************************************************************************
*
*   Releases the temporary internal page mapping.
*   This function can be called a number of times since it only writes back a
*   cached value; however, mapping should be called first!
*
******************************************************************************/
static void unmapPhysicalPage(void)
{
    DWORD *pPD;                         // Linear address of the PD

    // Get the page directory virtual address
    pPD = (DWORD *) (ice_page_offset() + deb.sysReg.cr3);

    // Restore the original values from the first two entries
    pPD[0] = selfMapping[0];
    pPD[1] = selfMapping[1];

    FlushTLB();                         // Flush the TLB
}

/******************************************************************************
*                                                                             *
*   BOOL cmdPage(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display page table information for the complete address space or a
*   section of it.
*
******************************************************************************/
BOOL cmdPage(char *args, int subClass)
{
    TPage *pPD, *pPTE;                  // Pointers to PG and PTE
    DWORD address, pg, pt;              // Linear address and its pd, pte
    int pages = 1;                      // Number of pages to display by default
    int nLine = 1;                      // Standard dprinth line counter

    // Get the page directory virtual address
    pPD = (TPage *) (ice_page_offset() + deb.sysReg.cr3);

    if( *args )
    {
        // ----------------------------------------------------------------------
        // Parameters are specified - we are dumping partial address space
        // ----------------------------------------------------------------------

        // Read the address parameter
        if( Expression(&address, args, &args) )
        {
            // Next parameter is optional: L <num_pages>
            if( *args=='L' || *args=='l' )
            {
                args++;

                // Read optional number of pages parameter
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
            pg = address >> 22;                 // 10 bits wide indexes array of DWORDs in page directory (PD)
            pt = (address >> 12) & 0x3FF;       // 10 bits wide indexes array of DWORDs into a page table (PTE)

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

                // Skip 4 Mb
                address += 1024 * 1024 * 4;
                pages -= 1024;
            }
            else
            {
                // Regular 4Kb page - use second level page table

                // This mapping used to work but now PTE pages are swappable
                // We still do it if the user asked for the first 2 entries in the PD
                if( pg==0 || pg==1 )
                    pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);
                else
                    pPTE = (TPage *) mapPhysicalPage(pPD[pg].Index * 4096);

                if( pPD[pg].fPresent )
                {
                    // PD marks pte present, so we can examine it

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
                        {
                            if( !(pg==0 || pg==1) ) unmapPhysicalPage();
                            break;
                        }
                    }
                    else
                    {
                        // Second level page table marks target page not present

                        if(dprinth(nLine++, "%08X  00000000  NP",
                                address )==FALSE)
                        {
                            if( !(pg==0 || pg==1) ) unmapPhysicalPage();
                            break;
                        }
                    }
                }
                else
                {
                    // PD marks pte not present, we are pretty much done

                    if(dprinth(nLine++, "%08X  00000000  NP",
                            address )==FALSE)
                    {
                        if( !(pg==0 || pg==1) ) unmapPhysicalPage();
                        break;
                    }
                }

                // Skip 4 K
                address += 1024 * 4;
                pages -= 1;

                if( !(pg==0 || pg==1) ) unmapPhysicalPage();
            }
        }
    }
    else
    {
        // ----------------------------------------------------------------------
        // No parameters - list all pages with somewhat different output
        // ----------------------------------------------------------------------

        // Print the first line with the page directory info
        dprinth(nLine++, "Page Directory Physical=%08X", deb.sysReg.cr3 );

        dprinth(nLine++, "Physical   Attributes          Linear Address Range");

        address = 0;                    // Start at address 0
        pages = 1024 * 1024;            // All memory

        // Start with the 8Mb due to our page window allocation
        address += 1024 * 1024 * 8;
        pages   -= 1024 * 2;

        while( pages > 0 )
        {
            pg = address >> 22;                 // 10 bits wide indexes array of DWORDs in page directory (PD)
            pt = (address >> 12) & 0x3FF;       // 10 bits wide indexes array of DWORDs into a page table (PTE)

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

                // Skip 4 Mb
                address += 1024 * 1024 * 4;
                pages -= 1024;
            }
            else
            {
                // Regular 4Kb page - use second level page table

                // This mapping used to work but now PTE pages are swappable
                // pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

                pPTE = (TPage *) mapPhysicalPage(pPD[pg].Index * 4096);

                // Print only present pages
                if( pPD[pg].fPresent )
                {
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
                        {
                            unmapPhysicalPage();
                            break;
                        }
                    }
                }

                // Skip 4 K
                address += 1024 * 4;
                pages -= 1;

                unmapPhysicalPage();
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

            // Search through the complete page table structure
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
                        // This mapping used to work but now PTE pages are swappable
                        // pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

                        pPTE = (TPage *) mapPhysicalPage(pPD[pg].Index * 4096);

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
                        unmapPhysicalPage();
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
*   TODO: Possibly expand this function to take the "L" length parameter?
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

            // This mapping used to work but now PTE pages are swappable
            // pPTE = (TPage *) (ice_page_offset() + pPD[pg].Index * 4096);

            pPTE = (TPage *) mapPhysicalPage(pPD[pg].Index * 4096);

            if( pPTE[pt].fPresent )
            {
                // Target page is present in the memory
                phys = pPTE[pt].Index << 12;
            }

            unmapPhysicalPage();
        }
    }

    return(phys);
}
