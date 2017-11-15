/******************************************************************************
*                                                                             *
*   Module:     module-header.h                                               *
*                                                                             *
*   Date:       09/03/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This header contains small bits and pieces of the constant defines
        and information pertaining Linux kernel.

        We assume this is not likely to vary between versions or change.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/03/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _MODULE_HEADER_H_
#define _MODULE_HEADER_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
// linkage.h
//-----------------------------------------------------------------------------

#ifndef SIM
#define asmlinkage  __attribute__((regparm(0)))
#else
#define asmlinkage
#endif // SIM

//-----------------------------------------------------------------------------
// posix-types.h
//-----------------------------------------------------------------------------

typedef unsigned short  __kernel_dev_t;
typedef unsigned long   __kernel_ino_t;
typedef unsigned short  __kernel_mode_t;
typedef unsigned short  __kernel_nlink_t;
typedef long            __kernel_off_t;
typedef int             __kernel_pid_t;
typedef unsigned short  __kernel_ipc_pid_t;
typedef unsigned short  __kernel_uid_t;
typedef unsigned short  __kernel_gid_t;
typedef unsigned int    __kernel_size_t;
typedef int             __kernel_ssize_t;
typedef int             __kernel_ptrdiff_t;
typedef long            __kernel_time_t;
typedef long            __kernel_clock_t;
typedef int             __kernel_daddr_t;
typedef char *          __kernel_caddr_t;
typedef unsigned short  __kernel_uid16_t;
typedef unsigned short  __kernel_gid16_t;
typedef unsigned int    __kernel_uid32_t;
typedef unsigned int    __kernel_gid32_t;
typedef unsigned short  __kernel_old_uid_t;
typedef unsigned short  __kernel_old_gid_t;

//-----------------------------------------------------------------------------
// types.h
//-----------------------------------------------------------------------------

typedef __kernel_dev_t      dev_t;
typedef __kernel_ino_t      ino_t;
typedef __kernel_mode_t     mode_t;
typedef __kernel_nlink_t    nlink_t;
typedef __kernel_off_t      off_t;
typedef __kernel_pid_t      pid_t;
typedef __kernel_daddr_t    daddr_t;
typedef __kernel_uid32_t    uid_t;
typedef __kernel_gid32_t    gid_t;
typedef __kernel_uid16_t    uid16_t;
typedef __kernel_gid16_t    gid16_t;

//-----------------------------------------------------------------------------
// ptrace.h
//-----------------------------------------------------------------------------
/* this struct defines the way the registers are stored on the
   stack during a system call. */

struct pt_regs
{
    long    ebx;
    long    ecx;
    long    edx;
    long    esi;
    long    edi;
    long    ebp;
    long    eax;
    int     xds;
    int     xes;
    long    orig_eax;
    long    eip;
    int     xcs;
    long    eflags;
    long    esp;
    int     xss;
};

//-----------------------------------------------------------------------------
// pci.h
//-----------------------------------------------------------------------------

#define PCI_DEVFN(slot,func)    ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn)         (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)         ((devfn) & 0x07)

//-----------------------------------------------------------------------------
// sched.h
//-----------------------------------------------------------------------------

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8


//-----------------------------------------------------------------------------
// module.h
//-----------------------------------------------------------------------------

/* Bits of module.flags.  */

#define MOD_UNINITIALIZED       0
#define MOD_RUNNING             1
#define MOD_DELETED             2
#define MOD_AUTOCLEAN           4
#define MOD_VISITED             8
#define MOD_USED_ONCE           16
#define MOD_JUST_FREED          32
#define MOD_INITIALIZING        64

struct module_symbol
{
    unsigned long value;
    const char *name;
};


//-----------------------------------------------------------------------------
// unistd.h
//-----------------------------------------------------------------------------

#define __NR_exit               1
#define __NR_fork               2
#define __NR_init_module        128
#define __NR_delete_module      129


//-----------------------------------------------------------------------------
// segment.h
//-----------------------------------------------------------------------------

#define __USER_CS               0x23
#define __USER_DS               0x2B


#endif //  _MODULE_HEADER_H_
