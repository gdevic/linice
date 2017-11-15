         
/******************************************************************************
*                                                                             *
*   Module:     Intel.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/4/97                                                        *
*                                                                             *
*   Copyright (c) 1997, 2000 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.- 
    Module Description:

        This is a header file containing Intel-processor specific stuff.


*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/4/97     1.00  Original                                       Goran Devic *
* 10/25/00   2.00  Revised for LinIce                             Goran Devic *
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

#define TOPNIBBLE(i)    (((i)>>4)&0xF)

///////////////////////////////////////////////////////////////////////////////
// Define gcc packed structure
///////////////////////////////////////////////////////////////////////////////

#define PACKED __attribute__((packed))

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

} PACKED TDescriptor;

#define GET_DESC_BASE(p)        (p)->base
#define SET_DESC_BASE(p, v)     (p)->base = (v)

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

} TIDT_Gate;

#define INT_TYPE_TASK       0x5         // Task gate
#define INT_TYPE_INT16      0x6         // 16 bit interrupt gate
#define INT_TYPE_TRAP16     0x7         // 16 bit trap gate
#define INT_TYPE_INT32      0xE         // 32 bit interrupt gate
#define INT_TYPE_TRAP32     0xF         // 32 bit trap gate

//-----------------------------------------------------------------------------
// GDT Gates (Code, Data, Task gates, Call gates, LDT descriptors)


typedef struct
{
    DWORD limitLow      :16;            // Limit [15:0]
    DWORD baseLow       :16;            // Base [15:0]
    DWORD baseMid       :8;             // Base [23:16]
    DWORD type          :5;             // Gate type: one of DESC_TYPE_*
    DWORD dpl           :2;             // Gate privilege level
    DWORD present       :1;             // Gate is present
    DWORD limitHigh     :4;             // Limit [20:16]
    DWORD avail         :1;             // Available to use
    DWORD               :1;             // Reserved
    DWORD size32        :1;             // Gate size: 1=32 bit, 0=16 bit
    DWORD granularity   :1;             // Granularity of limit: 1=page, 0=byte
    DWORD baseHigh      :8;             // Base [31:24]

} TGDT_Gate;

#define GET_GDT_BASE(pGDT_Gate)           ( (pGDT_Gate)->baseLow + ((pGDT_Gate)->baseMid << 16) + ((pGDT_Gate)->baseHigh << 24) )

#define GET_GDT_LIMIT(pGDT_Gate)  ( (pGDT_Gate)->limitLow + ((pGDT_Gate)->limitHigh << 16) )

#define DESC_TYPE_TSS16A    0x01        // 16 bit TSS (Available)
#define DESC_TYPE_LDT       0x02        // LDT (CodeData must be 0)
#define DESC_TYPE_TSS16B    0x03        // 16 bit TSS (Busy)
#define DESC_TYPE_CALLG16   0x04        // 16 bit call gate
#define DESC_TYPE_TASKG     0x05        // Task gate
#define DESC_TYPE_TSS32A    0x09        // 32 bit TSS (Available)
#define DESC_TYPE_TSS32B    0x0B        // 32 bit TSS (Busy)
#define DESC_TYPE_CALLG32   0x0C        // 32 bit call gate

#define DESC_TYPE_DATA      0x12        // Read/write data
#define DESC_TYPE_EXEC      0x1A        // Excecute/read

#define DESC_TYPE_NULL      0x00        // Reserved special purpose type

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
    DWORD fDirty      : 1;              // Page has been modified
    DWORD             : 1;              // Reserved field (set to 0)
    DWORD fGlobal     : 1;              // Global page
    DWORD Flags       : 3;              // Page's kernel flags
    DWORD Index       : 20;             // Physical page index

} PACKED TPage;

#define CleanTPage(ptr)   (*(DWORD *)ptr=0)

//-----------------------------------------------------------------------------
// Miscellaneous
//-----------------------------------------------------------------------------

#define INT3                0xCC

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

//-----------------------------------------------------------------------------

#define BITMASK(bit)        (1 << (bit))

//-----------------------------------------------------------------------------
// CR0 Register Defines
//-----------------------------------------------------------------------------
#define PE_BIT          0
#define PG_BIT          31

//-----------------------------------------------------------------------------
// CR4 Register Defines
//-----------------------------------------------------------------------------
#define VME_BIT         0
#define PVI_BIT         1
#define TSD_BIT         2
#define DE_BIT          3
#define PSE_BIT         4
#define PAE_BIT         5
#define MCE_BIT         6
#define PGE_BIT         7
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

    DWORD dr0;
    DWORD dr1;
    DWORD dr2;
    DWORD dr3;
    DWORD dr6;
    DWORD dr7;

} TSysreg;


// Restore packing value
//#pragma pack( pop, enter_intel )

#endif //  _INTEL_H_
