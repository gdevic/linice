/******************************************************************************
*                                                                             *
*   Module:     sysinfo.c                                                     *
*                                                                             *
*   Date:       04/25/01                                                      *
*                                                                             *
*   Copyright (c) 2001 - 2001 Goran Devic                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        System information commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/25/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

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

// Define Global Descriptor Table system selector types
static char *gdtSystem[16] = {
    "Reserved",  "TSS16   ",  "LDT     ",  "TSS16   ",
    "CallG16 ",  "TaskG16 ",  "IntG16  ",  "TrapG16 ",
    "Reserved",  "TSS32   ",  "Reserved",  "TSS32   ",
    "CallG32 ",  "TaskG32 ",  "IntG32  ",  "TrapG32 "
};

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

BOOL PrintGDT(int nLine, DWORD base, DWORD sel)
{
    TGDT_Gate *pGdt = (TGDT_Gate *) (base + (sel & ~7));

    return(dprinth(nLine, "%04X  %s  %08X  %08X  %d    %s  %s\n",
        sel,
        gdtSystem[pGdt->type & 0xF],
        GET_GDT_BASE(pGdt),
        pGdt->granularity?
            (GET_GDT_LIMIT(pGdt) << 12) | 0xFFF
           : GET_GDT_LIMIT(pGdt),
        pGdt->dpl,
        pGdt->present? "P ":"NP",
        "."));
}


BOOL PrintIDT(int nLine, DWORD base, DWORD intnum)
{
    TIDT_Gate *pIdt = (TIDT_Gate *) (base + intnum * sizeof(TIDT_Gate));

    return(dprinth(nLine, "%04X  %s  %04X:%08X  DPL=%d    %s\n",
        intnum,
        gdtSystem[pIdt->type & 0xF],
        pIdt->selector,
        GET_IDT_BASE(pIdt),
        pIdt->dpl,
        pIdt->present? "P ":"NP"));
}


/******************************************************************************
*                                                                             *
*   BOOL cmdGdt(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display the Global Descriptor Table
*
******************************************************************************/
BOOL cmdGdt(char *args, int subClass)
{
    int nLine = 2;
    TDescriptor *pDesc = &deb.gdt;
    DWORD sel;

    if( *args != 0 )
    {
        // Display just the given selector or, if the selector given is larger
        // than 80000000h, display the GDT from that address

        sel = Evaluate(args, &args);
        if( sel < 0x80000000 )
        {
            PrintGDT(1, pDesc->base, sel);
            return( TRUE );
        }

        // Proceed with the complete GDT at the given address
        pDesc = (TDescriptor *) sel;
    }

    // No parameters - display complete GDT

    dprinth(1, "GDT base=%08X  limit=%X  (%d entries)\n",
        pDesc->base, pDesc->limit, (pDesc->limit+1) / sizeof(TGDT_Gate));

    sel = 0x0008;
    while( PrintGDT(nLine++, pDesc->base, sel)==TRUE && sel<pDesc->limit)
    {
        sel += sizeof(TGDT_Gate);
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdLdt(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display the Local Descriptor Table
*
******************************************************************************/
BOOL cmdLdt(char *args, int subClass)
{

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdIdt(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display the Interrupt Descriptor Table
*
******************************************************************************/
BOOL cmdIdt(char *args, int subClass)
{
    int nLine = 2;
    TDescriptor *pDesc = &deb.idt;
    DWORD intnum;

    if( *args != 0 )
    {
        // Display just the given int number or, if the selector given is larger
        // than 80000000h, display the IDT starting from that address

        intnum = Evaluate(args, &args);
        if( intnum < 0x80000000 )
        {
            PrintIDT(1, pDesc->base, intnum);
            return( TRUE );
        }

        // Proceed with the complete IDT at the given address
        pDesc = (TDescriptor *) intnum;
    }

    // No parameters - display complete IDT

    dprinth(1, "IDT base=%08X  limit=%X  (%d entries)\n",
        pDesc->base, pDesc->limit, (pDesc->limit+1) / sizeof(TIDT_Gate));

    intnum = 0;
    while( PrintIDT(nLine++, pDesc->base, intnum)==TRUE && intnum<pDesc->limit/sizeof(TIDT_Gate))
    {
        intnum++;
    }

    return( TRUE );
}

