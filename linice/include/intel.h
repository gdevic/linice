/******************************************************************************
*                                                                             *
*   Module:     intel.h                                                       *
*                                                                             *
*   Date:       9/4/97                                                        *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

        This is a header file containing Intel-processor specific stuff.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/4/97     1.00  Original                                       Goran Devic *
* 4/25/00    2.00  Revised for Linice                             Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _INTEL_H_
#define _INTEL_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Save previous packing value and set byte-packing
//#pragma pack( push, enter_intel )
//#pragma pack( 1 )

///////////////////////////////////////////////////////////////////////////////
// Some commonly used macros and defines
///////////////////////////////////////////////////////////////////////////////

#undef LOWORD
#undef HIWORD
#define LOWORD(i)       ((WORD)((i)&0xFFFF))
#define HIWORD(i)       ((WORD)(((i)>>16) & 0xFFFF))

///////////////////////////////////////////////////////////////////////////////
// Define gcc packed structure
///////////////////////////////////////////////////////////////////////////////

#ifndef SIM
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif // SIM

/////////////////////////////////////////////////////////////////
// STRUCTURE DESCRIBING THE LIVE DEBUGEE
/////////////////////////////////////////////////////////////////
// Define structure that holds CPU register state after an interrupt

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

///////////////////////////////////////////////////////////////////////////////
// Defines of the Intel processor
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Task state segment structure
//-----------------------------------------------------------------------------

typedef struct
{
   WORD      link;                      // Backlink
   WORD      dummy1;                    // filler
   DWORD     esp0;                      // ESP for level 0
   WORD      ss0;                       // Stack selector for level 0
   WORD      dummy2;                    // filler
   DWORD     esp1;                      // ESP for level 1
   WORD      ss1;                       // Stack selector for level 1
   WORD      dummy3;                    // filler
   DWORD     esp2;                      // ESP for level 2
   WORD      ss2;                       // Stack selector for level 2
   WORD      dummy4;                    // filler
   DWORD     cr3;                       // PDPR, page directory address
   DWORD     eip;                       // current EIP
   DWORD     eflags;                    // current flags register
   DWORD     eax;                       // current eax register
   DWORD     ecx;                       // current ecx register
   DWORD     edx;                       // current edx register
   DWORD     ebx;                       // current ebx register
   DWORD     esp;                       // current esp register
   DWORD     ebp;                       // current ebp register
   DWORD     esi;                       // current esi register
   DWORD     edi;                       // current edi register
   WORD      es;                        // current es selector
   WORD      dummy5;                    // filler
   WORD      cs;                        // current cs selector
   WORD      dummy6;                    // filler
   WORD      ss;                        // current ss selector
   WORD      dummy7;                    // filler
   WORD      ds;                        // current ds selector
   WORD      dummy8;                    // filler
   WORD      fs;                        // current fs selector
   WORD      dummy9;                    // filler
   WORD      gs;                        // current gs selector
   WORD      dummyA;                    // filler
   WORD      ldt;                       // local descriptor table
   WORD      dummyB;                    // filler
   WORD      debug;                     // bit 0 is T, debug trap
   WORD      ioperm;                    // io permission offset

} PACKED TSS_Struct;


//-----------------------------------------------------------------------------
// Descriptor registers: IDT and GDT
//-----------------------------------------------------------------------------

// Generic IDT and GDT descriptors

typedef struct
{
    WORD    limit;                      // [0:15]  limit field
    DWORD   base;                       // [16:47] base address field

} PACKED TDescriptor, *PTDescriptor;

#define GET_DESC_BASE(p)        ((p)->base)
#define SET_DESC_BASE(p, v)     ((p)->base = (v))

//-----------------------------------------------------------------------------
// IDT Gates (Task, Interrupt and Trap)

typedef struct
{
    DWORD  offsetLow    : 16;           // Target handler offset [15:0]
    DWORD  selector     : 16;           // Which selector to use (code/TSS)
    DWORD  res          :  8;           // Reserved - must be 0
    DWORD  type         :  5;           // Gate type: one of INT_TYPE_*
    DWORD  dpl          :  2;           // Privilege level
    DWORD  present      :  1;           // Present
    DWORD  offsetHigh   : 16;           // Target handler offset [31:16]

} PACKED TIDT_Gate, *PTIDT_Gate;

#define INT_TYPE_TASK       0x5         // Task gate
#define INT_TYPE_INT16      0x6         // 16 bit interrupt gate (clears IF)
#define INT_TYPE_TRAP16     0x7         // 16 bit trap gate (leaves IF set)
#define INT_TYPE_INT32      0xE         // 32 bit interrupt gate (clears IF)
#define INT_TYPE_TRAP32     0xF         // 32 bit trap gate (leaves IF set)

#define GET_IDT_BASE(pIDT_Gate)   ((pIDT_Gate)->offsetLow + ((pIDT_Gate)->offsetHigh << 16))

//-----------------------------------------------------------------------------
// GDT Gates (Code, Data, Task gates, Call gates, LDT descriptors)

typedef struct
{
    DWORD limitLow      :16;            // Limit [15:0]
    DWORD baseLow       :16;            // Base [15:0]
    DWORD baseMid       :8;             // Base [23:16]
    DWORD type          :4;             // Gate type: one of DESC_TYPE_*
    DWORD system        :1;             // System segment (0) or data/code (1) [12]
    DWORD dpl           :2;             // Gate privilege level
    DWORD present       :1;             // Gate is present
    DWORD limitHigh     :4;             // Limit [20:16]
    DWORD avail         :1;             // Available to use
    DWORD res           :1;             // Reserved
    DWORD size32        :1;             // Gate size: 1=32 bit, 0=16 bit
    DWORD granularity   :1;             // Granularity of limit: 1=page, 0=byte
    DWORD baseHigh      :8;             // Base [31:24]

} PACKED TGDT_Gate, *PTGDT_Gate;

#define GET_GDT_BASE(pGDT_Gate)   ((pGDT_Gate)->baseLow + ((pGDT_Gate)->baseMid << 16) + ((pGDT_Gate)->baseHigh << 24))
#define GET_GDT_LIMIT(pGDT_Gate)  ((pGDT_Gate)->limitLow + ((pGDT_Gate)->limitHigh << 16))

// System segment types (system[12]=0):
#define DESC_TYPE_NULL      0x00        // Reserved special purpose type
#define DESC_TYPE_TSS16A    0x01        // 16 bit TSS (Available)
#define DESC_TYPE_LDT       0x02        // LDT (CodeData must be 0)
#define DESC_TYPE_TSS16B    0x03        // 16 bit TSS (Busy)
#define DESC_TYPE_CALLG16   0x04        // 16 bit call gate
#define DESC_TYPE_TASKG     0x05        // Task gate
#define DESC_TYPE_TSS32A    0x09        // 32 bit TSS (Available)
#define DESC_TYPE_TSS32B    0x0B        // 32 bit TSS (Busy)
#define DESC_TYPE_CALLG32   0x0C        // 32 bit call gate

//-----------------------------------------------------------------------------
// Page directory and page table bits
//-----------------------------------------------------------------------------

typedef struct
{
    DWORD fPresent    : 1;              // Page is present
    DWORD fWrite      : 1;              // Page can be written
    DWORD fUser       : 1;              // Page can be accessed by the user
    DWORD fWriteThr   : 1;              // Write-through enabled
    DWORD fNoCache    : 1;              // Disable cache
    DWORD fAccessed   : 1;              // Page has been accessed
    DWORD fDirty      : 1;              // Page has been modified (PTE only)
    DWORD fPS         : 1;              // 4Mb page (PD only)
    DWORD fGlobal     : 1;              // Global page (PTE only)
    DWORD Flags       : 3;              // Page's kernel flags
    DWORD Index       : 20;             // Physical page index

} PACKED TPage;

#define CleanTPage(ptr)   (*(DWORD *)ptr=0)

//-----------------------------------------------------------------------------
// Miscellaneous
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EFLAGS Bits
//-----------------------------------------------------------------------------

#define CF_BIT              0
#define PF_BIT              2
#define AF_BIT              4
#define ZF_BIT              6
#define SF_BIT              7
#define TF_BIT              8
#define IF_BIT              9
#define DF_BIT              10
#define OF_BIT              11
#define IOPL_BIT0           12
#define IOPL_BIT1           13
#define NT_BIT              14
#define RF_BIT              16
#define VM_BIT              17
#define AC_BIT              18
#define VIF_BIT             19
#define VIP_BIT             20
#define ID_BIT              21

#undef TF_MASK
#undef IF_MASK
#undef IOPL_MASK
#undef NT_MASK
#undef VM_MASK
#undef AC_MASK
#undef VIF_MASK
#undef VIP_MASK
#undef ID_MASK

#define CF_MASK             (1 << CF_BIT)
#define PF_MASK             (1 << PF_BIT)
#define AF_MASK             (1 << AF_BIT)
#define ZF_MASK             (1 << ZF_BIT)
#define SF_MASK             (1 << SF_BIT)
#define TF_MASK             (1 << TF_BIT)
#define IF_MASK             (1 << IF_BIT)
#define DF_MASK             (1 << DF_BIT)
#define OF_MASK             (1 << OF_BIT)
#define IOPL_MASK           (3 << IOPL_BIT0)
#define NT_MASK             (1 << NT_BIT)
#define RF_MASK             (1 << RF_BIT)
#define VM_MASK             (1 << VM_BIT)
#define AC_MASK             (1 << AC_BIT)
#define VIF_MASK            (1 << VIF_BIT)
#define VIP_MASK            (1 << VIP_BIT)
#define ID_MASK             (1 << ID_BIT)

#define SF_VALUE(flags)     ((flags>>SF_BIT)&1)
#define OF_VALUE(flags)     ((flags>>OF_BIT)&1)

//-----------------------------------------------------------------------------

#define BITMASK(bit)        (1 << (bit))

//-----------------------------------------------------------------------------
// CR0 Register Defines
//-----------------------------------------------------------------------------
#define PE_BIT          0
#define MP_BIT          1
#define EM_BIT          2
#define TS_BIT          3
#define ET_BIT          4
#define NE_BIT          5
#define WP_BIT          16
#define AM_BIT          18
#define NW_BIT          29
#define CD_BIT          30
#define PG_BIT          31

//-----------------------------------------------------------------------------
// CR3 Register Defines
//-----------------------------------------------------------------------------
#define PWT_BIT         3
#define PCD_BIT         4

//-----------------------------------------------------------------------------
// CR4 Register Defines
//-----------------------------------------------------------------------------
#define VME_BIT         0
#define PVI_BIT         1
#define TSD_BIT         2
#define DE_BIT          3
#define PSE_BIT         4   // Page Size Extension; 4Mb pages
#define PAE_BIT         5   // Page Address Extension; CR3 points to Page Directory Pointers (PDP), 36-bit total
#define MCE_BIT         6
#define PGE_BIT         7   // Paging Global Extensions; on CR3 reload, flush only pages with Global bit cleared
#define PCE_BIT         8

//#define mov_eax_cr4 __asm _emit 0x0F __asm _emit 0x20 __asm _emit 0xE0
//#define mov_cr4_eax __asm _emit 0x0F __asm _emit 0x22 __asm _emit 0xE0

//-----------------------------------------------------------------------------
// Debug Registers Defines
//-----------------------------------------------------------------------------
#define DR7_L0_BIT          0
#define DR7_G0_BIT          1
#define DR7_L1_BIT          2
#define DR7_G1_BIT          3
#define DR7_L2_BIT          4
#define DR7_G2_BIT          5
#define DR7_L3_BIT          6
#define DR7_G3_BIT          7

#define DR7_RW0_BIT         16
#define DR7_LEN0_BIT        18
#define DR7_RW1_BIT         20
#define DR7_LEN1_BIT        22
#define DR7_RW2_BIT         24
#define DR7_LEN2_BIT        26
#define DR7_RW3_BIT         28
#define DR7_LEN3_BIT        30


#define DR6_B0_BIT          0
#define DR6_B1_BIT          1
#define DR6_B2_BIT          2
#define DR6_B3_BIT          3

#define DR6_BD_BIT          13
#define DR6_BS_BIT          14
#define DR6_BT_BIT          15

//-----------------------------------------------------------------------------
// System Registers Defines
//-----------------------------------------------------------------------------
typedef struct
{
    DWORD cr0;
    DWORD cr2;
    DWORD cr3;
    DWORD cr4;

    DWORD dr[4];        // First 4 are debug linear address and for convinience
    DWORD dr6;          //  we keep them as arrays
    DWORD dr7;

} PACKED TSysreg;

// Define DR7 bits
#define DR7_EXEC            0           // Bp on execution only
#define DR7_WRITE           1           // Bp on data writes only
#define DR7_IO              2           // Bp on IO reads or writes
#define DR7_DATARW          3           // Bp on data reads/writes, but not exec

#define DR7_LEN1            4           // 1 byte len
#define DR7_LEN2            8           // 2 byte len
#define DR7_LEN4            0xC         // 4 byte len


///////////////////////////////////////////////////////////////////////////////
// Defines for the Intel IO APIC registers (82379AB Only)
///////////////////////////////////////////////////////////////////////////////

// Define IO APIC register 0 (Identification register)
typedef struct
{
    DWORD res1          : 24;           // Reserved
    DWORD Id            : 4;            // IO APIC Identification number
    DWORD res2          : 4;            // Reserved

} PACKED TIOAPIC0;

// Define IO APIC register 1 (Version register)
typedef struct
{
    DWORD Version       : 8;            // IO APIC Version number
    DWORD res1          : 8;            // Reserved
    DWORD nRedir        : 8;            // Maximum redirection entry
    DWORD res2          : 8;            // Reserved

} PACKED TIOAPIC1;

// Define IO APIC register 2 (Arbitration register)
typedef struct
{
    DWORD res1          : 24;           // Reserved
    DWORD Id            : 4;            // IO APIC Arbitration value
    DWORD res2          : 4;            // Reserved

} PACKED TIOAPIC2;

//-----------------------------------------------------------------------------
// IO APIC I/O Redirection Table Registers
//-----------------------------------------------------------------------------
typedef struct
{
    DWORD IntVec        : 8;            // Interrupt vector
    DWORD DelMod        : 3;            // Delivery mode
    DWORD DestMod       : 1;            // Destination mode
    DWORD Delivs        : 1;            // Delivery status
    DWORD IntPol        : 1;            // Interrupt polarity
    DWORD RemoteIrr     : 1;            // For level-triggered ints
    DWORD TriggerMode   : 1;            // Level or edge sensitive
    DWORD Mask          : 1;            // Interrupt mask
    DWORD res1          :15;            // Reserved
    WORD  dword2;                       // Reserved (padding)
    BYTE  res3;                         // Reserved (padding)
    BYTE  Dest;                         // Destination field

} PACKED TIOAPICREDIR;


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

#ifndef SIM

#define SET_IDT(_idt)    asm("lidt  %0" : : "m" (_idt))
#define SET_GDT(_gdt)    asm("lgdt  %0" : : "m" (_gdt))
#define SET_TR(_tr)      asm("ltr   %0" : : "m" (_tr))
#define SET_LDT(_tr)     asm("lldt  %0" : : "m" (_tr))

#define GET_IDT(_idt)    asm("sidt  %0" : : "m" (_idt))
#define GET_GDT(_gdt)    asm("sgdt  %0" : : "m" (_gdt))
#define GET_TR(_tr)      asm("str   %0" : : "m" (_tr))
#define GET_LDT(_tr)     asm("sldt  %0" : : "m" (_tr))

#define GET_CR0(_reg)    __asm__ __volatile__("movl %%cr0,%0":"=r" (_reg));
#define SET_CR0(_reg)    __asm__ __volatile__("movl %0,%%cr0"::"r" (_reg) );
#define GET_CR2(_reg)    __asm__ __volatile__("movl %%cr2,%0":"=r" (_reg));
#define SET_CR2(_reg)    __asm__ __volatile__("movl %0,%%cr2"::"r" (_reg) );
#define GET_CR3(_reg)    __asm__ __volatile__("movl %%cr3,%0":"=r" (_reg));
#define SET_CR3(_reg)    __asm__ __volatile__("movl %0,%%cr3"::"r" (_reg) );
#define GET_CR4(_reg)    __asm__ __volatile__("movl %%cr4,%0":"=r" (_reg));
#define SET_CR4(_reg)    __asm__ __volatile__("movl %0,%%cr4"::"r" (_reg) );

#define GET_DR0(_reg)    __asm__("movl %%dr0,%0":"=r" (_reg));
#define SET_DR0(_reg)    __asm__("movl %0,%%dr0"::"r" (_reg) );
#define GET_DR1(_reg)    __asm__("movl %%dr1,%0":"=r" (_reg));
#define SET_DR1(_reg)    __asm__("movl %0,%%dr1"::"r" (_reg) );
#define GET_DR2(_reg)    __asm__("movl %%dr2,%0":"=r" (_reg));
#define SET_DR2(_reg)    __asm__("movl %0,%%dr2"::"r" (_reg) );
#define GET_DR3(_reg)    __asm__("movl %%dr3,%0":"=r" (_reg));
#define SET_DR3(_reg)    __asm__("movl %0,%%dr3"::"r" (_reg) );
#define GET_DR6(_reg)    __asm__("movl %%dr6,%0":"=r" (_reg));
#define SET_DR6(_reg)    __asm__("movl %0,%%dr6"::"r" (_reg) );
#define GET_DR7(_reg)    __asm__("movl %%dr7,%0":"=r" (_reg));
#define SET_DR7(_reg)    __asm__("movl %0,%%dr7"::"r" (_reg) );

#define INT1()           __asm__("int $1" ::);
#define INT3()           __asm__("int $3" ::);

#define LocalCLI()       __asm__("cli" ::);
#define LocalSTI()       __asm__("sti" ::);

#define CLTS()           __asm__("clts" ::);

#define FNCLEX()         __asm__("fnclex" ::);

extern unsigned char inp(unsigned short port);

#define outp(port,value) __asm__ __volatile__("outb %b0,%w1\n\t":: "a" (value) , "d" (port) );
#define outw(port,value) __asm__ __volatile__("outw %w0,%w1\n\t":: "a" (value) , "d" (port) );
#define outd(port,value) __asm__ __volatile__("outl %w0,%w1\n\t":: "a" (value) , "d" (port) );

#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))

#else // SIM

extern void outp(int port, int value);
extern unsigned char inp(unsigned short port);

#define GET_GDT(a) GetGDT(&(a));
#define GET_IDT(a) GetIDT(&(a));
extern void GetGDT(TDescriptor *p);
extern void GetIDT(TDescriptor *p);

#define INT(x)

#endif // SIM

// Restore packing value
//#pragma pack( pop, enter_intel )

#endif //  _INTEL_H_

