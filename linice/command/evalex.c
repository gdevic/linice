/******************************************************************************
*                                                                             *
*   Module:     evalex.c                                                      *
*                                                                             *
*   Date:       5/15/97                                                       *
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

        This is an extended version of expression evaluator, based on eval.c
        It is extended to handle types associated with symbols.

        ----

        This is a generic expresion evaluator used by the linice.

        Numbers that are acepted are integers written in the notation with
        default base of 16.

        Numbers may be expressed in:
            hexadecimal, default number with optional prefix '0x'
            decimal, prefix '+' or '-'
            prefix '.' (line operator) changes radix to decimal
            character constant: 'a', 'ab', 'abc', 'abcd' note symbol: '
            character value: '\DEC' or '\xHEX'
            address-type: 18:FFFF or CS:EAX


        Evaluator attempts to evaluate a number in the following order:
            * if prefix is '0x' evaluate as hex
            * if prefix is '+' or '-', evaluate as decimal, if that fails,
                    evaluate as hex
            * register set values: al, ah, ax, eax,...
            * symbol name
            * build-in function: byte(), word(), CFL, ...


*******************************************************************************

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/15/97   Original                                             Goran Devic *
* 05/18/97   Added bitwise, boolean operators                     Goran Devic *
* 05/20/97   Literal handling                                     Goran Devic *
* 09/10/97   Literal function may call evaluator                  Goran Devic *
* 09/11/00   Modified for Linice                                  Goran Devic *
* 10/22/00   Significant mods for linice                          Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

extern BOOL SymbolFindByName(DWORD *pSym, TSYMTYPEDEF1 **ppType1, char *pName, int nNameLen);

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

#define ERR_TOO_COMPLEX     100         // Expression too complex
#define ERR_TOO_BIG         101         // Number (hex) too large
#define ERR_NOTAPOINTER     102         // Expression value is not a pointer
#define ERR_ELEMENTNOTFOUND 103         // Structure/union element not found
#define ERR_ADDRESS         104         // Expecting value, not address
#define ERR_SELECTOR        105         // Invalid selector value
#define ERR_INVALIDOP       106         // Invalid operation

int error;

//=============================================================================
//                              CPU REGISTERS
//=============================================================================
typedef struct
{
    char *sName;
    BYTE nameLen;
    DWORD Mask;
    BYTE rShift;
    BYTE offset;
}TRegister;


// Define offsets to register fields
static TRegister Reg[] = {
{ "al",  2, 0x000000FF, 0, offsetof(TREGS, eax) },
{ "ah",  2, 0x0000FF00, 8, offsetof(TREGS, eax) },
{ "ax",  2, 0x0000FFFF, 0, offsetof(TREGS, eax) },
{ "eax", 3, 0xFFFFFFFF, 0, offsetof(TREGS, eax) },
{ "bl",  2, 0x000000FF, 0, offsetof(TREGS, ebx) },
{ "bh",  2, 0x0000FF00, 8, offsetof(TREGS, ebx) },
{ "bx",  2, 0x0000FFFF, 0, offsetof(TREGS, ebx) },
{ "ebx", 3, 0xFFFFFFFF, 0, offsetof(TREGS, ebx) },
{ "cl",  2, 0x000000FF, 0, offsetof(TREGS, ecx) },
{ "ch",  2, 0x0000FF00, 8, offsetof(TREGS, ecx) },
{ "cx",  2, 0x0000FFFF, 0, offsetof(TREGS, ecx) },
{ "ecx", 3, 0xFFFFFFFF, 0, offsetof(TREGS, ecx) },
{ "dl",  2, 0x000000FF, 0, offsetof(TREGS, edx) },
{ "dh",  2, 0x0000FF00, 8, offsetof(TREGS, edx) },
{ "dx",  2, 0x0000FFFF, 0, offsetof(TREGS, edx) },
{ "edx", 3, 0xFFFFFFFF, 0, offsetof(TREGS, edx) },

{ "bp",  2, 0x0000FFFF, 0, offsetof(TREGS, ebp) },
{ "ebp", 3, 0xFFFFFFFF, 0, offsetof(TREGS, ebp) },
{ "sp",  2, 0x0000FFFF, 0, offsetof(TREGS, esp) },
{ "esp", 3, 0xFFFFFFFF, 0, offsetof(TREGS, esp) },  // Should go before es
{ "si",  2, 0x0000FFFF, 0, offsetof(TREGS, esi) },
{ "esi", 3, 0xFFFFFFFF, 0, offsetof(TREGS, esi) },  // Should go before es
{ "di",  2, 0x0000FFFF, 0, offsetof(TREGS, edi) },
{ "edi", 3, 0xFFFFFFFF, 0, offsetof(TREGS, edi) },
{ "fl",  2, 0x0000FFFF, 0, offsetof(TREGS, eflags) },
{ "efl", 3, 0xFFFFFFFF, 0, offsetof(TREGS, eflags) },
{ "ip",  2, 0x0000FFFF, 0, offsetof(TREGS, eip) },
{ "eip", 3, 0xFFFFFFFF, 0, offsetof(TREGS, eip) },

{ "cs",  2, 0x0000FFFF, 0, offsetof(TREGS, cs) },
{ "ds",  2, 0x0000FFFF, 0, offsetof(TREGS, ds) },
{ "es",  2, 0x0000FFFF, 0, offsetof(TREGS, es) },
{ "fs",  2, 0x0000FFFF, 0, offsetof(TREGS, fs) },
{ "gs",  2, 0x0000FFFF, 0, offsetof(TREGS, gs) },
{ "ss",  2, 0x0000FFFF, 0, offsetof(TREGS, ss) },

{ "CFL", 3, 1 << 0,     0, offsetof(TREGS, eflags) },
{ "PFL", 3, 1 << 2,     2, offsetof(TREGS, eflags) },
{ "AFL", 3, 1 << 4,     4, offsetof(TREGS, eflags) },

{ "ZFL", 3, 1 << 6,     6, offsetof(TREGS, eflags) },
{ "SFL", 3, 1 << 7,     7, offsetof(TREGS, eflags) },
{ "TFL", 3, 1 << 8,     8, offsetof(TREGS, eflags) },
{ "IFL", 3, 1 << 9,     9, offsetof(TREGS, eflags) },
{ "DFL", 3, 1 <<10,    10, offsetof(TREGS, eflags) },
{ "OFL", 3, 1 <<11,    11, offsetof(TREGS, eflags) },
{ "IOPL",4, 2 <<12,    12, offsetof(TREGS, eflags) },
{ "NTFL",4, 1 <<14,    14, offsetof(TREGS, eflags) },
{ "RFL", 3, 1 <<16,    16, offsetof(TREGS, eflags) },
{ "VMFL",4, 1 <<17,    17, offsetof(TREGS, eflags) },

{ NULL }
};


/*
{ "DataAddr", 8, 0xFFFFFFFF, 0, 0 },
{ "CodeAddr", 8, 0xFFFFFFFF, 0, 0 },
{ "EAddr",    5, 0xFFFFFFFF, 0, 0 },
{ "Evalue",   6, 0xFFFFFFFF, 0, 0 },
{ "DataAddr", 8, 0xFFFFFFFF, 0, 0 },

{ "bpcount", 7, 0xFFFFFFFF, 0, 0 },
{ "bptotal", 7, 0xFFFFFFFF, 0, 0 },
{ "bpmiss",  6, 0xFFFFFFFF, 0, 0 },
{ "bplog",   5, 0xFFFFFFFF, 0, 0 },
{ "bpindex", 7, 0xFFFFFFFF, 0, 0 }, */


//=============================================================================
//                             INTERNAL FUNCTIONS
//=============================================================================

typedef DWORD (*TFnPtr)(DWORD);

static DWORD fnByte(DWORD arg);
static DWORD fnWord(DWORD arg);
static DWORD fnDword(DWORD arg);
static DWORD fnHiword(DWORD arg);

extern DWORD fnBpCount(DWORD arg);
extern DWORD fnBpMiss(DWORD arg);
extern DWORD fnBpTotal(DWORD arg);
extern DWORD fnBpIndex(DWORD arg);
extern DWORD fnBpLog(DWORD arg);

extern DWORD fnPtr(DWORD arg);


typedef struct
{
    char *sName;                        // Name of the function
    BYTE nameLen;                       // Length of the name
    BYTE nArgs;                         // Number of arguments (so far only 0 or 1 supported in code)
    TFnPtr funct;                       // Function
} TFunction;

static TFunction Func[] = {
{ "byte",   4, 1, fnByte },
{ "word",   4, 1, fnWord },
{ "dword",  5, 1, fnDword },
{ "hiword", 6, 1, fnHiword },

{ "bpcount", 7, 0, fnBpCount },
{ "bpmiss",  6, 0, fnBpMiss },
{ "bptotal", 7, 0, fnBpTotal },
{ "bpindex", 7, 0, fnBpIndex },
{ "bplog",   5, 0, fnBpLog },

{ "ptr",     3, 1, fnPtr },
{ NULL }
};


//=============================================================================

#define MAX_STACK       10              // Max depth of a stack structure

static char *sTypeDefault = "DWORD";    // Default value data type

// The operators ID table implicitly contains the precedence and the group
enum
{
    OP_NULL = 0,                        // These are not used for calculation
    OP_PAREN_START,                     // but are explicitly checked
    OP_PAREN_END,                       // OP_NULL has to be 0

    OP_BOOL_OR     = 0x10,

    OP_BOOL_AND    = 0x20,

    OP_OR          = 0x30,

    OP_XOR         = 0x40,

    OP_AND         = 0x50,

    OP_EQ          = 0x60,
    OP_NE          = 0x61,

    OP_LE          = 0x70,
    OP_L           = 0x71,
    OP_GE          = 0x72,
    OP_G           = 0x73,

    OP_SHL         = 0x80,
    OP_SHR         = 0x81,

    OP_PLUS        = 0x90,
    OP_MINUS       = 0x91,

    OP_TIMES       = 0xA0,
    OP_DIV         = 0xA1,
    OP_MOD         = 0xA2,

    OP_PTR         = 0xB0,

    OP_LINE_NUMBER = 0xC0,              // shares symbol "." with OP_DOT
    OP_UNARY_PLUS  = 0xC1,              // shares symbol "-" with OP_MINUS
    OP_UNARY_MINUS = 0xC2,              // shares symbol "+" with OP_PLUS
    OP_NOT         = 0xC3,
    OP_BITWISE_NOT = 0xC4,

    OP_AT          = 0xD0,              // @
    OP_DOT         = 0xD1,              // .

    OP_SELECTOR    = 0xE0,              // :
};

// Precedence mask to be applied to the operator code to get the precedence group
#define OP_PRECEDENCE   0xF0

typedef struct
{
    char *pToken;
    BYTE nTokenLen;
    BYTE bValue;

} TOperators;

// Table of operators sorted by the operator length, to aid linear
// search for the matching operator
static TOperators TableOperators[] =
{
    { "||",  2,   OP_BOOL_OR      },
    { "&&",  2,   OP_BOOL_AND     },
    { "==",  2,   OP_EQ           },
    { "!=",  2,   OP_NE           },
    { "<<",  2,   OP_SHL          },
    { ">>",  2,   OP_SHR          },
    { "<=",  2,   OP_LE           },
    { ">=",  2,   OP_GE           },
    { "->",  2,   OP_PTR          },
    { "+",   1,   OP_PLUS         },
    { "-",   1,   OP_MINUS        },
    { "*",   1,   OP_TIMES        },
    { "/",   1,   OP_DIV          },
    { ":",   1,   OP_SELECTOR     },
    { "(",   1,   OP_PAREN_START  },
    { ")",   1,   OP_PAREN_END    },
    { ".",   1,   OP_DOT          },
    { "@",   1,   OP_AT           },
    { "|",   1,   OP_OR           },
    { "^",   1,   OP_XOR          },
    { "&",   1,   OP_AND          },
    { "<",   1,   OP_L            },
    { ">",   1,   OP_G            },
    { "%",   1,   OP_MOD          },
    { "!",   1,   OP_NOT          },
    { "~",   1,   OP_BITWISE_NOT  },
    { NULL,  0,   OP_NULL         }
};

typedef struct
{
    int Top;                            // Index of the topmost item
    TExItem Item[ MAX_STACK ];          // Stack data array

} TStack;

typedef struct
{
    int Top;                            // Top of stack index
    BYTE Op[ MAX_STACK ];               // Stack data (operand codes)

} TOpStack;

static const char sDelim[] = ",;\"";    // Expressions delimiters - break chars
static BOOL fDecimal;                   // Prefer decimal number

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern BOOL GetUserVar(DWORD *pValue, char *sStart, int nLen);
extern BOOL EvalBreakpointAddress(TADDRDESC *pAddr, int index);
extern char *Type2Element(TSYMTYPEDEF1 *pType, char *pName, int nLen);

BOOL SymName2LocalSymbol(TSYMBOL *pSymbol, char *pName, int nTokenLen);

BOOL EvaluateEx( TSYMBOL *pSymbol, char *sExpr, char **psNext );

static DWORD fnByte(DWORD arg) { return(arg & 0xFF); }
static DWORD fnWord(DWORD arg) { return(arg & 0xFFFF);}
static DWORD fnDword(DWORD arg) { return(arg);}
static DWORD fnHiword(DWORD arg) { return(arg >> 16);}

static BOOL EvaluateSubexpression(TExItem *pItem, char *pExpr, char **ppNext);


/******************************************************************************
*
*   Stack primitives: Operands and Operators
*
******************************************************************************
*
*   Operations on the operand stack:
*
*       void Push( TStack *Stack, TExItem *pItem )
*       void Pop( TStack *Stack, TExItem *pItem )
*
*   Operations on the operator stack:
*
*       void PushOp(TOpStack *Stack, BYTE Op)
*       BYTE PopOp(TOpStack *Stack)
*       BYTE PeekOp(TOpStack *Stack)
*       BOOL IsEmptyOp(TOpStack *Stack)
*
*   Special copy function is provided since the member pData may address
*   its own structure Data member, so the pointer is logically preserved.
*/
static void CopyItem(TExItem *dest, TExItem *src)
{
    dest->bType = src->bType;
    dest->Data  = src->Data;
    dest->pType = src->pType;
    if(src->pData==&src->Data)
        dest->pData = &dest->Data;
    else
        dest->pData = src->pData;
}

static void Push( TStack *Stack, TExItem *pItem )
{
    if( Stack->Top < MAX_STACK )
        CopyItem(&Stack->Item[Stack->Top++], pItem);
    else
        error = ERR_TOO_COMPLEX;
}

static void Pop( TStack *Stack, TExItem *pItem )
{
    if( Stack->Top == 0 )
    {
        deb.error = ERR_SYNTAX;
    }

    CopyItem(pItem, &Stack->Item[ --Stack->Top ]);
}

static void PushOp(TOpStack *Stack, BYTE Op)
{
    if( Stack->Top < MAX_STACK )
        Stack->Op[ Stack->Top++ ] = Op;
    else
        error = ERR_TOO_COMPLEX;
}

static BYTE PopOp(TOpStack *Stack)
{
    if( Stack->Top == 0 )
    {
        error = ERR_SYNTAX;
        return 0;                       // Return something
    }

    return( Stack->Op[ --Stack->Top ] );
}

static BYTE PeekOp(TOpStack *Stack)
{
    if( Stack->Top )
        return( Stack->Op[ Stack->Top-1 ] );
    return( OP_NULL );
}

static BOOL IsEmptyOp(TOpStack *Stack)
{
    return( !Stack->Top );
}


/******************************************************************************
*
*   BOOL GetHex(DWORD *pValue, char **psString)
*
*******************************************************************************
*
*   Converts string to a hex number.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       psString is the address of the string pointer - will be modified
*
*   Returns:
*       FALSE - we did not read any hex digits
*       TRUE - hex digit is read into pValue, psString is updated
*
******************************************************************************/
static BOOL GetHex(DWORD *pValue, char **psString)
{
    char *ptr = *psString;
    char nibble;
    int count = 8;
    DWORD value = 0;

    while(isxdigit(nibble = *ptr) && count--)
    {
        nibble = tolower(nibble);
        value <<= 4;
        value |= (nibble > '9')? nibble - 'a' + 10: nibble - '0';
        ptr++;
    }

    // Value may be too large to fit in a DWORD
    if(count<0)
        error = ERR_TOO_BIG;

    // If the ending string pointer is equal to starting, we did not read any hex digits
    if(ptr==*psString)
        return(FALSE);

    *psString = ptr;
    *pValue = value;

    return(TRUE);
}


/******************************************************************************
*
*   BOOL GetDec(DWORD *pValue, char **psString)
*
*******************************************************************************
*
*   Converts string to a decimal number.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       psString is the address of the string pointer - will be modified
*
*   Returns:
*       FALSE - we did not read any decimal digits
*       TRUE - decimal digit is read into pValue, psString is updated
*
******************************************************************************/
static BOOL GetDecB(DWORD *pValue, char **psString)
{
    // Max digit is 4294967295
    char *ptr = *psString;
    char digit;
    DWORD value = 0;

    while(isdigit(digit = *ptr))
    {
        // Check for overflow (number too big)
        if( value>429496729 )
            error = ERR_TOO_BIG;

        value *= 10;
        value += digit - '0';
        ptr++;
    }

    // If the ending string pointer is equal to starting, we did not read any digits
    if(ptr==*psString)
        return(FALSE);

    *psString = ptr;
    *pValue = value;

    return(TRUE);
}

static TRegister *IsRegister(char *ptr, int nTokenLen)
{
    TRegister *pReg = &Reg[0];

    while(pReg->sName!=NULL)
    {
        if(pReg->nameLen==nTokenLen && !strnicmp(pReg->sName, ptr, pReg->nameLen))
            return(pReg);
        pReg++;
    }

    return(NULL);
}


static TFunction *IsFunc(char *ptr, int nTokenLen)
{
    TFunction *pFunc = &Func[0];

    while(pFunc->sName!=NULL)
    {
        if(pFunc->nameLen==nTokenLen && !strnicmp(pFunc->sName, ptr, pFunc->nameLen))
            return(pFunc);
        pFunc++;
    }

    return(NULL);
}

int GetTokenLen(char *pToken)
{
    char *pTmp = pToken;

    while( *pTmp && (isalnum(*pTmp) || *pTmp=='_') ) pTmp++;

    return(pTmp - pToken);
}


/******************************************************************************
*                                                                             *
*   TExItem GetValue( char **sExpr )                                            *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string token into an item structure. Advances the given pointer.
*
*   Where:
*       sExpr is an address of a pointer to a string containing expression.
*
*   Returns:
*       Stack item set up
*
******************************************************************************/
static BOOL GetValue( TExItem *item, char **sExpr )
{
    TADDRDESC Addr;
    int value, n;
    char *sStart = *sExpr, *sTmp;
    TRegister *pReg;
    TFunction *pFunc;
    int nTokenLen;                      // Length of the input token
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition


    DWORD *pSym;
    char *pSymType;


//    item->pType = sTypeDefault;         // Start with default type
    item->pType = NULL;                 // DWORD is NULL type by default
    item->bType = EXTYPE_LITERAL;       // Assume literal type

    // Find the length of the token in the input buffer (We assume it is a token)

    nTokenLen = GetTokenLen(sStart);

    // If we switched to decimal radix, try a decimal number first
    if( fDecimal==TRUE && GetDecB(&value, &sStart) )
    {
        goto End;
    }

    // Check if the first two charcaters represent a hex number
    if( *sStart=='0' && tolower(*(sStart+1))=='x' )         // 0xHEX
    {
        sStart += 2;
        GetHex(&value, &sStart);
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
        if( *sStart!='\'' )
            ;// TODO - No-nterminated character constant 'abc'
    }
    else
    // Check if the first two characters represent a character literal
    if( *sStart=='\\' )                                 // \DEC or \xHEX
    {
        if( tolower(*(sStart+1))=='x' )                    // HEX
        {
            sStart += 2;
            GetHex(&value, &sStart);
        }
        else
        {
            sStart += 1;
            GetDecB(&value, &sStart);
        }
    }
    else
    // The literal value may be the explicit CPU register value
    if( (pReg = IsRegister(sStart, nTokenLen)) != 0 )
    {
        value = (DWORD) deb.r + pReg->offset;
        value = *(DWORD *)value;
        value = (value & pReg->Mask) >> pReg->rShift;
        sStart += pReg->nameLen;

        // Set this type to be the register type
        item->bType = EXTYPE_REGISTER;
        item->pType = pReg->sName;
    }
    else
    // Symbol name: local symbol
//    if( SymName2LocalSymbol(&item, sStart, nTokenLen) )
//    {
//        // Symbol name is found and assigned to value in the function above
//        value = item.Data;
//        sStart += nTokenLen;
//    }
//    else
    // The first precedence literal value is the symbol name
//    if( SymbolName2Value(pIce->pSymTabCur, (DWORD *)&value, sStart, nTokenLen) )
//    {
//        // Symbol name is found and assigned to value in the function above
//        sStart += nTokenLen;
//    }
//    else
    // Check if it is a breakpoint token 'bpN' or 'bpNN'
    if( tolower(*sStart)=='b' && tolower(*(sStart+1))=='p' && isxdigit(*(sStart+2)) )
    {
        sStart += 2;

        GetHex(&n, &sStart);

        // Preset selector part in the case it changes
        Addr.sel = evalSel;

        if( EvalBreakpointAddress(&Addr, n) )
        {
            // Store the resulting values
            value = Addr.offset;
            evalSel = Addr.sel;
        }
        else
            dprinth(1, "Invalid breakpoint");
    }
    else
    // The literal value may be a built-in function
    if( (pFunc = IsFunc(sStart, nTokenLen)) != 0 )
    {
        sStart += pFunc->nameLen;
        switch( pFunc->nArgs )
        {
            case 0:         // No arguments to a function
                value = (pFunc->funct)(0);
            break;

            case 1:         // Single argument to a function
            // TODO: Use EvaluateSubexpression()
                value = Evaluate(sStart, &sStart);
                value = (pFunc->funct)(value);
            break;
        }
    }
//    else
    // Check for the symbol name
//    if( SymbolName2Value(pIce->pSymTabCur, (DWORD *)&value, sStart, nTokenLen) )
//    {
//        goto ProcessSymbol;
//    }
    else
    // The liternal string may be the variable (symbol) name
    if( SymbolFindByName(&pSym, &pType1, sStart, nTokenLen) )
    {
        // Return the typed token of the symbol class
        item->bType = EXTYPE_SYMBOL;
        item->Data = pSym;
        item->pData = &item->Data;
        item->pType = GET_STRING( pType1->dDef );

        *sExpr = sStart + nTokenLen;
        return( TRUE );
    }
    else
    // The literal string may be the type name (type cast)
    if( (pType1=Type2Typedef(sStart, nTokenLen)) )
    {
        // String is a valid type whose descriptor we got now
        // Since this is a type cast, we expect form "type(value)" and need to get "(",")"
        TExItem subitem;
        char *pEnd, *pDef;              // End of the sub-expression, pointer to definition
        int major, minor;               // Element major and minor numbers
        int nOffset, nSize;             // Element offset information and size

        sStart += nTokenLen;            // Get to the end of the type string
        if( EvaluateSubexpression(&subitem, sStart, &sStart) )
        {
ProcessSymbol:
            // Since this may be a structure, check if it followed by a structure field dereferencing
            if( *sStart=='.' )
            {
                // Structure element dereference
                sStart += 1;
                *sExpr = sStart;

                // The type definition should reference a type descriptor
                pDef = GET_STRING( pType1->dDef );
                pType1 = Type2Typedef(pDef);

                // Get the token representing the structure element
                nTokenLen = GetTokenLen(sStart);

                pDef = Type2Element(pType1, sStart, nTokenLen);
                if( pDef )
                {
                    // We found the structure/union element. Read the offset info
//                    sscanf(pDef, "(%d,%d),%d,%d", &major, &minor, &nOffset, &nSize);

                    // Add the offset to the address of the structure
                    *(DWORD *)subitem.pData += nOffset / 8;
                    subitem.bType = EXTYPE_SYMBOL;

                    // Get the type of our element
                    pType1 = Type2Typedef(pDef);
                    subitem.pType = GET_STRING( pType1->dDef );

                    sStart += nTokenLen;
                    *sExpr = sStart;
                }
                else
                    error = ERR_ELEMENTNOTFOUND;
            }
            else
            if( *sStart=='-' && *(sStart+1)=='>' )
            {
                // Structure element dereference
                sStart += 2;
                *sExpr = sStart;

                // The type definition should dereference a pointer to type descriptor
                pDef = GET_STRING( pType1->dDef );
                if( *pDef=='*' )
                {
                    pDef++;
                    item->pType = Type2Typedef(pDef);

                    // Do actual dereference
                    subitem.pData = fnPtr(subitem.pData);
                }
                else
                    error = ERR_NOTAPOINTER;
            }
            else
            if( *sStart=='[' )
            {
                // Element of an array dereference

                // Evaluate the subexpression which gives the array index
                ;
            }

            // Return the typed token of the symbol class
            item->bType = EXTYPE_SYMBOL;
            item->Data = subitem.Data;
            item->pData = subitem.pData;
            item->pType = subitem.pType;
            if( subitem.pData = &subitem.Data )
                item->pData = &item->Data;

            *sExpr = sStart;
            return( !error );
        }
    }
    else
    // If everything else fails, it's gotta be a hex number
    {
        if( !GetHex(&value, &sStart) )
        {
            // Jibberish in the expression
            *sExpr = sStart;

            error = ERR_SYNTAX;

            return(FALSE);
        }
    }
End:
    fDecimal = FALSE;
    *sExpr = sStart;

    item->Data = value;
    item->pData = &item->Data;

    return( TRUE );
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
static void Execute( TStack *Values, int Operation )
{
    TExItem item1, item2, itemTop;

    // Most operations require 2 parameters. Just some are unary (one operand)
    switch( Operation )
    {
        case OP_BITWISE_NOT:
        case OP_UNARY_MINUS:
        case OP_UNARY_PLUS:
//        case OP_PTR:
//        case OP_DOT:
        case OP_AT:
        case OP_LINE_NUMBER:
            Pop(Values, &item1);
            break;

        default:
            Pop(Values, &item2);
            Pop(Values, &item1);
    }

    // If the left side is symbol, there are only certain operations allowed on it:
    if( item1.bType==EXTYPE_SYMBOL )
    {
        if( Operation!=OP_AT && item2.bType!=EXTYPE_LITERAL )
            deb.error = ERR_INVALIDOP;

        switch( Operation )
        {
            case OP_AT:                 // @symbol
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = item1.pData;
                itemTop.pData = &item1.Data;
                itemTop.pType = NULL;
                break;

            case OP_PLUS:               // symbol + 1
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = (DWORD) item1.pData + item2.Data;
                itemTop.pData = &item1.Data;
                itemTop.pType = NULL;
                break;

            case OP_MINUS:              // Symbol - 1
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = (DWORD) item1.pData - item2.Data;
                itemTop.pData = &item1.Data;
                itemTop.pType = NULL;
                break;

            default:
                deb.error = ERR_INVALIDOP;
                break;
        }

        Push(Values, &itemTop);

        return;
    }

    // Perform the operation
    switch( Operation )
    {
        //--------------------------------------------------------------------
        case OP_BOOL_OR:
            itemTop.Data = item1.Data || item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_BOOL_AND:
            itemTop.Data = item1.Data && item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_OR:
            itemTop.Data = item1.Data | item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_XOR:
            itemTop.Data = item1.Data ^ item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_AND:
            itemTop.Data = item1.Data & item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_EQ:
            itemTop.Data = item1.Data == item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_NE:
            itemTop.Data = item1.Data != item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_L:
            itemTop.Data = item1.Data < item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_LE:
            itemTop.Data = item1.Data <= item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_G:
            itemTop.Data = item1.Data > item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_GE:
            itemTop.Data = item1.Data >= item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_SHL:
            itemTop.Data = item1.Data << item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_SHR:
            itemTop.Data = item1.Data >> item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_PLUS:
            itemTop.Data = item1.Data + item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_MINUS:
            itemTop.Data = item1.Data - item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_TIMES:
            itemTop.Data = item1.Data * item2.Data;
            break;
        //--------------------------------------------------------------------
        case OP_DIV:
            if( item2.Data )
                itemTop.Data = item1.Data / item2.Data;
            else
                deb.error = ERR_DIV0;
            break;
        //--------------------------------------------------------------------
        case OP_MOD:
            if( item2.Data )
                itemTop.Data = item1.Data % item2.Data;
            else
                deb.error = ERR_DIV0;
            break;
        //--------------------------------------------------------------------
        case OP_NOT:
            itemTop.Data = !item1.Data;
            break;
        //--------------------------------------------------------------------
        case OP_BITWISE_NOT:
            itemTop.Data = ~item1.Data;
            break;
        //--------------------------------------------------------------------
        case OP_UNARY_MINUS:
            itemTop.Data = (unsigned)-(signed)item1.Data;
            break;
        //--------------------------------------------------------------------
        case OP_UNARY_PLUS:
            // We dont do anything special for unary plus
            itemTop.Data = item1.Data;
            break;
        //--------------------------------------------------------------------
        case OP_DOT:
            itemTop.Data = item1.Data;          // TEST
            break;
        //--------------------------------------------------------------------
        case OP_PTR:
            itemTop.Data = fnPtr(item1.Data);
            break;
        //--------------------------------------------------------------------
        case OP_LINE_NUMBER:
            itemTop.Data = SymLinNum2Address(item1.Data);   // TEST
            break;
        //--------------------------------------------------------------------
        case OP_SELECTOR:   // Selector:offset
            // The left side contains the selector token, the right side offset
            // We merge them into one address-type token
            itemTop.bType = EXTYPE_ADDRESS;
            itemTop.pData = *(DWORD *)item1.pData;      // Selector
            itemTop.Data  = *(DWORD *)item2.pData;      // Offset
            itemTop.pType = item1.pType;                // Name of the selector or NULL

            // Selector has to be the valid size
            if(*(DWORD *)item1.pData > 0xFFFF)
                error = ERR_SELECTOR;

            // Do a separate ending since we did many things differently
            Push(Values, &itemTop);

            return;
        //--------------------------------------------------------------------
    }

    // The final type is copied from the first item (given they were the same)
    itemTop.pType = item1.pType;
    itemTop.bType = item1.bType;

    // At the end, we always push the resulting value onto the stack
    itemTop.pData = &itemTop.Data;

    Push(Values, &itemTop);
}


/******************************************************************************
*                                                                             *
*   BYTE TableMatch( char *sTable, char **sToken )                            *
*                                                                             *
*******************************************************************************
*
*   Returns a matching string from a token table or 0 if there was no match.
*   sToken pointer is advanced accordingly.
*
*   Where:
*       sTable is an array of pointers pointing to tokens.  Last entry is NULL.
*       sToken is an address of a pointer to string to be examined.
*
*   Returns:
*       Token number from an array or 0 if failed to match a token.
*
******************************************************************************/
static BYTE TableMatch( char **sToken )
{
    TOperators *pOp = &TableOperators[0];

    // Find the matching substring in a table
    while( pOp->nTokenLen )
    {
        if( !strnicmp(pOp->pToken, *sToken, pOp->nTokenLen) )
        {
            // Advance the source token pointer by the token size and return the value
            *sToken += pOp->nTokenLen;

            return( pOp->bValue );
        }
        pOp++;
    }

    return( 0 );
}


/******************************************************************************
*
*   BOOL NextToken(char **ppNext)
*
*******************************************************************************
*
*   Parses the input string - detects the next valid token start
*
******************************************************************************/
static BOOL NextToken(char **ppNext)
{
    char *pExpr = *ppNext;              // Convinient pointer

    // Skip possible spaces
    while( *pExpr==' ' ) pExpr++;

    // Detect end of the string as the final terminator returning FALSE
    // The same end of the string may be one of the delimiter characters
    if( !*pExpr || strchr(sDelim,*pExpr) )
    {
        *ppNext = pExpr;
        return( FALSE );
    }

    // Set next token start address and return signal that we found it
    *ppNext = pExpr;

    return( TRUE );
}

/******************************************************************************
*
*   BOOL Evaluate2(char *pExpr, char **ppNext)
*
*******************************************************************************
*
*   Debuger command to evaluate an expression
*
******************************************************************************/
BOOL Evaluate2(TExItem *pItem, char *pExpr, char **ppNext)
{
    TOpStack Operators = { 0, };        // Operators stack, initialized to 0 elements
    TStack   Values    = { 0, };        // Values stack, initialized to 0 elements
    TExItem Item;
    char *myNext;                       // This pointer is used if ppNext==NULL
    BYTE NewOp;
    BOOL fUnary = TRUE;                 // Possible unary operator at the beginning

    error = 0;                          // Reset the error variable to no error
    fDecimal = FALSE;                   // Do not expect decimal number at first

    // If the ppNext is NULL, we will use local pointer instead
    if( !ppNext )
        ppNext = &myNext;

    // Precondition: Now we have ppNext != NULL

    // Just to be on the safe side, detect if we are given a NULL pointer
    if( !pExpr )
    {
        *ppNext = NULL;
        return( FALSE );
    }

    // Precondition: both input arguments are now valid

    // Loop for every token in the input string, bail out on any kind of error
    while( NextToken(&pExpr) && !error )
    {
        // See if the new token is an operator
        if( (NewOp = TableMatch( &pExpr)) )
        {
            if(fUnary)
            {
                // Modify some operators that are unary at this point
                switch(NewOp)
                {
                // Appropriately for this application, +/- also signal decimal number
                case OP_PLUS:   NewOp = OP_UNARY_PLUS,  fDecimal = TRUE; break;
                case OP_MINUS:  NewOp = OP_UNARY_MINUS, fDecimal = TRUE; break;
                case OP_DOT:    NewOp = OP_LINE_NUMBER, fDecimal = TRUE; break;
                };

                // Dont expect unary next time around
                fUnary = FALSE;

                // Push the new operator on the operator stack
                PushOp(&Operators, NewOp);

                continue;
            }

            // Right parenthesis needs stack unwinding
            if( NewOp==OP_PAREN_END )
            {
                while( PeekOp(&Operators)!=OP_PAREN_START && !error)
                {
                    Execute(&Values, PopOp(&Operators));
                }

                // Here we have to find left paren, otherwise it's syntax error
                if( PopOp(&Operators)!=OP_PAREN_START )
                    error = ERR_SYNTAX;

                continue;
            }

            // For anything except the left parenthesis, we recalculate the stack
            if( NewOp!=OP_PAREN_START )
            {
                // Level the precedence with regards to the TOS
                while( !IsEmptyOp(&Operators) && (PeekOp(&Operators)&OP_PRECEDENCE)>=(NewOp&OP_PRECEDENCE) )
                {
                    Execute(&Values, PopOp(&Operators));
                }

                // Push the new operator on the operator stack
                PushOp(&Operators, NewOp);

                continue;
            }

            // After a left parenthesis, we may have a unary operator
            fUnary = TRUE;

            // Left paremthesis is just pushed onto the stack as a placeholder
            PushOp(&Operators, OP_PAREN_START);

            continue;
        }

        // Get the next token - it should be an operand
        if( !GetValue(&Item, &pExpr) )
            break;

        Push(&Values, &Item);
        fUnary = FALSE;                 // After the first value no more unary (until left paren)
    }

    // Evaluate what's left on the stack
    while( !IsEmptyOp(&Operators) )
    {
        Execute(&Values, PopOp(&Operators));
    }

    // At this point the opeator stack should be empty, and the value stack should have only value in it
    if( IsEmptyOp(&Operators) && Values.Top==1 && !error)
    {
        // Success!
        *ppNext = pExpr;

        Pop(&Values, pItem);

        return( TRUE );
    }

    return( FALSE );
}


/******************************************************************************
*
*   BOOL EvaluateSubexpression(TExItem *pItem, char *pExpr, char **ppNext)
*
*******************************************************************************
*
*   This is a helper function that performs the same operation as its
*   regular evaluate, but it also expects (and discards) wrapping set of
*   parenthesis. It is used by the evaluator itself to evaluate subexpressions
*   given in function enclosures.
*
******************************************************************************/
static BOOL EvaluateSubexpression(TExItem *pItem, char *pExpr, char **ppNext)
{
    char *pStart, *pEnd;                // Start and end of sub-expression
    int nLevel = 1;                     // Level of nested parenthesis
    BOOL fResult;                       // Result of evaluation

    // Scope the start and end of subexpression - we need to have matching parenthesis
    pStart = pExpr;
    while( *pStart==' ' ) pStart++;
    if( *pStart != '(' )
        return( FALSE );

    pEnd = ++pStart;

    // Find the matching end of the subexpression caring for the nested parenthesis
    while( *pEnd && nLevel )
    {
        if( *pEnd=='(' )
            nLevel++;
        else
        if( *pEnd==')' )
            nLevel--;

        pEnd++;
    }

    // Make sure it's not empty string
    if( *--pEnd != ')' )
        return( FALSE );

    // Terminate the ending of the subexpression and get its evaluation
    *pEnd = 0;

    fResult = Evaluate2(pItem, pStart, ppNext);

    // Restore the original expression string
    *pEnd = ')';

    // If there were no errors, assign the next pointer to after the closing bracket
    if( fResult )
        *ppNext = pEnd + 1;

    return( fResult );
}

void Test(char *str, DWORD result)
{
    BOOL ret;
    char *args = str;
    TExItem Item;

    ret = Evaluate2(&Item, args, &args);

    if( ret==TRUE )
    {
        if( *(DWORD *)Item.pData==result )
            dprinth(1, "%s = (%s) %d OK", str, Item.pType, result);
        else
            dprinth(1, "%s INCORRECT: Got %d, expected %d\n", str, *(DWORD *)Item.pData, result);
    }
    else
    {
        dprinth(1, "%s  FAILED\n", str);
    }
}

void PrintError()
{
    // Evaluation returned an intrinsic error
    switch( error )
    {
        case ERR_TOO_COMPLEX:       dprinth(1, "Expression too complex!\n"); break;
        case ERR_TOO_BIG:           dprinth(1, "Number too large!\n");       break;
        case ERR_NOTAPOINTER:       dprinth(1, "Not a pointer!\n");          break;
        case ERR_ELEMENTNOTFOUND:   dprinth(1, "Element not found!\n");      break;
        case ERR_ADDRESS:           dprinth(1, "Expecting value, not address!\n"); break;
        case ERR_SELECTOR:          dprinth(1, "Invalid selector value!\n"); break;
        default:
            dprinth(1, "Unknown error %d!\n", error);
    }
}

/******************************************************************************
*                                                                             *
*   BOOL EvalGetValue(DWORD *pValue, char *pExpr, char **ppNext)              *
*                                                                             *
*******************************************************************************
*
*   Utility function that evaluates a string into a value.
*
*   Where:
*       pValue is the address where to store the resulting value (DWORD)
*       pExpr is the expression string
*       ppNext is a pointer to a (char *) variable that stores the address
*           of the end of the current expression or the first invalid character
*           encountered.  If NULL, no end is stored.
*
*   Returns:
*       TRUE - expression successfully evaluated; *pValue contains the result
*              *ppNext is updated (if not NULL)
*       FALSE - error evaluating expression
*
******************************************************************************/
BOOL EvalGetValue(DWORD *pValue, char *pExpr, char **ppNext)
{
    TExItem Item;                         // Store the result of the evaluation

    if( Evaluate2(&Item, pExpr, ppNext) )
    {
        // Literal contains the value in the *pData
        // Register contains the register value in the *pData
        // Symbol contains the address of the symbol in *pData

        if( Item.bType!=EXTYPE_ADDRESS )
        {
            // Assign the value from the item pointer
            *pValue = *(DWORD *)Item.pData;

            return( TRUE );
        }

        // Expecting value, not address
        error = ERR_ADDRESS;
    }

    PrintError();

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*
*                                                                             *
*******************************************************************************
*
*   Expands symbol one level
*
******************************************************************************/
void ExpandPrintSymbol(TExItem *pItem)
{
    char *buf[MAX_STRING+1];


    dprinth(1, "%s", buf);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdEvaluate(char *args, int subClass)                                *
*                                                                             *
*******************************************************************************
*
*   Debuger command to evaluate an expression
*
******************************************************************************/
BOOL cmdEvaluate2(char *args, int subClass)
{
    static char buf[MAX_STRING];        // Temp output buffer
    TExItem Item;                         // Store the result of the evaluation
    DWORD Data;                         // Temp data store
    int i;                              // Temp store

    if( *args )
    {
        if( Evaluate2(&Item, args, NULL) )
        {
            switch( Item.bType )
            {
                //-----------------------------------------------------------------
                // Literal contains the value in the *pData
                // Register contains the register value in the *pData
                //-----------------------------------------------------------------
                case EXTYPE_LITERAL:
                case EXTYPE_REGISTER:
                {
                    Data = *(DWORD *)Item.pData; // Get the actual data value

                    i = sprintf(buf, " Hex=%08X  Dec=%010u  ", Data, Data );

                    // Print negative decimal only if it is a negative number
                    if( (signed)Data<0 )
                        i += sprintf(buf+i, "(%d)  ", (signed)Data);

                    // Print ASCII representation of that number
                    i += sprintf(buf+i, "\"%c%c%c%c\"",
                            ((Data>>24) & 0xFF) >= ' '? ((Data>>24) & 0xFF) : '.',
                            ((Data>>16) & 0xFF) >= ' '? ((Data>>16) & 0xFF) : '.',
                            ((Data>> 8) & 0xFF) >= ' '? ((Data>> 8) & 0xFF) : '.',
                            ((Data>> 0) & 0xFF) >= ' '? ((Data>> 0) & 0xFF) : '.' );

                    dprinth(1, "%s", buf);
                }
                break;

                //-----------------------------------------------------------------
                // Symbol is expanded
                //-----------------------------------------------------------------
                case EXTYPE_SYMBOL:
                {
                    // Print the expanded symbol
                    ExpandPrintSymbol(&Item);
                }
                break;

                //-----------------------------------------------------------------
                // Address contains the offset in the Data field
                //  pData is the selector value
                //  pType is the name of the selector (or NULL)
                //-----------------------------------------------------------------
                case EXTYPE_ADDRESS:
                {
                    i = sprintf(buf, " Address: ");

                    if( Item.pType )
                        i += sprintf(buf+i, "(%s) ", Item.pType);

                    i += sprintf(buf+i, "%04X:%08X", (WORD)Item.pData, Item.Data);

                    dprinth(1, "%s", buf);
                }
                break;
            }

            return( TRUE );
        }

        PrintError();
    }
    else
        deb.error = ERR_EXP_WHAT;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdEvaluate(char *args, int subClass)                                *
*                                                                             *
*******************************************************************************
*
*   Debuger command to evaluate an expression
*
******************************************************************************/
BOOL cmdEvaluate_OLD(char *args, int subClass)
{
    static char buf[MAX_STRING];        // Temp output buffer
    TSYMBOL Symbol;
    int i;
    DWORD value;
    char *pType;
    TExItem Item;
    BOOL ret;

    if( *args=='?' )
    {
        char t[80];
        // Test

        Test("0", 0);
        Test("1+4", 1+4);
        Test("1+3*2-1", 1+3*2-1);
        Test("4*2-1+8*4-3", 4*2-1+8*4-3);

        Test("-23*2+(-3-1)", -23*2+(-3-1));
        Test("(4*5-1)*2/3", (4*5-1)*2/3);

        return( TRUE );
    }

    ret = Evaluate2(&Item, args, &args);

    dprinth(1, "[%s] Data=%X (%d)", ret? "OK":"ERROR", Item.Data, Item.Data);

#if 0
    if( SymName2LocalSymbol(&Symbol, args)==TRUE )
    {
        dprinth(1, "%08X %08X %s %s", Symbol.Address, Symbol.Data, Symbol.pName, Symbol.pType );
    }
    else
    {
        dprinth(1, "FALSE");
    }
#endif

#if 0
    if( *args )
    {
        memset(&Symbol, 0, sizeof(Symbol));

        if( EvaluateEx(&Symbol, args, &args) && (deb.error==NOERROR) )
        {
            if( !*args )
            {
                i = sprintf(buf, " Hex=%08X  Dec=%010u  ", Symbol.Data, Symbol.Data );

                // Print negative decimal only if it is a negative number
                if( (signed)Symbol.Data<0 )
                    i += sprintf(buf+i, "(%d)  ", (signed)Symbol.Data);

                // Print ASCII representation of that number
                i += sprintf(buf+i, "\"%c%c%c%c\"",
                        ((Symbol.Data>>24) & 0xFF) >= ' '? ((Symbol.Data>>24) & 0xFF) : '.',
                        ((Symbol.Data>>16) & 0xFF) >= ' '? ((Symbol.Data>>16) & 0xFF) : '.',
                        ((Symbol.Data>> 8) & 0xFF) >= ' '? ((Symbol.Data>> 8) & 0xFF) : '.',
                        ((Symbol.Data>> 0) & 0xFF) >= ' '? ((Symbol.Data>> 0) & 0xFF) : '.' );

                dprinth(1, "%s", buf);
                dprinth(1, "Type: <%s>", Symbol.pType);
            }
            else
                dprinth(1, "Syntax error at ->\"%s\"", args);
        }
    }
    else
        deb.error = ERR_EXP_WHAT;
#endif
    return( TRUE );
}

