/******************************************************************************
*                                                                             *
*   Module:     interrupt.c                                                   *
*                                                                             *
*   Date:       10/28/00                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains debugger interrupt handlers

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/28/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// TODO: Are we safe to assume Linux IDT has 256 entries ?

// Array where we store the original Linux IDT entries

static TIDT_Gate origIdt[256];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void DecodeIDT(TIDT_Gate *pIdt)
{
    char *pType;

    switch( pIdt->type )
    {
        case INT_TYPE_TASK:  pType = "Task Gate";       break;
        case INT_TYPE_INT32: pType = "Interrupt Gate";  break;
        case INT_TYPE_TRAP32:pType = "Trap Gate";       break;
        default:             pType = "UNKNOWN";
    }

    printk("<1> IDT %04X : %04X%04X  dpl: %d p:%d  %s\n", 
        pIdt->selector, pIdt->offsetHigh, pIdt->offsetLow, pIdt->dpl, pIdt->present, pType);
}    


/******************************************************************************
*                                                                             *
*   void Halt(void)                                                           *
*                                                                             *
*******************************************************************************
*
*   Unrecoverable fault within LinIce - dump some state information and halt
*
******************************************************************************/
void Halt(void)
{
    HaltCpu();
}    


/******************************************************************************
*                                                                             *
*   void HookIdt(BYTE bIntNumber)                                             *
*                                                                             *
*******************************************************************************
*
*   Hooks a single IDT entry
*
*   Where:
*       bIntNumber is the interrupt number to hook 
*
******************************************************************************/
void HookIdt(DWORD bIntNumber)
{
    TIDT_Gate *pGate;
    DWORD IntHandlerFunction;

    pGate = deb.pIdt + bIntNumber;
    IntHandlerFunction = IceIntHandlers[bIntNumber];

    pGate->offsetLow  = LOWORD(IntHandlerFunction);
    pGate->offsetHigh = HIWORD(IntHandlerFunction);
    pGate->selector   = SEL_ICE_CS;
    pGate->type       = INT_TYPE_INT32;
    pGate->dpl        = 0;
    pGate->present    = TRUE;
}    


/******************************************************************************
*                                                                             *
*   void HookDebuger(void)                                                    *
*                                                                             *
*******************************************************************************
*
*   Hooks all IDT entries for monitoring debugee
*
******************************************************************************/
void HookDebuger(void)
{
    // We hook selcted CPU interrupts and traps

    if( deb.fInt1Here==TRUE ) HookIdt(1);
    if( deb.fInt3Here==TRUE ) HookIdt(3);

//    HookIdt(0);
//    HookIdt(6);
//    HookIdt(8);
//    HookIdt(10);
//    HookIdt(13);
}    


/******************************************************************************
*                                                                             *
*   void HookIce(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Hooks all IDT entries needed for running debugger
*
******************************************************************************/
void HookIce(void)
{
    // We hook selcted CPU interrupts and traps

    HookIdt(0x01);            // Int1
    HookIdt(0x03);            // Int3
    HookIdt(0x06);            // Invalid opcode
    HookIdt(0x08);            // Double fault
    HookIdt(0x0A);            // Invalid TSS
    HookIdt(0x0B);            // Segment not present
    HookIdt(0x0C);            // Stack exception
    HookIdt(0x0D);            // GPF
    HookIdt(0x0E);            // Page Fault

    HookIdt(0x20);            // System timer
    HookIdt(0x21);            // Keyboard
    HookIdt(0x2C);            // PS/2 Mouse
}    


/******************************************************************************
*                                                                             *
*   void SaveIDT(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Saves original Linux IDT table before we hook anything
*
******************************************************************************/
void SaveIDT(void)
{
    // Store the original IDT table aside

    memcpy((void *)origIdt, deb.pIdt, sizeof(origIdt));
}    


/******************************************************************************
*                                                                             *
*   void RestoreIDT(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Restores original Linux IDT table
*
******************************************************************************/
void RestoreIDT(void)
{
    // Simply copy back all IDT entries that we have saved before

    memcpy(deb.pIdt, (void *)origIdt, sizeof(origIdt));
}    


/******************************************************************************
*                                                                             *
*   DWORD DebInterruptHandler( DWORD nInt, TRegs *pRegs )                     *
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
*       NULL
*
******************************************************************************/
DWORD DebInterruptHandler( DWORD nInt, TRegs *pRegs )
{
    // Depending on the execution context, branch

    if( deb.fRunningIce == TRUE )
    {
        //---------------------------------------------
        //  Exception occurred during the LinIce run
        //---------------------------------------------

        // Handle some limited number of interrupts, ignore the rest

        switch( nInt )
        {
            case 0x01:
                    dprint("LINICE: INT 1\n");
                    Halt();
                break;

            case 0x03:
                    dprint("LINICE: INT 3\n");
                    Halt();
                break;

            case 0x06:
                    dprint("LINICE: INVALID OPCODE\n");
                    Halt();
                break;

            case 0x08:
                    dprint("LINICE: DOUBLE FAULT\n");
                    Halt();
                break;

            case 0x0A:
                    dprint("LINICE: INVALID TSS\n");
                    Halt();
                break;

            case 0x0B:
                    dprint("LINICE: SEGMENT NP\n");
                    Halt();
                break;

            case 0x0C:
                    dprint("LINICE: STACK OVERFLOW\n");
                    Halt();
                break;

            case 0x0D:
                    dprint("LINICE: GPF\n");
                    Halt();
                break;

            case 0x0E:  // PAGE FAULT
                if( (pRegs->eip > (DWORD) GetByte) && (pRegs->eip < (DWORD) GetByte + 50) )
                {
                    pRegs->eip += 2;            // Skip  8A 03  mov al, gs:[ebx]
                    pRegs->eax  = 0xFFFFFFFF;   // Set invalid address value
                }
                else
                {
                    dprint("LINICE: PAGE FAULT\n");                    
                    Halt();
                }
                break;

            case 0x20:      // Timer
                break;

            case 0x21:      // Keyboard interrupt
                Deb_Keyboard_Handler();
            break;

            case 0x2C:
                break;

            default:   
                    dprint("LINICE: INVALID INTERRUPT %02X\n", nInt);
                    Halt();
                break;
        }

//        if( nInt != 0x20 )
//            dprint(">%02X<", nInt);

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

        deb.fRunningIce = TRUE;
                
        // Store the address of the registers into the debugee state structure
        // and the current interrupt number

        deb.r = pRegs;
        deb.nInterrupt = nInt;

        // Disassemble from the address of break

        deb.codeOffset = deb.r->eip;

        // Get the current GDT table

        GetGDT(&deb.gdt);
        deb.pGdt = (TGDT_Gate *) GET_DESC_BASE(&deb.gdt);
        
        // Restore original Linux interrupt table

        RestoreIDT();

        // Hook interrupts that we need to control to run debugger

        HookIce();

        // Be sure to enable interrupts so we can operate

        EnableInterrupts();

        ////////////////////////////// TA-DAAAA !!!
                EnterDebugger();
        //////////////////////////////

        // Disable interrupts so we can mess with IDT

        DisableInterrupts();

        // Restore original Linux interrupt table

        RestoreIDT();

        // Hook back the debugee IDT

        HookDebuger();

        deb.fRunningIce = FALSE;
    }

    // Return into the debugee where we left off or where the register structure
    // points to...

    return( NULL );
}    

