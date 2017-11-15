/******************************************************************************
*
*   Module:     keyboard.c
*
*   Date:       08/03/96
*
*   Copyright (c) 1996-2000 Goran Devic                                    
*                                                            
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This module contains the low-level keyboard handler code.

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/03/96   1.00  Original                                        Goran Devic
* 10/26/00         Modified for LinIce                             Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

/******************************************************************************
*   Global Variables
******************************************************************************/

#define CHAR_CTRL		0x8000			// <key> + CTRL
#define CHAR_ALT        0x4000			// <key> + ALT
#define CHAR_SHIFT		0x2000			// <key> + SHIFT


#define SC_CONTROL          29         // Control key key scan code
#define SC_ALT              56         // Alt key scan code
#define SC_LEFT_SHIFT       42         // Left shift key scan code
#define SC_RIGHT_SHIFT      54         // Right shift key scan code
#define SC_CAPS_LOCK        58         // Caps lock key scan code

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

#define LAYOUT_US			0			// US keyboard layout
#define LAYOUT_GERMAN		1			// German keyboard layout

static int layout = LAYOUT_US;

static const BYTE code_table[MAX_LAYOUT][2][128] = {
{    //                                         LAYOUT_US
	{// Normal keys
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
	},
	{// Shift + key
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
	}
},
{    //                                       LAYOUT_GERMAN
	{// Normal keys
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
	},
	{// Shift + key
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
	}
}
};


static BOOL fShift = FALSE;
static BOOL fControl = FALSE;
static BOOL fAlt = FALSE;
static BOOL fCapsLock = FALSE;


/******************************************************************************
*   Functions
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void SetKbdLayout(unsigned layout)                                        *
*                                                                             *
*******************************************************************************
*
*	Sets default keyboard scan-code layout.
*
******************************************************************************/
void SetKbdLayout(unsigned NewLayout)
{
	if(NewLayout>=LAYOUT_US && NewLayout<=LAYOUT_GERMAN)
	{
		layout = NewLayout;
	}
}


/******************************************************************************
*                                                                             *
*   void Deb_Key_Handler(void)                                                *
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
void Deb_Keyboard_Handler(void)
{
    CHAR AsciiCode, bNext;
    BYTE ScanCode;
    BYTE Code, Pressed;

    // Check the status of the controller; do nothing for now, but soon
    // we'll have to handle PS/2 mouse here
    Code = inp( KBD_STATUS );

    AsciiCode = 0;
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
            AsciiCode = code_table[layout][1][ Code ];
        else
            AsciiCode = code_table[layout][0][ Code ];

        // Caps Lock key inverts the caps state of the alphabetical characters
        if( isalpha(AsciiCode) && fCapsLock )
            AsciiCode ^= 0x20;

        // Ctrl, Alt, shift keys form new codes for funstion keys
        if( AsciiCode >= F1 && AsciiCode <= F12 )
        {
            if( fControl )
                AsciiCode += 12;
            else
            if( fAlt )
                AsciiCode += 2 * 12;
            else
            if( fShift )
                AsciiCode += 3 * 12;
        }
        else
        {
            // Shift, Ctrl and Alt keys add extra bits to a code
            if( fShift && !isascii(AsciiCode) )
                AsciiCode |= SHIFT;

            if( fControl )
                AsciiCode |= CTRL;

            if( fAlt )
                AsciiCode |= ALT;
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


