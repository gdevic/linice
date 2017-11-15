/******************************************************************************
*                                                                             *
*   Module:     pci.c                                                         *
*                                                                             *
*   Date:       10/25/00                                                      *
*                                                                             *
*   Copyright (c) 2001 - 2001 Goran Devic                                     *
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

        PCI information dump command

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

#include <linux/pci.h>                  // Include PCI bus defines

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures

#include "pcihdr.h"                     // Include PCI header file

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

static int nLine;                       // dprinth line counter

#define PCI_TERSE       0x0001
#define PCI_RAW         0x0002
#define PCI_EXTENDED    0x0004
#define PCI_BYTE        0x0010
#define PCI_WORD        0x0020
#define PCI_DWORD       0x0040

static int opt;                         // command line options

// Static buffer to store the PCI config bytes after we read them

static BYTE configBuf[256];

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

//============================================================================
// Returns the vendor name string
//============================================================================
static char *GetVendorName(int vendor)
{
    PCI_VENTABLE *pVendor;
    int count;

    // Search for the vendor name string
    pVendor = PciVenTable;
    count   = PCI_VENTABLE_LEN;

    while( count-- )
    {
        if( pVendor->VenId==vendor )
            return(pVendor->VenFull);
        pVendor++;
    }
    return("(unknown)");
}

//============================================================================
// Returns the chip name string
//============================================================================
static char *GetChipName(int vendor, int device)
{
    PCI_DEVTABLE *pDev;
    int count;

    // Search for the device name
    pDev  = PciDevTable;
    count = PCI_DEVTABLE_LEN;

    while( count-- )
    {
        if( pDev->VenId==vendor && pDev->DevId==device )
            return(pDev->Chip);
        pDev++;
    }
    return("");
}

//============================================================================
// Returns the device description string
//============================================================================
static char *GetChipDesc(int vendor, int device)
{
    PCI_DEVTABLE *pDev;
    int count;

    // Search for the device description
    pDev  = PciDevTable;
    count = PCI_DEVTABLE_LEN;

    while( count-- )
    {
        if( pDev->VenId==vendor && pDev->DevId==device )
            return(pDev->ChipDesc);
        pDev++;
    }
    return("");
}


/******************************************************************************
*                                                                             *
*   void ReadPCIConfig(struct pci_dev *dev, int start, int bytes)             *
*                                                                             *
*******************************************************************************
*
*   Fills the configBuf buffer with the values from the PCI config registers
*   for a particular device.
*
*   Where:
*       pci is the kernel PCI device
*       start is the starting PCI address (0-255)
*       bytes is the number of bytes to read
*
******************************************************************************/
static void ReadPCIConfig(struct pci_dev *dev, int start, int bytes)
{
    bytes /= 4;                         // Make it DWORDS
    while( bytes-- )
    {
        pci_read_config_dword(dev, start, (unsigned *)&configBuf[start]);
        start += 4;
    }
}

/******************************************************************************
*                                                                             *
*   BOOL DumpPCI(struct pci_dev *pci)                                         *
*                                                                             *
*******************************************************************************
*
*   This function dumps a specific PCI device information in all variations
*   of outputs.
*
*   Where:
*       pci is the kernel PCI structure to dump
*
*   Returns:
*       FALSE if the printing should be stopped
*
******************************************************************************/
static BOOL DumpPCI(struct pci_dev *p)
{
    int address = 0, count = 0, i;
    TPCI pci_store;                     // Place to store PCI information
    TPCI *pci = &pci_store;             // PCI device information pointer

    // Get the information about that PCI device
    ice_get_pci_info(&pci_store, p);

    if( opt & PCI_TERSE )
    {
        // =============================================================================
        // Brief PCI information : -terse
        // =============================================================================
        if(!dprinth(nLine++, "%02X/%02X/%02X %04X-%04X %s",
                pci->bus, PCI_SLOT(pci->devfn), PCI_FUNC(pci->devfn),
                pci->vendor, pci->device, GetVendorName(pci->vendor))) return(FALSE);
    }
    else
    {
        // =============================================================================
        // Default PCI information
        // =============================================================================
        if( pci->pDevice )
            if(!dprinth(nLine++, "%s", pci->pDevice)) return(FALSE);

        if(!dprinth(nLine++, "Bus %02X  Device %02X  Function: %02X",
                pci->bus, PCI_SLOT(pci->devfn), PCI_FUNC(pci->devfn))) return(FALSE);
        if(!dprinth(nLine++, "     Vendor: %04X    %s", pci->vendor, GetVendorName(pci->vendor)))  return(FALSE);

        if(!dprinth(nLine++, "     Device: %04X    %s  %s", pci->device,
            GetChipName(pci->vendor, pci->device), GetChipDesc(pci->vendor, pci->device)))  return(FALSE);

        for(i=0; i<6; i++)
        {
            if( pci->base_address[i] )
                if(!dprinth(nLine++, "     Base address %d: %08X", i+1, pci->base_address[i] ))  return(FALSE);
        }

        if( pci->rom_address )
            if(!dprinth(nLine++, "     ROM address: %08X", pci->rom_address ))  return(FALSE);

        if(!dprinth(nLine++, "     IRQ: %02X   Bus Master: %d", pci->irq, pci->master ))  return(FALSE);

        // If option was -extended, dump all 256 bytes out
        if( opt & PCI_EXTENDED )
        {
            address = 0x50;
            count   = 0x100 - address;
        }

        // If the option was -raw, dump first 0x40 bytes out
        if( opt & PCI_RAW )
        {
            address = 0x00;
            count   = 0x40 - address;
        }

        // =============================================================================
        // Extra PCI information : -raw -extended
        // =============================================================================
        // Dump them depending on the requested format
        if( (opt & PCI_EXTENDED) || (opt & PCI_RAW) )
        {
            // Read PCI config registers from address to address+count
            ReadPCIConfig(p, address, count);

            for(; count>0; count -= 16, address += 16 )
            {
                if( opt & PCI_BYTE )
                {
                    if( !dprinth(nLine++, "       %02X: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
                        address,
                        configBuf[address+0],  configBuf[address+1],  configBuf[address+2],  configBuf[address+3],
                        configBuf[address+4],  configBuf[address+5],  configBuf[address+6],  configBuf[address+7],
                        configBuf[address+8],  configBuf[address+9],  configBuf[address+10], configBuf[address+11],
                        configBuf[address+12], configBuf[address+13], configBuf[address+14], configBuf[address+15]))
                        return(FALSE);
                }
                else
                if( opt & PCI_WORD )
                {
                    if( !dprinth(nLine++, "       %02X: %04X %04X %04X %04X  %04X %04X %04X %04X",
                        address,
                        *(WORD *)&configBuf[address+0],  *(WORD *)&configBuf[address+2],
                        *(WORD *)&configBuf[address+4],  *(WORD *)&configBuf[address+6],
                        *(WORD *)&configBuf[address+8],  *(WORD *)&configBuf[address+10],
                        *(WORD *)&configBuf[address+12], *(WORD *)&configBuf[address+14]))
                        return(FALSE);
                }
                else
                {
                    if( !dprinth(nLine++, "       %02X: %08X %08X  %08X %08X",
                        address,
                        *(DWORD *)&configBuf[address+0],  *(DWORD *)&configBuf[address+4],
                        *(DWORD *)&configBuf[address+8],  *(DWORD *)&configBuf[address+12]))
                        return(FALSE);
                }
            }
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdPci(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display PCI device/bus stat.
*
******************************************************************************/
BOOL cmdPci(char *args, int subClass)
{
    struct pci_dev *pci;
    DWORD bus, device, function;
    nLine = 1;
    opt = 0;

    // Get optional command line options
    if( *args )
    {
        // ["-terse" | "-raw" | "-extended"]

        if( !strnicmp(args, "-t", 2) )
        {
            while( *args && *args!=' ' ) args++;
            opt  |= PCI_TERSE;
        }
        else
        if( !strnicmp(args, "-r", 2) )
        {
            while( *args && *args!=' ' ) args++;
            opt  |= PCI_RAW;
        }
        else
        if( !strnicmp(args, "-e", 2) )
        {
            while( *args && *args!=' ' ) args++;
            opt  |= PCI_EXTENDED;
        }

        // [-b | -w | -d]
        while( *args && *args==' ' ) args++;

        if( !strnicmp(args, "-b", 2) )
        {
            args += 2;
            opt  |= PCI_BYTE;
        }
        else
        if( !strnicmp(args, "-w", 2) )
        {
            args += 2;
            opt  |= PCI_WORD;
        }
        else
        if( !strnicmp(args, "-d", 2) )
        {
            args += 2;
            opt  |= PCI_DWORD;
        }
        else
            opt |= PCI_DWORD;           // Default dump format is dword

        if( Expression(&bus, args, &args) )
        {
            if( Expression(&device, args, &args) )
            {
                if( Expression(&function, args, &args) )
                {
                    if( bus<256 )
                    {
                        if( device<32 )
                        {
                            if( function<8 )
                            {
                                goto Proceed;       // All right!
                            }
                            else
                                dprinth(1, "Function number (%d) can't be greater than 7", function);
                        }
                        else
                            dprinth(1, "Device number (%d) can't be greater than 31", device);
                    }
                    else
                        dprinth(1, "Bus number (%d) can't be greater than 255", bus);

                    return(TRUE);
                }
            }
            dprinth(1, "Invalid command line");
            return(TRUE);
        }
    }
    // Value that indicates to list all devices
    bus = 0xFF;

Proceed:
    // Get the root PCI node and walk it
    pci = ice_get_pci();

    if( pci )
    {
        while( ice_is_pci(pci) )
        {
            // If we selected a particular device, check for it; otherwise, dump every one
            if( bus==0xFF )
            {
                if( DumpPCI(pci)==FALSE )
                    break;
            }
            else
            {
                // If we found a specific bus/device/functon, dump it and exit
                if( pci->bus->number==bus && PCI_SLOT(pci->devfn)==device && PCI_FUNC(pci->devfn)==function )
                {
                    DumpPCI(pci);
                    return(TRUE);
                }
            }

            pci = ice_get_pci_next(pci);
        }
    }
    else
    {
        dprinth(1, "pci_devices = NULL");
    }

    return(TRUE);
}

