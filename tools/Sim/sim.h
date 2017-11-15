/******************************************************************************
*                                                                             *
*   Module:     Sim.h                                                         *
*                                                                             *
*   Date:       05/24/2003                                                    *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains major defines for the simulator.

*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _SIM_H_
#define _SIM_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

typedef unsigned int UINT;

// Symbol map file needed to install linice

#define SYSTEM_MAP          "System.map"

// Kernel virtual mapping

#if 0
#define SIM_PAGE_OFFSET_SIZE    (1024 * 1024 * 1024)
#define PAGE_OFFSET             (0xC0000000)
#endif 

#if 1
#define SIM_PAGE_OFFSET_SIZE    (1024 * 1024 * 16)
#define PAGE_OFFSET             (0xD0000000)
#endif

extern char *pPageOffset;

// CPU descriptors

typedef struct
{
    WORD    limit;                      // [0:15]  limit field
    DWORD   base;                       // [16:47] base address field

} TDescriptor, *PTDescriptor;

extern TDescriptor GDT;
extern TDescriptor IDT;

// Kernel system calls

#define __NR_exit               1
#define __NR_fork               2
#define __NR_unlink             10
#define __NR_mknod              14
#define __NR_init_module        128
#define __NR_delete_module      129

typedef unsigned long ULONG;

/////////////////////////////////////////////////////////////////
// STRUCTURE DESCRIBING THE LIVE DEBUGEE
/////////////////////////////////////////////////////////////////
// Define structure that holds debugee state after an interrupt

typedef struct tagTRegs
{
    DWORD   esp;
    DWORD   ss;
    DWORD   es;
    DWORD   ds;
    DWORD   fs;
    DWORD   gs;
    DWORD   edi;
    DWORD   esi;
    DWORD   ebp;
    DWORD   temp;
    DWORD   ebx;
    DWORD   edx;
    DWORD   ecx;
    DWORD   eax;
    DWORD   ChainAddress;
    DWORD   ErrorCode;
    DWORD   eip;
    DWORD   cs;
    DWORD   eflags;
} TREGS, *PTREGS;

#define OPT_VERBOSE         0x00010000  // Option verbose, make output informative
extern unsigned int opt;                // Option variable

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void SimInit();
extern void SimInstall();
extern void SimUninstall();
extern void SimLoadSymbolFile();
extern void SimUnloadSymbolFile();
extern void SimINT1();
extern void SimKey(UINT wCode, DWORD lExt);

extern BOOL OptInstall(char *pSystemMap);
extern void OptUninstall();
extern void OptAddSymbolTable(char *sName);
extern void OptRemoveSymbolTable(char *sTableName);
extern int  printk(char *,...);
extern void Message(char *msg);

#endif  //  _SIM_H_
