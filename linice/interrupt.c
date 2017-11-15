/******************************************************************************
*                                                                             *
*   Module:     interrupt.c                                                   *
*                                                                             *
*   Date:       10/28/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains debugger interrupt handlers and functions.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/28/00   Original                                             Goran Devic *
* 03/11/01   Second edition                                       Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"
#include "debug.h"                      // Include our dprintk()
#include "intel.h"                      // Include processor specific stuff

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD IceIntHandlers[0x30];
extern DWORD IceIntHandler80;


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Array where we store the original Linux IDT entries at init time

TIDT_Gate LinuxIdt[256];

// Array that holds debugger private IDT table for the duration of its run

TIDT_Gate IceIdt[256];
TDescriptor IceIdtDescriptor;


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void LoadIDT(PTDescriptor pIdt);
extern void KeyboardHandler(void);
extern void SerialHandler(int port);
extern void MouseHandler(void);

extern void DebuggerEnter(void);

/******************************************************************************
*                                                                             *
*   void HookIdt(PTIDT_Gate pGate, int nIntNumber)                            *
*                                                                             *
*******************************************************************************
*
*   Hooks a single IDT entry
*
*   Where:
*       pGate is the pointer to an int entry to hook
*       nIntNumber is the interrupt number function to hook
*
******************************************************************************/
static void HookIdt(PTIDT_Gate pGate, int nIntNumber)
{
    DWORD IntHandlerFunction;

    IntHandlerFunction = IceIntHandlers[nIntNumber];

    pGate->offsetLow  = LOWORD(IntHandlerFunction);
    pGate->offsetHigh = HIWORD(IntHandlerFunction);
    pGate->selector   = __KERNEL_CS;
    pGate->type       = INT_TYPE_INT32;
    pGate->dpl        = 0;
    pGate->present    = TRUE;
}


/******************************************************************************
*                                                                             *
*   void InterruptInit(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Initializes interrupt tables. Called once from init.c
*
******************************************************************************/
void InterruptInit(void)
{
    GET_IDT(deb.idt);

    // Copy original Linux IDT to our local buffer

    memcpy(LinuxIdt, (void *)deb.idt.base, deb.idt.limit+1);

    // Copy the same IDT into our private IDT and initialize its descriptor

    memcpy(IceIdt, (void *)deb.idt.base, deb.idt.limit+1);
    IceIdtDescriptor.base = (DWORD) IceIdt;
    IceIdtDescriptor.limit = deb.idt.limit;

    // Hook private IDT with debuger-run hooks

    // These are BAD if they happen:
    HookIdt(&IceIdt[0x00], 0x0);        // Divide error
    HookIdt(&IceIdt[0x01], 0x1);        // INT 1
    HookIdt(&IceIdt[0x03], 0x3);        // INT 3
    HookIdt(&IceIdt[0x04], 0x4);        // Overflow error
    HookIdt(&IceIdt[0x06], 0x6);        // Invalid opcode
    HookIdt(&IceIdt[0x08], 0x8);        // Double-fault
    HookIdt(&IceIdt[0x0A], 0xA);        // Invalid TSS
    HookIdt(&IceIdt[0x0B], 0xB);        // Segment not present
    HookIdt(&IceIdt[0x0C], 0xC);        // Stack exception
    HookIdt(&IceIdt[0x0D], 0xD);        // GP fault

    // These we expect to happen:
    HookIdt(&IceIdt[0x0E], 0xE);        // Page fault
    HookIdt(&IceIdt[0x20], 0x20);       // System timer
    HookIdt(&IceIdt[0x21], 0x21);       // Keyboard
    HookIdt(&IceIdt[0x23], 0x23);       // COM2
    HookIdt(&IceIdt[0x24], 0x24);       // COM1
    HookIdt(&IceIdt[0x2C], 0x2C);       // PS/2 Mouse
}


/******************************************************************************
*                                                                             *
*   void HookDebuger(void)                                                    *
*                                                                             *
*******************************************************************************
*
*   Hooks all IDT entries of monitoring debugee.
*
******************************************************************************/
void HookDebuger(void)
{
    PTIDT_Gate pIdt = (PTIDT_Gate) deb.idt.base;

    // We hook selcted CPU interrupts and traps. This makes debugger active.

    HookIdt(&pIdt[0x01], 0x1);          // Int 1
    HookIdt(&pIdt[0x03], 0x3);          // Int 3
    HookIdt(&pIdt[0x08], 0x8);          // Double-fault
    HookIdt(&pIdt[0x0D], 0xD);          // GP fault
    HookIdt(&pIdt[0x0E], 0xE);          // Page fault
//    HookIdt(&pIdt[0x80], 0x80);         // System call interrupt
}


/******************************************************************************
*                                                                             *
*   void UnHookDebuger(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Unhooks all Linux IDT entries by reverting to the original saved values.
*
******************************************************************************/
void UnHookDebuger(void)
{
    // Copy back the original Linux IDT

    memcpy((void *)deb.idt.base, LinuxIdt, deb.idt.limit+1);
}


/******************************************************************************
*                                                                             *
*   void Panic(void)                                                          *
*                                                                             *
*******************************************************************************
*
*   This is a panic handler. It does the best to try to print some info,
*   and then hungs the system.
*
******************************************************************************/
void Panic(void)
{
    DWORD *p = (DWORD *)deb.r;
    int i;

    pOut->x = 0;
    pOut->y = 0;
    dprint("ICE-PANIC: Int: %d\r\n", deb.nInterrupt);
    dprint("EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESI=%08X EDI=%08X\r\n",
            deb.r->eax, deb.r->ebx, deb.r->ecx, deb.r->edx, deb.r->esi, deb.r->edi);
    dprint("CS:EIP=%04X:%08X  SS:ESP=%04X:%08X EBP=%08X\r\n",
            deb.r->cs, deb.r->eip, deb.r->ss, deb.r->esp, deb.r->ebp);
    dprint("DS=%04X ES=%04X FS=%04X GS=%04X\r\n",
            deb.r->ds, deb.r->es, deb.r->fs, deb.r->gs );

    for( i=0; i<13+8; i++)
    {
        dprint("%08X  \n", *p++);
    }

    cli();
    while( TRUE );
}


/******************************************************************************
*                                                                             *
*   DWORD InterruptHandler( DWORD nInt, TRegs *pRegs )                        *
*                                                                             *
*******************************************************************************
*
*   This function is called on an exception/interrupt/trap.  It may originate
*   either in the debugee or while running debugger.
*
*   Where:
*       nInt is the interrupt number that happened
*       pRegs is the register structure on the stack
*
*   Returns:
*       NULL to return to the original interrupted application/kernel
*       ADDRESS to chain the interrupt to that specified handler address
*
******************************************************************************/
DWORD InterruptHandler( DWORD nInt, PTREGS pRegs )
{
    DWORD chain = NULL;

    // Depending on the execution context, branch

    if(pIce->fRunningIce == TRUE)
    {
        //---------------------------------------------
        //  Exception occurred during the LinIce run
        //---------------------------------------------
        pIce->nIntsIce[nInt & 0x3F]++;

        // Handle some limited number of interrupts, puke on the rest

        switch( nInt )
        {
            case 0x20:      // Timer
                break;

            case 0x21:      // Keyboard interrupt
                KeyboardHandler();
            break;

            case 0x23:      // COM2
                SerialHandler(2);
                break;

            case 0x24:      // COM1
                SerialHandler(1);
                break;

            case 0x2C:      // PS/2 Mouse
                MouseHandler();
                break;

            case 0x0E:      // PAGE FAULT
                if( (pRegs->eip > (DWORD) GetByte) && (pRegs->eip < (DWORD) GetByte + 50) )
                {
                    pRegs->eip += 2;            // Skip  8A 03  mov al, gs:[ebx]
                    pRegs->eax  = 0xFFFFFFFF;   // Set invalid address value

                    break;
                }
            // .. from the PF we continue to the default panic handler...
            default:
                    deb.r = pRegs;
                    deb.nInterrupt = nInt;
                    Panic();
        }
        // Acknowledge interrupt controller

        if( (nInt >= 0x28) && (nInt < 0x30) )
            outp(PIC2, PIC_ACK);
        if( (nInt >= 0x20) && (nInt < 0x30) )
            outp(PIC1, PIC_ACK);
    }
    else
    {
        //---------------------------------------------
        //  Exception occurred during the debugee run
        //---------------------------------------------
        pIce->nIntsPass[nInt & 0x3F]++;

        // Store the address of the registers into the debugee state structure
        // and the current interrupt number

        deb.r = pRegs;
        deb.nInterrupt = nInt;

#if 0       // Pass-through
        chain = GET_IDT_BASE( &LinuxIdt[nInt] );
        return( chain );
#endif

        pIce->fRunningIce = TRUE;

        switch( nInt )
        {
            case 0x0E:                  // Page Fault
                    chain = GET_IDT_BASE( &LinuxIdt[0x0E] );
                break;

            default:
                // Get the current GDT table

                GET_GDT(deb.gdt);

                // Restore original Linux IDT table since we may want to examine it
                // using the debugger

                UnHookDebuger();

                // Switch to our private IDT that is already hooked

                SET_IDT(IceIdtDescriptor);

                // Be sure to enable interrupts so we can operate

                sti();

                DebuggerEnter();

                // Disable interrupts so we can mess with IDT

                cli();

                // Load debugee IDT

                SET_IDT(deb.idt);

                // Hook again the debugee IDT

                HookDebuger();
        }

        pIce->fRunningIce = FALSE;
    }

    return( chain );
}

