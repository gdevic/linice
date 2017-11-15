/******************************************************************************
*                                                                             *
*   Module:     disassembler.c                                                *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       3/17/2000                                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the generic Intel disassembler.
        The latest supported uprocessor is Pentium Pro.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 3/17/2000  Original                                             Goran Devic *
* 4/26/2000  Major rewrite, added coprocessor instructions.       Goran Devic *
* 11/4/2000  Modified for LinIce                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "ice.h"                        // Include global structures

#include "i386.h"                       // Include assembly code

#include "disassemblerdata.h"           // Include its own data

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


BYTE test[10] = 
{
    0x3C, 0xAA
};



/******************************************************************************
*                                                                             *
*   External functions (optional)                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static BYTE GetNextByte(WORD sel, BYTE *offset, BYTE *pCode)
{
    pCode[0] = GetByte((DWORD) offset + 0) & 0xFF;

    return( pCode[0] );
}    

static WORD GetNextWord(WORD sel, BYTE *offset, BYTE *pCode)
{
    pCode[0] = GetByte((DWORD) offset + 0) & 0xFF;
    pCode[1] = GetByte((DWORD) offset + 1) & 0xFF;

    return( *(WORD *) pCode );
}    

static DWORD GetNextDword(WORD sel, BYTE *offset, BYTE *pCode)
{
    pCode[0] = GetByte((DWORD) offset + 0) & 0xFF;
    pCode[1] = GetByte((DWORD) offset + 1) & 0xFF;
    pCode[2] = GetByte((DWORD) offset + 2) & 0xFF;
    pCode[3] = GetByte((DWORD) offset + 3) & 0xFF;

    return( *(DWORD *) pCode );
}    


#define NEXTBYTE    GetNextByte( pDis->wSel, bpTarget, bpCode); bpCode += 1; bpTarget += 1; bInstrLen += 1

#define NEXTWORD    GetNextWord( pDis->wSel, bpTarget, bpCode); bpCode += 2; bpTarget += 2; bInstrLen += 2

#define NEXTDWORD   GetNextDword(pDis->wSel, bpTarget, bpCode); bpCode += 4; bpTarget += 4; bInstrLen += 4


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BYTE Disassembler( TDisassembler *pDis );                                 *
*                                                                             *
*******************************************************************************
*
*   This is a generic Intel line disassembler.
*
*   Where:
*       TDisassembler:
*           bpTarget is the address of instruction to disassemble
*           szDisasm is the address of the buffer to print a line into
*           dwFlags contains the default operand and address bits
*           pCode is the address to store code bytes (up to 16)
*
*   Disassembled instruction is stored as an ASCIIZ string pointed by
*   szDisasm pointer (from the pDis structure).
*
*   Returns:
*       TDisassembler:
*           *szDisasm contains the disassembled instruction string
*           bAsciiLen is set to the length of the printed string
*           bInstrLen is set to instruction length in bytes
*           dwFlags - has operand and address size flags adjusted
*                   - DIS_ILLEGALOP set if that was illegal instruction
*       BYTE - instruction length in bytes
*
******************************************************************************/
BYTE Disassembler( TDisassembler *pDis )
{
    TOpcodeData *p;             // Pointer to a current instruction record
    BYTE   *bpTarget;           // Pointer to the target code to be disassembled
    BYTE   *bpCode;             // Pointer to code bytes
    DWORD   arg;                // Argument counter
    char   *sPtr;               // Message selection pointer
    int     nPos;               // Printing position in the output string
    BYTE   *pArg;               // Pointer to record where instruction arguments are
    DWORD   dwDword;            // Temporary dword storage
    WORD    wWord;              // Temporary word storage
    BYTE    bByte;              // Temporary byte storage
    BYTE    bInstrLen;          // Current instruction lenght in bytes
    BYTE    bOpcode;            // Current opcode that is being disassembled
    BYTE    bSegOverride;       // 0 default segment. >0, segment index
    BYTE    bMod;               // Mod field of the instruction
    BYTE    bReg;               // Register field of the instruction
    BYTE    bRm;                // R/M field of the instruction
    BYTE    bW;                 // Width bit for the register selection

    BYTE    bSib;               // S-I-B byte for the instruction
    BYTE    bSs;                // SS field of the s-i-b byte
    BYTE    bIndex;             // Index field of the s-i-b byte
    BYTE    bBase;              // Base field of the s-i-b byte


    bInstrLen = 0;              // Reset instruction lenght to zero
    bSegOverride = 0;           // Set default segment (no override)
    nPos = 0;                   // Reset printing position
    sPtr = NULL;                // Points to no message by default
    bpTarget = pDis->bpTarget;  // Set internal pointer to a target address
    bpCode = pDis->pCode;       // Set internal pointer to code bytes

    do
    {
        bOpcode = NEXTBYTE;     // Get the first opcode byte from the target address
        p = &Op1[bOpcode];      // Get the address of the instruction record

        if( p->flags & DIS_SPECIAL )
        {
            // Opcode is one of the special ones, so do what needs to be done there

            switch( p->name )
            {
                case _EscD8:
                case _EscD9:
                case _EscDA:
                case _EscDB:
                case _EscDC:
                case _EscDD:
                case _EscDE:
                case _EscDF:        // Coprocessor escape: bytes D8 - DF
                    bOpcode = NEXTBYTE;             // Get the modRM byte of the instruction

                    if( bOpcode < 0xC0 )
                    {
                        // Opcodes 00-BF use Coproc1 table

                        bReg = (bOpcode >> 3) & 7;
                        p = &Coproc1[ p->name - _EscD8 ][ bReg ];

                        goto StartInstructionParseMODRM;
                    }
                    // Opcodes C0-FF use Coproc2 table

                    p = &Coproc2[ p->name - _EscD8 ][ bOpcode - 0xC0 ];

                goto StartInstructionNoMODRM;

                case _S_ES:         // Segment override
                case _S_CS:
                case _S_SS:
                case _S_DS:
                case _S_FS:
                case _S_GS:
                    bSegOverride = p->name - _S_ES + 1;
                continue;

                case _OPSIZ:        // Operand size override - toggle
                    pDis->dwFlags ^= DIS_DATA32;
                continue;

                case _ADSIZ:        // Address size override - toggle
                    pDis->dwFlags ^= DIS_ADDRESS32;
                continue;

                case _REPNE:        // REPNE/REPNZ prefix
                    pDis->dwFlags |= DIS_REPNE;
                continue;

                case _REP:          // REP/REPE/REPZ prefix
                    pDis->dwFlags |= DIS_REP;
                continue;

                case _2BESC:        // 2 byte escape code 0x0F
                    bOpcode = NEXTBYTE;             // Get the second byte of the instruction
                    p = &Op2[bOpcode];              // Get the address of the instruction record

                    if( !(p->flags & DIS_SPECIAL) ) goto StartInstruction;
                    if( p->name < _GRP6 ) goto IllegalOpcode;

                case _GRP1a:        // Additional groups of instructions
                case _GRP1b:
                case _GRP1c:
                case _GRP2a:
                case _GRP2b:
                case _GRP2c:
                case _GRP2d:
                case _GRP2e:
                case _GRP2f:
                case _GRP3a:
                case _GRP3b:
                case _GRP4:
                case _GRP5:
                case _GRP6:
                case _GRP7:
                case _GRP8:
                case _GRP9:

                    bOpcode = NEXTBYTE;             // Get the Mod R/M byte whose...
                                                    // bits 3,4,5 select instruction

                    bReg = (bOpcode >> 3) & 7;
                    p = &Groups[p->name - _GRP1a][ bReg ];

                    if( !(p->flags & DIS_SPECIAL) ) goto StartInstructionParseMODRM;

                case _NDEF :        // Not defined or illegal opcode
                    goto IllegalOpcode;

                default :;          // Should not happen
            }
        }
        else
            goto StartInstruction;
    }
    while( bInstrLen < 15 );

IllegalOpcode:

    nPos += ice_sprintf( pDis->szDisasm+nPos, "invalid");
    pDis->dwFlags |= DIS_ILLEGALOP;

    goto DisEnd;

StartInstruction:

    // If this instruction needs additional Mod R/M byte, fetch it

    if( p->flags & DIS_MODRM )
    {
        // Get the next byte (modR/M bit field)
        bOpcode = NEXTBYTE;

        bReg = (bOpcode >> 3) & 7;

StartInstructionParseMODRM:

        // Parse that byte and get mod, reg and rm fields
        bMod = bOpcode >> 6;
        bRm  = bOpcode & 7;
    }

StartInstructionNoMODRM:

    // Print the possible repeat prefix followed by the instruction

    if( p->flags & DIS_COPROC )
        nPos += ice_sprintf( pDis->szDisasm+nPos, "%-6s ", sCoprocNames[ p->name ]);
    else
        nPos += ice_sprintf( pDis->szDisasm+nPos, "%s%-6s ",
                sRep[DIS_GETREPENUM(pDis->dwFlags)],
                sNames[ p->name + (DIS_GETNAMEFLAG(p->flags) & DIS_GETDATASIZE(pDis->dwFlags)) ] );

    // Do instruction argument processing, up to 3 times

    pArg = &p->dest;

    for( arg=p->args; arg!=0; arg--, pArg++, arg? nPos += ice_sprintf( pDis->szDisasm+nPos,", ") : 0 )
    {
        switch( *pArg )
        {
             case _Eb :                                         // modR/M used - bW = 0
                bW = 0;
                goto _E;

             case _Ev :                                         // modR/M used - bW = 1
                bW = 1;
                goto _E;

             case _Ew :                                         // always word size
                pDis->dwFlags &= ~DIS_DATA32;
                bW = 1;
                goto _E;

             case _Ms :                                         // fword ptr (sgdt,sidt,lgdt,lidt)
                sPtr = sFwordPtr;
                goto _E1;

             case _Mq :                                         // qword ptr (cmpxchg8b)
                sPtr = sQwordPtr;
                goto _E1;

             case _Mp :                                         // 32 or 48 bit pointer (les,lds,lfs,lss,lgs)
             case _Ep :                                         // Always a memory pointer (call, jmp)
                if( pDis->dwFlags & DIS_DATA32 )
                    sPtr = sFwordPtr;
                else
                    sPtr = sDwordPtr;
                goto _E1;

             _E:
                 // Do registers first so that the rest may be done together
                 if( bMod == 3 )
                 {
                      // Registers depending on the w field and data size
                      nPos+=ice_sprintf(pDis->szDisasm+nPos, "%s", sRegs1[DIS_GETDATASIZE(pDis->dwFlags)][bW][bRm] );

                      break;
                 }

                 if( bW==0 )
                     sPtr = sBytePtr;
                 else
                     if( pDis->dwFlags & DIS_DATA32 )
                         sPtr = sDwordPtr;
                     else
                         sPtr = sWordPtr;

             case _M  :                                         // Pure memory pointer (lea,invlpg,floats)
                if( bMod == 3 ) goto IllegalOpcode;

             _E1:

                 if( sPtr )
                     nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sPtr );

             case _Ma :                                         // Used by bound instruction, skip the pointer info

                 // Print the segment if it is overriden
                 //
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"%s", sSegOverride[ bSegOverride ] );

                 //
                 // Special case when sib byte is present in 32 address encoding
                 //
                 if( (bRm==4) && (pDis->dwFlags & DIS_ADDRESS32) )
                 {
                      //
                      // Get the s-i-b byte and parse it
                      //
                      bSib = NEXTBYTE;

                      bSs = bSib >> 6;
                      bIndex = (bSib >> 3) & 7;
                      bBase = bSib & 7;

                      // Special case for base=5 && mod==0 -> fetch 32 bit offset
                      if( (bBase==5) && (bMod==0) )
                      {
                          dwDword = NEXTDWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"[%08X", (unsigned int) dwDword );
                      }
                      else
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"[%s", sGenReg16_32[ 1 ][ bBase ] );

                      // Scaled index, no index if bIndex is 4
                      if( bIndex != 4 )
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"+%s%s", sScale[ bSs ], sGenReg16_32[ 1 ][ bIndex ] );
                      else
                          if(bSs != 0)
                              nPos += ice_sprintf( pDis->szDisasm+nPos,"<INVALID MODE>" );

                      // Offset 8 bit or 32 bit
                      if( bMod == 1 )
                      {
                          bByte = NEXTBYTE;
                          if( (signed char)bByte < 0 )
                                 nPos += ice_sprintf( pDis->szDisasm+nPos,"-%02X", 0-(signed char)bByte );
                          else
                                 nPos += ice_sprintf( pDis->szDisasm+nPos,"+%02X", bByte );
                      }

                      if( bMod == 2 )
                      {
                          dwDword = NEXTDWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"+%08X", (unsigned int) dwDword );
                      }

                      // Wrap up the instruction
                      nPos += ice_sprintf( pDis->szDisasm+nPos,"]" );
                      break;
                 }

                 //
                 // 16 or 32 address bit cases with mod zero, one or two
                 //
                 // Special cases when r/m is 5 and mod is 0, immediate d16 or d32
                 if( bMod==0 && ((bRm==6 && !(pDis->dwFlags & DIS_ADDRESS32)) || (bRm==5 && (pDis->dwFlags & DIS_ADDRESS32))) )
                 {
                      if( pDis->dwFlags & DIS_ADDRESS32 )
                      {
                          dwDword = NEXTDWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"[%08X]", (unsigned int) dwDword );
                      }
                      else
                      {
                          wWord = NEXTWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"[%04X]", wWord );
                      }

                      break;
                 }

                 // Print the start of the line
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"[%s", sAdr1[DIS_GETADDRSIZE(pDis->dwFlags)][ bRm ] );

                 // Offset (8 or 16) or (8 or 32) bit - 16, 32 bits are unsigned
                 if( bMod==1 )
                 {
                      bByte = NEXTBYTE;
                      if( (signed char)bByte < 0 )
                             nPos += ice_sprintf( pDis->szDisasm+nPos,"-%02X", 0-(signed char)bByte );
                      else
                             nPos += ice_sprintf( pDis->szDisasm+nPos,"+%02X", bByte );
                 }

                 if( bMod==2 )
                 {
                      if( pDis->dwFlags & DIS_ADDRESS32 )
                      {
                          dwDword = NEXTDWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"+%08X", (unsigned int) dwDword );
                      }
                      else
                      {
                          wWord = NEXTWORD;
                          nPos += ice_sprintf( pDis->szDisasm+nPos,"+%04X", wWord );
                      }
                 }

                 // Wrap up the instruction
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"]" );

             break;

             case _Gb :                                         // general, byte register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sRegs1[0][0][ bReg ] );
             break;

             case _Gv :                                         // general, (d)word register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sGenReg16_32[DIS_GETDATASIZE(pDis->dwFlags)][ bReg ] );
             break;

             case _Yb :                                         // ES:(E)DI pointer
             case _Yv :
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s%s", sSegOverrideDefaultES[ bSegOverride ], sYptr[DIS_GETADDRSIZE(pDis->dwFlags)] );
             break;

             case _Xb :                                         // DS:(E)SI pointer
             case _Xv :
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s%s", sSegOverrideDefaultDS[ bSegOverride ], sXptr[DIS_GETADDRSIZE(pDis->dwFlags)] );
             break;

             case _Rd :                                         // general register double word
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sGenReg16_32[ 1 ][ bRm ] );
             break;

             case _Rw :                                         // register word
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sGenReg16_32[ 0 ][ bMod ] );
             break;

             case _Sw :                                         // segment register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sSeg[ bReg ] );
             break;

             case _Cd :                                         // control register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sControl[ bReg ] );
             break;

             case _Dd :                                         // debug register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sDebug[ bReg ] );
             break;

             case _Td :                                         // test register
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sTest[ bReg ] );
             break;


             case _Jb :                                         // immediate byte, relative offset
                 bByte = NEXTBYTE;
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "short %08X", (unsigned int)(pDis->bpTarget + (signed char)bByte + bInstrLen) );
             break;

             case _Jv :                                         // immediate word or dword, relative offset
                 if( pDis->dwFlags & DIS_DATA32 )
                 {
                      dwDword = NEXTDWORD;
                      nPos += ice_sprintf( pDis->szDisasm+nPos, "%08X", (unsigned int)(pDis->bpTarget + (signed long)dwDword + bInstrLen) );
                 }
                 else
                 {
                     wWord = NEXTWORD;
                     nPos += ice_sprintf( pDis->szDisasm+nPos, "%08X", (unsigned int)(pDis->bpTarget + (signed short)wWord + bInstrLen) );
                 }
             break;

             case _O  :                                         // Simple word or dword offset
                  if( pDis->dwFlags & DIS_ADDRESS32 )           // depending on the address size
                  {
                      dwDword = NEXTDWORD;
                      nPos += ice_sprintf( pDis->szDisasm+nPos,"%s[%08X]", sSegOverride[ bSegOverride ], (unsigned int) dwDword );
                  }
                  else
                  {
                      wWord = NEXTWORD;
                      nPos += ice_sprintf( pDis->szDisasm+nPos,"%s[%04X]", sSegOverride[ bSegOverride ], wWord );
                  }
             break;

             case _Ib :                                         // immediate byte
                 bByte = NEXTBYTE;
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"%02X", bByte );
             break;

             case _Iv :                                         // immediate word or dword
                 if( pDis->dwFlags & DIS_DATA32 )
                 {
                      dwDword = NEXTDWORD;
                      nPos += ice_sprintf( pDis->szDisasm+nPos, "%08X", (unsigned int) dwDword );
                 }
                 else
                 {
                     wWord = NEXTWORD;
                     nPos += ice_sprintf( pDis->szDisasm+nPos, "%04X", wWord );
                 }
             break;

             case _Iw :                                         // Immediate word
                 wWord = NEXTWORD;
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%04X", wWord );
             break;

             case _Ap :                                         // 32 bit or 48 bit pointer (call far, jump far)
                 if( pDis->dwFlags & DIS_DATA32 )
                 {
                      dwDword = NEXTDWORD;
                      wWord = NEXTWORD;
                      nPos += ice_sprintf( pDis->szDisasm+nPos, "%04X:%08X", wWord, (unsigned int) dwDword );
                 }
                 else
                 {
                     dwDword = NEXTDWORD;
                     nPos += ice_sprintf( pDis->szDisasm+nPos, "%08X", (unsigned int) dwDword );
                 }
             break;

             case _1 :                                          // numerical 1
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"1" );
             break;

             case _3 :                                          // numerical 3
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"3" );
             break;

                                                                // Hard coded registers
             case _DX: case _AL: case _AH: case _BL: case _BH: case _CL: case _CH:
             case _DL: case _DH: case _CS: case _DS: case _ES: case _SS: case _FS:
             case _GS:
                 nPos += ice_sprintf( pDis->szDisasm+nPos,"%s", sRegs2[ *pArg - _DX ] );
             break;

             case _eAX: case _eBX: case _eCX: case _eDX:
             case _eSP: case _eBP: case _eSI: case _eDI:
                 nPos += ice_sprintf( pDis->szDisasm+nPos, "%s", sGenReg16_32[DIS_GETDATASIZE(pDis->dwFlags)][ *pArg - _eAX ]);
             break;

             case _ST:                                          // Coprocessor ST
                nPos += ice_sprintf( pDis->szDisasm+nPos,"%s", sST[9] );
             break;

            case _ST0:                                         // Coprocessor ST(0) - ST(7)
            case _ST1:
            case _ST2:
            case _ST3:
            case _ST4:
            case _ST5:
            case _ST6:
            case _ST7:
               nPos += ice_sprintf( pDis->szDisasm+nPos,"%s", sST[ *pArg - _ST0 ] );
            break;

            case _AX:                                           // Coprocessor AX
                nPos += ice_sprintf( pDis->szDisasm+nPos,"%s", sGenReg16_32[0][0] );
            break;
        }
    }

DisEnd:

    // Set the returning values and return with the bInstrLen field

    pDis->bAsciiLen = (BYTE) nPos;
    pDis->bInstrLen = bInstrLen;

    return bInstrLen;
}

