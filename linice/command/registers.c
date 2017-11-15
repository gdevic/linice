/******************************************************************************
*                                                                             *
*   Module:     registers.c                                                   *
*                                                                             *
*   Date:       05/15/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        This module contains code that prints and edits registers

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/15/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "disassembler.h"               // Include disassembler for ea

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

typedef struct
{
    BYTE xStart;                        // Start X coordinate of the field
    BYTE xEnd;                          // End X coordinate of the field
    BYTE y;                             // Y coordinate of the field
    BYTE offset;                        // Offset in TREGS of the register
    DWORD flagBit;                      // If eflags, bitmask of that flag

    BYTE prevIndex;                     // Index of a previous register field
    BYTE nextIndex;                     // Index of a next register field
    BYTE tabIndex;                      // Index of a register if pressed TAB key
    BYTE upIndex;                       // Index of a register if pressed UP key
    BYTE downIndex;                     // Index of a register if pressed DOWN key
} TRegField, *PTRegField;

static TRegField RegField[] = {
/*               x1  x2  y  offset                flagBit     prev next tab up  down */
/*  0: EAX */  {  5, 12, 1, offsetof(TREGS, eax), 0,          22,   1,  1, 17,  5 },
/*  1: EBX */  { 20, 27, 1, offsetof(TREGS, ebx), 0,           0,   2,  2, 18,  6 },
/*  2: ECX */  { 35, 42, 1, offsetof(TREGS, ecx), 0,           1,   3,  3, 19,  7 },
/*  3: EDX */  { 50, 57, 1, offsetof(TREGS, edx), 0,           2,   4,  4, 20,  8 },
/*  4: ESI */  { 65, 72, 1, offsetof(TREGS, esi), 0,           3,   5,  5, 21,  9 },

/*  5: EDI */  {  5, 12, 2, offsetof(TREGS, edi), 0,           4,   6,  6,  0, 17 },
/*  6: EBP */  { 20, 27, 2, offsetof(TREGS, ebp), 0,           5,   7,  7,  1, 18 },
/*  7: ESP */  { 35, 42, 2, offsetof(TREGS, esp), 0,           6,   8,  8,  2, 19 },
/*  8: EIP */  { 50, 57, 2, offsetof(TREGS, eip), 0,           7,   9,  9,  3, 20 },

/*  9:  O  */  { 61, 61, 2, offsetof(TREGS, eflags), 1 << 11,  8,  10, 17,  4, 21 },
/* 10:  D  */  { 63, 63, 2, offsetof(TREGS, eflags), 1 << 10,  9,  11, 17,  4, 21 },
/* 11:  I  */  { 65, 65, 2, offsetof(TREGS, eflags), 1 <<  9, 10,  12, 17,  4, 21 },
/* 12:  S  */  { 67, 67, 2, offsetof(TREGS, eflags), 1 <<  7, 11,  13, 17,  4, 21 },
/* 13:  Z  */  { 69, 69, 2, offsetof(TREGS, eflags), 1 <<  6, 12,  14, 17,  4, 21 },
/* 14:  A  */  { 71, 71, 2, offsetof(TREGS, eflags), 1 <<  4, 13,  15, 17,  4, 21 },
/* 15:  P  */  { 73, 73, 2, offsetof(TREGS, eflags), 1 <<  2, 14,  16, 17,  4, 21 },
/* 16:  C  */  { 75, 75, 2, offsetof(TREGS, eflags), 1 <<  0, 15,  17, 17,  4, 21 },

/* 17: CS  */  {  4,  7, 3, offsetof(TREGS, cs ), 0,          16,  18, 18,  5,  0 },
/* 18: DS  */  { 14, 17, 3, offsetof(TREGS, ds ), 0,          17,  19, 19,  6,  1 },
/* 19: SS  */  { 24, 27, 3, offsetof(TREGS, ss ), 0,          18,  20, 20,  7,  2 },
/* 20: ES  */  { 34, 37, 3, offsetof(TREGS, es ), 0,          19,  21, 21,  8,  3 },
/* 21: FS  */  { 44, 47, 3, offsetof(TREGS, fs ), 0,          20,  22, 22,  9,  4 },
/* 22: GS  */  { 54, 57, 3, offsetof(TREGS, gs ), 0,          21,   0,  0, 17,  0 },

               { 0 }
};

typedef struct
{
    char *sRegName;                     // Register name
    BYTE nameLen;                       // Length of the register name
    DWORD max;                          // Register maximum value
    BYTE offset;                        // Offset in TREGS of the register
    BYTE delta;                         // Delta x coordinate of the field
    BYTE fieldIndex;                    // Index into RegField[] of that register
} TRegEdit, *PTRegEdit;

static TRegEdit RegEdit[] = {
{ "al",  2, 0x000000FF, offsetof(TREGS, eax), 7, 0 },
{ "ah",  2, 0x000000FF, offsetof(TREGS, eax), 6, 0 },
{ "ax",  2, 0x0000FFFF, offsetof(TREGS, eax), 4, 0 },
{ "eax", 3, 0xFFFFFFFF, offsetof(TREGS, eax), 0, 0 },
{ "bl",  2, 0x000000FF, offsetof(TREGS, ebx), 7, 1 },
{ "bh",  2, 0x000000FF, offsetof(TREGS, ebx), 6, 1 },
{ "bx",  2, 0x0000FFFF, offsetof(TREGS, ebx), 4, 1 },
{ "ebx", 3, 0xFFFFFFFF, offsetof(TREGS, ebx), 0, 1 },
{ "cl",  2, 0x000000FF, offsetof(TREGS, ecx), 7, 2 },
{ "ch",  2, 0x000000FF, offsetof(TREGS, ecx), 6, 2 },
{ "cx",  2, 0x0000FFFF, offsetof(TREGS, ecx), 4, 2 },
{ "ecx", 3, 0xFFFFFFFF, offsetof(TREGS, ecx), 0, 2 },
{ "dl",  2, 0x000000FF, offsetof(TREGS, edx), 7, 3 },
{ "dh",  2, 0x000000FF, offsetof(TREGS, edx), 6, 3 },
{ "dx",  2, 0x0000FFFF, offsetof(TREGS, edx), 4, 3 },
{ "edx", 3, 0xFFFFFFFF, offsetof(TREGS, edx), 0, 3 },

{ "si",  2, 0x0000FFFF, offsetof(TREGS, esi), 4, 4 },
{ "esi", 3, 0xFFFFFFFF, offsetof(TREGS, esi), 0, 4 },
{ "di",  2, 0x0000FFFF, offsetof(TREGS, edi), 4, 5 },
{ "edi", 3, 0xFFFFFFFF, offsetof(TREGS, edi), 0, 5 },

{ "bp",  2, 0x0000FFFF, offsetof(TREGS, ebp), 4, 6 },
{ "ebp", 3, 0xFFFFFFFF, offsetof(TREGS, ebp), 0, 6 },

{ "sp",  2, 0x0000FFFF, offsetof(TREGS, esp), 4, 7 },
{ "esp", 3, 0xFFFFFFFF, offsetof(TREGS, esp), 0, 7 },

{ "ip",  2, 0x0000FFFF, offsetof(TREGS, eip), 4, 8 },
{ "eip", 3, 0xFFFFFFFF, offsetof(TREGS, eip), 0, 8 },

{ "cs",  2, 0x0000FFFF, offsetof(TREGS, cs ), 0, 17},
{ "ds",  2, 0x0000FFFF, offsetof(TREGS, ds ), 0, 18},
{ "ss",  2, 0x0000FFFF, offsetof(TREGS, ss ), 0, 19},
{ "es",  2, 0x0000FFFF, offsetof(TREGS, es ), 0, 20},
{ "fs",  2, 0x0000FFFF, offsetof(TREGS, fs ), 0, 21},
{ "gs",  2, 0x0000FFFF, offsetof(TREGS, gs ), 0, 22},

{ "fl",  2, 0x0000FFFF, offsetof(TREGS, eflags), 0, 9 },
{ "efl", 3, 0xFFFFFFFF, offsetof(TREGS, eflags), 0, 9 },

{ NULL }
};


#define ACCEPT  *(DWORD *)((DWORD)deb.r + pReg->offset) = value

#define ATTR(reg)                DP_SETCOLINDEX, deb.r->reg==deb.r_prev.reg? COL_NORMAL : COL_BOLD, deb.r->reg, DP_SETCOLINDEX, COL_NORMAL
#define ATTRFL(mask, valU, valL) DP_SETCOLINDEX, (deb.r->eflags & mask)==(deb.r_prev.eflags & mask)? COL_NORMAL : COL_BOLD, (deb.r->eflags & mask)? valU : valL, DP_SETCOLINDEX, COL_NORMAL

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern BOOL GlobalReadDword(DWORD *ppDword, DWORD dwAddress);

/******************************************************************************
*                                                                             *
*   void RegDraw(BOOL fForce)                                                 *
*                                                                             *
*******************************************************************************
*
*   Function that actually prints the register information.
*
******************************************************************************/
void RegDraw(BOOL fForce)
{
    static char bufEA[16];              // Buffer to print effective address substring
    DWORD EA, PtrEA;                    // Effective address variable and the value

    if( pWin->r.fVisible==TRUE )
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, 0+1);
    else
        if( fForce==FALSE )
            return;

    dprinth(1, "EAX=%c%c%08X%c%c   EBX=%c%c%08X%c%c   ECX=%c%c%08X%c%c   EDX=%c%c%08X%c%c   ESI=%c%c%08X%c%c",
            ATTR(eax), ATTR(ebx), ATTR(ecx), ATTR(edx), ATTR(esi) );

    dprinth(2, "EDI=%c%c%08X%c%c   EBP=%c%c%08X%c%c   ESP=%c%c%08X%c%c   EIP=%c%c%08X%c%c   %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c",
            ATTR(edi), ATTR(ebp), ATTR(esp), ATTR(eip),
            ATTRFL(OF_MASK, 'O','o'),
            ATTRFL(DF_MASK, 'D','d'),
            ATTRFL(IF_MASK, 'I','i'),
            ATTRFL(SF_MASK, 'S','s'),
            ATTRFL(ZF_MASK, 'Z','z'),
            ATTRFL(AF_MASK, 'A','a'),
            ATTRFL(PF_MASK, 'P','p'),
            ATTRFL(CF_MASK, 'C','c') );

    // Print the optional current instruction's effective address
    if( IsEffectiveAddress() )
    {
        EA = fnEAddr(0);

        // Get the DWORD value from the effective address
        if( GlobalReadDword(&PtrEA, EA) )
            sprintf(bufEA, "%08X=%08X", EA, PtrEA);
        else
            sprintf(bufEA, "%08X=????????", EA);
    }
    else
        bufEA[0] = 0;

    dprinth(3, "CS=%c%c%04X%c%c   DS=%c%c%04X%c%c   SS=%c%c%04X%c%c   ES=%c%c%04X%c%c   FS=%c%c%04X%c%c   GS=%c%c%04X%c%c   %s",
        ATTR(cs), ATTR(ds), ATTR(ss), ATTR(es), ATTR(fs), ATTR(gs), bufEA);

    if( pWin->r.fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}

/******************************************************************************
*                                                                             *
*   void EditInPlace(PTRegField pReg, int xDisp)                              *
*                                                                             *
*******************************************************************************
*
*   Edits register values.s
*
******************************************************************************/
static void EditInPlace(PTRegField pReg, int xDisp)
{
    BOOL fContinue = TRUE;              // Continuation flag
    int xOrig, yOrig;                   // Cursor temporary store
    int xCur = 0, yCur = 0;             // Current cursor coordinates
    DWORD nibble;                       // Current typed nibble
    DWORD value = 0;                    // and complete value
    WCHAR Key;                          // Current key pressed

    // If the register window is not visible, make it visible
    if( pWin->r.fVisible==FALSE )
    {
        // Make a window visible and redraw whole screen
        pWin->r.fVisible = TRUE;
        RecalculateDrawWindows();
    }

    // Print the help line for the register edit
    dprint("%c%c%c%c%c%cValid control keys: %s %s %s %s Tab Enter Esc    Insert: Toggle flag\r%c",
    DP_SAVEXY,
    DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
    DP_SETCOLINDEX, COL_HELP,
    "Left", "Right", "Up", "Dn",
    DP_RESTOREXY);

    // Save cursor coordinates manually since we can't have more than one
    // level of save & restore and RegDraw() is doing it
    xOrig = pOut->x;
    yOrig = pOut->y;

    do
    {
        // Read the new cursor coordinates and current register value
        if( xCur==0 )
        {
            xCur = pReg->xStart + xDisp;
            xDisp = 0;                  // Use this one only once!
            yCur = pReg->y;
            value = *(DWORD *)((DWORD)deb.r + pReg->offset);
        }

        // Position the cursor at the right register coordinate
        dprint("%c%c%c", DP_SETCURSORXY, xCur, yCur);

        Key = GetKey(TRUE);
        Key = toupper(Key);

        switch( Key )
        {
            case ESC:       // ESC key aborts change and quits edit
                fContinue = FALSE;
                break;

            case ENTER:     // Enter key accepts change and quits edit
                ACCEPT;
                fContinue = FALSE;
                break;

            case TAB:       // Tab key accepts change and selects next register/field
                ACCEPT;
                pReg = &RegField[pReg->tabIndex];
                xCur = 0;                   // Signal read new cursor coordinate
                break;

            case LEFT:      // Left key moves cursor, possibly changing the field (accepting)
                if( --xCur < pReg->xStart )
                {
                    ACCEPT;
                    pReg = &RegField[pReg->prevIndex];
                    xCur = 0;
                }
                break;

            case UP:        // Up key accepts and cycles one line up
                ACCEPT;
                pReg = &RegField[pReg->upIndex];
                xCur = 0;
                break;

            case DOWN:      // Down key accepts and cycles one line down
                ACCEPT;
                pReg = &RegField[pReg->downIndex];
                xCur = 0;
                break;
                            // All hex characters are accepted
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            case '8': case '9': case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                if( pReg->flagBit==0 )      // Dont do numbers when on flags field
                {
                    nibble = (Key>'9')? Key - 'A' + 10 : Key - '0';

                    // Mask the current nibble in the value
                    value &= 0xF0000000 >> (xCur - pReg->xStart);

                    // Add the new nibble key that we just typed
                    value |= nibble << (4 * (7 - (xCur - pReg->xStart)));

                    // Print the new nibble character
                    dprint("%c", Key);
                }
                // No break.. Continue similar to the LEFT key:

            case RIGHT:     // Right key moves cursor, possibly changing the field (accepting)
                if( ++xCur > pReg->xEnd )
                {
                    // End of field reached.. Accept the new value and go to the next field
                    ACCEPT;
                    pReg = &RegField[pReg->nextIndex];
                    xCur = 0;
                }
                break;

            case INS:       // Insert key toggles flag (if on flags)
                if( pReg->flagBit )
                {
                    value ^= pReg->flagBit;
                    deb.r->eflags = value;  // Commit immediately
                    RegDraw(TRUE);          // Redraw the register window
                }
                break;

            default:        // Anything else implicitly quits edit
                fContinue = FALSE;
        }
    }
    while( fContinue );

    // Restore cursor coordinates
    pOut->x = xOrig;
    pOut->y = yOrig;
}

/******************************************************************************
*                                                                             *
*   BOOL cmdReg(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Register command. If there was no arguments, edit registers in place
*   (register window will be made visible).
*   Otherwise, set a register value.
*
*   Argument -d will display registers in the command window.
*
******************************************************************************/
BOOL cmdReg(char *args, int subClass)
{
    PTRegEdit pReg;
    DWORD value, prev_value;
    BOOL fRegVisible;                   // Temp store for regs visible flag

    if( *args==0 )
    {
        // No arguments - if the register window is not visible, make it visible then edit in place
        EditInPlace(&RegField[0], 0);

        // If we changed cs:eip, we need to recalculate the whole context
        SetSymbolContext(deb.r->cs, deb.r->eip);

        deb.fRedraw = TRUE;
    }
    else
    {
        // Display registers in the command window
        if( !strnicmp(args, "-d", 2) )
        {
            // Fake the register window is not visible, and then redraw it
            fRegVisible = pWin->r.fVisible;
            pWin->r.fVisible = FALSE;

            RegDraw(TRUE);

            pWin->r.fVisible = fRegVisible;
        }
        else
        {
            // Find which register got referenced
            pReg = &RegEdit[0];

            while( pReg->sRegName != NULL )
            {
                if( !strnicmp(pReg->sRegName, args, pReg->nameLen) )
                    break;
                pReg++;
            }

            if( pReg->sRegName != NULL )
            {
                // Skip over the register name
                args += pReg->nameLen;

                // If there are no more parameters, go to edit in place mode
                if( *args==0 )
                {
                    // r <register>
                    // Position cursor on the selected register and edit register in place
                    EditInPlace(&RegField[pReg->fieldIndex], pReg->delta );
                }
                else
                {
                    // r <register> [=] <value>
                    // Assign register a value ('=' is optional)

                    // Skip the blanks and optional '='
                    while( *args==' ' || *args=='=' ) args++;

                    // Now we have to have some expression
                    if( *args && Expression(&value, args, &args )==TRUE)
                    {
                        dprinth(1, "reg: %s = %X", pReg->sRegName, value);

                        // Check that the value is within the range of the selected register
                        if( value <= pReg->max )
                        {
                            prev_value = *(DWORD *)((DWORD)deb.r + pReg->offset);
                            *(DWORD *)((DWORD)deb.r + pReg->offset) = (prev_value & ~pReg->max) | value;

                            // If we changed eip, we better readjust the code window address
                            deb.codeTopAddr.offset = deb.r->eip;

                            // If we changed cs:eip, we need to recalculate the whole context
                            SetSymbolContext(deb.r->cs, deb.r->eip);

                            deb.fRedraw = TRUE;
                        }
                        else
                            dprinth(1, "Value out of range for selected register");
                    }
                }
            }
        }
    }

    return( TRUE );
}

