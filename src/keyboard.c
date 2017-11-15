/******************************************************************************
*
*   Module:     keyboard.c
*
*   Revision:   1.00
*
*   Date:       08/03/96
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


#define SC_CONTROL          29         // Control key key scan code
#define SC_ALT              56         // Alt key scan code
#define SC_LEFT_SHIFT       42         // Left shift key scan code
#define SC_RIGHT_SHIFT      54         // Right shift key scan code
#define SC_CAPS_LOCK        58         // Caps lock key scan code

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

#define NEXT_KQUEUE(i) (((i)+1 >= MAX_INPUT)? 0 : (i)+1)

static volatile WORD kQueue[ MAX_INPUT ];   // Keyboard circular queue
static volatile int head = 0, tail = 0;     // Head and Tail of that queue


static const BYTE ascii_table[2][128] = {
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
*   BOOL CheckHotKey(void)                                                    *
*                                                                             *
*******************************************************************************
*
*   This handler is used when we snoop on the keyboard
*   -------------------------------------------------------
*   THIS DOES NOT WORK (YET?)
*
*   This handler gets called every time a keyboard interrupt is issued.
*   We need to check if the Ice control keys are pressed and interrupt.
*
*   Returns:
*       TRUE if a hot key sequence is completed
*       FALSE otherwise
*
******************************************************************************/
BOOL CheckHotKey(void)
{
    BYTE ScanCode;

    ScanCode = inp( KBD_DATA );

//    printk("<1>  ->%02X<-  \n", ScanCode);

    // Tell keyboard to resend the scan code

    // THIS DOESN'T WORK...

    outp( KBD_DATA, 0xFE );

    while( (inp(KBD_STATUS) & 1)==0 );

    return( FALSE );
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
    WORD AsciiCode, bNext;
    BYTE ScanCode;
    BYTE Code, Pressed;


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
            AsciiCode = ascii_table[1][ Code ];
        else
            AsciiCode = ascii_table[0][ Code ];

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
        // the input keyboard queue if it is not full

        bNext = NEXT_KQUEUE( tail );

        if( bNext != head )
        {
            kQueue[ tail ] = AsciiCode;
            tail = bNext;
        }
    }

    // Acknowledge the keyboard controller

    ScanCode = inp( KBD_CONTROL );
    outp( ScanCode | 0x80, KBD_CONTROL );
    inp( PORT_DUMMY );
    outp( ScanCode, KBD_CONTROL );
    inp( PORT_DUMMY );
}


/******************************************************************************
*                                                                             *
*   WORD GetKey( IN BOOL fBlock )                                             *
*                                                                             *
*******************************************************************************
*
*   This function returns a key from the keyboard buffer.
*   If a key is not available and the fBlock argument is True, it polls until
*   a key becomes available.  Otherwise, it returns 0 (False).
*
*   Where:
*       fBlock is a blocking request.  If set to True, the function polls
*       the keyboard until a key is available.
*
*   Returns:
*       0 If no key was available
*       Pseudo-ASCII code of a next key in a queue
*
******************************************************************************/
WORD GetKey( BOOL fBlock )
{
    WORD c;

    // If the blocking is False, return 0 if a key is not available

    if( fBlock==FALSE && head==tail )
        return( 0 );

    // Poll for the input character

    while( head == tail ) {;}

    // Get a character from the keyboard queue - make it uninterruptible

    DisableInterrupts();

    c = kQueue[ head ];

    head = NEXT_KQUEUE( head );

    EnableInterrupts();

    return( c );
}

