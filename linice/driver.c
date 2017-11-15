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

#include <linux/autoconf.h>

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#ifdef CONFIG_SMP
#define __SMP__ 1
#endif

#include <linux/kernel.h>               // Include this only in this file

#include <linux/module.h>               // Include required module include
#include <asm/uaccess.h>                // User space memory access functions
#include <linux/fs.h>                   // Include file operations file
#include <asm/unistd.h>                 // Include system call numbers

#include "ioctl.h"                      // Include our own IOCTL numbers
#include "versions.h"                   // Deal with different kernel versions
#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

MODULE_AUTHOR("Goran Devic");
MODULE_DESCRIPTION("Linux kernel debugger");

//=============================================================================
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

static int major_device_number;

static int IceOpen(struct inode *inode, struct file *file);
static DEV_CLOSE_RET IceClose(struct inode *inode, struct file *file);
static int IceIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param);


struct file_operations ice_fops = {
    NULL,               /* seek    */
    NULL,               /* read    */
    NULL,               /* write   */
    NULL,               /* readdir */
    NULL,               /* select  */
    IceIOCTL,           /* ioctl   */
    NULL,               /* mmap    */
    IceOpen,            /* open    */
    FLUSH_FOPS(NULL)    /* flush   */
    IceClose,           /* close   */
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

            if(val >= 0)
            {
                // Module loaded ok

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

    // Delete a devce node in the /dev/ directory

    if(sys_unlink != 0)
    {
        // Dont perform argument validity checking..
        oldFS = get_fs(); set_fs(KERNEL_DS);

        sys_unlink("/dev/"DEVICE_NAME);

        set_fs(oldFS);
    }

    // Unregister driver

    unregister_chrdev(major_device_number, "ice");
    
    return;
}


static int IceOpen(struct inode *inode, struct file *file)
{
    INFO(("IceOpen\n"));

    MOD_INC_USE_COUNT;

    return(0);
}


static DEV_CLOSE_RET IceClose(struct inode *inode, struct file *file)
{
    INFO(("IceClose\n"));

    MOD_DEC_USE_COUNT;

    return DEV_CLOSE_RET_VAL;
}

static int IceIOCTL(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param)
{
    INFO(("IceIOCTL %d:%d\n", ioctl, (int)param));

    // Make sure it is for us
    if(_IOC_TYPE(ioctl) == ICE_IOCTL_MAGIC)
    {
        switch(ioctl)
        {
            case ICE_IOCTL_LOAD:
                break;

            case ICE_IOCTL_UNLOAD:
                break;

            case ICE_IOCTL_BREAK:
                break;

            case ICE_IOCTL_LOAD_SYM:
                break;

            case ICE_IOCTL_UNLOAD_SYM:
                break;
        }

        return 0;
    }

    return -EINVAL;
}



