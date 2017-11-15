/******************************************************************************
*                                                                             *
*   Module:     disassembler.h                                                *
*                                                                             *
*   Date:       3/17/2000                                                     *
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

        This is a header file containing the interface definitions for
        a generic Intel disassembler.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 3/17/2000  Original                                             Goran Devic *
* 4/26/2000  Major rewrite                                        Goran Devic *
* 1/13/2002  Cleanup from vmsim; setup for better scanner         Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DISASM_INT_H_
#define _DISASM_INT_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_DISB  16            // Maximum length of an instruction

/******************************************************************************
*
*   This structure is used to pass parameters and options to the
*   opcode disassembler and return the result in various fileds.
*
******************************************************************************/
typedef struct
{
    WORD wSel;                  // Selector to use when fetching bytes
    DWORD dwOffset;             // Target pointer offset to disassemble
    BYTE *szDisasm;             // String where to put ascii result
    BYTE bState;                // Defines the disassembler state

    BYTE bAsciiLen;             // Length of the ascii result
    BYTE bInstrLen;             // Instruction lenght in bytes
    BYTE bCodes[MAX_DISB];      // Actual codes that were read
    BYTE bAccess;               // Instruction access codes
    BYTE bFlags;                // Instruction flags

    DWORD dwTargetAddress;      // Target instruction decoded address value
    DWORD dwTargetData;         // Target instruction decoded data value

} TDISASM, *PTDISASM;

// `bState' defines some disassembler states:

#define DIS_DATA32                  0x01    // Data size 16/32 bits (0/1)
#define   DIS_GETDATASIZE(flags) ((flags)&DIS_DATA32)
#define DIS_ADDRESS32               0x02    // Address size 16/32 bits (0/1)
#define   DIS_GETADDRSIZE(flags) (((flags)&DIS_ADDRESS32)>>1)

#define DIS_SEGOVERRIDE             0x04    // Default segment has been overriden

#define DIS_REP                     0x10    // Return: REP prefix found (followed by..)
#define DIS_REPNE                   0x20    // Return: REPNE prefix found
#define   DIS_GETREPENUM(flags)  (((flags)>>4)&3)
#define DIS_ILLEGALOP               0x80    // Return: illegal opcode found

// bAccess returns the current instruction access type flags
// Data access flags are used with memory access instructions:

#define INSTR_READ                  0x80    // Faulting instruction reads memory
#define INSTR_WRITE                 0x40    // Faulting instruction writes to memory
#define INSTR_READ_WRITE            0x20    // Faulting instruction is read-modify-write

// Low nibble contains the data length code - do not change these values as
// they represent the data width value as well

#define INSTR_BYTE                  0x01    // Byte access instruction
#define INSTR_WORD                  0x02    // Word access instruction
#define INSTR_WORD_DWORD            0x03    // Word or dword, depending on operand size
#define INSTR_DWORD                 0x04    // Dword access instruction

// bFlags field returns generic instruction type:
// Disassembler flags: top nibble

#define DIS_SPECIAL                 0x80    // Special opcode
#define DIS_NAME_FLAG               0x40    // Mnemonic name changes
#define   DIS_GETNAMEFLAG(flags)    (((flags)>>6)&1)
#define DIS_COPROC                  0x20    // Coprocessor instruction
#define DIS_MODRM                   0x10    // Uses additional Mod R/M byte

// Instruction scanner enums: bottom nibble

#define SCAN_NATIVE                 0x00    // Regular linear instruction
#define SCAN_JUMP                   0x01    // Jump instruction
#define SCAN_COND_JUMP              0x02    // Conditional jump
#define SCAN_CALL                   0x03    // Call instruction
#define SCAN_RET                    0x04    // Return instruction
#define SCAN_INT                    0x05    // INT instruction
#define SCAN_MASK                   0x0F    // - Nibble mask value -


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BYTE Disassembler( PTDISASM pDis );
extern BYTE DisassemblerLen( PTDISASM pDis );
extern DWORD GetDisFlags(void);
extern int GetInstructionLen(WORD cs, DWORD eip);
extern BOOL IsEffectiveAddress(void);
extern DWORD fnEAddr(DWORD arg);


#endif //  _DISASM_INT_H_

