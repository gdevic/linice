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

#include "module-header.h"              // Versatile module header file

#define __NO_VERSION__
#include <linux/module.h>               // Include required module include
#include <linux/sched.h>                // What could we do w/o this one?

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD *pmodule;                  // Head of the module list

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
*   void MkBits(char *dest, DWORD bitfield, TBITS *bits)                      *
*                                                                             *
*******************************************************************************
*
*   MkBits - concat the bitmask meaning subfunction
*   Where:
*     dest is the destination string
*     bitfield is the value to examine
*     bits is the address of the description structure TBITS
*
******************************************************************************/
void MkBits(char *dest, DWORD bitfield, TBITS *bits)
{
    *dest = 0;

    while( bits->mask )
    {
        if( bitfield & bits->mask )
            strcat(dest, bits->name);
        bits++;
    }
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
        sel,
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
        if( sel < PAGE_OFFSET )
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
        if( intnum < PAGE_OFFSET )
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
******************************************************************************/
BOOL cmdCpu(char *args, int subClass)
{
    dprinth(1, "CS:EIP=%04X:%08X   SS:ESP=%04X:%08X", deb.r->cs, deb.r->eip, deb.r->ss, deb.r->esp);
    dprinth(2, "EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X", deb.r->eax, deb.r->ebx, deb.r->ecx, deb.r->edx);
    dprinth(3, "ESI=%08X   EDI=%08X   EBP=%08X   EFL=%08X", deb.r->esi, deb.r->edi, deb.r->ebp, deb.r->eflags);
    dprinth(4, "DS=%04X   ES=%04X   FS=%04X   GS=%04X", deb.r->ds, deb.r->es, deb.r->fs, deb.r->gs);

    MkBits(bits, deb.sysReg.cr0, bitsCR0);
    dprinth(5,  "CR0=%08X   %s", deb.sysReg.cr0, bits);
    dprinth(6,  "CR2=%08X", deb.sysReg.cr2);

    MkBits(bits, deb.sysReg.cr3, bitsCR3);
    dprinth(7,  "CR3=%08X   %s", deb.sysReg.cr3, bits);
    MkBits(bits, deb.sysReg.cr4, bitsCR4);
    dprinth(8,  "CR4=%08X   %s", deb.sysReg.cr4, bits);

    dprinth(9,  "DR0=%08X", deb.sysReg.dr[0]);
    dprinth(10, "DR1=%08X", deb.sysReg.dr[1]);
    dprinth(11, "DR2=%08X", deb.sysReg.dr[2]);
    dprinth(12, "DR3=%08X", deb.sysReg.dr[3]);
    dprinth(13, "DR6=%08X", deb.sysReg.dr6);
    dprinth(14, "DR7=%08X", deb.sysReg.dr7);

    MkBits(bits, deb.r->eflags, bitsEFL);
    dprinth(15, "EFL=%08X   %s IOPL=%d", deb.r->eflags, bits, (deb.r->eflags >> IOPL_BIT0 & 3));

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdModule(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display kernel loadable modules:
*       if no name is given, list all loaded modules
*       if a partial name is given (module*) display those modules
*       if a full name is given, display extende info for that module
*
******************************************************************************/
BOOL cmdModule(char *args, int subClass)
{
    int nLine = 1;                      // Line counter
    struct module* pMod, *pExact=NULL;  // Pointer to a current module and exact module
    int nLen = 0;                       // Assume every module
    int count;                          // Generic counter
    BOOL fExact = FALSE;                // Assume all modules
    const char *pModName;               // Pointer to a module name

    pMod = (struct module*) *pmodule;   // Get to the head of the module list
    if( pMod==NULL )
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

        for(; pMod; pMod = pMod->next )
        {
            // Get the pointer to a module name. We do it using a temp pointer
            // in order to fake the kernel name from NULL to "kernel"
            if( *pMod->name )
                pModName = pMod->name;
            else
                pModName = "kernel";

            if( nLen != 0 )
            {
                if( fExact && strcmp(pModName, args)!=0 )
                    continue;
                if( fExact && strcmp(pModName, args)==0 )
                    pExact = pMod;
                if( !fExact && strnicmp(pModName, args, nLen)!=0 )
                    continue;
            }

            MkBits(bits, pMod->flags, bitsModule);
            if( bits[0]==0 )
                strcat(bits, "UNINIT");

            if(!dprinth(nLine++, "%08X %-16s  %-6d  %-3d  %-3d %08X %08X   %d   %2X  %s",
                    (DWORD) pMod,
                    pModName,
                    pMod->size,
                    pMod->nsyms,
                    pMod->ndeps,
                    (DWORD) pMod->init,
                    (DWORD) pMod->cleanup,
                    GET_USE_COUNT(pMod),
                    pMod->flags,
                    bits ))
                break;
        }

        // For a single module, display additional info
        if( fExact && pExact )
        {
            struct module_symbol* pSym;         // Module symbols for extra info

            // We will display a list of exported symbols for this module
            pSym = pExact->syms;
            if( pSym )
            {
                count = pExact->nsyms;

                if( *pExact->name )
                    pModName = pExact->name;
                else
                    pModName = "kernel";
    
                // Do some basic sanity check
                if( pExact->nsyms>0 && pExact->nsyms<2000 )
                {
                    for(count=0; count<pExact->nsyms; count++ )
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
*   struct module *FindModule(const char *name)                               *
*                                                                             *
*******************************************************************************
*
*   Searches for a module with a given name.
*
*   Where:
*       name is the module name
*
*   Returns:
*       Address of the module descriptor
*       NULL if the module is not found
*
******************************************************************************/
struct module *FindModule(const char *name)
{
    struct module* pMod;                // Pointer to a current module

    pMod = (struct module*) *pmodule;   // Get to the head of the module list

    if( pMod )
    {
        for(; pMod ; pMod = pMod->next )
        {
            if( !strcmp(pMod->name, name) )
            {
                return( pMod );
            }
        }
    }

    return( NULL );
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
    dprinth(1, "Linice (C) 2000-2001 Goran Devic");

    if( *args )
    {
        dprinth(1, "Symbols check: %d", CheckSymtab(pIce->pSymTabCur));
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
*   current context to that process.
*
******************************************************************************/
BOOL cmdProc(char *args, int subClass)
{
    int nLine = 1;
    int pid;
    struct task_struct *pTask;

    if( *args )
    {
        // Switch to a given process ID (decimal)

        pid = (int) GetDec(&args);

        // Search the process list for the given PID
        for_each_task(pTask)
        {
            if( pTask->pid==pid )
                break;
        }
        
        if( pTask->pid==pid )
        {
            // Switch to that process ID

            // TODO: We can't do this safely this way

//            if( pTask!=current )
            {
                /* Re-load page tables */
                {
                    unsigned long new_cr3 = pTask->tss.cr3;
                    asm volatile("movl %0,%%cr3": :"r" (new_cr3));
                }
            }
        }
        else
        {
            // Did not find that process ID
            dprinth(nLine++, "Process ID=%d not found.", pid);
        }
    }
    else
    {
        // List the process information

        // Display the process head line
        dprinth(nLine++, "%c%cPID   TSS  Task      state    uid  gid  name",
            DP_SETCOLINDEX, COL_BOLD);

        for_each_task(pTask)
        {
            if( !dprinth(nLine++, "%c%c%4d  %04X %08X  %s %4d %4d %s",
                DP_SETCOLINDEX, pTask==current? COL_BOLD : COL_NORMAL,
                pTask->pid,
                pTask->tss.tr,
                (DWORD)pTask, 
                pTask->state==TASK_RUNNING?         "RUNNING ":
                pTask->state==TASK_INTERRUPTIBLE?   "SLEEPING":
                pTask->state==TASK_UNINTERRUPTIBLE? "UNINTR  ":
                pTask->state==TASK_ZOMBIE?          "ZOMBIE  ":
                pTask->state==TASK_STOPPED?         "STOPPED ":
                pTask->state==TASK_SWAPPING?        "SWAPPING":
                                                    "<?>     ",
                pTask->uid,
                pTask->gid,
                pTask->comm
                ))
                break;
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTss(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display current TSS structure. If the argument is given, display that
*   TSS selector.
*
******************************************************************************/
BOOL cmdTss(char *args, int subClass)
{
    int nLine = 1;
    int tr;
    TGDT_Gate *pGdt;
    struct thread_struct *tss;

    if( *args )
    {
        // Display a selected TSS structure

        tr = Evaluate(args, &args);
    }
    else
    {
        // TSS that is current should be the same as the one in current->tss
        tr = current->tss.tr;
    }

    pGdt = (TGDT_Gate *) (deb.gdt.base + (tr & ~7));

    if( pGdt->type==DESC_TYPE_TSS16B || pGdt->type==DESC_TYPE_TSS16A ||
        pGdt->type==DESC_TYPE_TSS32B || pGdt->type==DESC_TYPE_TSS32A )
    {
        tss = (struct thread_struct *) GET_GDT_BASE(pGdt);

        dprinth(nLine++, "TR=%04X   BASE=%08X  LIMIT=%X",
                tr,
                GET_GDT_BASE(pGdt),
                GET_GDT_LIMIT(pGdt));

        dprinth(nLine++, "LDT=%04X  GS=%04X  FS=%04X  DS=%04X  SS=%04X  CS=%04X  ES=%04X",
                tss->ldt,
                tss->gs,
                tss->fs,
                tss->ds,
                tss->ss,
                tss->cs,
                tss->es);

        dprinth(nLine++, "CR3=%08X", current->tss.cr3);

        dprinth(nLine++, "EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X  EIP=%08X",
                tss->eax,
                tss->ebx,
                tss->ecx,
                tss->edx,
                tss->eip);

        dprinth(nLine++, "ESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X  EFL=%08X",
                tss->esi,
                tss->edi,
                tss->ebp,
                tss->esp,
                tss->eflags);

        dprinth(nLine++, "SS0=%04X:%08X  SS1=%04X:%08X  SS2=%04X:%08X",
                tss->ss0,
                tss->esp0,
                tss->ss1,
                tss->esp1,
                tss->ss2,
                tss->esp2);

        dprinth(nLine++, "I/O Map Base=%04X  I/O Map Size=%X",
                (int)&tss->io_bitmap[0] - (int)tss,
                IO_BITMAP_SIZE);

        dprinth(nLine++, "CR2=%08X  trap_no=%08X  error_code=%X",
                tss->cr2,
                tss->trap_no,
                tss->error_code);
    }
    else
    {
        dprinth(nLine++, "Invalid TSS descriptor %04X", tr);
    }


    return( TRUE );
}

