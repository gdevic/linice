/******************************************************************************
*                                                                             *
*   Module:     driver.c                                                      *
*                                                                             *
*   Date:       09/01/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

TINITPACKET Init;                       // Major Init packet
TXINITPACKET XInit;                     // X-ice init packet

TDEB deb;                               // Live debugee state structure

TWINDOWS Win;                           // Output windowing structure
PTWINDOWS pWin;                         // And a pointer to it

PTOUT pOut;                             // Pointer to a current Out class

unsigned long **sys_table;             // alias for sys_call_table

//=============================================================================

extern char *linice;                           // default value
extern DWORD kbd;                              // default value
extern DWORD scan;                             // default value
extern DWORD *pmodule;                         // default value
extern DWORD sys;                              // default value
extern DWORD iceface;                          // default value
extern int ice_debug_level;                    // default value


//=============================================================================
// DEV DEVICE NODE ACCESS
//=============================================================================

static int major_device_number;


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

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

extern void KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode);
extern void KeyboardUnhook();
extern void UnHookDebuger(void);
extern void UnHookSyscall(void);
extern void DisarmBreakpoints(void);

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

    INFO(("init_module\n"));

    // Clean up structures
    memset(&deb, 0, sizeof(TDEB));

    memset(&Win, 0, sizeof(TWINDOWS));
    pWin = &Win;

    // Since we dont know at the compile time what is the kernel DS and CS will be,
    // we query it now. We also need to self-modify one place in the interrupt
    // handler that will load kernel DS
    sel_ice_ds = GetKernelDS();

    // Register driver

    major_device_number = ice_register_chrdev(DEVICE_NAME);

    if(major_device_number >= 0 )
    {
        INFO(("Params:\n"));
        INFO(("  kbd     %08X\n", kbd));
        INFO(("  scan    %08X\n", scan));
        INFO(("  pmodule %08X\n", pmodule));
        INFO(("  sys     %08X\n", sys));

        // Set up our own pointer to sys_call_table
        // Starting with the kernel version 2.4.20 sys_call_table is not exported any more
        sys_table = (unsigned long **)sys;

        // Create a device node in the /dev directory
        // and also make sure we have the functions in the systable

        sys_mknod = (PFNMKNOD) sys_table[ice_get__NR_mknod()];
        sys_unlink = (PFNUNLINK) sys_table[ice_get__NR_unlink()];

        INFO(("sys_mknod = %08X\n", sys_mknod));
        INFO(("sys_unlink = %08X\n", sys_unlink));

        if(sys_mknod && sys_unlink)
        {
            val = ice_mknod(sys_mknod, "/dev/"DEVICE_NAME, major_device_number);

            // Module loaded ok
            if(val >= 0)
            {
                // Hook the Linux keyboard handler function
                KeyboardHook(kbd, scan);

                // Register /proc/linice virtual file
                if( InitProcFs()==0 )
                {
                    INFO(("Linice successfully loaded.\n"));

                    return 0;
                }
                else
                {
                    ERROR(("Unable to create /proc entry!\n"));
                }
            }
            else
            {
                ERROR(("sys_mknod failed (%d)\n", val));
            }
        }
        else
        {
            ERROR(("Can't get sys_mknod=%X sys_unlink=%X\n", (int)sys_mknod, (int)sys_unlink));
        }
    }
    else
    {
        ERROR(("Failed to register character device (%d)\n", major_device_number));
    }

    ice_unregister_chrdev(major_device_number, DEVICE_NAME);

    return -EFAULT;
}


/******************************************************************************
*                                                                             *
*   void cleanup_module(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Called once when driver unloads
*
******************************************************************************/
void IceCleanupModule(void)
{
    INFO(("cleanup_module\n"));

    // Unhook the keyboard hook
    KeyboardUnhook();

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
    ice_unregister_chrdev(major_device_number, "ice");

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
    INFO(("IceOpen\n"));

    ice_mod_inc_use_count();

    return(0);
}


int DriverClose(void)
{
    INFO(("IceClose\n"));

    ice_mod_dec_use_count();

    // We should be really returning DEV_CLOSE_RET_VAL value
    return(0);
}

int DriverIOCTL(void *p1, void *p2, unsigned int ioctl, unsigned long param)
{
    int retval = -EINVAL;                   // Return error code
    char *pBuf;                             // Temporary line buffer pointer

    INFO(("IceIOCTL %X param %X\n", ioctl, (int)param));

    switch(ioctl)
    {
        //==========================================================================================
        case ICE_IOCTL_INIT:            // Original initialization packet
            INFO(("ICE_IOCTL_INIT\n"));

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
            INFO(("ICE_IOCTL_XDGA\n"));

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

            // Clear all breakpoints; this will reinstate the original BYTE code at the places of embedded INT3,
            // it will also disable HW breakpoints (DR0...DR3)
            DisarmBreakpoints();

            // Unhook the system call table hooks early here so we dont collide with the
            // module unload hook function when unloading itself
            UnHookSyscall();

            break;

        //==========================================================================================
        case ICE_IOCTL_EXIT_FORCE:      // Decrement usage count to 1 so we can unload the module

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
            INFO(("ICE_IOCTL_ADD_SYM\n"));

            retval = UserAddSymbolTable((void *)param);
            break;

        //==========================================================================================
        case ICE_IOCTL_REMOVE_SYM:      // Remove a symbol table
            INFO(("ICE_IOCTL_REMOVE_SYM\n"));

            retval = UserRemoveSymbolTable((void *)param);
            break;

        //==========================================================================================
        case ICE_IOCTL_HISBUF_RESET:    // Fetch a seria of history lines - reset the internal reader
            INFO(("ICE_IOCTL_HISBUF_RESET\n"));

            retval = HistoryReadReset();    // It should normally return 0
            break;

        //==========================================================================================
        case ICE_IOCTL_HISBUF:          // Fetch a number of history lines, called multiple times
            INFO(("ICE_IOCTL_HISBUF\n"));

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
