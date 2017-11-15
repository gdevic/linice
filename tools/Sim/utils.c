/******************************************************************************
*                                                                             *
*   Module:     utils.c                                                       *
*                                                                             *
*   Date:       05/23/2003                                                    *
*                                                                             *
*   Copyright (c) 2003-2005 Goran Devic                                       *
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

        This module contains utility functions.

*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos

#undef open

#include <assert.h>                     // Include assert macros
#include <stdlib.h>                     // Include standard library file
#include <stdio.h>                      // Include standard IO library file
#include <string.h>                     // Include string header file

#include "sim.h"                        // Include sim header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD kbd;
extern DWORD scan;
extern DWORD *pmodule;
extern DWORD start_sym;
extern DWORD stop_sym;
extern int ice_debug_level;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

BOOL fLoaded = FALSE;                   // Is module virtually loaded?

// This pattern is used to fake search to the keyboard hook code
BYTE pattern[] = { 0, 0xE8, 0, 0, 0, 0 };

// This is the content of VGA CRTC registers
BYTE CRTC[256] = { 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, };

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   int system2(char *pString)                                                *
*                                                                             *
*******************************************************************************
*   Call the shell with the command
******************************************************************************/
int system2(char *pString)
{
    char *p;

    // Check the special linsym commands
    if(!strnicmp(pString, "insmod", 6))
    {
        if(fLoaded) return(-1);

        // Request to load a module
        // Get the line parameters and assign them
        // "insmod -x -f linice_`uname -r`/linice.o ice_debug_level=1 kbd=%d scan=%d pmodule=%d start_sym=%d stop_sym=%d "

        p = strstr(pString, "ice_debug_level=");
        sscanf(p+16, "%d", &ice_debug_level);

        p = strstr(pString, "scan=");
        sscanf(p+6, "%d", &scan);

        p = strstr(pString, "pmodule=");
        sscanf(p+8, "%d", &pmodule);

        p = strstr(pString, "start_sym=");
        sscanf(p+10, "%d", &start_sym);

        p = strstr(pString, "stop_sym=");
        sscanf(p+9, "%d", &stop_sym);

        // Assign the fake keyboard hook pattern
        kbd = (DWORD)pattern;
        scan = kbd + 6;

        // Virtually load this device driver

        IceInitModule();

        fLoaded = TRUE;
    }
    else
    if(!strnicmp(pString, "rmmod", 5))
    {
        if(!fLoaded) return(-1);

        // Request to unload a module
        IceCleanupModule();

        fLoaded = FALSE;
    }

    return( 0 );
}

int SimOpen(char *pFile, int mode)
{
    int fd;

    // Special case is opening a linice device to communicate to it
    if(!strcmp(pFile, "/dev/"DEVICE_NAME))
    {
        // Return false if the module is not "loaded"
        if( !fLoaded )
            return( -1 );

        // Opening a device driver "file"

        fd = 1;
    }
    else
    {
        // Opening a regular file
        fd = open(pFile, mode);
    }

    return(fd);
}

int SimIoctl(int handle, int code, void *p)
{
    // Simulate IOCTL call to the device driver

    return( DriverIOCTL(NULL, NULL, code, p) );
}

char *getlogin(void)
{
    return("root");
}

/******************************************************************************
*                                                                             *
*   BOOL ReadKbdMapping(BYTE TargetLayout[3][128], char *pRequestedLayout)    *
*                                                                             *
*******************************************************************************
*   Copy the alternate keyboard mapping.
******************************************************************************/
BOOL ReadKbdMapping(BYTE TargetLayout[3][128], char *pRequestedLayout)
{
    return( 0 );
}


/******************************************************************************
*   The following functions have to be in this order:
******************************************************************************/

void MemAccess_START() {}

int GetByte(WORD sel, DWORD offset)
{
    if(offset>=0x0400000 && offset<0x07F00000)
    {
        return(*(BYTE *)offset);
    }
#if 0
    else
    if(offset>=PAGE_OFFSET && offset<(PAGE_OFFSET+SIM_PAGE_OFFSET_SIZE))
    {
        return(*(BYTE *)(offset - PAGE_OFFSET + pPageOffset));
    }
#endif
    return( 0x100 );
}

DWORD GetDWORD(WORD sel, DWORD offset)
{
    if(offset>=0x0400000 && offset<0x07F00000)
    {
        return(*(DWORD *)offset);
    }
#if 0
    else
    if(offset>=PAGE_OFFSET && offset<(PAGE_OFFSET+SIM_PAGE_OFFSET_SIZE))
    {
        return(*(DWORD *)(offset - PAGE_OFFSET + pPageOffset));
    }
#endif
    return( 0x100 );
}

void SetDWORD(WORD sel, DWORD offset, DWORD value)
{
//    assert(0);
}


DWORD SetByte(WORD sel, DWORD offset, BYTE value)
{
    assert(0);
    return(0);
}

void MemAccess_FAULT() {}

void MemAccess_END() {}

/******************************************************************************
*   End of functions that have to be in that order.
******************************************************************************/

DWORD IceIntHandlers[0x30];
DWORD IceIntHandler80;

void DebuggerEnterBreak(void);
void DebuggerEnterDelayedArm(void);

void GetIDT(TDescriptor *p)
{
    p->base = IDT.base;
    p->limit = IDT.limit;
}

void GetGDT(TDescriptor *p)
{
    p->base = GDT.base;
    p->limit = GDT.limit;
}

void SET_IDT(DWORD IDT)
{
//    assert(0);
}

void SET_CR0(DWORD cr0)
{
//    assert(0);
}

WORD getTR(void)
{
    assert(0);
    return( 0 );
}

DWORD SelLAR(WORD Sel)
{
    // Allow getting any value
    return( 1 );
}

DWORD GetRdtsc(BYTE *p)
{
    BYTE b = *(p+7);
    b++;
    *(p+7) = b;
    return(b);
}

void LocalCLI()
{
}

void LocalSTI()
{
}

DWORD TestAndReset(DWORD *pSpinlock)
{
    DWORD spin = *pSpinlock;
    *pSpinlock = 0;
    return( spin );
}

DWORD SpinlockTest(DWORD *pSpinlock)
{
    return( *pSpinlock );
}

void  SpinlockSet(DWORD *pSpinlock)
{
    *pSpinlock = 1;
}

void  SpinlockReset(DWORD *pSpinlock)
{
    *pSpinlock = 0;
}

DWORD SpinUntilReset(DWORD *pSpinlock)
{
    assert(0);
    return( 0 );
}

void GetSysreg( TSysreg * pSys )
{
//    assert(0);
}

void SetSysreg( TSysreg * pSys )
{
//    assert(0);
}

void SetDebugReg(TSysreg * pSys)
{
}

void FlushTLB(void)
{
}

void memset_w(void *dest, WORD data, int size)
{
    for(size; size>0; size--)
    {
        *(WORD *)((int)dest + size*2 - 2) = data;
    }
}

void memset_d(void *dest, DWORD data, int size)
{
    for(size; size>0; size--)
    {
        *(DWORD *)((int)dest + size*4 - 4) = data;
    }
}

void SyscallExecve(int p1, int p2)
{
}

BYTE ReadCRTC(int index)
{
    return( CRTC[index&0xFF] );
}

void WriteCRTC(int index, int value)
{
    CRTC[index&0xFF] = value & 0xFF;
}

void WriteMdaCRTC(int index, int value)
{
}

BYTE ReadSR(BYTE index)
{
    return(0xFF);
}

void WriteSR(BYTE index, BYTE value)
{
}

void machine_restart(char *cmd)
{
    assert(0);
}

void machine_halt(void)
{
    assert(0);
}

void machine_power_off(void)
{
    assert(0);
}


void Outpb(DWORD port, DWORD value)
{
}

void Outpw(DWORD port, DWORD value)
{
}

void Outpd(DWORD port, DWORD value)
{
}

DWORD Inpb(DWORD port)
{
    assert(0);
    return( 0xFF );
}

DWORD Inpw(DWORD port)
{
    assert(0);
    return( 0xFFFF );
}

DWORD Inpd(DWORD port)
{
    assert(0);
    return( 0xFFFFFFFF );
}

void ArmHookSwitch(void)
{
}

void InsertTaskSwitchHandler()
{
}

BYTE TaskSwitchHookBuffer[32];
BYTE TaskSwitchHookBufferKernelCodeLine;

DWORD Checksum1(DWORD start, DWORD len)
{
    return( 0 );
}

void ObjectStart(void)
{
}

void ObjectEnd(void)
{
}

