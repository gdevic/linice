/******************************************************************************
*                                                                             *
*   Module:     iceface.c                                                     *
*                                                                             *
*   Copyright (c) 1997-2004 Goran Devic                                       *
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

    This is the kernel-specific interface module for Linice.

*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#ifdef IO_APIC
#define CONFIG_X86_IO_APIC
#define CONFIG_X86_LOCAL_APIC
//#define CONFIG_X86_GOOD_APIC
#endif

#ifdef SMP
#define CONFIG_SMP
#define __SMP__
#endif

#if 0

#define EXPORT_SYMTAB
#include <linux/config.h>
#if defined(CONFIG_MODVERSIONS) &&!defined(MODVERSIONS)
#define MODVERSIONS
#endif
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#endif

#include <linux/module.h>
#include <linux/kernel.h>


MODULE_LICENSE("GPL");

#include <linux/version.h>

/*
 * Distinguish relevant classes of Linux kernels.
 *
 * The convention is that version X defines all
 * the KERNEL_Y symbols where Y <= X.
 */

#ifdef KERNEL_VERSION
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 1, 0)
#    define KERNEL_2_1
#  endif
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 2, 0)
#    define KERNEL_2_2
#  endif
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 1)
#    define KERNEL_2_3_1
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 15)
/*     new networking */
#      define KERNEL_2_3_15
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 23)
/*     4GB & 64GB support */
#      define KERNEL_2_3_23
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 25)
/*     new procfs */
#      define KERNEL_2_3_25
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 29)
/*     even newer procfs */
#      define KERNEL_2_3_29
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 43)
/*     softnet changes */
#      define KERNEL_2_3_43
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 47)
/*     more softnet changes */
#      define KERNEL_2_3_47
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 99)
/*     name in netdevice struct is array and not pointer */
#      define KERNEL_2_3_99
#    endif
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 0)
/*     New 'owner' member at the beginning of struct file_operations */
#      define KERNEL_2_4_0
#    endif
#  endif
#endif


#ifndef KERNEL_2_4_0
#   define get_page(p) atomic_inc(&(p)->count)
#   ifdef KERNEL_2_1
#      define put_page(p) __free_page(p)
#   else
/*     2.0 kernels don't export __free_page() */
#      define put_page(p) free_page(((p) - mem_map) << PAGE_SHIFT)
#   endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

/* flush call is added in the kernel version 2.2.0 */
#define FLUSH_FOPS(function)    function,

#else // LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

#define FLUSH_OPS(function)

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

#include <linux/config.h>
#include <linux/init.h>


#include <linux/module.h>               // Include required module include
#include <linux/init.h>
#include <linux/kernel.h>               // Include this only in this file
#include <linux/types.h>                // Include kernel data types
#include <linux/times.h>
#include <asm/uaccess.h>                // User space memory access functions
#include <linux/devfs_fs_kernel.h>      // Include file operations file
#include <asm/unistd.h>                 // Include system call numbers

// Needed for proc virtual file
#include <linux/proc_fs.h>              // Include proc filesystem support

// Needed for PAGE_OFFSET
#include <asm/page.h>                   // We need page offset

// Needed for ioremap etc.
#include <asm/io.h>


#include <linux/mm.h>
#include <asm/pgtable.h>


//#include <linux/malloc.h>
#include <linux/vmalloc.h>              // Kernel memory allocation

#include <linux/pci.h>                  // Include PCI bus defines


#ifdef SMP
#include <asm/smp.h>
#include <asm/mtrr.h>
#include <asm/mpspec.h>
#include <asm/pgalloc.h>

//#include <linux/smp.h>
//#include <linux/smp_lock.h>
//#include <asm/io_apic.h>
#endif

#include "iceface.h"

typedef unsigned int DWORD;

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

//=============================================================================
MODULE_AUTHOR("Goran Devic");
MODULE_DESCRIPTION("Linux kernel debugger");
//
// Define parameters for the module:
//  linice=<string>                     What???
//  kbd=<address>                       Address of the handle_kbd_event function
//  scan=<address>                      Address of the handle_scancode function
//  mod=<address>                       Address of the module_list variable
//  sys=<address>                       Address of the sys_call_table symbol
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

MODULE_PARM(sys, "i");                  // sys=<integer>
DWORD sys = 0;                          // default value

MODULE_PARM(ice_debug_level, "i");      // ice_debug_level=<integer>
int ice_debug_level = 1;                // default value

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

//struct mpc_config_ioapic mp_ioapics[MAX_IO_APICS];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

unsigned int ice_get_flags()
{
    unsigned int flags = 0;
#ifdef IO_APIC
    flags |= 1;
#endif // IO_APIC

#ifdef SMP
    flags |= 2;
#endif // SMP
    return( flags );
}

void ice_ack_APIC_irq(void)
{
#ifdef IO_APIC
    ack_APIC_irq();
#endif // IO_APIC
}

int ice_get_io_bitmap_size(void)
{
    return( IO_BITMAP_SIZE );
}

void ice_smp_call_function(void (*func)(void *), void *info, int retry, int wait)
{
#ifdef SMP
    smp_call_function(func, info, retry, wait);
#else
#endif // SMP
}

int ice_smp_processor_id()
{
#ifdef SMP
    return( smp_processor_id() );
#else
    return( 0 );
#endif // SMP
}


DWORD ice_io_apic_read(int n, DWORD reg)
{
#ifdef IO_APIC
    return( io_apic_read(n, reg) );
#else
    return( 0 );
#endif // IO_APIC
}

void ice_io_apic_write(int n, DWORD reg, DWORD val)
{
#ifdef IO_APIC
    io_apic_write(n, reg, val);
#else
#endif // IO_APIC
}

/******************************************************************************
*                                                                             *
*   /proc/linice file support                                                 *
*                                                                             *
******************************************************************************/

static struct proc_dir_entry *pProcEntry;

int ice_init_proc(int ProcRead, int ProcWrite)
{
    pProcEntry = create_proc_entry("linice", 0644, &proc_root);
    if( pProcEntry )
    {
        // We do it using casting pointers to get rid of a type mismatch warning..
        *(int*) &pProcEntry->read_proc  = (int) ProcRead;
        *(int*) &pProcEntry->write_proc = (int) ProcWrite;
    }
    else
        return( -1 );                   // Return failure

    return( 0 );                        // Return success
}

int ice_close_proc()
{
    remove_proc_entry("linice", &proc_root);

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   /dev/ice IOCTL interface                                                  *
*                                                                             *
******************************************************************************/

typedef int (*PFNOPEN)(struct inode *inode, struct file *file);
typedef int (*PFNCLOSE)(struct inode *inode, struct file *file);
typedef int (*PFNIOCTL)(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param);

extern int DriverOpen(void);
extern int DriverClose(void);
extern int DriverIOCTL(void *p1, void *p2, unsigned int ioctl, unsigned long param);


// File operations structure; in the 2.4 kernel we define it new way, otherwise
// define it old way:
//----------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
//----------------------------------------------
static struct file_operations ice_fops = {
    owner:      THIS_MODULE,
    ioctl:      (PFNIOCTL)DriverIOCTL,
    open:       (PFNOPEN)DriverOpen,
    release:    (PFNCLOSE)DriverClose,
};
//----------------------------------------------
#else   // 2.2, 2.1 kernel version
//----------------------------------------------
struct file_operations ice_fops = {
    NULL,                       /* seek    */
    NULL,                       /* read    */
    NULL,                       /* write   */
    NULL,                       /* readdir */
    NULL,                       /* select  */
    (PFNIOCTL)DriverIOCTL,      /* ioctl   */
    NULL,                       /* mmap    */
    (PFNOPEN)DriverOpen,        /* open    */
    FLUSH_FOPS(NULL)            /* flush   */
    (PFNCLOSE)DriverClose,      /* close   */
};
#endif
//----------------------------------------------

int ice_register_chrdev(char *pDevice)
{
    return( register_chrdev(0, pDevice, &ice_fops) );
}

void ice_unregister_chrdev(int major_device_number, char *pDevice)
{
    unregister_chrdev(major_device_number, pDevice);
}

typedef asmlinkage int (*PFNMKNOD)(const char *filename, int mode, dev_t dev);
typedef asmlinkage int (*PFNUNLINK)(const char *filename);

int ice_mknod(PFNMKNOD sys_mknod, char *pDevice, int major_device_number)
{
    mm_segment_t oldFS;
    int val;

    oldFS = get_fs(); set_fs(KERNEL_DS);
    val = sys_mknod(pDevice, S_IFCHR | S_IRWXUGO, major_device_number<<8);
    set_fs(oldFS);

    return( val );
}

void ice_rmnod(PFNUNLINK sys_unlink, char *pDevice)
{
    mm_segment_t oldFS;

    oldFS = get_fs(); set_fs(KERNEL_DS);
    sys_unlink(pDevice);
    set_fs(oldFS);
}

int ice_get__NR_mknod(void)
{
    return( __NR_mknod );
}

int ice_get__NR_unlink(void)
{
    return( __NR_unlink );
}

void *ice_ioremap(unsigned int mem1, unsigned int mem2)
{
    return( ioremap(mem1, mem2) );
}

void ice_iounmap(void *pMemory)
{
    iounmap(pMemory);
}

unsigned int ice_page_offset(void)
{
    return( PAGE_OFFSET );
}

long ice_copy_to_user(void *p1, void *p2, int len)
{
    return( copy_to_user(p1, p2, len) );
}

long ice_copy_from_user(void *p1, void *p2, int len)
{
    return( copy_from_user(p1, p2, len) );
}

#if 0
typedef struct
{
    unsigned char bus;                  // Bus number
    unsigned int devfn;                 // Device and function
    unsigned short vendor;              // Vendor ID
    unsigned short device;              // Device ID
    unsigned int irq;                   // Interrupt number
    unsigned int master;                // Bus master
    unsigned long base_address[6];      // List of base addresses
    unsigned long rom_address;          // ROM address
    char *pDevice;                      // Device name

} TPCI;
#endif

//void ice_get_pci_info(TPCI *pci, struct pci_dev *p)
void ice_get_pci_info(TPCI *pci, void *ptr)
{
    struct pci_dev *p = ptr;

    int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    pci->bus = p->bus->number;
    pci->devfn = p->devfn;
    pci->vendor = p->vendor;
    pci->device = p->device;
    pci->irq = p->irq;
    pci->master = 0;
    pci->rom_address = 0;
    for(i=0; i<6; i++)
        pci->base_address[i] = p->resource[i].start;
    pci->pDevice = p->name;
#else
    pci->bus = p->bus->number;
    pci->devfn = p->devfn;
    pci->vendor = p->vendor;
    pci->device = p->device;
    pci->irq = p->irq;
    pci->master = p->master;
    pci->rom_address = p->rom_address;
    for(i=0; i<6; i++)
        pci->base_address[i] = p->base_address[i];
    pci->pDevice = NULL;
#endif
}

void *ice_get_pci(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
    return( pci_devices );
#else
    return( pci_dev_g(pci_devices.next) );
#endif
}

void *ice_get_pci_next(void *p)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
    return( ((struct pci_dev *)p)->next );
#else
    return( pci_dev_g(((struct pci_dev *)p)->global_list.next) );
#endif
}

int ice_is_pci(void *p)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
    return( p!=0 );
#else
    return( ((struct pci_dev *)p) != pci_dev_g(&pci_devices) );
#endif
}

int ice_pci_read_config_dword(void *dev, int where, unsigned int *val)
{
    return( pci_read_config_dword((struct pci_dev *)dev, where, val) );
}


void *ice_vmalloc(unsigned int size)
{
    return( vmalloc(size) );
}

void  ice_vfree(char *p)
{
    vfree(p);
}


/******************************************************************************
*                                                                             *
*   int init_module(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Called once to initialize the module.
*
******************************************************************************/
extern void IceInitModule(void);

int init_module(void)
{
    IceInitModule();

    return( 0 );
}


void *ice_get_module(void *pm, TMODULE *pMod)
{
    if( pm==NULL )
        pm = (void *) *pmodule;
    else
    {
        // Get the next module in the list
        pm = ((struct module *)pm)->next;
    }

    if( pm )
    {
        // Copy the module information into the private API structure
        pMod->pmodule = pm;
        pMod->name = ((struct module *)pm)->name;
        pMod->flags = ((struct module *)pm)->flags;
        pMod->size = ((struct module *)pm)->size;
        pMod->nsyms = ((struct module *)pm)->nsyms;
        pMod->syms = ((struct module *)pm)->syms;
        pMod->ndeps = ((struct module *)pm)->ndeps;
        pMod->init = ((struct module *)pm)->init;
        pMod->cleanup = ((struct module *)pm)->cleanup;
        pMod->use_count = GET_USE_COUNT((struct module *)pm);

        // Address the Linux kernel module with the name "kernel"
        if( !pMod->name || *pMod->name=='\0' )
            pMod->name = "kernel";
    }

    return( pm );
}

void *ice_get_module_init(void *pm)
{
    return( ((struct module *)pm)->init );
}


void ice_for_each_task(int *ref, TTASK *pIceTask, int (ice_for_each_task_cb)(int *,TTASK *))
{
    struct task_struct *pTask;

    // for_each_task() was renamed to for_each_process() in 2.5.35

#ifdef for_each_task
    for_each_task(pTask)
#else
    for_each_process(pTask)
#endif
    {
        pIceTask->ptask = (void *)pTask;
        pIceTask->state = pTask->state;
        pIceTask->pid   = pTask->pid;
        pIceTask->uid   = pTask->uid;
        pIceTask->gid   = pTask->gid;
        pIceTask->comm  = pTask->comm;

        if( !(*ice_for_each_task_cb)(ref, pIceTask) )
            return;
    }
}

void *ice_get_current(void)
{
    return( (void *) current );
}

char *ice_get_current_comm(void)
{
    return( current->comm );
}


/******************************************************************************
*                                                                             *
*   void cleanup_module(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Called once when module unloads.
*
******************************************************************************/
extern void IceCleanupModule(void);

void cleanup_module(void)
{
    IceCleanupModule();
}

int ice_mod_in_use(void)
{
    return( MOD_IN_USE );
}

void ice_mod_inc_use_count(void)
{
    MOD_INC_USE_COUNT;
}

void ice_mod_dec_use_count(void)
{
    MOD_DEC_USE_COUNT;
}
