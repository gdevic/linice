/******************************************************************************
*                                                                             *
*   Module:     i386.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/25/2000                                                    *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This header file contains global exports from the i386.asm module.


*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 10/25/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _I386_H_
#define _I386_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------
// PIC1 - Programmable Interrupt Controller defines (8259)
//-----------------------------------------------------------------------------

#define PIC1                0x20
#define PIC2                0xA0 
#define PIC_ACK             0x20

//-----------------------------------------------------------------------------
// PIC2 - Programmable Interrupt Controller defines (8259)
//-----------------------------------------------------------------------------

#define PIC2                0xA0
#define PIC2_MASK           0xA1

//-----------------------------------------------------------------------------
// Keyboard Controller (8041)
//-----------------------------------------------------------------------------

#define KBD_DATA            0x60
#define KBD_CONTROL         0x64
#define KBD_STATUS          0x64

//-----------------------------------------------------------------------------
// Real Time Clock command port
//-----------------------------------------------------------------------------

#define RTC_CMD             0x70
#define DISABLE_NMI         0x80
#define ENABLE_NMI          0x00

//-----------------------------------------------------------------------------
// Port that causes slight delay without side effects
//-----------------------------------------------------------------------------

#define PORT_DUMMY          0x80


// Array of interrupt handlers for running debugger

extern DWORD IceIntHandlers[];


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

extern void GetIDT(TDescriptor *p);
extern void SetIDT(TDescriptor *p);
extern void IceKeyboard(void);
extern void DebKeyboard(void);
extern void IceInterrupt(DWORD nInt);
                       
extern BYTE inp(int port);
extern void outp(int port, int value);
extern void IssueInt3();

extern void DisableInterrupts();
extern void EnableInterrupts();

extern void Interrupt_0(void);
extern void Interrupt_1(void);
extern void Interrupt_3(void);
extern void Interrupt_6(void);
extern void Interrupt_8(void);
extern void Interrupt_10(void);
extern void Interrupt_13(void);
extern void Interrupt_33(void);


#endif //  _I386_H_
