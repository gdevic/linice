/******************************************************************************
*                                                                             *
*   Module:     syscall.c                                                     *
*                                                                             *
*   Date:       12/20/01                                                      *
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

        System call hook functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 12/20/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

#include <asm/unistd.h>                 // Include system call numbers
#define __NO_VERSION__
#include <linux/module.h>


/******************************************************************************
*                                                                             *
*   External functions                                                        *
*                                                                             *
******************************************************************************/

extern void GetSysreg( TSysreg * pSys );
extern void SetSysreg( TSysreg * pSys );
extern int ArmDebugReg(int nDr, TADDRDESC Addr);
extern TSYMTAB *SymTabFind(char *name);
extern void SymTabRelocate(TSYMTAB *pSymTab, int offCode, int offData);
extern struct module *FindModule(const char *name);

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern unsigned long sys_call_table[];

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef asmlinkage int (*PFNEXIT)(int status);
static PFNEXIT sys_exit = NULL;

typedef asmlinkage int (*PFNFORK)(struct pt_regs regs);
static PFNFORK sys_fork = NULL;

typedef asmlinkage int (*PFNINITMODULE)(const char *name, struct module *image);
static PFNINITMODULE sys_init_module = NULL;

typedef asmlinkage int (*PFNDELETEMODULE)(const char *name);
static PFNDELETEMODULE sys_delete_module = NULL;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

asmlinkage int SyscallExit(int status)
{
    int retval;

    dprinth(1, "SYSCALL: %s: exit(%d)", current->comm, status);

    retval = sys_exit(status);

    return( retval );
}


asmlinkage int SyscallFork(struct pt_regs regs)
{
    int retval;

    retval = sys_fork(regs);

    dprinth(1, "SYSCALL: fork(%s) = %d", current->comm, retval);

    return( retval );
}


asmlinkage int SyscallInitModule(const char *name, struct module *image)
{
    TSYMTAB *pSymTab;
    int retval;
    TADDRDESC Addr;
    BYTE *pInitFunctionOffset;
    DWORD dwInitFunctionSymbol;
    DWORD dwDataReloc, dwSampleOffset;
    TSYMRELOC  *pReloc;                 // Symbol table relocation header
    struct module *pMod;                // *this* module kernel descriptor

    dprinth(1, "SYSCALL: init_module(`%s', %08X)", name, image);

    // Find a kernel module descriptor that has been created after a previous system call
    // create_module. That should be found - else this call will fail anyways...
    pMod = FindModule(name);
    if( pMod )
    {
        // Try to find if we have a symbol table loaded for this module, so we can relocate it
        // Find the symbol table with the name of this module
        pSymTab = SymTabFind((char *)name);
        if( pSymTab )
        {
            // Get the offset of the init_module global symbol from the symbol table
            if( SymbolName2Value(pSymTab, &dwInitFunctionSymbol, "init_module") )
            {
                dprinth(1, "SYSCALL: Relocating symbols for `%s'", pSymTab->sTableName);

                // Details of relocation scheme are explained in ParseReloc.c file

                // --- relocating code section ---

                // Get the real kernel address of the init_module function after relocation
                pInitFunctionOffset = (BYTE *) image->init;
    
                // Store private reloc adjustment values
                pSymTab->pPriv->relocCode = (int)(pInitFunctionOffset - dwInitFunctionSymbol);

                // --- relocating data section ---

                // Find the symbol table relocation block
                pReloc = (TSYMRELOC *) SymTabFindSection(pSymTab, HTYPE_RELOC);
                if( pReloc )
                {
//                    dprinth(1, "refOffset=%08X symOffset=%08X", pReloc->refOffset, pReloc->symOffset );

                    // Find the address within the code segment from which we will read the offset to
                    // our data. Relocation block contains the relative offset from the init_module function
                    // to our dword sample that we need to take.

                    // The only thing is we need to read sample from the user copy since the
                    // module had not been copied to kernel space...

                    dwSampleOffset = ((DWORD)image->init - pReloc->refOffset) - (DWORD)pMod;

                    dwDataReloc = *(DWORD *) ((DWORD)image + dwSampleOffset);

                    // image is in the user address space since we still did not call sys_init_module to
                    // copy it over to the kernel address space. 
                    
                    // TODO: Do we need to be more careful about all these accessing of user space? 
                    // Remember - we do not have debugger PF/GPF hooked at this point ?!?!?

                    pSymTab->pPriv->relocData = dwDataReloc - pReloc->symOffset;

                }
                else
                {
                    // There was not a single global variable to use for relocation. Odd, but
                    // possible... In that case it does not really matter not to relocate data...

                    dprinth(1, "SYSCALL: Symbol table missing HTYPE_RELOC so setting data==code");

                    pSymTab->pPriv->relocData = pSymTab->pPriv->relocCode;
                }
        
                // Relocate symbol table by the required offset
                SymTabRelocate(pSymTab, pSymTab->pPriv->relocCode, pSymTab->pPriv->relocData);

                // Make that symbol table the current one

                pIce->pSymTabCur = pSymTab;

                //===================================================================

                // For now, set a breakpoint on an init_module function that gets called when
                // we execute this system call.
        
                // Read in all the system state registers so we can modify DR3 and write back
                GetSysreg(&deb.sysReg);
        
                // We set and arm DR3 register and then write back to the CPU all system registers
                Addr.sel    = __KERNEL_CS;
                Addr.offset = (DWORD) image->init;
        
                ArmDebugReg(3, Addr);
        
                // Write back all system registers including DR3/DR7 in order to break in
                SetSysreg(&deb.sysReg);
        
                //===================================================================
            }
        }
    }
    else
    {
        dprinth(1, "SYSCALL: Kernel module descriptor not found!");
    }

    retval = sys_init_module(name, image);

    if( retval==0 )
    {
        // Module loaded ok
        ;
    }
    else
    {
        // Module load failed - Need to relocate back its symbol table, if present

        // We search for the module again since it might have been deleted in the meantime
        pSymTab = SymTabFind((char *)name);

        if( pSymTab )
        {
            SymTabRelocate(pSymTab, -pSymTab->pPriv->relocCode, -pSymTab->pPriv->relocData);

            pSymTab->pPriv->relocCode = 0;
            pSymTab->pPriv->relocData = 0;
        }

        dprinth(1, "SYSCALL: init_module returned nonzero! (%d)", retval);
    }

    return( retval );
}


asmlinkage int SyscallDeleteModule(const char *name)
{
    TSYMTAB *pSymTab;
    int retval;

    dprinth(1, "SYSCALL: delete_module(`%s')", name);

    // We will not break on cleanup_module since we break on init_module, so
    // a breakpoint can be placed at that time if this function needs to be debugged.

    retval = sys_delete_module(name);

    if( retval==0 )
    {
        // If we had any breakpoints placed within this module, we
        // need to disable them


        // Module has been deleted from the kernel space. We need to find
        // if we had a symbol table mapped to it so we can mark it as
        // not relocated (?) or not assigned

        // Find the symbol table with the name of this module
        pSymTab = SymTabFind((char *)name);
        if( pSymTab )
        {
            // Symbol table was loaded.. Relocate it back to 0-based
            dprinth(1, "SYSCALL: Relocating module symbols back to 0 from %08X\n", pSymTab->pPriv->relocCode);

            // Relocate symbol table back by the reloc offset
            SymTabRelocate(pSymTab, -pSymTab->pPriv->relocCode, -pSymTab->pPriv->relocData);

            pSymTab->pPriv->relocCode = 0;
            pSymTab->pPriv->relocData = 0;
        }
    }
    else
    {
        dprinth(1, "SYSCALL: delete_module returned nonzero! (%d)", retval);
    }

    return( retval );
}


/******************************************************************************
*                                                                             *
*   void HookSyscall(void)                                                    *
*                                                                             *
*******************************************************************************
*
*   Hooks system calls
*
******************************************************************************/
void HookSyscall(void)
{
    INFO(("HookSyscall() at %08X\n", (DWORD)sys_call_table));

    sys_exit = (PFNEXIT) sys_call_table[__NR_exit];
    sys_call_table[__NR_exit] = (unsigned long) SyscallExit;

    sys_fork = (PFNFORK) sys_call_table[__NR_fork];
    sys_call_table[__NR_fork] = (unsigned long) SyscallFork;

    sys_init_module = (PFNINITMODULE) sys_call_table[__NR_init_module];
    sys_call_table[__NR_init_module] = (unsigned long) SyscallInitModule;

    sys_delete_module = (PFNDELETEMODULE) sys_call_table[__NR_delete_module];
    sys_call_table[__NR_delete_module] = (unsigned long) SyscallDeleteModule;
}

/******************************************************************************
*                                                                             *
*   void UnHookSyscall(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Unhooks system calls
*
******************************************************************************/
void UnHookSyscall(void)
{
    INFO(("UnHookSyscall()\n"));

    if( sys_exit )
        sys_call_table[__NR_exit] = (unsigned long) sys_exit;

    if( sys_fork )
        sys_call_table[__NR_fork] = (unsigned long) sys_fork;

    if( sys_init_module )
        sys_call_table[__NR_init_module] = (unsigned long) sys_init_module;

    if( sys_delete_module )
        sys_call_table[__NR_delete_module] = (unsigned long) sys_delete_module;

    sys_exit = NULL;
    sys_fork = NULL;
    sys_init_module = NULL;
    sys_delete_module = NULL;
}

