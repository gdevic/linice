/******************************************************************************
*                                                                             *
*   Module:     ibm-pc.h                                                      *
*                                                                             *
*   Date:       09/11/00                                                      *
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

        This is a header file containing generic IBM PC hardware defines

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _IBM_PC_H_
#define _IBM_PC_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Defines of the generic IBM hardware
///////////////////////////////////////////////////////////////////////////////

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
#define STATUS_INPB         0x02
#define STATUS_AUXB         0x20
#define KBD_AUXD            0x02

//-----------------------------------------------------------------------------
// Keyboard Scancodes
//-----------------------------------------------------------------------------

#define SC_CONTROL          29          // Control key key scan code
#define SC_ALT              56          // Alt key scan code
#define SC_LEFT_SHIFT       42          // Left shift key scan code
#define SC_RIGHT_SHIFT      54          // Right shift key scan code
#define SC_CAPS_LOCK        58          // Caps lock key scan code

//-----------------------------------------------------------------------------
// Serial 8250/16450/16550
//-----------------------------------------------------------------------------

#define COM1_PORT           0x3F8
#define COM2_PORT           0x2F8
#define COM3_PORT           0x3E8
#define COM4_PORT           0x2E8

#define SER_DATA            0           //
#define SER_INTE            1           //
#define SER_INTID           2           //
#define SER_DF              3           // Data format register
#define SER_MODEM           4           // Modem control register
#define SER_LINE            5           // Line status register
#define SER_MODEMSTATUS     6           // Modem status register

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


#endif //  _IBM_PC_H_

