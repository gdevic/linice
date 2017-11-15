/******************************************************************************
*                                                                             *
*   Module:     driver.c                                                      *
*                                                                             *
*   Date:       03/01/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 03/01/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include <linux/kernel.h>               // Include this only in this file
#include <linux/module.h>               // Include required module include
#include <linux/types.h>                // Include kernel data types
#include <linux/times.h>
#include <asm/uaccess.h>                // User space memory access functions
#include <linux/fs.h>                   // Include file operations file
#include <asm/unistd.h>                 // Include system call numbers
#include <linux/proc_fs.h>              // Include proc filesystem support

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
//  ice_debug_level=[0 - 1]             Set the level for the output messages:
//                                      0 - Do not display INFO level
//                                      1 - Display INFO level messages

MODULE_PARM(linice, "s");               // linice=<string>
char *linice = "";                      // default value

MODULE_PARM(ice_debug_level, "i");      // ice_debug_level=<integer>
int ice_debug_level = 1;                // default value

//=============================================================================
// DEV DEVICE NODE ACCESS
//=============================================================================

static int major_device_number;

static int DriverOpen(struct inode *inode, struct file *file);
static DEV_CLOSE_RET DriverClose(struct inode *inode, struct file *file);
static int DriverIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param);


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

//=============================================================================
// PROC VIRTUAL FILE
//=============================================================================

extern int ProcLinice(char *buf, char **start, off_t offset, int len, int unused);

struct proc_dir_entry linice_proc_entry =
{
    0,                                  // low_ino: the inode--dynamic
    6, "linice",                        // Name len and name string
    S_IFREG | S_IRUGO,                  // mode
    1, 0, 0,                            // nlinks, owner, group
    0,                                  // size - unused
    NULL,                               // operations - unused
    &ProcLinice,                        // Read function
};


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern unsigned long sys_call_table[];
typedef asmlinkage int (*PFNMKNOD)(const char *filename, int mode, dev_t dev);
typedef asmlinkage int (*PFNUNLINK)(const char *filename);

static PFNMKNOD sys_mknod;
static PFNUNLINK sys_unlink;

extern int InitPacket(PTINITPACKET pInit);

extern void ice_free(BYTE *p);
extern void ice_free_heap(BYTE *pHeap);

extern void UnHookDebuger(void);

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

    memset(&Win, 0, sizeof(TWINDOWS));
    pWin = &Win;

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
                // Register /proc/linice virtual file
                proc_register(&proc_root, &linice_proc_entry);


                INFO(("LinIce successfully loaded.\n"));

                return 0;
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

    // Unregister /proc virtual file
    proc_unregister(&proc_root, linice_proc_entry.low_ino);

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
        case ICE_IOCTL_INIT:
            INFO(("ICE_IOCTL_INIT\n"));

            // Copy the init block to the driver
            if( copy_from_user(&Init, (void *)param, sizeof(TINITPACKET))==0 )
            {
                retval = InitPacket(&Init);
            }
            else
                retval = -EFAULT;       // Faulty memory access
        break;

        case ICE_IOCTL_ADD_SYM:         // Add a symbol table
            break;

        case ICE_IOCTL_REMOVE_SYM:      // Remove a symbol table
            break;
    }

    return( retval );
}
