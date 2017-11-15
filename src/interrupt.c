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

        This module contains interrupt handlers

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

#define MAX_SAVE_IDT        34          // Number of IDT entries to handle

// Array where we store the original IDT entries

TIDT_Gate origIdt[MAX_SAVE_IDT];

// Private IDT that debugger uses

#define MAX_ICE_IDT         0x30

TIDT_Gate ice_idt[MAX_ICE_IDT];

// And the descriptor for it

TDescriptor ice_idt_descriptor;

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
*   void HookIdt(TIDT_Gate *pGate, DWORD IntHandlerFunction)                  *
*                                                                             *
*******************************************************************************
*
*   Hooks a single IDT entry
*
*   Where:
*       pIdt is the pointer to an IDT entry to hook
*
******************************************************************************/
void HookIdt(TIDT_Gate *pGate, DWORD IntHandlerFunction)
{
#ifdef DBG
    DecodeIDT(pGate);
#endif
    pGate->offsetLow  = LOWORD(IntHandlerFunction);
    pGate->offsetHigh = HIWORD(IntHandlerFunction);
    pGate->selector   = SEL_ICE_CS;
    pGate->type       = INT_TYPE_INT32;
    pGate->dpl        = 0;
    pGate->present    = TRUE;

    DecodeIDT(pGate);
}    


/******************************************************************************
*                                                                             *
*   void HookDebuger(void)                                                    *
*                                                                             *
*******************************************************************************
*
*   Hooks all IDT entries for monitoring debugee
*
*   Where:
*       pIdt is the pointer to the IDT[0]
*
******************************************************************************/
void HookDebuger(void)
{
    TIDT_Gate *pIdt;

    // Get the address of the debugee IDT

    pIdt = GET_DESC_BASE(&deb.idt);
#ifdef DBG
    printk("<1>IDT: %08X [%X]\n", GET_DESC_BASE(&deb.idt), deb.idt.limit);
#endif

    // Store the original IDT table aside

    memcpy((void *)origIdt, pIdt, sizeof(TIDT_Gate) * MAX_SAVE_IDT);

    // We hook selcted CPU interrupts and traps

    if( deb.fInt1Here==TRUE )
        HookIdt(pIdt+1,  (DWORD) Interrupt_1);

    if( deb.fInt3Here==TRUE )
        HookIdt(pIdt+3,  (DWORD) Interrupt_3);

    HookIdt(pIdt+0,  (DWORD) Interrupt_0);
    HookIdt(pIdt+6,  (DWORD) Interrupt_6);
    HookIdt(pIdt+8,  (DWORD) Interrupt_8);
    HookIdt(pIdt+10, (DWORD) Interrupt_10);
    HookIdt(pIdt+13, (DWORD) Interrupt_13);
}    


/******************************************************************************
*                                                                             *
*   void UnhookDebuger(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Unhooks all IDT entries for monitoring debugee
*
******************************************************************************/
void UnhookDebuger(void)
{
    TIDT_Gate *pIdt;

    // Get the address of the debugee IDT

    pIdt = GET_DESC_BASE(&deb.idt);

    // Simply copy back all IDT entries that we saved before

    memcpy(pIdt, (void *)origIdt, sizeof(TIDT_Gate) * MAX_SAVE_IDT);
}    


/******************************************************************************
*                                                                             *
*   void InitIceIdt(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Initializes private debugger IDT
*
******************************************************************************/
void InitIceIdt(void)
{
    int i;

    // Set up the debugger privte IDT

    SET_DESC_BASE(&ice_idt_descriptor, ice_idt);
    ice_idt_descriptor.limit = sizeof(ice_idt) - 1;

    for( i=0; i<MAX_ICE_IDT; i++)
    {
        ice_idt[i].offsetLow = LOWORD(IceIntHandlers[i]);
        ice_idt[i].offsetHigh = HIWORD(IceIntHandlers[i]);
        ice_idt[i].selector = SEL_ICE_CS;
        ice_idt[i].type = INT_TYPE_INT32;
        ice_idt[i].dpl = 0;
        ice_idt[i].present = TRUE;
    }
}    


/******************************************************************************
*                                                                             *
*   DWORD DebInterruptHandler( DWORD nInt, TRegs *pRegs )                     *
*                                                                             *
*******************************************************************************
*
*   This function is called on an exception/interrupt/trap originating in
*   the debugee that we hooked up.
*
*   Where:
*       nInt is the interrupt number that happen
*       pRegs is the debugee register structure
*
*   Returns:
*
******************************************************************************/
DWORD DebInterruptHandler( DWORD nInt, TRegs *pRegs )
{
DWORD pKeyMod;
    // Store the address of the registers into the debugee state structure
    // and the current interrupt number

    deb.r = pRegs;
    deb.nInterrupt = nInt;

    // Unhook all the interrupts since so they can be examined by the debugger

    UnhookDebuger();

    // Set up alternate IDT to run the debuger with

    SetIDT(&ice_idt_descriptor);

    // Be sure to enable interrupts so we can operate

    EnableInterrupts();

    video.SaveBackground();
  
    {
        TIDT_Gate *p;
        DWORD mod;
        volatile int i;
        char c = 0;
        
        printk("<1> %02X \n", nInt);

        p = GET_DESC_BASE(&ice_idt_descriptor);
        printk("<1> IDT: %08X:%04X\n", p, ice_idt_descriptor.limit);

        for( i=1; i<4; i++)
        {
            DecodeIDT(p++);
        }

        while( c != 27 )
        {
            c = GetKey( TRUE, &mod);
            printk("<1>%c\n", c);
        }
    }

    video.RestoreBackground();

    // Disable interrupts so we can mess with IDT

    DisableInterrupts();

    // Restore primary IDT so we can continue running the debugee

    SetIDT(&deb.idt);

    // Hook the debugee IDT

    HookDebuger();

    // Return into the debugee where we left off or where the register structure
    // points to...

    return( NULL );
}    


/******************************************************************************
*                                                                             *
*   void IceInterrupt( DWORD nInt )                                           *
*                                                                             *
*******************************************************************************
*
*   This function is called on an exception/interrupt/trap when the
*   debugger is running
*
*   Where:
*       nInt is the interrupt number that happen
*
******************************************************************************/
void IceInterrupt( DWORD nInt )
{
    // Handle some limited number of interrupts, ignore the rest

//    printk("<1> >> %02X \n", nInt);

    switch( nInt )
    {
        case 0x21:      // Keyboard interrupt
            Deb_Keyboard_Handler();
        break;
    }

    return( NULL );
}    

