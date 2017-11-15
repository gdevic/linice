/******************************************************************************
*                                                                             *
*   Module:     driver.c                                                      *
*                                                                             *
*   Date:       09/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This module contains linice device driver functions.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/01/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()
#include "errno.h"                      // Include kernel error numbers
#include "ioctl.h"                      // Include IO control macros

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TDEB         deb   = {0};               // Live debugee state structure
TINITPACKET  Init  = {0};               // Major Init packet
TXINITPACKET XInit = {0};               // X-ice init packet
TWINDOWS     Win   = {{0}};             // Output windowing structure
PTWINDOWS    pWin  = NULL;              // And a pointer to it
PTOUT        pOut  = NULL;              // Pointer to a current Out class

unsigned long **sys_table = NULL;       // alias for sys_call_table

//=============================================================================

extern DWORD kbd;
extern DWORD scan;
extern DWORD *pmodule;
extern DWORD sys;
extern DWORD switchto;
extern DWORD start_sym;
extern DWORD stop_sym;
extern DWORD start_sym_gpl;
extern DWORD stop_sym_gpl;
extern int ice_debug_level;


//=============================================================================
// DEV DEVICE NODE ACCESS
//=============================================================================

static int major_device_number;


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern DWORD Checksum1(DWORD start, DWORD len);
extern int InitProcFs();
extern int CloseProcFs();

typedef asmlinkage int (*PFNMKNOD)(const char *filename, int mode, dev_t dev);
typedef asmlinkage int (*PFNUNLINK)(const char *filename);

static PFNMKNOD sys_mknod = NULL;
static PFNUNLINK sys_unlink = NULL;

extern int ice_mknod(PFNMKNOD sys_mknod, char *pDevice, int major_device_number);
extern void ice_rmnod(PFNUNLINK sys_unlink, char *pDevice);

extern int InitPacket(PTINITPACKET pInit);
extern int XInitPacket(TXINITPACKET *pXInit);
extern int UserAddSymbolTable(void *pSymtab);
extern int UserRemoveSymbolTable(void *pSymtab);

extern void UnHookSwitch(void);
extern BOOL KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode);
extern void KeyboardUnhook();
extern void UnhookPrintk(void);
extern void UnHookDebuger(void);
extern void UnHookSyscall(void);
extern void DisarmBreakpoints(void);
extern int QueryExtModule();

extern int HistoryReadReset();
extern char *HistoryReadNext(void);

extern WORD GetKernelDS();
extern WORD GetKernelCS();

extern void memFreeHeap(BYTE *hHeap);

extern WORD sel_ice_ds;                 // Place to self-modify the code with this value

/******************************************************************************
*                                                                             *
*   int init_module(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Called once when driver loads
*
******************************************************************************/
int IceInitModule(void)
{
    int val;

    INFO("init_module\n");
    INFO("  kbd           %08X\n", kbd);
    INFO("  scan          %08X\n", scan);
    INFO("  pmodule       %08X\n", pmodule);
    INFO("  sys           %08X\n", sys);
    INFO("  switchto      %08X\n", switchto);
    INFO("  start_sym     %08X\n", start_sym);
    INFO("  stop_sym      %08X\n", stop_sym);
    INFO("  start_sym_gpl %08X\n", start_sym_gpl);
    INFO("  stop_sym_gpl  %08X\n", stop_sym_gpl);

    // If we are loaded via simple command line 'insmod' dont do anything
    if( kbd==0 || scan==0 || pmodule==NULL || sys==0 || switchto==0 || !start_sym || !stop_sym )
    {
        ice_printk("ERROR: Use 'linsym -i' to install Linice debugger.\n");
        return( -EFAULT );
    }

    // Make sure all the addresses are valid
    // We did not check these as they are not present in 2.4 kernels
    if( !start_sym_gpl ) start_sym_gpl = start_sym;
    if( !stop_sym_gpl )  stop_sym_gpl  = stop_sym;

    // Clean up structures
    memset(&deb, 0, sizeof(TDEB));

    memset(&Win, 0, sizeof(TWINDOWS));
    pWin = &Win;

    // Since we dont know at the compile time what is the kernel DS and CS will be,
    // we query it now. We also need to self-modify one place in the interrupt
    // handler that will load kernel DS. This is done here only since the i386.asm
    // module needs it. At other places in the Linice we call it as we need it.
    sel_ice_ds = GetKernelDS();

    // Hook the Linux keyboard handler function
    if( KeyboardHook(kbd, scan) )
    {
        // Register device driver
        major_device_number = ice_register_chrdev(DEVICE_NAME);
        if(major_device_number >= 0 )
        {
            // Set up our own pointer to sys_call_table
            // Starting with the kernel version 2.4.20 sys_call_table is not exported any more
            sys_table = (unsigned long **)sys;

            // Create a device node in the /dev directory
            // and also make sure we have the functions in the systable

            sys_mknod = (PFNMKNOD) sys_table[ice_get__NR_mknod()];
            sys_unlink = (PFNUNLINK) sys_table[ice_get__NR_unlink()];

            if(sys_mknod && sys_unlink)
            {
                // This function should fail unless there is a device there (which should NOT be)
                // This saves us from having to delete it manually (Peter K.)
		        ice_rmnod(sys_unlink, "/dev/"DEVICE_NAME);

                val = ice_mknod(sys_mknod, "/dev/"DEVICE_NAME, major_device_number);

                // Dev node created successfully
                if(val >= 0)
                {
                    // Register /proc/linice virtual file
                    if( InitProcFs()==0 )
                    {
                        // Calculate the checksum of the Linice code
                        deb.LiniceChecksum = Checksum1((DWORD)ObjectStart, (DWORD)ObjectEnd - (DWORD)ObjectStart);

                        INFO(("Linice successfully loaded.\n"));

                        return 0;
                    }
                    else
                    {
                        ice_printk("ERROR: Failed to create /proc entry.");
                    }

                    ice_rmnod(sys_unlink, "/dev/"DEVICE_NAME);
                }
                else
                {
                    ice_printk("ERROR: Failed to mknod a /dev/"DEVICE_NAME" node.");
                }
            }
            else
            {
                ice_printk("ERROR: Incorrect sys_mknod and sys_unlink.\n");
                ice_printk("       Possibly a kernel symbol map mismatch. Is the System.map correct?\n");
            }

            ice_unregister_chrdev(major_device_number, DEVICE_NAME);
        }
        else
        {
            ice_printk("ERROR: Failed to register character device\n");
        }

        KeyboardUnhook();
    }
    else
    {
        ice_printk("ERROR: Incorrect address of handle_scancode for keyboard hook.\n");
        ice_printk("       Possibly a kernel symbol map mismatch. Is the System.map correct?\n");
    }

    return -EFAULT;
}


/******************************************************************************
*                                                                             *
*   void cleanup_module(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Called once when the driver unloads.
*
*   It is not allowed to uninstall Linice by using the "rmmod" command. There
*   is a code in syscall.c that prevent us from doing it from within the
*   delete module hook.
*
*   ICE_IOCTL_EXIT is needed first to unhook system calls, after which we run
*   cleanup_module (this) to finally remove all traces of us.
*
******************************************************************************/
void IceCleanupModule(void)
{
    INFO("cleanup_module\n");

    // Unhook the debug out printk() hook
    UnhookPrintk();

    // Clear all breakpoints; this will reinstate the original BYTE code at the places of embedded INT3,
    // it will also disable HW breakpoints (DR0...DR3)
    DisarmBreakpoints();

    // Unhook the keyboard hook
    KeyboardUnhook();

    // Unhook the task switch hook
    UnHookSwitch();

    // Unregister /proc virtual file
    CloseProcFs();

    // Delete a devce node in the /dev/ directory
    if(sys_unlink != 0)
    {
        ice_rmnod(sys_unlink, "/dev/"DEVICE_NAME);
    }

    // Restore original Linux IDT table
    UnHookDebuger();

    // Unregister driver
    ice_unregister_chrdev(major_device_number, DEVICE_NAME);

    // Free memory structures
    if( deb.hHistoryBufferHeap != NULL )
        memFreeHeap(deb.hHistoryBufferHeap);

    if( deb.hSymbolBufferHeap != NULL )
        memFreeHeap(deb.hSymbolBufferHeap);

    if( deb.pXDrawBuffer != NULL )
        ice_vfree(deb.pXDrawBuffer);

    if( deb.pXFrameBuffer != NULL )
        ice_iounmap(deb.pXFrameBuffer);

    if( deb.hHeap != NULL )
        memFreeHeap(deb.hHeap);

    return;
}


/******************************************************************************
*                                                                             *
*   IO CONTROL CALLS                                                          *
*                                                                             *
*******************************************************************************
*
*   Function handling various IO Controls callbacks.
*
*   These are the "real" function prototypes:
*
*   int DriverOpen(struct inode *inode, struct file *file)
*   int DriverClose(struct inode *inode, struct file *file)
*   int DriverIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param)
*
******************************************************************************/
int DriverOpen(void)
{
    INFO("IceOpen\n");

    ice_mod_inc_use_count();

    return(0);
}


int DriverClose(void)
{
    INFO("IceClose\n");

    ice_mod_dec_use_count();

    // We should be really returning DEV_CLOSE_RET_VAL value
    return(0);
}

int DriverIOCTL(void *p1, void *p2, unsigned int ioctl, unsigned long param)
{
    int retval = -EINVAL;                   // Return error code
    char *pBuf;                             // Temporary line buffer pointer

    INFO("IceIOCTL %X param %X\n", ioctl, (int)param);

    switch(ioctl)
    {
        //==========================================================================================
        case ICE_IOCTL_INIT:            // Original initialization packet
            INFO("ICE_IOCTL_INIT\n");

            // Copy the init block to the driver
            if( ice_copy_from_user(&Init, (void *)param, sizeof(TINITPACKET))==0 )
            {
                retval = InitPacket(&Init);
            }
            else
                retval = -EFAULT;       // Faulty memory access
        break;

        //==========================================================================================
        case ICE_IOCTL_XDGA:            // Start using X linear framebuffer as the output device
            INFO("ICE_IOCTL_XDGA\n");

            // Copy the X-init block to the driver
            if( ice_copy_from_user(&XInit, (void *)param, sizeof(TXINITPACKET))==0 )
            {
                retval = XInitPacket(&XInit);
            }
            else
                retval = -EFAULT;       // Faulty memory access
            break;

        //==========================================================================================
        case ICE_IOCTL_EXIT:            // Unload the module

            // If we have any extension modules still registered, we cannot unload
            // since they linked into our symbols.
            if( QueryExtModule()==0 )
            {
                // Linice is not operational any more
                deb.fOperational = FALSE;

                // Unhook the system call table hooks early here so we dont collide with the
                // module unload hook function when unloading itself
                UnHookSyscall();
            }
            else
                retval = 1;             // Special return value of 1

            break;

        //==========================================================================================
        case ICE_IOCTL_EXIT_FORCE:      // Decrement usage count to 1 so we can unload the module

            // Linice is not operational any more
            deb.fOperational = FALSE;

            // Unhook the system call table hooks early here so we dont collide with the
            // module unload hook function when unloading itself
            UnHookSyscall();

            // This loop comes really handy when linice does not want to be unloaded
            // This call is mainly useful during the development
            while( ice_mod_in_use() )
            {
                ice_mod_dec_use_count();
            }
            ice_mod_inc_use_count();    // Back to 1

            break;

        //==========================================================================================
        case ICE_IOCTL_ADD_SYM:         // Add a symbol table
            INFO("ICE_IOCTL_ADD_SYM\n");

            retval = UserAddSymbolTable((void *)param);
            break;

        //==========================================================================================
        case ICE_IOCTL_REMOVE_SYM:      // Remove a symbol table
            INFO("ICE_IOCTL_REMOVE_SYM\n");

            retval = UserRemoveSymbolTable((void *)param);
            break;

        //==========================================================================================
        case ICE_IOCTL_HISBUF_RESET:    // Fetch a seria of history lines - reset the internal reader
            INFO("ICE_IOCTL_HISBUF_RESET\n");

            retval = HistoryReadReset();    // It should normally return 0
            break;

        //==========================================================================================
        case ICE_IOCTL_HISBUF:          // Fetch a number of history lines, called multiple times
            INFO("ICE_IOCTL_HISBUF\n");

            pBuf = HistoryReadNext();
            if( pBuf && ice_copy_to_user((void *)param, pBuf, MIN(MAX_STRING, strlen(pBuf)+1))==0 )
            {
                retval = 0;
            }
            else
                retval = -EFAULT;       // Faulty memory access OR end of history stream

            break;
    }

    return( retval );
}
