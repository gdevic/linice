/******************************************************************************
*                                                                             *
*   Module:     sysinfo.c                                                     *
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

        System information commands

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
#include "ice-version.h"                // Include version file

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
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

static char *gdtRW[8] = {
    "RO", "RW", "RO ED", "RW ED",
    "EO", "RE", "EO CO", "RE CO"
};


// Define the bitmask structure defining the mask value and a name
typedef struct
{
    DWORD mask;
    char *name;
} TBITS, *PTBITS;

// Define bits of the register EFLAGS
static TBITS bitsEFL[] = {
{ VM_MASK, "VM " },
{ RF_MASK, "RF " },
{ NT_MASK, "NT " },
{ OF_MASK, "OF " },
{ DF_MASK, "DF " },
{ IF_MASK, "IF " },
{ TF_MASK, "TF " },
{ SF_MASK, "SF " },
{ ZF_MASK, "ZF " },
{ AF_MASK, "AF " },
{ PF_MASK, "PF " },
{ CF_MASK, "CF " },
{ 0 }
};

// Define bits of the register CR0
static TBITS bitsCR0[] = {
{ BITMASK(PG_BIT), "PG " },
{ BITMASK(CD_BIT), "CD " },
{ BITMASK(NW_BIT), "NW " },
{ BITMASK(AM_BIT), "AM " },
{ BITMASK(WP_BIT), "WP " },
{ BITMASK(NE_BIT), "NE " },
{ BITMASK(ET_BIT), "ET " },
{ BITMASK(TS_BIT), "TS " },
{ BITMASK(EM_BIT), "EM " },
{ BITMASK(MP_BIT), "MP " },
{ BITMASK(PE_BIT), "PE " },
{ 0 }
};

// Define bits of the register CR3
static TBITS bitsCR3[] = {
{ BITMASK(PCD_BIT), "PCD " },
{ BITMASK(PWT_BIT), "PWT " },
{ 0 }
};

// Define bits of the register CR4
static TBITS bitsCR4[] = {
{ BITMASK(PCE_BIT), "PCE " },
{ BITMASK(PGE_BIT), "PGE " },
{ BITMASK(MCE_BIT), "MCE " },
{ BITMASK(PAE_BIT), "PAE " },
{ BITMASK(PSE_BIT), "PSE " },
{ BITMASK(DE_BIT),  "DE " },
{ BITMASK(TSD_BIT), "TSD " },
{ BITMASK(PVI_BIT), "PVI " },
{ BITMASK(VME_BIT), "VME " },
{ 0 }
};

// Define bits of module flags
static TBITS bitsModule[] = {
{ MOD_JUST_FREED, "FREED " },
{ MOD_USED_ONCE, "ONCE " },
{ MOD_VISITED, "VIS " },
{ MOD_AUTOCLEAN, "ACL " },
{ MOD_DELETED, "DEL " },
{ MOD_RUNNING, "RUN " },
{ 0 }
};


// Buffer space for the bits string
static char bits[40];

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int CheckSymtab(TSYMTAB *pSymtab);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   int MkBits(char *dest, DWORD bitfield, TBITS *bits)                       *
*                                                                             *
*******************************************************************************
*
*   MkBits - concat the bitmask meaning subfunction
*   Where:
*     dest is the destination string
*     bitfield is the value to examine
*     bits is the address of the description structure TBITS
*
*   Returns: Always 1
*
******************************************************************************/
int MkBits(char *dest, DWORD bitfield, TBITS *bits)
{
    *dest = 0;

    while( bits->mask )
    {
        if( bitfield & bits->mask )
            strcat(dest, bits->name);
        bits++;
    }

    return( 1 );
}


BOOL PrintGDT(int nLine, DWORD base, DWORD sel)
{
    char *pType, *pRW;
    TGDT_Gate *pGdt = (TGDT_Gate *) (base + (sel & ~7));

    if( pGdt->system )
    {
        if( pGdt->size32 )
            if( pGdt->type & 8 )
                pType = "Code32  ";
            else
                pType = "Data32  ";
        else
            if( pGdt->type & 8 )
                pType = "Code16  ";
            else
                pType = "Data16  ";

        pRW = gdtRW[(pGdt->type >> 1) & 3];
    }
    else
    {
        pType = gdtSystem[pGdt->type];

        if( pGdt->type==DESC_TYPE_TSS16B || pGdt->type==DESC_TYPE_TSS32B )
            pRW = "B";
        else
            pRW = "";
    }

    return(dprinth(nLine, "%04X  %s  %08X  %08X  %d    %s  %s",
        (sel & ~7) + pGdt->dpl,
        pType,
        GET_GDT_BASE(pGdt),
        pGdt->granularity?
            (GET_GDT_LIMIT(pGdt) << 12) | 0xFFF
           : GET_GDT_LIMIT(pGdt),
        pGdt->dpl,
        pGdt->present? "P ":"NP",
        pRW));
}


BOOL PrintIDT(int nLine, DWORD base, DWORD intnum)
{
    TIDT_Gate *pIdt = (TIDT_Gate *) (base + intnum * sizeof(TIDT_Gate));

    return(dprinth(nLine, "%04X  %s  %04X:%08X  DPL=%d    %s",
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
        // than PAGE_OFFSET, display the GDT from that address

        sel = Evaluate(args, &args);
        if( sel < ice_page_offset() )
        {
            PrintGDT(1, pDesc->base, sel);
            return( TRUE );
        }

        // Proceed with the complete GDT at the given address
        pDesc = (TDescriptor *) sel;
    }

    // No parameters - display complete GDT

    dprinth(1, "GDT base=%08X  limit=%X  (%d entries)",
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
        // than PAGE_OFFSET, display the IDT starting from that address

        intnum = Evaluate(args, &args);
        if( intnum < ice_page_offset() )
        {
            PrintIDT(1, pDesc->base, intnum);
            return( TRUE );
        }

        // Proceed with the complete IDT at the given address
        pDesc = (TDescriptor *) intnum;
    }

    // No parameters - display complete IDT

    dprinth(1, "IDT base=%08X  limit=%X  (%d entries)",
        pDesc->base, pDesc->limit, (pDesc->limit+1) / sizeof(TIDT_Gate));

    intnum = 0;
    while( PrintIDT(nLine++, pDesc->base, intnum)==TRUE && intnum<pDesc->limit/sizeof(TIDT_Gate))
    {
        intnum++;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdCpu(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display CPU registers
*
*   Additional options:
*       CPU s       - stores CPU registers into a virtual register slot
*       CPU r       - restores CPU registers
*
******************************************************************************/
BOOL cmdCpu(char *args, int subClass)
{
    static TREGS CPU;                   // CPU register store
    int nLine = 1;                      // Line counter

    if( *args=='s' )
    {
        memcpy(&CPU, deb.r, sizeof(TREGS));

        dprinth(1, "CPU registers saved");

        return( TRUE );
    }
    else
    if( *args=='r' )
    {
        memcpy(deb.r, &CPU, sizeof(TREGS));

        deb.fRedraw = TRUE;

        dprinth(1, "CPU registers recalled");

        // If we changed eip, we need to recalculate the whole context
        SetSymbolContext(deb.r->cs, deb.r->eip);

        return( TRUE );
    }

    if(dprinth(nLine++, "CPU #%d", deb.cpu)
    && dprinth(nLine++, "CS:EIP=%04X:%08X   SS:ESP=%04X:%08X", deb.r->cs, deb.r->eip, deb.r->ss, deb.r->esp)
    && dprinth(nLine++, "EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X", deb.r->eax, deb.r->ebx, deb.r->ecx, deb.r->edx)
    && dprinth(nLine++, "ESI=%08X   EDI=%08X   EBP=%08X   EFL=%08X", deb.r->esi, deb.r->edi, deb.r->ebp, deb.r->eflags)
    && dprinth(nLine++, "DS=%04X   ES=%04X   FS=%04X   GS=%04X", deb.r->ds, deb.r->es, deb.r->fs, deb.r->gs)

    && MkBits(bits, deb.sysReg.cr0, bitsCR0)
    && dprinth(nLine++, "CR0=%08X   %s", deb.sysReg.cr0, bits)
    && dprinth(nLine++, "CR2=%08X", deb.sysReg.cr2)

    && MkBits(bits, deb.sysReg.cr3, bitsCR3)
    && dprinth(nLine++, "CR3=%08X   %s", deb.sysReg.cr3, bits)
    && MkBits(bits, deb.sysReg.cr4, bitsCR4)
    && dprinth(nLine++, "CR4=%08X   %s", deb.sysReg.cr4, bits)

    && dprinth(nLine++, "DR0=%08X", deb.sysReg.dr[0])
    && dprinth(nLine++, "DR1=%08X", deb.sysReg.dr[1])
    && dprinth(nLine++, "DR2=%08X", deb.sysReg.dr[2])
    && dprinth(nLine++, "DR3=%08X", deb.sysReg.dr[3])
    && dprinth(nLine++, "DR6=%08X", deb.sysReg.dr6)
    && dprinth(nLine++, "DR7=%08X", deb.sysReg.dr7)

    && MkBits(bits, deb.r->eflags, bitsEFL)
    && dprinth(nLine++, "EFL=%08X   %s IOPL=%d", deb.r->eflags, bits, (deb.r->eflags >> IOPL_BIT0 & 3)) );

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdModule(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display list of kernel loadable modules or more information about a single
*   module.
*       if no name is given, list all loaded modules
*       if a partial name is given (module*) display those modules
*       if a full name is given, display extended info for that module
*
******************************************************************************/
BOOL cmdModule(char *args, int subClass)
{
    int nLine = 1;                      // Line counter
    int nLen = 0;                       // Assume every module
    int count;                          // Generic counter
    BOOL fExact = FALSE;                // Assume all modules
    const char *pModName;               // Pointer to a module name
    void *pmodule, *pExact = NULL;      // Kernel pmodule pointer and exact module
    struct module_symbol* pSym;         // Module symbols for extra info

    TMODULE Mod;                        // Current module internal structure
    TMODULE Exact;                      // Module information that matched user query

    // Get the pointer to the module structure (internal)
    pmodule = ice_get_module(NULL, &Mod);

    if( pmodule==NULL )
    {
        dprinth(1, "module_list symbol not found.. Module info not available");
    }
    else
    {
        if( *args )
        {
            // Module name or a partial name is given
            if( args[strlen(args)-1]=='*' )
                nLen = strlen(args)-1;
            else
                nLen = strlen(args), fExact=TRUE;
        }

        // Display all modules matching search criteria
        dprinth(nLine++, "%c%cModule   Name              Size   Syms Deps init()   cleanup() Use Flags:",
            DP_SETCOLINDEX, COL_BOLD);

        while( pmodule )
        {
            // Get the pointer to a module name. We do it using a temp pointer
            // in order to fake the kernel name from NULL to "kernel"
            if( Mod.name && *Mod.name )
                pModName = Mod.name;
            else
                pModName = "kernel";

            // This little argument gymnastic lets us separate different ways we
            // specify module(s) as parameter to this command (all/single/single extra)
            if( nLen != 0 )
            {
                if( fExact && strcmp(pModName, args)!=0 )
                    continue;
                if( fExact && strcmp(pModName, args)==0 )
                {
                    pExact = pmodule;
                    memcpy(&Exact, &Mod, sizeof(TMODULE));
                }
                if( !fExact && strnicmp(pModName, args, nLen)!=0 )
                    continue;
            }

            MkBits(bits, Mod.flags, bitsModule);
            if( bits[0]==0 )
                strcat(bits, "UNINIT");

            if(!dprinth(nLine++, "%08X %-16s  %-6d  %-4d %-3d %08X %08X   %d   %2X  %s",
                    (DWORD) pmodule,
                    pModName,
                    Mod.size,
                    Mod.nsyms,
                    Mod.ndeps,
                    (DWORD) Mod.init,
                    (DWORD) Mod.cleanup,
                    Mod.use_count,
                    Mod.flags,
                    bits ))
                break;

            // Get the next module in the linked list
            pmodule = ice_get_module(pmodule, &Mod);
        }

        // For a single module, display additional info
        if( fExact && pExact )
        {
            // We will display a list of exported symbols for this module
            pSym = Exact.syms;
            if( pSym )
            {
                count = Exact.nsyms;

                if( Exact.name )
                    pModName = Exact.name;
                else
                    pModName = "kernel";

                // Do some basic sanity check
                if( Exact.nsyms>0 && Exact.nsyms<2000 )
                {
                    for(count=0; count<Exact.nsyms; count++ )
                    {
                        // TODO: Check the validity of these pointers before using them!

                        if(!dprinth(nLine++, "%02X) %08X  %s!%s", count, pSym->value, pModName, pSym->name))
                            break;

                        pSym++;
                    }
                }
            }


            // TODO: What additional info would we like to see???
            ;
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL FindModule(const char *name, TMODULE *pMod)                          *
*                                                                             *
*******************************************************************************
*
*   Searches for a module with a given name.
*
*   Where:
*       name is the module name
*       pMod is the address of the structure to store module members
*           Note: This structure will be modified even if a module was not found!!
*
*   Returns:
*       TRUE - module with a given name was found
*              the module members are copied into pMod
*       FALSE - module with a given name was not found
*
******************************************************************************/
BOOL FindModule(const char *name, TMODULE *pMod)
{
    void *pmodule;                      // Kernel pmodule pointer

    // Get the pointer to the module structure (internal)
    pmodule = ice_get_module(NULL, pMod);

    while( pmodule )
    {
        if( !strcmp(pMod->name, name) )
            return( TRUE );

        // Get the next module in the linked list
        pmodule = ice_get_module(pmodule, pMod);
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdVer(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display version information. Also, display some system debugger info.
*
******************************************************************************/
BOOL cmdVer(char *args, int subClass)
{
    dprinth(1, "Linice (C) 2000-2002 by Goran Devic.  All Rights Reserved.");
    dprinth(1, "Version: %d.%d", LINICEVER >> 8, LINICEVER & 0xFF);

    if( *args )
    {
        dprinth(1, "Symbol check: %d", CheckSymtab(pIce->pSymTabCur));
        dprinth(1, "OS Page Offset: %08X", ice_page_offset());
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdProc(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display process information. If process ID is given, it switches the
*   current context to that process (TODO)
*
******************************************************************************/

//----------------------------------------------------------------------------
// Task structure callback
//----------------------------------------------------------------------------
int ice_for_each_task_cb(int *pLine, TTASK *pTask)
{
    (*pLine)++;

    if( !dprinth(*pLine, "%c%c%4d  %04X %08X  %s %4d %4d %s",
        DP_SETCOLINDEX, pTask->ptask==ice_get_current()? COL_BOLD : COL_NORMAL,
        pTask->pid,
        /*pTask->tss.tr*/ 0,
        pTask->ptask,
        pTask->state==TASK_RUNNING?         "RUNNING ":
        pTask->state==TASK_INTERRUPTIBLE?   "SLEEPING":
        pTask->state==TASK_UNINTERRUPTIBLE? "UNINTR  ":
        pTask->state==TASK_ZOMBIE?          "ZOMBIE  ":
        pTask->state==TASK_STOPPED?         "STOPPED ":
/*      pTask->state==TASK_SWAPPING?        "SWAPPING":*/
                                            "<?>     ",
        pTask->uid,
        pTask->gid,
        pTask->comm
        ))
        return( 0 );

    return( 1 );
}

//----------------------------------------------------------------------------
// Main cmdProc()
//----------------------------------------------------------------------------
BOOL cmdProc(char *args, int subClass)
{
    int nLine = 1;
    TTASK Task;

    if( *args )
    {
        // Switch to a given process ID (decimal)
    }
    else
    {
        // List the process information

        // Display the process head line
        dprinth(nLine++, "%c%cPID   TSS  Task      state    uid  gid  name",
            DP_SETCOLINDEX, COL_BOLD);

        ice_for_each_task(&nLine, &Task, ice_for_each_task_cb);

    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL DumpTSS(TADDRDESC *pAddr, int tr, int limit)                         *
*                                                                             *
*******************************************************************************
*
*   Helper function that verifies and displays a TSS structure
*
*   Where:
*       pAddr is the address of the TSS structure
*       tr if 0, ignored; otherwise it is a TSS selector to print
*       limit is the TSS selector limit value
*
******************************************************************************/
BOOL DumpTSS(TADDRDESC *pAddr, int tr, DWORD limit)
{
    int nLine = 1;                      // Print line counter
    TSS_Struct *pTss;                   // Final verified TSS address

    if( VerifyRange(pAddr, limit)==TRUE )
    {
        // We can safely access what is possibly a TSS
        pTss = (TSS_Struct *) pAddr->offset;

        // If a TR selector is given, display additional line, otherwise skip it
        // since we really dont know this information if only given an address
        if( tr )
        {
            if(dprinth(nLine++, "TR=%04X   BASE=%08X  LIMIT=%X",
                    tr,
                    pAddr->offset,
                    limit)==FALSE) return( TRUE );
        }

        if(dprinth(nLine++, "LDT=%04X  GS=%04X  FS=%04X  DS=%04X  SS=%04X  CS=%04X  ES=%04X",
                pTss->ldt,
                pTss->gs,
                pTss->fs,
                pTss->ds,
                pTss->ss,
                pTss->cs,
                pTss->es)==FALSE) return( TRUE );

        if(dprinth(nLine++, "CR3=%08X", pTss->cr3)==FALSE) return( TRUE );

        if(dprinth(nLine++, "EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X  EIP=%08X",
                pTss->eax,
                pTss->ebx,
                pTss->ecx,
                pTss->edx,
                pTss->eip)==FALSE) return( TRUE );

        if(dprinth(nLine++, "ESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X  EFL=%08X",
                pTss->esi,
                pTss->edi,
                pTss->ebp,
                pTss->esp,
                pTss->eflags)==FALSE) return( TRUE );

        if(dprinth(nLine++, "SS0=%04X:%08X  SS1=%04X:%08X  SS2=%04X:%08X",
                pTss->ss0,
                pTss->esp0,
                pTss->ss1,
                pTss->esp1,
                pTss->ss2,
                pTss->esp2)==FALSE) return( TRUE );

        if(dprinth(nLine++, "I/O Map Base=%04X  I/O Map Size=%X",
                (int)&pTss->ioperm - (int)pTss,
                ice_get_io_bitmap_size())==FALSE) return( TRUE );

        return( TRUE );

    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTss(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display current TSS structure. If the argument is given, it can be either
*   a GDT selector or an address of a TSS in memory:
*
*   If the parameter is <  4096, it is a selector
*   If the parameter is >= 4096, it is an address
*
******************************************************************************/
BOOL cmdTss(char *args, int subClass)
{
    DWORD tr;                           // Task register value
    TGDT_Gate *pGdt;
    TADDRDESC Addr;

    Addr.sel = GetKernelDS();             // Only use kernel DS for this command

    if( *args )
    {
        // Get the Task Register selector from the user argument

        tr = Evaluate(args, &args);

        if( tr>=4096 )
        {
            // Argument specified an address

            Addr.offset = tr;

            if( DumpTSS(&Addr, 0, 4095)==FALSE )
            {
                dprinth(1, "Invalid TSS address %04X:%08X", Addr.sel, Addr.offset);
            }

            return( TRUE );
        }
    }
    else
    {
        // Get the Task Register selector number from the running kernel
        tr = getTR();
    }

    // Verify TR selector - privilege level and the limit
    if( (tr&7)==0 && tr<=deb.gdt.limit )
    {
        pGdt = (TGDT_Gate *) (deb.gdt.base + tr);

        // Verify that the descriptor indeed contains a TSS structure
        if( pGdt->type==DESC_TYPE_TSS16B || pGdt->type==DESC_TYPE_TSS16A ||
            pGdt->type==DESC_TYPE_TSS32B || pGdt->type==DESC_TYPE_TSS32A )
        {
            Addr.offset = GET_GDT_BASE(pGdt);

            if( DumpTSS(&Addr, tr, GET_GDT_LIMIT(pGdt)) )
            {
                return( TRUE );
            }
        }
    }

    // For everything else, we had a bad selector
    dprinth(1, "Invalid TSS selector %04X", tr);

    return( TRUE );
}

