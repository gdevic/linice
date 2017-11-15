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

#define SC_CONTROL          29          // Control key key scan code
#define SC_ALT              56          // Alt key scan code
#define SC_LEFT_SHIFT       42          // Left shift key scan code
#define SC_RIGHT_SHIFT      54          // Right shift key scan code
#define SC_CAPS_LOCK        58          // Caps lock key scan code

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
    CHAR AsciiCode, bNext;
    BYTE ScanCode;
    BYTE Code, Pressed;
    TMPACKET mPacket;
    BYTE packet[3];

    // Check the status of the controller; do nothing for now, but soon
    // we'll have to handle PS/2 mouse here
    Code = inp( KBD_STATUS );

    AsciiCode = 0;

    // If a PS/2 mouse was moved, eat the codes
    if( Code & STATUS_AUXB )
    {
        // PS2 Mouse

        packet[0] = GetAux();
        packet[1] = GetAux();
        packet[2] = GetAux();

        // Map the PS2 mouse packets into internal mouse packet structure
        //
        //  D7   D6   D5   D4   D3   D2   D1   D0
        //  YV   XV   YS   XS   1    CB   RB   LB
        //  < Y displacement                    >
        //  < X displacement                    >

        mPacket.buttons =
            ((packet[0] & 1) << 2) |        // LB
            ((packet[0] & 2) >> 1) |        // RB
            ((packet[0] & 4) >> 1);         // CB

        if( packet[0] & 0x20 )
            mPacket.Yd = -(256 - packet[1]);
        else
            mPacket.Yd = packet[1];

        if( packet[0] & 0x10 )
            mPacket.Xd = -(256 - packet[2]);
        else
            mPacket.Xd = packet[2];

        // Call the common mouse handler

        MouseHandler(&mPacket);
    }
    else
    {
        // Read the keyboard scan code

        ScanCode = inp( KBD_DATA );

        // On a key press, bit 7 of the scan code is 0.  When a key is being
        // released, bit 7 is 1.

        Code = ScanCode & 0x7F;
        Pressed = (ScanCode >> 7) ^ 1;

        // Check for shift keys
        //
        if( (Code == SC_LEFT_SHIFT) || (Code == SC_RIGHT_SHIFT) )
        {
            // Determine if shift was pressed or depressed (bit 7):
            // fShift is 1 if shift has been pressed
            //           0 if shift has been released
            fShift = Pressed;
        }
        else

        // Check for control key
        //
        if( Code == SC_CONTROL )
        {
            // Determine if control was pressed or depressed (bit 7)
            // fControl is 1 if control key has been pressed
            //             0 if control key has been released
            fControl = Pressed;
        }
        else

        // Check for alt keys
        //
        if( Code == SC_ALT )
        {
            // Determine if alt was pressed or depressed (bit 7)
            // fAlt is 1 if alt key has been pressed
            //         0 if alt key has been released
            fAlt = Pressed;
        }
        else

        // Check for caps lock key
        //
        if( Code == SC_CAPS_LOCK )
        {
            // Toggle caps lock state on press
            fCapsLock ^= Pressed;
        }
        else

        // Now return if key was released.  Nothing to do.
        // Store the code in a queue if a key was pressed.
        //
        if( Pressed )
        {
            // Map a scancode to an ASCII code
            //
            if( fShift )
                AsciiCode = code_table[pIce->layout][1][ Code ];
            else
                AsciiCode = code_table[pIce->layout][0][ Code ];

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

                // Shift key also inverts the caps state of the alphabetical characters
                if( isalpha(AsciiCode) && fShift )
                    AsciiCode ^= 0x20;
                else
                if( fShift )
                    AsciiCode |= CHAR_SHIFT;

                if( fAlt )
                    AsciiCode |= CHAR_ALT;

                if( fControl )
                    AsciiCode |= CHAR_CTRL;
            }
        }

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

