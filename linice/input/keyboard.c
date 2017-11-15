/******************************************************************************
*                                                                             *
*   Module:     keyboard.c                                                    *
*                                                                             *
*   Date:       08/03/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2000 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 10/26/00   Modified for LinIce                                  Goran Devic *
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

static const BYTE code_table[MAX_LAYOUT][2][128] = {
{    //                                         LAYOUT_US
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
    }
},
{    //                                       LAYOUT_GERMAN
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
    }
}
};


static BOOL fShift = FALSE;
static BOOL fAlt = FALSE;
static BOOL fControl = FALSE;
static BOOL fCapsLock = FALSE;

// Keyboard hook variables

typedef void (*TLinuxHandleScancode)(unsigned char, unsigned int);
static TLinuxHandleScancode LinuxHandleScancode;
static DWORD *pKbdHook;                     // Original kbd hook address
static DWORD KbdHook;                       // Original kbd hook value

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void MouseHandler(PTMPACKET pPacket);


/******************************************************************************
*
*   Reads a byte from the AUX port, used for PS/2 mouse.
*
******************************************************************************/
BYTE GetAux()
{
    int timeout = 1000;

    while( ((inp(KBD_STATUS) & STATUS_AUXB)==0 )  && timeout) timeout--;

    if( timeout==0 )
        dprint("PS2 TIMEOUT\n");

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
    CHAR AsciiCode;
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
                // Map a scancode to an ASCII code
                AsciiCode = code_table[pIce->layout][fShift][ Code ];

                // Treat function codes as special case
                if( (AsciiCode>=F1) && (AsciiCode<=F12) )
                {
                    if( fShift )
                        AsciiCode += 1 * 12;
                    else
                    if( fAlt )
                        AsciiCode += 2 * 12;
                    else
                    if( fControl )
                        AsciiCode += 3 * 12;
                }
                else
                {
                    // Caps Lock key inverts the caps state of the alphabetical characters
                    if( isalpha(AsciiCode) && fCapsLock )
                        AsciiCode ^= 0x20;

                    if( fAlt )
                        AsciiCode |= CHAR_ALT;

                    if( fControl )
                        AsciiCode |= CHAR_CTRL;
                }
            }
        } // switch

        if( AsciiCode != 0 )
        {
            // If a key was pressed (as opposed of released), queue it in
            // the input keyboard queue

            PutKey(AsciiCode);
            //dprint("%c%c%c%02X ", DP_SETCURSORXY, 1, 1, AsciiCode);
        }

        // Acknowledge the keyboard controller

        ScanCode = inp( KBD_CONTROL );
        outp( ScanCode | 0x80, KBD_CONTROL );
        inp( PORT_DUMMY );
        outp( ScanCode, KBD_CONTROL );
        inp( PORT_DUMMY );
    }
}


void LiniceHandleScancode(BYTE scancode, BOOL fPressed)
{
    static CHAR Key = 0;

    // Keep track of the CTRL and ALT keys since the break combination
    // have to include one of them

    // Insert the current ASCII code
    Key = (Key & ~0xFF) | code_table[pIce->layout][0][scancode];

    // Add the shift state
    if( scancode==SC_CONTROL )
        Key = fPressed? Key | CHAR_CTRL : Key & ~CHAR_CTRL;

    if( scancode==SC_ALT )
        Key = fPressed? Key | CHAR_ALT : Key & ~CHAR_ALT;

    if( fPressed && deb.BreakKey==Key )
    {
        // Break into the linice...
        // Since we will take control over the keyboard, need to ack it
        // and put host into some known state

        // Depress control or alt key
        if( Key & SC_CONTROL )
            (LinuxHandleScancode)(SC_CONTROL, !fPressed);
        else
            (LinuxHandleScancode)(SC_ALT, !fPressed);

        // We need to rebuild the stack frame... Back up what the Linux keyboard
        // handler did and form the frame of our own at that location...

//        INFO(("BREAK!!\n"));

        return;
    }

    // Chain the call to the original handle_scancode
    (LinuxHandleScancode)(scancode, fPressed);
}


void KeyboardHook(DWORD handle_kbd_event, DWORD handle_scancode)
{
    BYTE pattern[5] = { 0xE8, 0, 0, 0, 0 };
    DWORD *pOffset = (DWORD *)&pattern[1];
    int nSearchLen = 1000;               // Located within so many bytes

    // Search starting from address handle_kbd_event for the call to a function handle_scancode
    do  // We increment handle_kbd_event as we go...
    {
        handle_kbd_event++;

        // Calculate the possible offset that a CALL instruction would have if
        // located at this address
        *pOffset = handle_scancode - (handle_kbd_event + 5);

    } while( memcmp((void *)handle_kbd_event, pattern, 5)!=0 && --nSearchLen );

    if( nSearchLen==0 )
    {
        // Did not find the call ?!
        INFO(("Did not find a call to handle_scancode!\n"));
    }
    else
    {
        INFO(("Call to handle_scancode at %08X\n", (int)handle_kbd_event));
        pOffset = (DWORD *)(handle_kbd_event + 1);

        // Store the original offset and insert our handler in place of the call
        pKbdHook = pOffset;       // Address to hook
        KbdHook = *pOffset;       // Original value that was there

        *pOffset = (DWORD) LiniceHandleScancode - ((DWORD) pOffset + 4);
        LinuxHandleScancode = (TLinuxHandleScancode) handle_scancode;
    }
}


void KeyboardUnhook()
{
    // Restore original call to handle_scancode within the handle_kbd_event function

    *(pKbdHook) = KbdHook;
}

