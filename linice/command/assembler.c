/******************************************************************************
*                                                                             *
*   Module:     assembler.c                                                   *
*                                                                             *
*   Date:       03/31/02                                                      *
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

        This module contains the code for the generic Intel assembler.
        The latest supported uprocessor is Pentium Pro.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 3/31/2002  Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#if 0

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#endif

#include <string.h>


#include "assembler.h"                  // Include interface header file

#include "assemblerdefines.h"           // Include assembler shared structures

#include "asmdata1.h"                   // Include assembler data file (1)
#include "asmdata2.h"                   // Include assembler data file (2)

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   External functions (optional)                                             *
*                                                                             *
******************************************************************************/

extern int AsmParamEval(TASMEVAL *pEvalParam);

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static int cond_index;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

TASMREC *FindAsmRecord(char **ppStr)
{
    char *pTokStart;                    // Pointer to a token start
    char *pTokEnd;                      // Pointer to a token end
    int i, len, index;

    pTokStart = *ppStr;

    // Skip over spaces and store the new start
    while( *pTokStart && *pTokStart==' ' ) pTokStart++;
    *ppStr = pTokStart;

    // Find the length of our token
    pTokEnd = strchr(pTokStart, ' ');
    if( pTokEnd )
        len = pTokEnd - pTokStart;      // Compute len of the token
    else
    {
        len = strlen(pTokStart);        // This token span to the end of string
        pTokEnd = pTokStart + len;
    }

    // Do a linear search over the instruction records to find the right token
    // TODO: We might do a binary search for speed sometimes in the future?
    for(i=0; i<MAX_ASM_REC; i++ )
    {
        if( !strnicmp(pTokStart, pAsm[i].pName, len) )
        {
            // Check for redirection flag (conditional instructions)
            if( pAsm[i].pAsm->op[0]==ASM_REDIRECT )
            {
                // Decode conditional part of instruction into a conditional index
                // when we are still at the original instruction. Flags of the second
                // opcode in a redirection record will tell us at what offset to look
                if( pAsm[i].pAsm->op[1] )
                {
                    *ppStr += len;
                    pTokStart += pAsm[i].pAsm->op[1];
                    len -= pAsm[i].pAsm->op[1];
                    len += 1;

                         if( !strnicmp(pTokStart, "o ", len ))      index = 0;
                    else if( !strnicmp(pTokStart, "no ", len ))     index = 1;
                    else if( !strnicmp(pTokStart, "b ", len ))      index = 2;
                    else if( !strnicmp(pTokStart, "nb ", len ))     index = 3;
                    else if( !strnicmp(pTokStart, "z ", len ))      index = 4;
                    else if( !strnicmp(pTokStart, "nz ", len ))     index = 5;
                    else if( !strnicmp(pTokStart, "be ", len ))     index = 6;
                    else if( !strnicmp(pTokStart, "nbe ", len ))    index = 7;
                    else if( !strnicmp(pTokStart, "s ", len ))      index = 8;
                    else if( !strnicmp(pTokStart, "ns ", len ))     index = 9;
                    else if( !strnicmp(pTokStart, "p ", len ))      index = 10;
                    else if( !strnicmp(pTokStart, "np ", len ))     index = 11;
                    else if( !strnicmp(pTokStart, "l ", len ))      index = 12;
                    else if( !strnicmp(pTokStart, "nl ", len ))     index = 13;
                    else if( !strnicmp(pTokStart, "le ", len ))     index = 14;
                    else if( !strnicmp(pTokStart, "nle ", len ))    index = 15;
                    else
                        return( NULL );

                    printf("Redirect to cond_index=%d\n", index);
                    cond_index = index;
                }

                pTokStart = pAsm[i].pAsm->pOp;
                return( FindAsmRecord(&pTokStart) );
            }

            // Store the string pointer
            *ppStr = pTokEnd;

            return( &pAsm[i] );
        }
    }

    return( NULL );
}

BOOL AsmAddCode(TASM *pAsm, BYTE bCode)
{
    if( pAsm->bInstrLen < MAX_ASMB )
    {
        pAsm->bCodes[pAsm->bInstrLen] = bCode;
        pAsm->bInstrLen++;

        return( TRUE );
    }
    
    return( FALSE );
}

#define MATCH(a,b)  (((a)&(b))==(a))

TASMADDR *AsmMatch(TASM *pAsm, TASMARG Arg[3], TASMREC *pRec)
{
    TASMADDR *pAddr;                    // Pointer to a current instruction address descriptor
    int args;                           // Argument count
    int index = 0;
    int params;                         // Number of parameters in a table (current)
    int arg_params;

    pAddr = pRec->pAsm;
    arg_params = 0 + Arg[0].dwFlags? 1:0 + Arg[1].dwFlags? 1:0 + Arg[2].dwFlags? 1:0;

    for(args=0; args<pRec->nAddr; args++, pAddr++)
    {
        index++;

        params = 0 + pAddr->op[0]? 1:0 + pAddr->op[1]? 1:0 + pAddr->op[2]? 1:0;

//        if( params==arg_params )
        {
            if( MATCH(pAddr->op[0],Arg[0].dwFlags) && MATCH(pAddr->op[1],Arg[1].dwFlags) && MATCH(pAddr->op[2],Arg[2].dwFlags) )
            {
                printf("Match index=%d\n", index);
                return( pAddr );
            }
        }
    }

    return( NULL );
}

BOOL DiffAddrSize(TASM *pAsm, TASMARG *pArg)
{
    BOOL fAsm, fArg;

    fArg = (pArg->dwFlags & ASM_DWORD)? 1:0;
    fAsm = (pAsm->dwFlags & ASM_ADDR32)? 1:0;

    if( fAsm ^ fArg )
        return( TRUE );

    return( FALSE );
}

enum
{
    AX, CX, DX, BX, SP, BP, SI, DI
};


BOOL AsmCodeGen(TASM *pAsm, TASMEVAL *pEval, TASMADDR *pAddr)
{
    // ES, CS, SS, DS, FS, GS
    const BYTE seg_table[8] = { 0x3E, 0x2E, 0x26, 0x36, 0x64, 0x65  };
    BYTE *pCode;
    int value;                          // Temp value (signed !)
    int reg;                            // Register field
    TASMARG *pArg;                      // Temp current arg
    TASMARG args[3];
    BYTE rm, mod;

    memcpy((void*)&args, (void*)pEval->pArgs, sizeof(TASMARG[3]));

    pCode = pAddr->pOp;

    do
    {
        // Cases 01AB are modRM using EA of arg A + register of arg B
        // Cases 02AB are modRM using EA of arg A + spare digit B
        if( *pCode>=0100 && *pCode<=0227 )
        {
            pArg = &args[(*pCode>>3) & 3];

            // Pick up a reg field for RM byte
            if( *pCode<0200 )
                reg = args[*pCode & 3].reg;
            else
                reg = *pCode & 7;

            // Segment override check
            if( pEval->dwEvalFlags & EFL_HAVE_SEG )
                AsmAddCode(pAsm, seg_table[pEval->seg]);

            // 16-bit vs 32-bit

            if( (pArg->dwExtFlags & EXT_HAVE_INDEX) && (pArg->dwExtFlags & EXT_HAVE_REG))
            {
                    if( pArg->reg==BX && pArg->index==SI) rm = 0;
               else if( pArg->reg==SI && pArg->index==BX) rm = 0;
               else if( pArg->reg==BX && pArg->index==DI) rm = 1;
               else if( pArg->reg==DI && pArg->index==BX) rm = 1;
               else if( pArg->reg==BP && pArg->index==SI) rm = 2;
               else if( pArg->reg==SI && pArg->index==BP) rm = 2;
               else if( pArg->reg==BP && pArg->index==DI) rm = 3;
               else if( pArg->reg==DI && pArg->index==BP) rm = 3;
            }
            else
                if( pArg->dwExtFlags & EXT_HAVE_REG )
                {
                         if( pArg->reg==SI ) rm = 4;
                    else if( pArg->reg==DI ) rm = 5;
                    else if( pArg->reg==BP ) rm = 6;
                    else if( pArg->reg==BX ) rm = 7;
                }

            mod = 0;

            if( pArg->dwFlags & ASM_IMM )
            {
                if( pArg->dwFlags & ASM_BYTE )
                    mod = 1;
                else
                if( pArg->dwFlags & ASM_WORD )
                    mod = 2;
            }

            AsmAddCode(pAsm, (mod<<6) | (reg<<3) | (rm<<0) );

            if( pArg->dwFlags & ASM_IMM )
            {
                if( pArg->dwFlags & ASM_BYTE )
                    AsmAddCode(pAsm, (BYTE)pArg->value);
                else
                if( pArg->dwFlags & ASM_WORD )
                {
                    AsmAddCode(pAsm, (BYTE)(pArg->value & 0xFF));
                    AsmAddCode(pAsm, (BYTE)(pArg->value >> 8));
                }
            }


        }
        else
        switch( *pCode )
        {
            case 3:     AsmAddCode(pAsm, *++pCode);
            case 2:     AsmAddCode(pAsm, *++pCode);
            case 1:     AsmAddCode(pAsm, *++pCode);
                break;

            case 4:     // Embed register [5:3] into the next byte (Push/Pop)
                        pCode++;
                        value = *pCode | (args[0].reg<<3);
                        AsmAddCode(pAsm, value);
                break;

            case 010:   // Embed register [2:0] into a next byte
            case 011:
            case 012:
                        value = args[*pCode-010].reg;
                        pCode++;
                        value |= *pCode;
                        AsmAddCode(pAsm, value);
                break;

            case 014:   // Signed byte immediate operand
            case 015:
            case 016:
                        value = args[*pCode-014].value;
                        if( value>=-256 && value<=255 )
                            AsmAddCode(pAsm, value & 0xFF);
                        else
                            pAsm->error = ASMERR_BYTEVAL;
                break;

            case 017:   // Literal byte 0
                        AsmAddCode(pAsm, 0);
                break;

            case 020:   // Byte immediate operand - both positive and negative
            case 021:   // We need to check if a value is in the byte range
            case 022:
                        value = args[*pCode-020].value;
                        if( value>=-256 && value<=255 )
                            AsmAddCode(pAsm, value & 0xFF);
                        else
                            pAsm->error = ASMERR_BYTEVAL;
                break;

            case 024:   // Byte immediate operand
            case 025:   // We need to check if a value is in the byte range
            case 026:
                        value = args[*pCode-024].value;
                        if( value>=0 && value<=255 )
                            AsmAddCode(pAsm, (BYTE)value);
                        else
                            pAsm->error = ASMERR_BYTEVAL;
                break;

            case 030:   // Word immediate operand
            case 031:   // We need to check if a value is in the word range
            case 032:
                        value = args[*pCode-030].value;
                        if( value>=0 && value<=65535 )
                        {
                            AsmAddCode(pAsm, (BYTE)value & 0xFF);
                            AsmAddCode(pAsm, (BYTE)(value>>8));
                        }
                        else
                            pAsm->error = ASMERR_WORDVAL;
                break;

            case 050:   // Byte relative address operand from the first arg
                        // We need to check if a value is in the range
                        value = args[0].value - (pAsm->eip + pAsm->bInstrLen + 1);
                        if( value>127 || value< -128 )
                            pAsm->error = ASMERR_ADRTOOFAR;
                        else
                            AsmAddCode(pAsm, (BYTE)value);
                break;

case 064:
    break;
            case 0300:  // Possible address-size prefix
            case 0301:
                if( DiffAddrSize(pAsm,&args[*pCode-0300]) )
                    AsmAddCode(pAsm, 0x67);
                break;

            case 0310:  // Force 16-bit address size (loop cx)
                if( pAsm->dwFlags & ASM_ADDR32 )
                    AsmAddCode(pAsm, 0x67);
                break;

            case 0311:  // Force 32-bit address size (loop ecx)
                if( !(pAsm->dwFlags & ASM_ADDR32) )
                    AsmAddCode(pAsm, 0x67);
                break;

            case 0320:  // Force 16-bit operand size
                if( pAsm->dwFlags & ASM_DATA32 )
                    AsmAddCode(pAsm, 0x66);
                break;

            case 0321:  // Force 32-bit operand size
                if( !(pAsm->dwFlags & ASM_DATA32) )
                    AsmAddCode(pAsm, 0x66);
                break;

            case 0330:  // Add conditional code to the next literal byte
                    pCode++;
                    AsmAddCode(pAsm, cond_index | *pCode);
                break;

            default:
                printf("bCode=%o\n", *pCode);
                break;
        }
        
        pCode++;                        // Next byte in the stream

    } while(*pCode);

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BYTE Assembler( TASM *pAsm )                                              *
*                                                                             *
*******************************************************************************
*
*   This is a generic x86 line assembler.
*
*   Where:
*
*   Returns:
*       Instruction length in bytes
*       0 for error, read error code in pAsm->error
*
******************************************************************************/
BYTE Assembler( TASM *pAsm )
{
    TASMEVAL Eval;
    TASMARG args[3];                    // Up to 3 arguments
    TASMREC *pRec;                      // Current instruction record
    TASMADDR *pAddr;                    // Instruction address mode descriptor
    char *pStr;                         // Parsing string pointer
    int error;                          // Local error code value

    pAsm->bInstrLen = 0;                // Zero out instruction len
    pAsm->error = ASMERR_NOERROR;       // Zero out error code (no error)

    // Parse assembly line - first token might be an instruction or a prefix
    pStr = pAsm->szLine;
    if( pStr )
    {
        // Decode the mnemonic token
        pRec = FindAsmRecord(&pStr);

        if( pRec )
        {
            // If it is a prefix, store it immediately
            if( pRec->pAsm->op[0]==ASM_PREFIX )
            {
                AsmAddCode(pAsm, pRec->pAsm->pOp[0]);

                // Get the next mnemonic
                pRec = FindAsmRecord(&pStr);
            }

            // Decode the arguments to instruction as given in the input string
            memset((void*)&Eval, 0, sizeof(Eval));
            memset((void*)&args, 0, sizeof(args));
            Eval.pArgs = args;
            Eval.szAsm = pStr;
            Eval.dwEvalFlags = 0;
            AsmParamEval(&Eval);

            if( Eval.error )
                pAsm->error = error;

            // Match instruction opcodes
            pAddr = AsmMatch(pAsm, args, pRec);

            if( pAddr )
            {
                // Decode the address mode and store final opcodes
                AsmCodeGen(pAsm, &Eval, pAddr);
            }
            else
                pAsm->error = 255;
        }
    }

    if( pAsm->error )
        pAsm->bInstrLen = 0;

    return( pAsm->bInstrLen );
}
