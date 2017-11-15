/******************************************************************************
*                                                                             *
*   Module:     input.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10-26-2000                                                    *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains code for user input

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/26/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/


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
#define MAX_INPBUF      80              // Max input buffer size in characters

static inpbuf[MAX_INPBUF];              // Input edit line content
static int curpos;                      // Cursor position

static BOOL fInsert = TRUE;             // Keyboard insert mode

static const BYTE ScanCodeTable[4][128] = {
{
    '?',  ESC,  '1',  '2',  '3',  '4',  '5',  '6',       '7',  '8',  '9',  '0',  '-',  '=',  '\b',  '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',       'o',  'p',  '[',  ']',  ENTER,'?',  'a',   's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',       '\'', '`',  '?',  '\\', 'z',  'x',  'c',   'v',
    'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',       '?',  ' ',  '?',  F1,   F2,   F3,   F4,   F5,
    F6,   F7,   F8,   F9,   F10, NUMLOCK, SCROLL, HOME,  UP,  PGUP,  '?',  LEFT, '5', RIGHT, '?',   END,
    DOWN, PGDN, INS,  DEL,  '?',  '?',  '?',  F11,       F12,
},
{// Shifted keys
    '?',  ESC,  '!',  '@',  '#',  '$',  '%',  '^',       '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',       'O',  'P',  '{',  '}',  ENTER,'?',  'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',       '"',  '~',  '?',  '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  '?',  '*',       '?',  ' ',  '?',  SF1,  SF2,  SF3,  SF4,  SF5,
     SF6,  SF7,  SF8,  SF9,  SF10, NUMLOCK, SCROLL, HOME, UP,   PGUP,'?',  LEFT, '5',  RIGHT,'?',  END,
     DOWN, PGDN, INS,  DEL, '?',  '?',  '?',  SF11,      SF12,
},
{// Ctrl keys
    '?',        ESC,        '1',        '2',        '3',        '4',        '5',        '6',
    '7',        '8',        '9',        '0',        '-',        '=',        '\b',       '\t',
    CONTROL_Q,  CONTROL_W,  CONTROL_E,  CONTROL_R,  CONTROL_T,  CONTROL_Y,  CONTROL_U,  CONTROL_I,
    CONTROL_O,  CONTROL_P,  '[',        ']',        ENTER,      '?',        CONTROL_A,  CONTROL_S,
    CONTROL_D,  CONTROL_F,  CONTROL_G,  CONTROL_H,  CONTROL_J,  CONTROL_K,  CONTROL_L,  ';',
    '\'',       '`',        '?',        '\\',       CONTROL_Z,  CONTROL_X,  CONTROL_C,  CONTROL_V,
    CONTROL_B,  CONTROL_N,  CONTROL_M,  ',',        '.',        '/',        '?',        '*',
    '?',        ' ',        '?',        CF1,        CF2,        CF3,        CF4,        CF5,
    CF6,        CF7,        CF8,        CF9,        CF10,       NUMLOCK,    SCROLL,     HOME,
    UP,         PGUP,       '?',        LEFT,       '5',        RIGHT,      '?',        END,
    DOWN,       PGDN,       INS,        DEL,        '?',        '?',        '?',        CF11,
    CF12,
},
{// Alt keys
    '?',        ESC,        '1',        '2',        '3',        '4',        '5',        '6',
    '7',        '8',        '9',        '0',        '-',        '=',        '\b',       '\t',
    CONTROL_Q,  CONTROL_W,  CONTROL_E,  CONTROL_R,  CONTROL_T,  CONTROL_Y,  CONTROL_U,  CONTROL_I,
    CONTROL_O,  CONTROL_P,  '[',        ']',        ENTER,      '?',        CONTROL_A,  CONTROL_S,
    CONTROL_D,  CONTROL_F,  CONTROL_G,  CONTROL_H,  CONTROL_J,  CONTROL_K,  CONTROL_L,  ';',
    '\'',       '`',        '?',        '\\',       CONTROL_Z,  CONTROL_X,  CONTROL_C,  CONTROL_V,
    CONTROL_B,  CONTROL_N,  CONTROL_M,  ',',        '.',        '/',        '?',        '*',
    '?',        ' ',        '?',        F1,         F2,         F3,         F4,         F5,
    F6,         F7,         F8,         F9,         F10,        NUMLOCK,    SCROLL,     HOME,
    UP,         PGUP,       '?',        LEFT,       '5',        RIGHT,      '?',        END,
    DOWN,       PGDN,       INS,        DEL,        '?',        '?',        '?',        F11,
    F12,
}
};


static BYTE fShift = 0;
static BYTE fControl = 0;
static BYTE fCapsLock = 0;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/
/******************************************************************************
*                                                                             *
*   void MainLoop()                                                           *
*                                                                             *
*******************************************************************************
*
*   This is the main loop including the input line
*
******************************************************************************/
void MainLoop()
{
    BOOL fExec;
    char c;

    // Clear the input buffer and initialize some editing stuff
    inpbuf[0] = ' /0';
    curpos = 1;

    PrintCmd(":");         // Print out the prompt in the command window
    PrintHelpLine("      Enter a command (H for help)");

    fExec = FALSE;
    while( fExec==FALSE )
    {
        // Get the input character (polling)
        c = KeyGetChar();

        // Check for the special keys:
        switch( c )
        {
            case :
                break;
        }
        // Stuff the new character into the buffer
        if( fInsert )
        {
            // Insert mode (overlapping copy)
            memmove(&inpbuf[curpos+1], &inpbuf[curpos], MAX_INPBUF-curpos);
        }
        else
        {
            // Overwrite mode
            if( inpbuf[curpos]=='/0' )
                inpbuf[curpos+1] = '/0';
        }

        inpbuf[curpos] = c;
        PrintCmd("%s", inpbuf[curpos]);
        curpos++;

    }
}
