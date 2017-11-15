/******************************************************************************
*                                                                             *
*   Module:     asmeval.c                                                     *
*                                                                             *
*   Date:       3/23/2002                                                     *
*                                                                             *
*   Copyright (c) 1997, 2002 Goran Devic                                      *
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

        This is a operand expression evaluator for line assembler.

*******************************************************************************

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/23/02   Modified from eval.c                                 Goran Devic *
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
#include <ctype.h>

#include "assembler.h"                  // Include interface header file - remove me!!

#include "assemblerdefines.h"           // Include assembler shared structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int nEvalDefaultBase = 16;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static TASMEVAL *pEval;                 // Current eval structure
static TASMARG *pArg;                   // Current argument to process
static int error;                       // Current error code

#define MEM_HAVE_BASE       0x00000001  // Base register defined
#define MEM_HAVE_INDEX      0x00000002  // Index register defined


// Define node that contains operands

typedef struct
{
    DWORD dwNodeFlags;                  // Flags containing the node qualifier
    DWORD value;                        // Node value
    
} TNODE;

#define NODE_VALUE          ASM_IMM     // Node contains numerical value
#define NODE_REG            ASM_REG     // Node contains CPU register index


#define MAX_TOKEN_LEN       80          // Max length of a token

static char sToken[MAX_TOKEN_LEN];



#define MAX_RECURSE     5               // Max literal reentrancy depth

#define MAX_STACK       10              // Max depth of a stack structure

#define BOTTOM_STACK    0               // Stack empty code

#define OP_MEMREF       1               // Memory reference [
#define OP_MEMREFEND    2               // End of memory reference ]
#define OP_PAREN_START  3               // Priority codes and also indices
#define OP_PAREN_END    4               //   into the sTokens array
#define OP_SEGMENT      5
#define OP_OFFSET       6
#define OP_TIMES        7
#define OP_DIV          8
#define OP_MOD          9
#define OP_SHL          10
#define OP_SHR          11
#define OP_PLUS         12
#define OP_MINUS        13
#define OP_EQ           14
#define OP_NE           15
#define OP_L            16
#define OP_LE           17
#define OP_G            18
#define OP_GE           19
#define OP_NOT          20
#define OP_AND          21
#define OP_OR           22
#define OP_XOR          23
#define OP_NEG          255             // Unary minus has highest priority


//-------------- OPERATORS ----------------

typedef struct
{
    char *sToken;                       // Operator string name
    BYTE len;                           // length of the operator string

} TOPERATOR;

static TOPERATOR sOperator[] = {
    { "[", 1 },     { "]", 1 },
    { "(", 1 },     { ")", 1 },
    { ":", 1 },
    { "offset ", 7},
    { "*", 1 },     { "/", 1 },     { "mod ", 4 }, { "shl ", 4 }, { "shr ", 4 },
    { "+", 1 },     { "-", 1 },
    { "eq ", 3 },   { "ne ", 3 },   { "lt ", 3 },  { "le ", 3 },  { "gt ", 3 },  { "ge ", 3 },
    { "not ", 4 },
    { "and ", 4 },
    { "or ", 3 },   { "xor ", 4 },
    { NULL }
};

//-------------- OPERANDS ----------------

typedef struct
{
    char *sName;                        // String operand name
    DWORD dwFlags;                      // Operand flags / properties
    BYTE ref;                           // Reference ID byte
    
} TOPERAND;


#define ASM_SPECIAL     0xFF            // Special token

static TOPERAND sOperand[] = {
    { "al", ASM_RM + ASM_REG + ASM_BYTE + ASM_AL, 0 },
    { "cl", ASM_RM + ASM_REG + ASM_BYTE + ASM_CL, 1 },
    { "dl", ASM_RM + ASM_REG + ASM_BYTE, 2 },
    { "bl", ASM_RM + ASM_REG + ASM_BYTE, 3 },
    { "ah", ASM_RM + ASM_REG + ASM_BYTE, 4 },
    { "ch", ASM_RM + ASM_REG + ASM_BYTE, 5 },
    { "dh", ASM_RM + ASM_REG + ASM_BYTE, 6 },
    { "bh", ASM_RM + ASM_REG + ASM_BYTE, 7 },

    { "ax", ASM_RM + ASM_REG + ASM_WORD + ASM_AX, 0 },
    { "cx", ASM_RM + ASM_REG + ASM_WORD + ASM_CX, 1 },
    { "dx", ASM_RM + ASM_REG + ASM_WORD + ASM_DX, 2 },
    { "bx", ASM_RM + ASM_REG + ASM_WORD, 3 },
    { "sp", ASM_RM + ASM_REG + ASM_WORD, 4 },
    { "bp", ASM_RM + ASM_REG + ASM_WORD, 5 },
    { "si", ASM_RM + ASM_REG + ASM_WORD, 6 },
    { "di", ASM_RM + ASM_REG + ASM_WORD, 7 },

    { "eax", ASM_RM + ASM_REG + ASM_DWORD + ASM_EAX, 0 },
    { "ecx", ASM_RM + ASM_REG + ASM_DWORD + ASM_ECX, 1 },
    { "edx", ASM_RM + ASM_REG + ASM_DWORD, 2 },
    { "ebx", ASM_RM + ASM_REG + ASM_DWORD, 3 },
    { "esp", ASM_RM + ASM_REG + ASM_DWORD, 4 },
    { "ebp", ASM_RM + ASM_REG + ASM_DWORD, 5 },
    { "esi", ASM_RM + ASM_REG + ASM_DWORD, 6 },
    { "edi", ASM_RM + ASM_REG + ASM_DWORD, 7 },

    { "es", ASM_REG + ASM_SR + ASM_WORD + ASM_DESS, 0 },
    { "cs", ASM_REG + ASM_SR + ASM_WORD, 1 },
    { "ss", ASM_REG + ASM_SR + ASM_WORD + ASM_DESS, 2 },
    { "ds", ASM_REG + ASM_SR + ASM_WORD + ASM_DESS, 3 },
    { "fs", ASM_REG + ASM_SR + ASM_WORD + ASM_FSGS, 4 },
    { "gs", ASM_REG + ASM_SR + ASM_WORD + ASM_FSGS, 5 },
    
    { "cr0", ASM_REG + ASM_CR + ASM_DWORD, 0 },
    { "cr2", ASM_REG + ASM_CR + ASM_DWORD, 2 },
    { "cr3", ASM_REG + ASM_CR + ASM_DWORD, 3 },

    { "dr0", ASM_REG + ASM_DR + ASM_DWORD, 0 },
    { "dr1", ASM_REG + ASM_DR + ASM_DWORD, 1 },
    { "dr2", ASM_REG + ASM_DR + ASM_DWORD, 2 },
    { "dr3", ASM_REG + ASM_DR + ASM_DWORD, 3 },
    { "dr6", ASM_REG + ASM_DR + ASM_DWORD, 6 },
    { "dr7", ASM_REG + ASM_DR + ASM_DWORD, 7 },

    { "tr3", ASM_REG + ASM_TR + ASM_DWORD, 3 },
    { "tr4", ASM_REG + ASM_TR + ASM_DWORD, 4 },
    { "tr5", ASM_REG + ASM_TR + ASM_DWORD, 5 },
    { "tr6", ASM_REG + ASM_TR + ASM_DWORD, 6 },
    { "tr7", ASM_REG + ASM_TR + ASM_DWORD, 7 },

    { "byte", ASM_SPECIAL, 0 },
    { "word", ASM_SPECIAL, 1 },
    { "dword",ASM_SPECIAL, 2 },
    { "ptr",  ASM_SPECIAL, 3 },
    { "far",  ASM_SPECIAL, 4 },
    { "near", ASM_SPECIAL, 5 },
    { NULL }
};

//-------------- QUALIFIERS ----------------

typedef struct
{
    char *sName;                        // String qualifier name
    BYTE ref;                           // Reference byte
    
} TQUAL2;


// Define stack structure for operands

typedef struct
{
    TNODE Data[ MAX_STACK ];            // Stack data
//    int Data[ MAX_STACK ];              // Stack data
    int Top;                            // Top of stack index
} TStack;

static TStack Stack;

// Define stack structure for operators

typedef struct
{
    BYTE Data[ MAX_STACK ];
    int Top;
} TStackOp;

static TStackOp StackOp;

static const char sDelim[] = ",;\"";    // Expressions delimiters - break chars
static const char sLiteral[] = "@!_";   // These are allowed in literal names

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL PushNode(TNODE *pNode)
{
    if(Stack.Top<MAX_STACK)
    {
        Stack.Data[Stack.Top].value = pNode->value;
        Stack.Data[Stack.Top].dwNodeFlags = pNode->dwNodeFlags;
        Stack.Top++;
        return(TRUE);
    }

    return(FALSE);
}

BOOL PopNode(TNODE *pNode)
{
    if(Stack.Top)
    {
        Stack.Top--;
        pNode->value = Stack.Data[Stack.Top].value;
        pNode->dwNodeFlags = Stack.Data[Stack.Top].dwNodeFlags;
        return(TRUE);
    }

    return(FALSE);
}

BOOL Push(BYTE bOp)
{
    if(StackOp.Top<MAX_STACK)
    {
        StackOp.Data[StackOp.Top] = bOp;
        StackOp.Top++;
        return(TRUE);
    }
    return(FALSE);
}

BOOL Pop(BYTE *pOp)
{
    if(StackOp.Top)
    {
        StackOp.Top--;
        *pOp = StackOp.Data[StackOp.Top];
        return(TRUE);
    }
    return(FALSE);
}

BOOL GetToken(char **ppStr)
{
    char *pDest = sToken;               // Destination token buffer
    char *p = *ppStr;                   // Get the pointer to strings

    while( *p && *p==' ' ) p++;         // Skip spaces

    if( *p==0 || *p==';' || *p==',' )   // If end of string, or a comment or next argument
    {
        *ppStr = p;                     // Store the final pointer
        return( FALSE );                // No more characters to take
    }
    else
    {
        do
        {
            *pDest++ = *p++;            // Copy a character
        }
        while( *p && (isalnum(*p) || *p=='_') );
    }

    *pDest = 0;                         // Zero terminate the token buffer
    *ppStr = p;                         // Store the new source string pointer

    return( TRUE );
}

int TableMatch(char **ppString)
{
    TOPERATOR *pOp;                     // Pointer to array of operator descriptors
    int opId;                           // Operator id number
    char *pStr;                         // Pointer to string

    pStr = *ppString;
    pOp = &sOperator[0];
    opId = 1;

    while(pOp->sToken)
    {
        if(!strncmp(pOp->sToken, pStr, pOp->len))
        {
            *ppString += pOp->len;
            return(opId);
        }
        pOp++;
        opId++;
    }

    return(0);
}

TOPERAND *TokenSearch()
{
    TOPERAND *p;

    p = &sOperand[0];
    while(p->sName)
    {
        if(!strcmp(p->sName, sToken))
        {
            return(p);
        }
        p++;
    }
    return(NULL);
}

DWORD GetDec(char **psString)
{
    char *ptr = *psString;
    char digit;
    DWORD value = 0;

    while(isdigit(digit = *ptr++))
    {
        value *= 10;
        value += digit - '0';
    }

    *psString = ptr - 1;
    return(value);
}

DWORD GetHex(char **psString)
{
    char *ptr = *psString;
    char nibble;
    int count = 8;
    DWORD value = 0;

    while(isxdigit(nibble = *ptr++) && count--)
    {
        nibble = tolower(nibble);
        value <<= 4;
        value |= (nibble > '9')? nibble - 'a' + 10: nibble - '0';
    }

    if(count==0)
    {
        // Value is too large: (%s)
    }

    *psString = ptr - 1;
    return(value);
}

BOOL fDecimal = FALSE;

static int GetValue( char **sExpr )
{
    int value, n;
    char *sStart = *sExpr, *sTmp;

    // If we switched to decimal radix, try a decimal number first
    if( fDecimal==TRUE )
    {
        sTmp = sStart;
        value = GetDec(&sStart);
        // If we end up with a hex digit, the number was not decimal
        if(!isxdigit(*sStart))
            goto End;
        sStart = sTmp;  // so revert the string and try defaults..
    }

    // Check if the first two charcaters represent a hex number
    if( *sStart=='0' && tolower(*(sStart+1))=='x' )         // 0xHEX
    {
        sStart += 2;
        value = GetHex(&sStart);
    }
    else
    // Check for character constants: '1', '12', '123', '1234'
    if( *sStart=='\'' )
    {
        value = 0;
        n = 4;
        sStart++;
        while(*sStart && *sStart!='\'' && n--)
        {
            value <<= 8;
            value |= *sStart++;
        }
    }
    else
    // Check if the first two characters represent a character literal
    if( *sStart=='\\' && *(sStart+1)=='\'' )       // '\DEC' or '\xHEX'
    {
        if( *(sStart+2)!='x' || *(sStart+2)!='X' )          // HEX
        {
            value = GetHex(&sStart);
        }
        else
        {
            sStart += 1;
            value = GetDec(&sStart);
        }
    }
    else
    // If everything else fails, it's gotta be a hex number
    {
        value = GetHex(&sStart);
    }
End:

    // Assume it is an immediate value. That is going to be cleared
    // if any other addressing mode is detected
    // TDOD

    pArg->dwFlags |= ASM_IMM;

    fDecimal = FALSE;
    *sExpr = sStart;

    return( value );
}


void AddIndex(TNODE *pNode)
{
    if( pArg->dwExtFlags & EXT_HAVE_INDEX )    
        error = 100;                                    // TODO
    else
    {
        pArg->dwFlags |= pNode->dwNodeFlags;
        pArg->dwExtFlags |= EXT_HAVE_INDEX;
        pArg->index = pNode->value;
    }
}

void AddReg(TNODE *pNode)
{
    if(pArg->dwExtFlags & EXT_HAVE_REG)
        AddIndex(pNode);
    else
    {
        pArg->dwFlags |= pNode->dwNodeFlags;
        pArg->dwExtFlags |= EXT_HAVE_REG;
        pArg->reg = pNode->value;
    }
}

void AddSeg(DWORD seg)
{
    if( pEval->dwEvalFlags & EFL_HAVE_SEG )
        error = 105;                                    // TODO
    else
    {
        pEval->dwEvalFlags |= EFL_HAVE_SEG;
        pEval->seg = seg;
    }
}

void AddValue(TNODE *pNode)
{
    pArg->dwFlags |= pNode->dwNodeFlags;
    pArg->value = pNode->value;

    if( pArg->value < 256 )
        pArg->dwFlags |= ASM_BYTE;
//    else
    if( pArg->value < 65536 )
        pArg->dwFlags |= ASM_WORD;
//    else
        pArg->dwFlags |= ASM_DWORD;

    // Special case of 1
    if( pArg->value==1 )
        pArg->dwFlags |= ASM_1;
}

/******************************************************************************
*                                                                             *
*   void Execute( TStack *Values, int Operation )                             *
*                                                                             *
*******************************************************************************
*
*   Evaluates numbers on the stack depending on the operation opcode.
*
*   Where:
*       Values stack
*       Operation is a code of the operation to be performed on a stack values.
*
*   Returns:
*       Operates on stacks Values and Operators and updates them accordingly.
*
******************************************************************************/
static void Execute( BYTE bOp )
{
    TNODE Op1, Op2;
    Op1.dwNodeFlags = Op2.dwNodeFlags = 0;

    if( bOp <= OP_OFFSET )
    {
        pArg->dwFlags |= ASM_MEM;
        return;
    }

    // Pop 1 operand from the stack
    PopNode(&Op1);

    // All operations pop at least 1 operand from the stack, and many of them pop 2
    if(bOp!=OP_NOT)
    {
        PopNode(&Op2);
    }
    else
        Op2.dwNodeFlags = 0;

    // All operations can be performed on numbers, but only some of them can
    // have 1 or 2 register arguments:
    //       value + reg    (assign reg + value)
    //       reg   + reg    (assign reg + index)
    //       reg   * scale  (assign index and scale factor)
    switch(bOp)
    {
        case OP_PLUS:
            if(Op1.dwNodeFlags & NODE_VALUE && Op2.dwNodeFlags & NODE_VALUE)
            {
                // Regular case where both operands are numerics, we add them
                Op1.value += Op2.value;
            }
            else
            {   
                if( Op1.dwNodeFlags & NODE_REG && Op2.dwNodeFlags & NODE_REG )
                {
                    // Both operands are registers.. Store one and push the other one
                    AddReg(&Op2);
                    pArg->dwFlags |= ASM_MEM;

                    // Op1 will be pushed at the end
                }
                else
                    if( Op1.dwNodeFlags & NODE_VALUE )
                    {
                        // Op1 is value, Op2 is a register.. Store register
                        AddReg(&Op2);
                        pArg->dwFlags |= ASM_MEM;

                        // Op1 will be pushed at the end
                    }
                    else
                    {
                        // Op1 is a register, Op2 is a value, Store register
                        AddReg(&Op1);
                        pArg->dwFlags |= ASM_MEM;

                        PushNode(&Op2);
                        return;
                    }
            }
            break;

        case OP_TIMES:
            if(Op1.dwNodeFlags & NODE_VALUE && Op2.dwNodeFlags & NODE_VALUE)
            {
                // Both operands are values.. Multiply them and push Op1 later
                Op1.value *= Op2.value;
            }
            else
            {
#if 0
                // Only 1 argument can be a register.. The other has to be factor 1, 2, 4 or 8
                if(Op1.dwNodeFlags & NODE_REG)
                {
                    pOpReg = &Op1;
                    pOpFact = &Op2;
                }
                else
                {
                    pOpReg = &Op2;
                    pOpFact = &Op1;
                }

                if(pOpFact->dwNodeFlags & NODE_REG)
                    error = 101;                                    // TODO

                if(pOpFact->value==1 || pOpFact->value==2 || pOpFact->value==4 || pOpFact->value==8)
                {
                    pArg->scale = pOpFact->value;
                }
                else
                    error = 102;                                    // TODO

                // Store index register into index field
                AddIndex(pOpReg);
                pArg->dwFlags |= ASM_MEM;
#endif
            }
            break;

            // For all other operations, we can't have registers
        default:
            if(Op1.dwNodeFlags & NODE_REG || Op2.dwNodeFlags & NODE_REG)
                return;
                error = 103;                                        // TODO
    }

    // Perform the operation
    switch(bOp)
    {
        case OP_SEGMENT:    AddSeg(Op1.value);
                                return;
                            break;
        case OP_DIV:        if(Op2.value!=0)
                                Op1.value = Op1.value / Op2.value;
                            else
                                error = 106;                        // TODO
                            break;
        case OP_MOD:        if(Op2.value!=0)
                                Op1.value = Op1.value % Op2.value;
                            else
                                error = 107;                        // TODO
                            break;
        case OP_SHL:        Op1.value = Op1.value << Op2.value; break;
        case OP_SHR:        Op1.value = Op1.value >> Op2.value; break;
        case OP_MINUS:      Op1.value = Op1.value -  Op2.value; break;
        case OP_EQ:         Op1.value = Op1.value == Op2.value; break;
        case OP_NE:         Op1.value = Op1.value != Op2.value; break;
        case OP_L:          Op1.value = Op1.value <  Op2.value; break;
        case OP_LE:         Op1.value = Op1.value <= Op2.value; break;
        case OP_G:          Op1.value = Op1.value >  Op2.value; break;
        case OP_GE:         Op1.value = Op1.value >= Op2.value; break;
        case OP_NOT:        Op1.value = !Op1.value; break;
        case OP_AND:        Op1.value = Op1.value &  Op2.value; break;
        case OP_OR:         Op1.value = Op1.value |  Op2.value; break;
        case OP_XOR:        Op1.value = Op1.value ^  Op2.value; break;
    }

    PushNode(&Op1);
}


/******************************************************************************
*                                                                             *
*                                                                             *
*******************************************************************************
*
*
*   Where:
*       sExpr is a pointer to a zero-terminated string containing the
*           expression to be evaluated.
*       psNext is a pointer to a (char *) variable that stores the address
*           of the end of the current expression or the first invalid character
*           encountered.  If NULL, no end is stored.
*
*   Returns:
*       Result of evaluation
*       *psNext, if not NULL, is set to the end of the expression
*
******************************************************************************/
int AsmEvalArg(char **ppExpr )
{
    TOPERAND *pOperand;
    TNODE Node;
    char *pStr;
    BOOL fGetToken;
    BYTE bNewOp, bOldOp;                // New and old operation to perform
    char *pToken;

    pStr = *ppExpr;

    do
    {
        // Eat the spaces
        while(*pStr==' ') pStr++;

        // Check if the string comprise an operator
        bNewOp = TableMatch(&pStr);
        if(bNewOp)
        {
            // If the new op priority is less than the one on the stack, we
            // need to go down and evaluate terms
            if(Pop(&bOldOp))
            {
                if(bNewOp > bOldOp)
                {
                    Execute(bOldOp);
                }
                else
                {
                    Push(bOldOp);
                }
            }

            // Push the new operator on the stack
            Push(bNewOp);
        }
        else
        {
            // Copy a token to the buffer
            fGetToken = GetToken(&pStr);
            if(!fGetToken)
                break;

            // Search the tokens to identify known reserved keywords
            pOperand = TokenSearch();
            if( pOperand!=NULL )
            {
                // Reserved word or register

                if(pOperand->dwFlags == ASM_SPECIAL)
                {
                    // Special keywords that we evaluate immediately
                    switch(pOperand->ref)
                    {
                    case 0:     pArg->dwFlags |= ASM_BYTE; break;   // BYTE
                    case 1:     pArg->dwFlags |= ASM_WORD; break;   // WORD
                    case 2:     pArg->dwFlags |= ASM_DWORD; break;  // DWORD
                    case 3:     pArg->dwFlags |= ASM_MEM; break; // [BYTE | WORD | DWORD] PTR
                    case 4:     pArg->dwFlags |= ASM_FAR;  break;   // FAR [PTR]
                    case 5:     pArg->dwFlags |= ASM_NEAR; break;   // NEAR [PTR]
                    }
                }
                else
                {
                    // CPU register
                    Node.dwNodeFlags = pOperand->dwFlags;
                    Node.value = pOperand->ref;
                    PushNode(&Node);
                }
            }
            else
            {
                // The token is not a reserved word, so it must be a number
                pToken = sToken;
                Node.value = GetValue(&pToken);
                Node.dwNodeFlags = NODE_VALUE;
                PushNode(&Node);
            }

            printf("'%s'\n", sToken);
        }
        
    }while(1);

    // Clean the stack by the means of evaluating expression in RPN
    while( Pop(&bNewOp) )
    {
        Execute( bNewOp );
    }

    *ppExpr = pStr;

    // Return the possible last value on the stack
    if( PopNode(&Node) )
    {
        if( Node.dwNodeFlags & NODE_VALUE )
        {
            AddValue(&Node);
        }
        else
        {
            AddReg(&Node);
        }
    }

    // Fixup if it is a memory operation, it can't be a register any more
    if( pArg->dwFlags & ASM_MEM )
        pArg->dwFlags &= ~ASM_REG;

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int AsmParamEval(TASMEVAL *pEval, TASMARG args[3], char *pStr)            *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*
*   Returns:
*
******************************************************************************/
int AsmParamEval(TASMEVAL *pEvalParam)
{
    int nArg;                           // Current argument number
    char *pStr;                         // Running pointer to arguments

    pEval = pEvalParam;                 // Make it file global
    pStr = pEval->szAsm;                // Load the string pointer
    error = 0;                          // Zero out local error code

    printf("%s\n", pStr);

    // Loop up to 3 times to get arguments
    for(nArg=0; nArg<3 && *pStr && *pStr!=';'; nArg++)
    {
        pArg = &pEval->pArgs[nArg];

        AsmEvalArg(&pStr);

        printf("    (next arg)\n");

        // Skip the comma that delimits arguments
        if(*pStr==',') pStr++;
    }
    
    pEval->error = error;               // Assign error code
    pEval->szAsm = pStr;                // And load final string pointer

    return( TRUE );
}

