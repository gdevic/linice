/******************************************************************************
*                                                                             *
*   Module:     iceface.h                                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   Copyright © 2004 by Goran Devic.  All rights reserved.                    *
*                                                                             *
*   Redistribution and use in source and binary forms, with or without        *
*   modification, are permitted provided that the following conditions        *
*   are met:                                                                  *
*   1. Redistributions of source code must retain the above copyright         *
*      notice, this list of conditions and the following disclaimer.          *
*   2. Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*   3. You may copy and distribute copies of this program but you may not     *
*      charge money or fees for the software product to anyone except to      *
*      cover distribution fees. The Author reserves the right to reclassify   *
*      this software as a non-freeware product at a later date. Doing so      *
*      will not modify the license agreement of previously distributed        *
*      executables.                                                           *
*                                                                             *
*   THIS SOFTWARE IS PROVIDED BY THE AUTHOR “AS IS” WITHOUT WARRANTIES OF     *
*   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,     *
*   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR    *
*   PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,*
*   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING OUT OF   *
*   THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING, BUT NOT LIMITED TO    *
*   LOSS OF DATA OR LOSSES SUSTAINED BY YOU OR THIRD PARTES OR A FAILURE OF   *
*   THE PROGRAM IN ANY WAY).                                                  *
*                                                                             *
*   THE AUTHOR RETAINS TITLE TO AND OWNERSHIP IN THE COPYRIGHT OF THE SOFTWARE*
*   PROGRAM AND THE ASSOCIATED MATERIALS. THIS SOFTWARE IS NOT PROVIDED AS    *
*   PUBLIC DOMAIN SOFTWARE.                                                   *
*                                                                             *
*   AGREEMENT: By using this software product you are automatically agreeing  *
*   to and show that you have read and understood the terms and conditions    *
*   contained within this Freeware Software License Agreement. This Agreement *
*   is then effective while you use and continue to make use of this software *
*   product. If you do not agree with this Agreement, you must not use this   *
*   software product. This Agreement is subject to change without notice.     *
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

typedef struct
{
    void *pmodule;

    const char *name;
    unsigned long flags;
    unsigned long size;
    unsigned nsyms;
    unsigned ndeps;
    int (*init)(void);
    void (*cleanup)(void);
    struct module_symbol *syms;
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
extern void  ice_printk(char *);
extern int   ice_mod_in_use(void);
extern void  ice_mod_inc_use_count(void);
extern void  ice_mod_dec_use_count(void);

#endif // _ICEFACE_H_
