/******************************************************************************
*                                                                             *
*   Module:     assemblerdefines.h                                            *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       3/31/2002                                                     *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

        This is a header file containing the assembler defines that are
        used in assembler code.

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
#ifndef _ADEF_H_
#define _ADEF_H_

/******************************************************************************
*
*   Define assembler table individual instructions' addressing mode structure
*
******************************************************************************/
typedef struct
{
    DWORD op[3];                        // Up to 3 arguments types
    BYTE *pOp;                          // Pointer to the codegen descriptor

} TASMADDR;

// Argument types -

#define ASM_BYTE        0x00000001      // Byte size
#define ASM_WORD        0x00000002      // Word size
#define ASM_DWORD       0x00000004      // Dword size

#define ASM_REG         0x00000010      // Direct specified Register
#define ASM_MEM         0x00000020      // Effective memory reference (EA)
#define ASM_RM          0x00000040      // Register or memory reference
#define ASM_IMM         0x00000080      // Immediate value
#define ASM_1           0x00000100      // Special case: 1
#define ASM_REL         0x00000200      // Relative offset

#define ASM_CR          0x00001000      // Control register
#define ASM_DR          0x00002000      // Debug register
#define ASM_TR          0x00004000      // Test register
#define ASM_SR          0x00008000      // Segment register
#define ASM_DESS        0x00010000      // Special cases DS, ES, SS
#define ASM_FSGS        0x00020000      // Special cases FS and GS

#define ASM_AL          0x00100000      // AL
#define ASM_AX          0x00200000      // AX
#define ASM_EAX         0x00400000      // EAX
#define ASM_CL          0x00800000      // CL
#define ASM_CX          0x01000000      // CX
#define ASM_ECX         0x02000000      // ECX
#define ASM_DX          0x04000000      // DX

#define ASM_NEAR        0x10000000      // Near jump reference
#define ASM_FAR         0x20000000      // Far jump reference

#define ASM_REGMMX      0x40000000
#define ASM_FPUREG      0x80000000
#define ASM_FPU0        0x80000000

#define ASM_PREFIX      0xFFFF0001      // Special prefix opcode
#define ASM_REDIRECT    0xFFFF0002      // Instruction redirection


/******************************************************************************
*
*   Define assembler table instruction record
*
******************************************************************************/
typedef struct
{
    char *pName;                        // Instruction name (mnemonic)
    TASMADDR *pAsm;                     // Pointer to array of addressing modes
    WORD nAddr;                         // Number of addressing modes

} TASMREC;


/******************************************************************************
*
*   Define argument record after evaluation
*
******************************************************************************/
typedef struct
{
    DWORD dwFlags;                      // Argument flags ASM_*
    DWORD dwExtFlags;                   // Extra addressing mode flags

    DWORD value;                        // Numerical value
    BYTE reg;                           // CPU Register (base or single)
    BYTE index;                         // CPU Register (index)
    BYTE scale;                         // Scale factor (for scaled index)

} TASMARG;

#define EXT_HAVE_VALUE      0x00000001  // Have a valid value
#define EXT_HAVE_REG        0x00000002  // Base register defined
#define EXT_HAVE_INDEX      0x00000004  // Index register defined
#define EXT_HAVE_SCALE      0x00000008  // Scale factor defined

// Evaluation record

typedef struct
{
    char *szAsm;                        // Input assembly ASCIIZ argument line
    DWORD dwEvalFlags;                  // Evaluation flags
    TASMARG *pArgs;                     // Up to 3 argument records

    BYTE nArg;                          // How many arguments are present in arg[]
    BYTE seg;                           // Segment override (only if EFL_HAVE_SEG)
    int error;                          // Error code (only if EFL_ERROR)

} TASMEVAL;

#define EFL_HAVE_SEG        0x00000001  // Segment override is specified
#define EFL_ERROR           0x80000000  // Error evaluating parameters


#endif // _ADEF_H_
