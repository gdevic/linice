/******************************************************************************
*                                                                             *
*   Module:     iceface.h                                                     *
*                                                                             *
*   Date:       08/15/02                                                      *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

        Define functions exported via iceface module, stubbed in linface.c

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/15/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICEFACE_H_
#define _ICEFACE_H_

extern DWORD ice_get_flags();
extern void  ice_ack_APIC_irq();
extern DWORD ice_io_apic_read(int n, DWORD reg);
extern void  ice_io_apic_write(int n, DWORD reg, DWORD val);
extern int   ice_smp_processor_id();
extern void  ice_smp_call_function(void (*func)(void *), void *info, int retry, int wait);

extern int   ice_init_proc(int ProcRead, int ProcWrite);
extern int   ice_close_proc();

extern int   ice_register_chrdev(char *pDevice);
extern void  ice_unregister_chrdev(int major_device_number, char *pDevice);

extern int   ice_get__NR_mknod(void);
extern int   ice_get__NR_unlink(void);

extern void *ice_ioremap(unsigned int mem1, unsigned int mem2);
extern void  ice_iounmap(void *pMemory);
extern unsigned int ice_page_offset(void);

extern long  ice_copy_to_user(void *p1, void *p2, int len);
extern long  ice_copy_from_user(void *p1, void *p2, int len);

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

extern void  ice_get_pci_info(TPCI *pci, void *p);
extern void *ice_get_pci(void);
extern void *ice_get_pci_next(void *p);
extern int   ice_is_pci(void *p);

extern void *ice_vmalloc(unsigned int size);
extern void  ice_vfree(char *p);

extern int   ice_mod_in_use(void);
extern void  ice_mod_inc_use_count(void);
extern void  ice_mod_dec_use_count(void);


#endif // _ICEFACE_H_

