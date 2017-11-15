/******************************************************************************
*                                                                             *
*   Module:     disassembler.h                                                *
*                                                                             *
*   Date:       3/17/2000                                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This source code and produced executable is copyrighted by Goran Devic.   *
*   This source, portions or complete, and its derivatives can not be given,  *
*   copied, or distributed by any means without explicit written permission   *
*   of the copyright owner. All other rights, including intellectual          *
*   property rights, are implicitly reserved. There is no guarantee of any    *
*   kind that this software would perform, and nobody is liable for the       *
*   consequences of running it. Use at your own risk.                         *
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
*   line disassembler.
*
******************************************************************************/
typedef struct
{
    DWORD dwFlags;              // Generic flags (described below)
    WORD wSel;                  // Selector to use when fetching bytes
    DWORD dwOffset;             // Target pointer offset to disassemble
    BYTE *szDisasm;             // String where to put ascii result
    BYTE bAsciiLen;             // Length of the ascii result
    BYTE bInstrLen;             // Instruction lenght in bytes
    BYTE bCodes[MAX_DISB];      // Actual codes that were read

    int nDisplacement;          // Scanner: possible constant displacement
    int nScanEnum;              // Scanner: specific flags SCAN_*

} TDISASM, *PTDISASM;

// dwFlags contains a set of boolean flags with the following functionality

#define DIS_DATA32          0x0001  // Data size 16/32 bits (0/1)
#define   DIS_GETDATASIZE(flags) ((flags)&DIS_DATA32)
#define DIS_ADDRESS32       0x0002  // Address size 16/32 bits (0/1)
#define   DIS_GETADDRSIZE(flags) (((flags)&DIS_ADDRESS32)?1:0)

#define DIS_SEGOVERRIDE     0x0004  // Default segment has been overriden

#define DIS_REP             0x0100  // Return: REP prefix found (followed by..)
#define DIS_REPNE           0x0200  // Return: REPNE prefix found
#define   DIS_GETREPENUM(flags)  (((flags)>>8)&3)
#define DIS_ILLEGALOP       0x8000  // Return: illegal opcode

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BYTE Disassembler( PTDISASM pDis );
extern DWORD GetDisFlags(void);


#endif //  _DISASM_INT_H_
