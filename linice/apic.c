/******************************************************************************
*                                                                             *
*   Module:     apic.c                                                        *
*                                                                             *
*   Date:       08/02/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
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

        This module contains SMP and IO APIC support functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/02/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include PC architecture defines
#include "intel.h"                      // Include processor specific stuff
#include "iceface.h"                    // Include iceface module stub protos


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

// This is a redirection table for hardware interrupts that are normally mapped
// to interrupt vectors 0x20 to 0x2F.  When IO APIC is present, the hw IRQs
// are remapped. This table keeps that re-mapping as read from the IO APIC so
// we can abstract legacy case and IO APIC case.

static int IrqRedir[0x10] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F };


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern DWORD SpinUntilReset(DWORD *pSpinlock);


/******************************************************************************
*                                                                             *
*   void GetIrqRedirection(void)                                              *
*                                                                             *
*******************************************************************************
*
*   When IO APIC is present, we need to read its registers and change our
*   IO redirection table. We do that once on init.
*
******************************************************************************/
void GetIrqRedirection(void)
{
    int n;

    if( deb.fIoApic )
    {
        // Read first two IO APIC registers

        *(DWORD *)& deb.IoApic0 = ice_io_apic_read(0, 0);
        *(DWORD *)& deb.IoApic1 = ice_io_apic_read(0, 4);

        // Our redirection table contains a number of entries; read them in

        for(n=0; n<MAX_IOAPICREG; n++)
        {
            *(DWORD *)& deb.IoApicRedir[n]        = ice_io_apic_read(0, 0x10+n*2);      // Low-order dword
            *(DWORD *)& deb.IoApicRedir[n].dword2 = ice_io_apic_read(0, 0x10+n*2+1);    // High-order dword

            // If IO APIC redirection entry is not masked, it is active,
            // so we need to copy its interrupt into our redirection array
            if( !deb.IoApicRedir[n].Mask )
            {
                IrqRedir[n] = deb.IoApicRedir[n].IntVec;
            }
        }

        // Special case is 0x20, IRQ0 that is assigned to a local timer vector

        IrqRedir[0] = 0xEF; //LOCAL_TIMER_VECTOR;
    }
    // Else, the redirection map is unchanged...
}

int IrqRedirect(int nIrq)
{
    if( nIrq>=0x20 && nIrq<=0x2F )
        return( IrqRedir[nIrq-0x20] );

    return( nIrq );
}

int ReverseMapIrq(int nInt)
{
    if( nInt>=0x20 && nInt<=0x2F )
        return( IrqRedir[nInt-0x20] );

    return( nInt );
}


/******************************************************************************
*                                                                             *
*   void IntAck(int nInt)                                                     *
*                                                                             *
*******************************************************************************
*
*   Sends ack for a hardware interrupt either through a legacy PIC or
*   IO APIC
*
*   Where:
*       nInt is the current interrupt number that we are acknowledging
*
******************************************************************************/
void IntAck(int nInt)
{
    // We only ack hardware interrupts, not internal CPU
    if( nInt>=0x20 && nInt<0x30 )
    {
        // Either we use legacy PIC or IOAPIC for a particular interrupt:
        // By default, we use legacy PIC unless APIC is enabled and that
        // particular IRQ is being serviced by it

        if( deb.fIoApic )
        {
            // We find out if an interrupt uses APIC by looking up to our IRQ
            // translation table - should contain non-legacy interrupt number

            if( IrqRedir[nInt-0x20] != nInt )
            {
                // We are using IO APIC
                ice_ack_APIC_irq();

                return;
            }
        }

        // We are using legacy PIC

        if( (nInt >= 0x28) && (nInt < 0x30) )
            outp(PIC2, PIC_ACK);
        if( (nInt >= 0x20) && (nInt < 0x30) )
            outp(PIC1, PIC_ACK);
    }
}


/******************************************************************************
*                                                                             *
*   void IoApicClamp(int cpu)                                                 *
*                                                                             *
*******************************************************************************
*
*   Clamp all IO to a given cpu number, only if using IO APIC and SMP.
*   If we are only using APIC, we dont need to clamp since all IRQs should
*   already go to a single CPU.
*
*   Where:
*       cpu is the logical CPU number that we are directing all interrupts to
*
******************************************************************************/
void IoApicClamp(int cpu)
{
    int n;
    DWORD mask;

    if( deb.fIoApic && deb.fSmp )
    {
        // We modify IO APIC redirection registers to send all interrupts to the given CPU

        mask = 1 << cpu;
        mask <<= 24;                    // Up to the Destination field

        // Change only the redirection entries that are enabled

        for(n=0; n<MAX_IOAPICREG; n++)
        {
            if( !deb.IoApicRedir[n].Mask )
            {
                // Set the destination field to the given CPU

                ice_io_apic_write(0, 0x10+n*2+1, ((*(DWORD *)& deb.IoApicRedir[n].dword2) & 0x00FFFFFF) | mask);
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   void IoApicUnclamp()                                                      *
*                                                                             *
*******************************************************************************
*
*   Restore all IO APIC redirection registers to what they were before
*   we clamped them.
*
******************************************************************************/
void IoApicUnclamp()
{
    int n;

    if( deb.fIoApic && deb.fSmp )
    {
        for(n=0; n<MAX_IOAPICREG; n++)
        {
            if( !deb.IoApicRedir[n].Mask )
            {
                // We really only need to set the CPU mask...
                // io_apic_write(0, 0x10+n*2,   *(DWORD *)& deb.IoApicRedir[n]);
                ice_io_apic_write(0, 0x10+n*2+1, *(DWORD *)& deb.IoApicRedir[n].dword2);
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   void smpSpin(void *ptr)                                                   *
*                                                                             *
*******************************************************************************
*
*   Function that is called by smpSpinOtherCpus(void) to run on all other CPUs
*   while the primary CPU (the one that faulted into the debugger) is
*   executing the debugger
*
******************************************************************************/
void smpSpin(void *ptr)
{
    SpinUntilReset(&deb.fRunningIce);

    SetSysreg(&deb.sysReg);
}

/******************************************************************************
*                                                                             *
*   void smpSpinOtherCpus(void)                                               *
*                                                                             *
*******************************************************************************
*
*   Sends an IPI to all other CPU to start spinning on a 'fRunningIce'
*   semaphore.
*
******************************************************************************/
void smpSpinOtherCpus(void)
{
    ice_smp_call_function(smpSpin, NULL, TRUE, 0);
}

