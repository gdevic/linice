/******************************************************************************
*                                                                             *
*   Module:     syscall.c                                                     *
*                                                                             *
*   Date:       12/20/01                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()


/******************************************************************************
*                                                                             *
*   External functions                                                        *
*                                                                             *
******************************************************************************/

extern void ArmDebugReg(int nDr, TADDRDESC Addr);
extern TSYMTAB *SymTabFind(char *name);
extern BOOL SymTabSetupRelocOffset(TSYMTAB *pSymTab, DWORD dwInitModule, DWORD dwInitModuleSample);
extern void SymTabRelocate(TSYMTAB *pSymTab, int factor);
extern BOOL FindModule(TMODULE *pMod, char *pName, int nNameLen);
extern BOOL FindSymbol(TExItem *item, char *pName, int *pNameLen);

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern unsigned long *sys_table;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef asmlinkage int (*PFNEXIT)(int status);
static PFNEXIT sys_exit = NULL;

typedef asmlinkage int (*PFNFORK)(struct pt_regs regs);
static PFNFORK sys_fork = NULL;

typedef asmlinkage int (*PFNINITMODULE)(const char *name, void *image);
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
    dprinth(1, "SYSCALL: %s: exit(%d)", ice_get_current_comm(), status);

    return( sys_exit(status) );
}


asmlinkage int SyscallFork(struct pt_regs regs)
{
    int retval;

    retval = sys_fork(regs);

    dprinth(1, "SYSCALL: fork(%s) = %d", ice_get_current_comm(), retval);

    return( retval );
}


asmlinkage int SyscallInitModule(const char *name, void *image)
{
    TSYMTAB *pSymTab;
    int retval;
    TADDRDESC Addr;
    DWORD dwInitModule, dwSampleBase;   // Relocation helper variables
    TMODULE Mod;                        // Module information structure

    dprinth(1, "SYSCALL: init_module(`%s', %08X)", name, image);

    // Find a kernel module descriptor that has been created after a previous system call
    // create_module. That should be found - else this call will fail anyways...
    // We dont expect this to happen - at this point we should have the module descriptor.
    if( FindModule(&Mod, (char *) name, strlen(name)) )
    {
        // Try to find if we have a symbol table loaded for this module, so we can relocate it
        // Find the symbol table with the name of this module
        pSymTab = SymTabFind((char *)name);
        if( pSymTab )
        {
            // image       is the user memory image to load
            // Mod.pmodule is the module descriptor
            // ice_get_module_init() returns the actual address of the init_module once it gets copied there
            //
            // Example:
            //    image = 08092AB0 (user space)
            //    Mod.pmodule = CC8DF000     (kernel buffer to copy the module image)
            //    get_module_init = CC8DF077 (init_module in the kernel space, not yet copied there)

            dwInitModule = (DWORD) ice_get_module_init(image);
            dwSampleBase = (DWORD) image + ((DWORD) dwInitModule - (DWORD) Mod.pmodule);

            // Setup relocation offsets
            if( SymTabSetupRelocOffset(pSymTab, dwInitModule, dwSampleBase) )
            {
                // Relocate symbol table
                SymTabRelocate(pSymTab, 1);

                // Make that symbol table the current one
                deb.pSymTabCur = pSymTab;
            }

            //===================================================================

            // For now, set a breakpoint on an init_module function that gets called when
            // we execute this system call.

            // Read in all the system state registers so we can modify DR3 and write back
            GetSysreg(&deb.sysReg);

            // We set and arm DR3 register and then write back to the CPU all system registers
            Addr.sel    = GetKernelCS();
            Addr.offset = (DWORD) ice_get_module_init(image);

            ArmDebugReg(3, Addr);

            // Write back all system registers including DR3/DR7 in order to break in
            SetSysreg(&deb.sysReg);

            //===================================================================
        }
    }

    // Call the original Linux kernel module load routine to complete the load
    retval = sys_init_module(name, image);

    if( retval!=0 )
    {
        // Module load failed - Need to relocate back its symbol table, if present

        // We search for the module again since it might have been deleted in the meantime
        pSymTab = SymTabFind((char *)name);
        if( pSymTab )
        {
            SymTabRelocate(pSymTab, -1);
        }

        dprinth(1, "SYSCALL: init_module failed with error code=%d", retval);
    }

    return( retval );
}


asmlinkage int SyscallDeleteModule(const char *name)
{
    TSYMTAB *pSymTab;
    int retval = -1;                    // By default assume error

    dprinth(1, "SYSCALL: delete_module(`%s')", name);

    // We will not break on cleanup_module since we break on init_module, so
    // a breakpoint can be placed at that time if this function needs to be debugged.

    // If the linice is still operational, but we got a call to unload it,
    // refuse since we should be only unloading it using our utility

    if( deb.fOperational && !strcmp(name, "linice") )
    {
        ice_printk("ERROR: Use 'linsym -x' to unload Linice debugger.\n");
    }
    else
    {
        // Call the original delete_module
        retval = sys_delete_module(name);

        if( retval==0 )
        {
            // TODO: If we had any breakpoints placed within this module, we need to disable them







            // Module has been deleted from the kernel space. We need to find
            // if we had a symbol table mapped to it so we can revert the relocation

            // Find the symbol table with the name of this module
            pSymTab = SymTabFind((char *)name);
            if( pSymTab )
            {
                // Relocate symbol table back by the reloc offset
                SymTabRelocate(pSymTab, -1 );
            }
        }
        else
        {
            dprinth(1, "SYSCALL: delete_module returned nonzero! (%d)", retval);
        }
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
    INFO("HookSyscall() at %08X\n", (DWORD)sys_table[0]);

    sys_exit = (PFNEXIT) sys_table[__NR_exit];
    sys_table[__NR_exit] = (unsigned long) SyscallExit;

    sys_fork = (PFNFORK) sys_table[__NR_fork];
    sys_table[__NR_fork] = (unsigned long) SyscallFork;

    sys_init_module = (PFNINITMODULE) sys_table[__NR_init_module];
    sys_table[__NR_init_module] = (unsigned long) SyscallInitModule;

    sys_delete_module = (PFNDELETEMODULE) sys_table[__NR_delete_module];
    sys_table[__NR_delete_module] = (unsigned long) SyscallDeleteModule;
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
    INFO("UnHookSyscall()\n");

    if( sys_exit )
        sys_table[__NR_exit] = (unsigned long) sys_exit;

    if( sys_fork )
        sys_table[__NR_fork] = (unsigned long) sys_fork;

    if( sys_init_module )
        sys_table[__NR_init_module] = (unsigned long) sys_init_module;

    if( sys_delete_module )
        sys_table[__NR_delete_module] = (unsigned long) sys_delete_module;

    sys_exit = NULL;
    sys_fork = NULL;
    sys_init_module = NULL;
    sys_delete_module = NULL;
}

