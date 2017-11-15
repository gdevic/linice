/******************************************************************************
*                                                                             *
*   Module:     iceface.h                                                     *
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

        Define functions exported via iceface module.

*******************************************************************************
*   Defines                                                                   *
******************************************************************************/
#ifndef _ICEFACE_H_
#define _ICEFACE_H_

/////////////////////////////////////////////////////////////////
// PRIVATE STRUCTURES
/////////////////////////////////////////////////////////////////

// *_gpl are part of the 2.6 support

typedef struct
{
    void *pmodule;

    const char *name;
    unsigned long flags;
    unsigned long size;
    unsigned nsyms;
    unsigned nsyms_gpl;
    unsigned ndeps;
    int (*init)(void);
    void (*cleanup)(void);
    struct module_symbol *syms;
    struct module_symbol *syms_gpl;
    int use_count;

} TMODULE;

typedef struct
{
    void *ptask;

    long    state;
    pid_t   pid;
    uid_t   uid;
    gid_t   gid;
    char    *comm;

} TTASK;

typedef struct
{
    unsigned char bus;
    unsigned int devfn;
    unsigned short vendor;
    unsigned short device;
    unsigned int irq;
    unsigned int master;
    unsigned long base_address[6];
    unsigned long rom_address;
    char *pDevice;
} TPCI;

/////////////////////////////////////////////////////////////////
// SHARED FUNCTION PROTOS
/////////////////////////////////////////////////////////////////

extern void *ice_get_module(void *, TMODULE *);
extern void *ice_get_module_init(void *);
extern void ice_for_each_task(int *, TTASK *, int (ice_for_each_task_cb)(int *,TTASK *));
extern void *ice_get_current(void);
extern char *ice_get_current_comm(void);
extern int ice_get_io_bitmap_size(void);
extern unsigned int ice_get_flags(void);
extern void  ice_ack_APIC_irq(void);
extern unsigned int  ice_io_apic_read(int, unsigned int);
extern void  ice_io_apic_write(int, unsigned int, unsigned int);
extern int   ice_smp_processor_id(void);
extern void  ice_smp_call_function(void (*func)(void *), void *, int, int);
extern int   ice_init_proc(int, int);
extern int   ice_close_proc(void);
extern int   ice_register_chrdev(char *);
extern void  ice_unregister_chrdev(int, char *);
extern int   ice_get__NR_mknod(void);
extern int   ice_get__NR_unlink(void);
extern void *ice_ioremap(unsigned int, unsigned int);
extern void  ice_iounmap(void *);
extern unsigned int ice_page_offset(void);
extern long  ice_copy_to_user(void *, void *, int);
extern long  ice_copy_from_user(void *, void *, int);
extern void  ice_get_pci_info(TPCI *, void *);
extern void *ice_get_pci(void);
extern void *ice_get_pci_next(void *);
extern int   ice_is_pci(void *);
extern int ice_pci_read_config_dword(void *, int, unsigned int *);
extern void *ice_vmalloc(unsigned int);
extern void  ice_vfree(char *);
extern int   ice_get_printk(void);
extern void  ice_printk(char *);
extern int   ice_mod_in_use(void);
extern void  ice_mod_inc_use_count(void);
extern void  ice_mod_dec_use_count(void);

// Support for some things that we do differently between kernel versions:
extern unsigned int ice_get_kernel_version(void);
#define KERNEL_VERSION_2_6      0x020600
#define KERNEL_VERSION_2_4      0x020400

#endif // _ICEFACE_H_
