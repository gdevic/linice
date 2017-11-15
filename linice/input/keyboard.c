/******************************************************************************
*                                                                             *
*   Module:     keyboard.c                                                    *
*                                                                             *
*   Date:       08/03/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2005 Goran Devic                                       *
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

          This module contains the low-level keyboard handler code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/03/96   Original                                             Goran Devic *
* 04/26/00   Modified for Linice                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include hardware defines
#include "intel.h"                      // Include processor specific stuff
#include "debug.h"                      // Include our dprintk()


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

static BYTE code_table[3][128] =
{
    {// Normal keys
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
    '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',       '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?'
    },
    {// Shift + key
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
    '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',       '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?'
    },
    {// Alt + key
    0,    0,    0,    '@',  0,    '$',  0,    0,         '{',  '[',  ']',  '}',  '\\', 0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    '~',  ENTER,0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,         0,    0,    0,    0,    0,    0,    0,    0
    }
};


static BOOL fShift = FALSE;
static BOOL fAlt = FALSE;
static BOOL fControl = FALSE;
static BOOL fCapsLock = FALSE;

// Keyboard hook variables

typedef void (*TLinuxHandleScancode)(unsigned int, int, int, struct pt_regs *);
static TLinuxHandleScancode LinuxHandleScancode;
static DWORD *pKbdHook = NULL;              // Original kbd hook address
static DWORD KbdHook;                       // Original kbd hook value

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void MouseHandler(PTMPACKET pPacket);

/******************************************************************************
*                                                                             *
*   void InitKeyboardLayout(char Layout[2][128])                              *
*                                                                             *
*******************************************************************************
*
*   Initializes update to default (US) keyboard layout. If we are using
*   some other layout, we need to update some keys.
*
******************************************************************************/
void InitKeyboardLayout(char Layout[2][128])
{
    int i, j;

    // Update only values that were passed on as non-zero
    for(i=0; i<3; i++)
    {
        for(j=0; j<128; j++)
        {
            if( Layout[i][j]!=0 )
                code_table[i][j] = Layout[i][j];
        }
    }
}

/******************************************************************************
*
*   Reads a byte from the AUX port, used for PS/2 mouse.
*
******************************************************************************/
BYTE GetAux()
{
// TODO: The complete mouse interface has to be designed.
//       In a vmware box, the timeout happens every time mouse is moved, seems like
//       the status is not being virtualized well
//       Check the real hw mouse...
#if 0
    int timeout = 100;

    while( ((inp(KBD_STATUS) & STATUS_AUXB)==0 )  && timeout) timeout--;

    if( timeout==0 )
        dprint("PS2 TIMEOUT\n");
#endif
    return( inp(KBD_DATA) );
}


/******************************************************************************
*                                                                             *
*   void KeyboardHandler(void)                                                *
*                                                                             *
*******************************************************************************
*
*   This handler is used when the debugger has control.
*
*   This is a low-level keyboard handler.  It translates hardware key codes
*   into ASCII and stores them in a circular keyboard buffer for the use by
*   the debugger.
*
*   A pseudo-ASCII code 16 bit wide is used where the top byte contains
*   the state of the SHIFT/CONTROL/ALT key.
*
******************************************************************************/
void KeyboardHandler(void)
{
    WCHAR AsciiCode;
    BYTE ScanCode;
    BYTE Code, fPressed;
    TMPACKET mPacket;
    BYTE packet[3];

    // Check the status of the controller; do nothing for now, but soon
    // we'll have to handle PS/2 mouse here
    Code = inp( KBD_STATUS );

    // If a PS/2 mouse was moved, eat the codes
    if( Code & STATUS_AUXB )
    {
        // PS2 Mouse

        packet[0] = GetAux();
        packet[1] = GetAux();
        packet[2] = GetAux();

        //dprint("%02X %02X %02X \n", packet[0], packet[1], packet[2]);

        // Map the PS2 mouse packets into internal mouse packet structure
        //
        //  D7   D6   D5   D4   D3   D2   D1   D0
        //  YV   XV   YS   XS   1    CB   RB   LB
        //  < X displacement                    >
        //  < Y displacement                    >

        mPacket.buttons =
            ((packet[0] & 1) << 2) |        // LB
            ((packet[0] & 2) >> 1) |        // RB
            ((packet[0] & 4) >> 1);         // CB

        if( packet[0] & 0x20 )
            mPacket.Yd = -(256 - packet[2]);
        else
            mPacket.Yd = packet[2];

        if( packet[0] & 0x10 )
            mPacket.Xd = -(256 - packet[1]);
        else
            mPacket.Xd = packet[1];

        // Call the common mouse handler

        MouseHandler(&mPacket);
    }
    else
    {
        // Read the keyboard scan code

        ScanCode = inp( KBD_DATA );
        AsciiCode = 0;

        // On a key press, bit 7 of the scan code is 0.  When a key is being
        // released, bit 7 is 1.

        Code = ScanCode & 0x7F;
        fPressed = (ScanCode >> 7) ^ 1;

        switch( Code )
        {
            case SC_LEFT_SHIFT:         // Check for shift keys: left and right
            case SC_RIGHT_SHIFT:
                fShift = fPressed;
                break;

            case SC_CONTROL:            // Check for CTRL key
                fControl = fPressed;
                break;

            case SC_ALT:                // Check for ALT keys
                fAlt = fPressed;
                break;

            case SC_CAPS_LOCK:          // Check for caps lock key
                fCapsLock ^= fPressed;  // Toggle caps lock state on press
                break;

            default:

            // Now return if key was released.  Nothing to do.
            // Store the code in a queue if a key was pressed.

            if( fPressed )
            {
                // Map a scancode to an ASCII code - select from normal, shift or alt
                // key combinations maps
                AsciiCode = code_table[fShift][ Code ];

                if( fAlt )
                {
                    // Alternate map (Alt+key) should contain some extra symbols
                    // but mostly 0, in which case we only add a flag CHAR_ALT to a
                    // regular character mapped there
                    if( code_table[2][ Code ] )
                        AsciiCode = code_table[2][ Code ];
                    else
                        AsciiCode |= CHAR_ALT;
                }

                // Caps Lock key inverts the caps state of the alphabetical characters
                if( isalpha(AsciiCode) && fCapsLock )
                    AsciiCode ^= 0x20;

                // Set the shift flag only on function keys, ignore otherwise
                if( AsciiCode>=F1 && AsciiCode<=F12 )
                    if( fShift )
                        AsciiCode |= CHAR_SHIFT;

                if( fControl )
                    AsciiCode |= CHAR_CTRL;
            }
        } // switch

        if( AsciiCode != 0 )
        {
            // If a key was pressed (as opposed of released), queue it in
            // the input keyboard queue

            PutKey(AsciiCode);
        }

        // Acknowledge the keyboard controller

        ScanCode = inp( KBD_CONTROL );
        outp( ScanCode | 0x80, KBD_CONTROL );
        inp( PORT_DUMMY );
        outp( ScanCode, KBD_CONTROL );
        inp( PORT_DUMMY );
    }
}


/******************************************************************************
*                                                                             *
*   void LiniceHandleScancode(BYTE scancode, BOOL fPressed)                   *
*                                                                             *
*******************************************************************************
*
*   Hook function for the standard Linux handle_scancode() which it chains.
*
*   Note: The last 2 parameters are added for kernels 2.6.9 and above.
*
******************************************************************************/
void LiniceHandleScancode(unsigned int scancode, int fPressed, int hw_raw, struct pt_regs *regs)
{
    static WCHAR Key = 0;
    BYTE code = scancode & 0x7F;

    // Keep track of the CTRL and ALT keys since the break combination
    // have to include one of them

    // Insert the current ASCII code
    Key = (Key & ~0xFF) | code_table[1][code];

    // Modify our copy of shift state
    if( code==SC_CONTROL )
        Key = fPressed? Key | CHAR_CTRL : Key & ~CHAR_CTRL;

    if( code==SC_ALT )
        Key = fPressed? Key | CHAR_ALT : Key & ~CHAR_ALT;

    if( fPressed && deb.BreakKey==Key )
    {
        // Break into the linice...
        // Since we will take control over the keyboard, need to ack it
        // and put host into a known state

        // Depress control or alt key
        if( Key & CHAR_CTRL )
            (LinuxHandleScancode)(SC_CONTROL, !fPressed, hw_raw, regs);
        else
            (LinuxHandleScancode)(SC_ALT, !fPressed, hw_raw, regs);

        Key &= ~(CHAR_CTRL | CHAR_ALT);

        // Ok let's try something different...

        // Since we are pressing a key, that can not be time sensitive in the
        // terms of CPU time, so we can afford to simply "schedule" a break
        // into the debugger on another PIC interrupt (that will happen within
        // 10 ms on Linux anyways).

        deb.nScheduleKbdBreakTimeout = 2;
    }
    else
    {
        // Chain the call to the original handle_scancode
        (LinuxHandleScancode)(scancode, fPressed, hw_raw, regs);
    }
}


/******************************************************************************
*                                                                             *
*   BOOL KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode)          *
*                                                                             *
*******************************************************************************
*
*   This function hooks the Linux kernel keyboard handler
*
*   Where:
*       handle_kbd_event is the address of that Linux function
*       handle_scancode is the address of that Linux function
*
*   Returns:
*       TRUE - keyboard hooked
*       FALSE - could not locate function call to patch - abort load
*
******************************************************************************/
BOOL KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode)
{
    BYTE pattern[5] = { 0xE8, 0, 0, 0, 0 };
    DWORD *pOffset = (DWORD *)&pattern[1];
    int nSearchLen = 10000;               // Located within so many bytes

    INFO("KeyboardHook()\n");

    // Search starting from address handle_kbd_event for the call to a function handle_scancode
    do  // We increment handle_kbd_event as we go...
    {
        handle_kbd_event++;

        // Calculate the possible offset that a CALL instruction would have if
        // located at this address
        *pOffset = handle_scancode - (handle_kbd_event + 5);

    } while( memcmp((void *)handle_kbd_event, pattern, 5)!=0 && --nSearchLen );

    if( nSearchLen )
    {
        // Found the function call to hook

        INFO("Call to handle_scancode at %08X\n", (int)handle_kbd_event);
        pOffset = (DWORD *)(handle_kbd_event + 1);

        // Store the original offset and insert our handler in place of the call
        pKbdHook = pOffset;       // Address to hook
        KbdHook = *pOffset;       // Original value that was there

        *pOffset = (DWORD) LiniceHandleScancode - ((DWORD) pOffset + 4);
        LinuxHandleScancode = (TLinuxHandleScancode) handle_scancode;

        return( TRUE );
    }

    // Did not find the call ?!

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   void KeyboardUnhook()                                                     *
*                                                                             *
*******************************************************************************
*
*   Unhooks the debugger keyboard hook.
*
******************************************************************************/
void KeyboardUnhook()
{
    INFO("KeyboardUnhook()\n");

    // Restore original call to handle_scancode within the handle_kbd_event function
    if( pKbdHook )
        *(pKbdHook) = KbdHook;
}

