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

#include "module-header.h"              // Versatile module header file

#include <linux/module.h>               // Include required module include
#include <linux/init.h>
#include <linux/kernel.h>               // Include this only in this file
#include <linux/types.h>                // Include kernel data types
#include <linux/times.h>
#include <asm/uaccess.h>                // User space memory access functions
#include <linux/devfs_fs_kernel.h>      // Include file operations file
#include <asm/unistd.h>                 // Include system call numbers

#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TINITPACKET Init;                       // Init packet
TXINITPACKET XInit;                     // X-ice init packet

TICE Ice;                               // The main debugger structure
PTICE pIce;                             // And a pointer to it

TDEB deb;                               // Live debugee state structure

TWINDOWS Win;                           // Output windowing structure
PTWINDOWS pWin;                         // And a pointer to it

PTOUT pOut;                             // Pointer to a current Out class

//=============================================================================
MODULE_AUTHOR("Goran Devic");
MODULE_DESCRIPTION("Linux kernel debugger");
//
// Define parameters for the module:
//  linice=<string>                     What???
//  kbd=<address>                       Address of the handle_kbd_event function
//  scan=<address>                      Address of the handle_scancode function
//  mod=<address>                       Address of the module_list variable
//  ice_debug_level=[0 - 1]             Set the level for the output messages:
//                                      0 - Do not display INFO level
//                                      1 - Display INFO level messages

MODULE_PARM(linice, "s");               // linice=<string>
char *linice = "";                      // default value

MODULE_PARM(kbd, "i");                  // kbd=<integer>
DWORD kbd = 0;                          // default value

MODULE_PARM(scan, "i");                 // scan=<integer>
DWORD scan = 0;                         // default value

MODULE_PARM(pmodule, "i");              // mod=<integer>
DWORD *pmodule = NULL;                  // default value

MODULE_PARM(ice_debug_level, "i");      // ice_debug_level=<integer>
int ice_debug_level = 1;                // default value

//=============================================================================
// DEV DEVICE NODE ACCESS
//=============================================================================

static int major_device_number;

static int DriverOpen(struct inode *inode, struct file *file);
static DEV_CLOSE_RET DriverClose(struct inode *inode, struct file *file);
static int DriverIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param);

// File operations structure; in the 2.4 kernel we define it new way, otherwise
// define it old way:
//----------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
//----------------------------------------------
struct file_operations ice_fops = {
    owner:      THIS_MODULE,
    ioctl:      DriverIOCTL,
    open:       DriverOpen,
    release:    DriverClose,
};
//----------------------------------------------
#else   // 2.2, 2.1 kernel version
//----------------------------------------------
struct file_operations ice_fops = {
    NULL,               /* seek    */
    NULL,               /* read    */
    NULL,               /* write   */
    NULL,               /* readdir */
    NULL,               /* select  */
    DriverIOCTL,        /* ioctl   */
    NULL,               /* mmap    */
    DriverOpen,         /* open    */
    FLUSH_FOPS(NULL)    /* flush   */
    DriverClose,        /* close   */
};
#endif
//----------------------------------------------

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern int InitProcFs();
extern int CloseProcFs();

extern unsigned long sys_call_table[];
typedef asmlinkage int (*PFNMKNOD)(const char *filename, int mode, dev_t dev);
typedef asmlinkage int (*PFNUNLINK)(const char *filename);

static PFNMKNOD sys_mknod;
static PFNUNLINK sys_unlink;

extern int InitPacket(PTINITPACKET pInit);
extern int XInitPacket(TXINITPACKET *pXInit);
extern int UserAddSymbolTable(void *pSymtab);
extern int UserRemoveSymbolTable(void *pSymtab);

extern void ice_free(BYTE *p);
extern void ice_free_heap(BYTE *pHeap);

extern void KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode);
extern void KeyboardUnhook();
extern void UnHookDebuger(void);
extern void UnHookSyscall(void);

/******************************************************************************
*                                                                             *
*   int init_module(void)                                                     *
*                                                                             *
*******************************************************************************
*
*
*
******************************************************************************/
int init_module(void)
{
    mm_segment_t oldFS;
    int val;

    INFO(("init_module\n"));

    // Clean up structures
    memset(&Ice, 0, sizeof(TICE));
    pIce = &Ice;

    memset(&deb, 0, sizeof(TDEB));

    memset(&Win, 0, sizeof(TWINDOWS));
    pWin = &Win;

//    memset(&Init, 0, sizeof(TINITPACKET));
//    memset(&XInit, 0, sizeof(TXINITPACKET));

    // Register driver

    major_device_number = register_chrdev(0, DEVICE_NAME, &ice_fops);

    if(major_device_number >= 0 )
    {
        // Create a device node in the /dev directory
        // and also make sure we have the functions in the systable

        sys_mknod = (PFNMKNOD) sys_call_table[__NR_mknod];
        sys_unlink = (PFNUNLINK) sys_call_table[__NR_unlink];

        if(sys_mknod && sys_unlink)
        {
            // Dont perform argument validity checking..
            oldFS = get_fs(); set_fs(KERNEL_DS);

            val = sys_mknod("/dev/"DEVICE_NAME, S_IFCHR | S_IRWXUGO, major_device_number<<8);

            set_fs(oldFS);

            // Module loaded ok
            if(val >= 0)
            {
                // Hook the Linux keyboard handler function
                KeyboardHook(kbd, scan);

                // Register /proc/linice virtual file
                if( InitProcFs()==0 )
                {
                    INFO(("LinIce successfully loaded.\n"));

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

    unregister_chrdev(major_device_number, DEVICE_NAME);

    return -EFAULT;
}


/******************************************************************************
*                                                                             *
*   void cleanup_module(void)                                                 *
*                                                                             *
*******************************************************************************
*
*
*
******************************************************************************/
void cleanup_module(void)
{
    mm_segment_t oldFS;

    INFO(("cleanup_module\n"));

    // Unhook the keyboard hook
    KeyboardUnhook();

    // Unregister /proc virtual file
    CloseProcFs();

    // Delete a devce node in the /dev/ directory
    if(sys_unlink != 0)
    {
        // Dont perform argument validity checking..
        oldFS = get_fs(); set_fs(KERNEL_DS);

        sys_unlink("/dev/"DEVICE_NAME);

        set_fs(oldFS);
    }

    // Restore original Linux IDT table
    UnHookDebuger();

    // Unregister driver
    unregister_chrdev(major_device_number, "ice");

    // Free memory structures
    if( pIce->pHistoryBuffer != NULL )
        ice_free(pIce->pHistoryBuffer);

    if( pIce->hSymbolBuffer != NULL )
        ice_free_heap(pIce->hSymbolBuffer);

    if( pIce->pXDrawBuffer != NULL )
        ice_free_heap(pIce->pXDrawBuffer);

    if( pIce->pXFrameBuffer != NULL )
        iounmap(pIce->pXFrameBuffer);

    if( pIce->hHeap != NULL )
        ice_free_heap(pIce->hHeap);

    return;
}


static int DriverOpen(struct inode *inode, struct file *file)
{
    INFO(("IceOpen\n"));

    MOD_INC_USE_COUNT;

    return(0);
}


static DEV_CLOSE_RET DriverClose(struct inode *inode, struct file *file)
{
    INFO(("IceClose\n"));

    MOD_DEC_USE_COUNT;

    return DEV_CLOSE_RET_VAL;
}

static int DriverIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param)
{
    int retval = -EINVAL;

    INFO(("IceIOCTL %X param %X\n", ioctl, (int)param));

    switch(ioctl)
    {
        case ICE_IOCTL_INIT:            // Original initialization packet
            INFO(("ICE_IOCTL_INIT\n"));

            // Copy the init block to the driver
            if( copy_from_user(&Init, (void *)param, sizeof(TINITPACKET))==0 )
            {
                retval = InitPacket(&Init);
#if 0
                // If we also supplied a valid XInit packet at the start,
                // that means XWindows was running at the time of init.
                if( Init.XInit.pFrameBuf != 0 )
                {
                    // Copy structure into the XInit where it belongs
                    memcpy(&XInit, (void *)&Init.XInit, sizeof(TXINITPACKET));

                    retval = XInitPacket(&XInit);

                    if( retval==0 )
                    {
                        // If everything went well, we can probably switch to using DGA
//                        pOut = &outDga;
                    }
                }
#endif
            }
            else
                retval = -EFAULT;       // Faulty memory access
        break;

        case ICE_IOCTL_XDGA:            // Start using X linear framebuffer as the output device
            INFO(("ICE_IOCTL_XDGA\n"));

            // Copy the X-init block to the driver
            if( copy_from_user(&XInit, (void *)param, sizeof(TXINITPACKET))==0 )
            {
                dprinth(1, "ICE_IOCTL_XDGA");
                retval = XInitPacket(&XInit);
            }
            else
                retval = -EFAULT;       // Faulty memory access
            break;

        case ICE_IOCTL_EXIT:            // Decrement usage count to 1 so we can unload the module

            // Unhook the system call table hooks early here so we dont collide with the
            // module unload hook function when unloading itself
            UnHookSyscall();

#if 0
            // This loop comes really handy when linice does not want to be unloaded
            while( MOD_IN_USE )
            {
                MOD_DEC_USE_COUNT;
            }
            MOD_INC_USE_COUNT;          // Back to 1
#endif
            break;

        case ICE_IOCTL_ADD_SYM:         // Add a symbol table
            INFO(("ICE_IOCTL_ADD_SYM\n"));

            retval = UserAddSymbolTable((void *)param);
            break;

        case ICE_IOCTL_REMOVE_SYM:      // Remove a symbol table
            INFO(("ICE_IOCTL_REMOVE_SYM\n"));

            retval = UserRemoveSymbolTable((void *)param);
            break;
    }

    return( retval );
}
