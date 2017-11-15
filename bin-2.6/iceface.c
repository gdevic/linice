/******************************************************************************
*                                                                             *
*   Module:     iceface.c                                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   Copyright © 2005 by Goran Devic.  All rights reserved.                    *
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

    This is the kernel-specific interface module for Linice.

    This interface glues to the kernels 2.6.

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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");

#include <linux/version.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/times.h>
#include <asm/uaccess.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/unistd.h>
#include <linux/proc_fs.h>
#include <asm/page.h>
#include <asm/io.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>

#ifdef SMP
#include <asm/smp.h>
#include <asm/mtrr.h>
#include <asm/mpspec.h>
#include <asm/pgalloc.h>
#endif

#ifdef _PCIHDR
#include "pcihdr.h"
#else // _PCIHDR
#define PCI_VENTABLE_LEN        0
#define PCI_DEVTABLE_LEN        0
#define PCI_CLASSCODETABLE_LEN  0
int PciVenTable = 0;
int PciDevTable = 0;
#endif // _PCIHDR

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
//  kbd=<address>                       Address of the handle_kbd_event function
//  scan=<address>                      Address of the handle_scancode function
//  mod=<address>                       Address of the module_list variable
//  sys=<address>                       Address of the sys_call_table symbol
//  ice_debug_level=[0 - 1]             Set the level for the output messages:
//                                      0 - Do not display INFO level
//                                      1 - Display INFO level messages

MODULE_PARM(kbd, "i");                  // kbd=<integer>
DWORD kbd = 0;                          // default value

MODULE_PARM(scan, "i");                 // scan=<integer>
DWORD scan = 0;                         // default value

MODULE_PARM(pmodule, "i");              // mod=<integer>
DWORD *pmodule = NULL;                  // default value

MODULE_PARM(sys, "i");                  // sys=<integer>
DWORD sys = 0;                          // default value

MODULE_PARM(switchto, "i");             // switchto=<integer>
DWORD switchto = 0;                     // default value

MODULE_PARM(start_sym, "i");            // Kernel 2.6 symbols - start location
DWORD *start_sym = NULL;                // default value

MODULE_PARM(stop_sym, "i");             // Kernel 2.6 symbols - end location
DWORD *stop_sym = NULL;                 // default value

MODULE_PARM(start_sym_gpl, "i");        // Kernel 2.6 GPL symbols - start location
DWORD *start_sym_gpl = NULL;            // default value

MODULE_PARM(stop_sym_gpl, "i");         // Kernel 2.6 GPL symbols - end location
DWORD *stop_sym_gpl = NULL;             // default value

MODULE_PARM(ice_debug_level, "i");      // ice_debug_level=<integer>
int ice_debug_level = 1;                // default value


// Uncomment this on RH9.0 SMP
#ifdef IO_APIC
struct mpc_config_ioapic mp_ioapics[MAX_IO_APICS];
#endif // IO_APIC

static struct proc_dir_entry *pProcEntry;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

unsigned int ice_get_flags(void)
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
    return( IO_BITMAP_BYTES );
}

void ice_smp_call_function(void (*func)(void *), void *info, int retry, int wait)
{
#ifdef SMP
    smp_call_function(func, info, retry, wait);
#endif // SMP
}

int ice_smp_processor_id(void)
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
#endif // IO_APIC
}

int ice_init_proc(int ProcRead, int ProcWrite)
{
    pProcEntry = create_proc_entry("linice", 0644, &proc_root);
    if( pProcEntry )
    {
        *(int*) &pProcEntry->read_proc  = (int) ProcRead;
        *(int*) &pProcEntry->write_proc = (int) ProcWrite;
    }
    else
        return( -1 );
    return( 0 );
}

int ice_close_proc(void)
{
    remove_proc_entry("linice", &proc_root);

    return( 0 );
}

typedef int (*PFNOPEN)(struct inode *inode, struct file *file);
typedef int (*PFNCLOSE)(struct inode *inode, struct file *file);
typedef int (*PFNIOCTL)(struct inode *inode, struct file *file, unsigned int ioctl, unsigned long param);

extern int DriverOpen(void);
extern int DriverClose(void);
extern int DriverIOCTL(void *, void *, unsigned int, unsigned long);

static struct file_operations ice_fops = {
    owner:      THIS_MODULE,
    ioctl:      (PFNIOCTL)DriverIOCTL,
    open:       (PFNOPEN)DriverOpen,
    release:    (PFNCLOSE)DriverClose,
};

int ice_register_chrdev(char *pDevice)
{
    return( register_chrdev(0, pDevice, &ice_fops) );
}

void ice_unregister_chrdev(int major_device_number, char *pDevice)
{
    unregister_chrdev(major_device_number, pDevice);
}

typedef asmlinkage int (*PFNMKNOD)(const char *, int, dev_t);
typedef asmlinkage int (*PFNUNLINK)(const char *);

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

int ice_PCI_VENTABLE_LEN = PCI_VENTABLE_LEN;
int ice_PCI_DEVTABLE_LEN = PCI_DEVTABLE_LEN;
int ice_PCI_CLASSCODETABLE_LEN = PCI_CLASSCODETABLE_LEN;

void ice_get_pci_info(TPCI *pci, void *ptr)
{
    struct pci_dev *p = ptr;

    int i;
    pci->bus = p->bus->number;
    pci->devfn = p->devfn;
    pci->vendor = p->vendor;
    pci->device = p->device;
    pci->irq = p->irq;
    pci->master = 0;
    pci->rom_address = 0;
    for(i=0; i<6; i++)
        pci->base_address[i] = p->resource[i].start;
#ifdef CONFIG_PCI_NAMES
    pci->pDevice = p->pretty_name;
#else  // CONFIG_PCI_NAMES
    pci->pDevice = "";
#endif // CONFIG_PCI_NAMES
}

void *ice_get_pci(void)
{
#if 0
    return( pci_dev_g(pci_devices.next) );
#endif
return( 0 );
}

void *ice_get_pci_next(void *p)
{
    return( pci_dev_g(((struct pci_dev *)p)->global_list.next) );
}

int ice_is_pci(void *p)
{
#if 0
    return( ((struct pci_dev *)p) != pci_dev_g(&pci_devices) );
#endif
return( 0 );
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

extern int IceInitModule(void);


void ice_printk(char *p)
{
    printk(p);
}

int ice_get_printk(void)
{
    return( (int)printk );
}

static struct module kernel_fake =
{
    .state = 0,
    .name = "kernel",
    .core_size = 0,
    .init = 0,
    .syms = 0,
    .num_syms = 0,
    .gpl_syms = 0,
    .num_gpl_syms = 0,

};

int init_module_2_6(void)
{
    kernel_fake.syms = (struct kernel_symbol*)start_sym;
    kernel_fake.num_syms = (stop_sym - start_sym)/sizeof(struct kernel_symbol);
    kernel_fake.gpl_syms = (struct kernel_symbol*)start_sym_gpl;
    kernel_fake.num_gpl_syms = (stop_sym_gpl - start_sym_gpl)/sizeof(struct kernel_symbol);

    return( IceInitModule() );
}

struct module_use
{
    struct list_head list;
    struct module *module_which_uses;
};

void *ice_get_module(void *pm, TMODULE *pMod)
{
    struct module_use *use;
    if( pm==NULL )
    pm = list_entry(((struct list_head*)pmodule)->next, struct module, list);
    else if (pm == &kernel_fake)
    {
    return NULL;
    }
    else
    {
        pm = list_entry(((struct module *)pm)->list.next, struct module, list);
    }

    if (pm == list_entry(((struct list_head*)pmodule), struct module, list))
    {
        pm = &kernel_fake;
        pMod->pmodule = pm;
        pMod->name  = ((struct module *)pm)->name;
        pMod->flags = 0;
        pMod->size  = 0;                        // _etext - _stext;
        pMod->init  = 0;                        // start_kernel;
        pMod->nsyms = ((struct module *)pm)->num_syms;
        pMod->syms  = (struct module_symbol *)((struct module *)pm)->syms;
        pMod->nsyms_gpl = ((struct module *)pm)->num_gpl_syms;
        pMod->syms_gpl  = (struct module_symbol *)((struct module *)pm)->gpl_syms;
        pMod->use_count = 1;
        pMod->cleanup =   0;
        pMod->ndeps   = 0;

        return pm;
    }

    if( pm )
    {
        pMod->pmodule = pm;
        pMod->name  = ((struct module *)pm)->name;
        pMod->flags = ((struct module *)pm)->state;
        pMod->size  = ((struct module *)pm)->core_size;
        pMod->nsyms = ((struct module *)pm)->num_syms;
        pMod->syms  = (struct module_symbol *)((struct module *)pm)->syms;
        pMod->nsyms_gpl = ((struct module *)pm)->num_gpl_syms;
        pMod->syms_gpl  = (struct module_symbol *)((struct module *)pm)->gpl_syms;
        pMod->init  = ((struct module *)pm)->init;

#ifdef CONFIG_MODULE_UNLOAD
        pMod->use_count = module_refcount(pm);
        pMod->cleanup =((struct module *)pm)->exit;
        pMod->ndeps = 0;
        list_for_each_entry(use, &((struct module *)pm)->modules_which_use_me, list)
        pMod->ndeps++;
#else
        pMod->use_count = 0;
        pMod->cleanup = 0;
        pMod->ndeps = 0;
#endif
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

extern void IceCleanupModule(void);

void cleanup_module_2_6(void)
{
    IceCleanupModule();
}

int ice_mod_in_use(void)
{
    return( 0 );
}

void ice_mod_inc_use_count(void)
{
}

void ice_mod_dec_use_count(void)
{
}

// We assume that we are running 2.6
unsigned int ice_get_kernel_version(void)
{
    return( KERNEL_VERSION_2_6 );
}

module_init(init_module_2_6);
module_exit(cleanup_module_2_6);

