/******************************************************************************
*                                                                             *
*   Module:     ice.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/27/2000                                                    *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains global Ice data structures


*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 10/27/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_H_
#define _ICE_H_

#include "intel.h"  

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/
             
#define SEL_ICE_CS      0x10
#define SEL_ICE_DS      0x18
#define SEL_ICE_SS      0x18
#define SEL_ICE_ES      0x18
#define SEL_ICE_FS      0   
#define SEL_ICE_GS      0   

/////////////////////////////////////////////////////////////////
// Define structure that holds debugee state after an interrupt
/////////////////////////////////////////////////////////////////

typedef struct
{
    DWORD       pmES;
    DWORD       pmGS;
    DWORD       pmFS;
    DWORD       pmDS;
    DWORD       edi;
    DWORD       esi;
    DWORD       ebp;
    DWORD       temp;
    DWORD       ebx;
    DWORD       edx;
    DWORD       ecx;
    DWORD       eax;
    DWORD       ErrorCode;
    DWORD       eip;
    DWORD       pmCS;
    DWORD       eflags;
    DWORD       esp;
    DWORD       SS;
    DWORD       vmES;
    DWORD       vmDS;
    DWORD       vmFS;
    DWORD       vmGS;
} TRegs;


typedef struct
{
    TRegs       *r;                     // Pointer to a register structure
    TSS_Struct  *pTss;                  // Pointer to a TSS structure

    TDescriptor idt;                    // interrupt descriptor table
    TDescriptor gdt;                    // global descriptor table

    TGDT_Gate   *pGdt;                  // Pointer to actual GDT table
    TIDT_Gate   *pIdt;                  // Pointer to actual IDT table

    DWORD       nInterrupt;             // Current interrupt number

    BOOL        fInt1Here;              // Break on INT1
    BOOL        fInt3Here;              // Break on INT3

} TDeb;

extern TDeb deb;                        // Debugee state structure


typedef struct
{
    void (*SaveBackground)(void);       // Display: Save background
    void (*RestoreBackground)(void);    // Display: Restore background

} TVIDEO;

extern TVIDEO video;

/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

extern DWORD DebInterruptHandler( DWORD nInt, TRegs *pRegs );

extern void HookDebuger(void);
extern void UnhookDebuger(void);

extern BOOL CheckHotKey(void);
extern void VgaInit(TVIDEO *video);


#endif //  _ICE_H_
