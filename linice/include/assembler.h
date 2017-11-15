/******************************************************************************
*                                                                             *
*   Module:     assembler.h                                                   *
*                                                                             *
*   Date:       3/31/2002                                                     *
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
        a generic x86 line assembler.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 3/31/2002  Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/

// --------------------------------- TMP TMP TMP -----------------------------

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BOOL;

#define TRUE        1
#define FALSE       0

// --------------------------------- TMP TMP TMP -----------------------------


#ifndef _ASM_INT_H_
#define _ASM_INT_H_

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

#define MAX_ASMB        16      // Maximum number of bytes assembler will produce

/******************************************************************************
*
*   This structure is used to pass parameters and options to the
*   line assembler and return the result in various fileds.
*
*   Where:
*       szLine is a pointer to the string to be assembled
*       dwFlags are input state flags, defined below
*       eip is the current instruction address. This has to be set so the
*           loop instructions that use relative offsets can be computed
*
*   Returns:
*       bInstrLen is the final instruction length in bytes
*       error should be 0 for no error, otherwise error code as defined below
*
*
******************************************************************************/
typedef struct
{
    char *szLine;                       // Input assembly mnemonic string
    DWORD dwFlags;                      // Flags define some states
    DWORD eip;                          // Current instruction address

    BYTE bCodes[MAX_ASMB];              // Final resulting opcodes
    BYTE bInstrLen;                     // Final opcode len in bytes
    BYTE error;                         // Error code, defined below ASMERR_*

} TASM, *PTASM;

// dwFlags contains following bits:

#define ASM_ADDR32      0x00000001      // 32 bit address mode
#define ASM_DATA32      0x00000010      // 32 bit data mode

// Error codes:

#define ASMERR_NOERROR          0x00    // No error
#define ASMERR_ADRTOOFAR        0x01    // Jump address out of range
#define ASMERR_BYTEVAL          0x02    // Byte value expected
#define ASMERR_WORDVAL          0x03    // Word value expected


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

//extern BYTE "C" Assembler( TASM *pAsm );


#endif //  _ASM_INT_H_

