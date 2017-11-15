/******************************************************************************
*                                                                             *
*   Module:     simface.c                                                     *
*                                                                             *
*   Date:       05/23/2003                                                    *
*                                                                             *
*   Copyright (c) 1996-2004 Goran Devic                                       *
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

        This module contains the sim interface functions that link with
        the linice library.

*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos

#include <malloc.h>                     // Include memory allocation header
#include <memory.h>                     // Include memory access header
#include <assert.h>                     // Include assert macros
#include <stdlib.h>
#include <fcntl.h>                      // Include IO functions
#include <sys/types.h>                  // Include file functions
#include <stdio.h>                      // Include standard I/O header file

typedef unsigned short uid_t;           // User ID of the file owner
typedef unsigned short gid_t;           // Group ID of the file owner
typedef unsigned long  off_t;           // File size
typedef int            pid_t;           // Process ID type

#include "iceface.h"                    // Include interface defines

#include "sim.h"                        // Include sim header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

#define KBD_DATA            0x60
#define KBD_STATUS          0x64

extern BYTE port_KBD_STATUS;
extern BYTE port_KBD_DATA;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Used this fake code buffer since the switchto task handler actually hooks
// the code by writing a call to it (5 bytes)
BYTE FakeCodeBuffer[5];

char *linice = NULL;
DWORD kbd = 0;
DWORD scan = 0;
DWORD *pmodule = NULL;
DWORD switchto = FakeCodeBuffer;
int ice_debug_level = 0;

// We dont support reading of the PCI space from within the Sim, so it is OK
// to zero all of this out (also, that means we dont include the PCI database header file).

int ice_PCI_VENTABLE_LEN = 0;
int ice_PCI_DEVTABLE_LEN = 0;
int ice_PCI_CLASSCODETABLE_LEN = 0;

int PciVenTable;
int PciDevTable;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void ice_printk(char *p)
{
    printk("%s", p);
}

unsigned int ice_get_flags()
{
    return( 0 );
}

void ice_ack_APIC_irq(void)
{
}

int ice_get_io_bitmap_size(void)
{
#define IO_BITMAP_SIZE  32
    return( IO_BITMAP_SIZE );
}

void ice_smp_call_function(void (*func)(void *), void *info, int retry, int wait)
{
}

int ice_smp_processor_id()
{
    return( 0 );
}

DWORD ice_io_apic_read(int n, DWORD reg)
{
    return( 0 );
}

void ice_io_apic_write(int n, DWORD reg, DWORD val)
{
}


int ice_init_proc(int ProcRead, int ProcWrite)
{
    return( 0 );                        // Return success
}

int ice_close_proc()
{
    return( 0 );
}

int ice_register_chrdev(char *pDevice)
{
    return( 1 );
}

void ice_unregister_chrdev(int major_device_number, char *pDevice)
{
}


typedef int (*PFNMKNOD)(const char *filename, int mode, int dev);
typedef int (*PFNUNLINK)(const char *filename);

int ice_mknod(PFNMKNOD sys_mknod, char *pDevice, int major_device_number)
{
    printk("Simface: MKNOD(""%s"", %d)", pDevice, major_device_number);
    return( 1 );
}

void ice_rmnod(PFNUNLINK sys_unlink, char *pDevice)
{
    printk("Simface: RMNOD(""%s"")", pDevice);
}

int ice_get__NR_mknod(void)
{
    printk("Simface: MKNOD");
    return( __NR_mknod );
}

int ice_get__NR_unlink(void)
{
    printk("Simface: UNLINK");
    return( __NR_unlink );
}

void *ice_ioremap(unsigned int mem1, unsigned int mem2)
{
    assert(0);
//   return( ioremap(mem1, mem2) );
    return(0);
}

void ice_iounmap(void *pMemory)
{
    assert(0);
//    iounmap(pMemory);
}

unsigned int ice_page_offset(void)
{
    // This value is used to map all direct memory accesses to the various kernel
    // regions. We allocate a rather large amount of memory and let linice use it.
    return((unsigned int)pPageOffset);
}

long ice_copy_to_user(void *p1, void *p2, int len)
{
    assert(0);
//    return( copy_to_user(p1, p2, len) );
    return(0);
}

long ice_copy_from_user(void *p1, void *p2, int len)
{
    memcpy(p1, p2, len);
    return(0);
}



void ice_get_pci_info(TPCI *pci, void *ptr)
{
}

void *ice_get_pci(void)
{
    assert(0);
    return( NULL );
}

void *ice_get_pci_next(void *p)
{
    assert(0);
    return( NULL );
}

int ice_is_pci(void *p)
{
    assert(0);
    return( 0 );
}

int ice_pci_read_config_dword(void *dev, int where, unsigned int *val)
{
    assert(0);
    return( 0 );
}


void *ice_vmalloc(unsigned int size)
{
    return( (BYTE *)malloc(size) );
}

void ice_vfree(char *p)
{
    free(p);
}


void *ice_get_module(void *pm, TMODULE *pMod)
{
#if 0
    if( pm==NULL )
        pm = (void *) pmodule;
    else
    {
        // Get the next module in the list
        pm = (DWORD)(((TMODULE *)pm)->pmodule) - PAGE_OFFSET;
    }

    if( pm )
    {
        // Copy the module parameters into the given buffer
        memcpy(pMod, ((DWORD)pm - PAGE_OFFSET) + pPageOffset, sizeof(TMODULE));
    }
#endif
    return( pm );
}

void *ice_get_module_init(void *pm)
{
    assert(0);
//    return( ((struct module *)pm)->init );
    return( NULL );
}


void ice_for_each_task(int *ref, TTASK *pIceTask, int (ice_for_each_task_cb)(int *,TTASK *))
{
    assert(0);
#if 0
    struct task_struct *pTask;

    for_each_task(pTask)
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
#endif
}

void *ice_get_current(void)
{
    assert(0);
    return( NULL );
}

char *ice_get_current_comm(void)
{
    return( "types" ); // :)
}

void cleanup_module(void)
{
    assert(0);
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


void outp(int port, int value)
{
}

unsigned char inp(unsigned short port)
{
    // Depending on the port, we do some virtualization (simulation)

    switch(port)
    {
    case KBD_STATUS:
        return(port_KBD_STATUS);

    case KBD_DATA:
        return(port_KBD_DATA);
    }

    return( 0xFF );
}

WORD GetKernelCS()
{
    return 0x10;
}

WORD GetKernelDS()
{
    return 0x68;
}

void LoadModuleFile(int fd)
{
    off_t size;                         // Size of the capture file
    DWORD *next;
    TMODULE Module;                     // Temp module descriptor

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Read only the module descriptor; decrement the size remaining
    size -= read(fd, &Module, sizeof(TMODULE));

    // Get the address of the module in the virtual memory and load
    // the rest of the module there
    read(fd, pPageOffset + (DWORD)Module.pmodule - PAGE_OFFSET, size);

    // Overlay the TMODULE structure to the beginning of that space
    memcpy(pPageOffset + (DWORD)Module.pmodule - PAGE_OFFSET, &Module, sizeof(TMODULE));

    close(fd);

    // Link the new module within the module chain
    next = pmodule;
    pmodule = Module.pmodule;
    Module.pmodule = next;
}