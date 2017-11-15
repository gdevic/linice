/******************************************************************************
*                                                                             *
*   Module:     evalex.c                                                      *
*                                                                             *
*   Date:       5/15/97                                                       *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

        This is an extended version of expression evaluator, based on eval.c
        It is extended to handle types associated with symbols.

        ----

        This is a generic expresion evaluator used by the linice.

        Numbers that are acepted are integers written in the notation with
        default base of 16.

        Numbers may be expressed in:
            hexadecimal, default number with optional prefix '0x'
            binary, prefix '0b'
            octal, prefix '0o'
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

            Functions can have only 1 parameter since we process them the same as
            type override, that is in the form: "name(expression)"

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

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// TODO: Get rid of these and make it elegant

WORD evalSel = 0x0000;                  // Selector result of the expression (optional)
int nEvalDefaultBase = 16;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

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


//=============================================================================
//                             INTERNAL FUNCTIONS
//=============================================================================

typedef DWORD (*TFnPtr)(DWORD);

static DWORD fnByte(DWORD arg);
static DWORD fnWord(DWORD arg);
static DWORD fnDword(DWORD arg);
static DWORD fnHiword(DWORD arg);
static DWORD fnHibyte(DWORD arg);
static DWORD fnSword(DWORD arg);

extern DWORD fnBpCount(DWORD arg);
extern DWORD fnBpMiss(DWORD arg);
extern DWORD fnBpTotal(DWORD arg);
extern DWORD fnBpIndex(DWORD arg);
extern DWORD fnBpLog(DWORD arg);
extern DWORD fnDataAddr(DWORD arg);
extern DWORD fnCodeAddr(DWORD arg);
extern DWORD fnEAddr(DWORD arg);
extern DWORD fnEValue(DWORD arg);

extern DWORD fnPtr(DWORD arg);


typedef struct
{
    char *sName;                        // Name of the function
    BYTE nameLen;                       // Length of the name
    BYTE nArgs;                         // Number of arguments (so far only 0 or 1 supported in code)
    TFnPtr funct;                       // Function
} TFunction;

#define MAX_FUNCTION    16              // Ordinal index of the last function in the array

static TFunction Func[] = {
                                        // Functions with 1 parameter
{ "byte",   4, 1, fnByte },
{ "word",   4, 1, fnWord },
{ "dword",  5, 1, fnDword },
{ "hibyte", 6, 1, fnHibyte },
{ "hiword", 6, 1, fnHiword },
{ "sword",  5, 1, fnSword },
{ "ptr",    3, 1, fnPtr },
                                        // Functions with 0 parameters
{ "bpcount", 7, 0, fnBpCount },
{ "bpmiss",  6, 0, fnBpMiss },
{ "bptotal", 7, 0, fnBpTotal },
{ "bpindex", 7, 0, fnBpIndex },
{ "bplog",   5, 0, fnBpLog },
{ "DataAddr",8, 0, fnDataAddr },
{ "CodeAddr",8, 0, fnCodeAddr },
{ "EAddr",   5, 0, fnEAddr },
{ "EValue",  6, 0, fnEValue },

{ NULL }
};

//=============================================================================

#define MAX_STACK       10              // Max depth of a stack structure

// The operators ID table implicitly contains the precedence and the group
enum
{
    OP_NULL = 0,                        // These are not used for calculation
    OP_PAREN_START,                     // but are explicitly checked
    OP_PAREN_END,                       // OP_NULL has to be 0
    OP_BRACKET_START,
    OP_BRACKET_END,

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
    OP_UNARY_AND   = 0xC3,              // shares symbol "&" with OP_AND
    OP_UNARY_PTR   = 0xC4,              // shares symbol "*" with OP_TIMES
    OP_NOT         = 0xC5,
    OP_BITWISE_NOT = 0xC6,

    OP_UNARY_AT    = 0xD0,              // @
    OP_DOT         = 0xD1,              // .

    OP_SELECTOR    = 0xE0,              // :

    OP_TYPECAST    = 0xF0,              // Type cast
    OP_FUNCTION1   = 0xF1,              // Function with 1 argument
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
    { "[",   1,   OP_BRACKET_START},
    { "]",   1,   OP_BRACKET_END  },
    { ".",   1,   OP_DOT          },
    { "@",   1,   OP_UNARY_AT     },
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

//=============================================================================
// STACK STORAGE FOR OPERANDS AND OPERATORS
//=============================================================================

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

extern BOOL QueryExtToken(DWORD *pResult, char **pToken, int len);
extern BOOL EvalBreakpointAddress(TADDRDESC *pAddr, int index);
extern char *Type2Element(TSYMTYPEDEF1 *pType, char *pName, int nLen);
extern void TypedefCanonical(TSYMTYPEDEF1 *pType1);
extern TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen, WORD file_id);
extern BOOL GlobalReadDword(DWORD *ppDword, DWORD dwAddress);
extern UINT GetTypeSize(TSYMTYPEDEF1 *pType1);
extern BOOL GetUserVar(DWORD *pValue, char *sStart, int nLen);
extern void ExpandPrintSymbol(TExItem *Item, char *pName);
extern BOOL FindSymbol(TExItem *item, char *pName, int *pNameLen);

static DWORD fnByte(DWORD arg) { return(arg & 0xFF); }
static DWORD fnWord(DWORD arg) { return(arg & 0xFFFF);}
static DWORD fnDword(DWORD arg) { return(arg);}
static DWORD fnHiword(DWORD arg) { return(arg >> 16);}
static DWORD fnHibyte(DWORD arg) { return((arg >> 8) & 0xFF);}
static DWORD fnSword(DWORD arg) { return((arg & 0x80)? 0xFF00 | arg : arg );}


/******************************************************************************
*
*   BOOL ExItemCompare(TExItem *pItem1, TExItem *pItem2)
*
*******************************************************************************
*
*   Compares two TExItem structures and returns TRUE if they describe
*   equivalent value.
*
*   Where:
*       pItem1 - first value
*       pItem2 - second value
*
*   Returns:
*       TRUE - The two values are equivalent
*       FALSE - Values are different
*
******************************************************************************/
BOOL ExItemCompare(TExItem *pItem1, TExItem *pItem2)
{
    if( pItem1->bType  == pItem2->bType
    &&  !memcmp(&pItem1->Type, &pItem2->Type, sizeof(TSYMTYPEDEF1)))
        return( TRUE );

    return( FALSE );
}

/******************************************************************************
*
*   Stack primitives: Operands and Operators
*
*******************************************************************************
*
*   Operations on the operand stack:
*
*       void Push( TStack *Stack, TExItem *pItem )
*       void Pop( TStack *Stack, TExItem *pItem )
*       TExItem *Peek( TStack *Stack, TExItem *pItem )
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
*
*******************************************************************************/

/******************************************************************************
*   Copy source item into the destination item structures
*******************************************************************************/
static void CopyItem(TExItem *dest, TExItem *src)
{
    // Copy all the elements of the source structure
    memcpy(dest, src, sizeof(TExItem));

    // If the source pointer to data was addressing its own data store, do the same with the destination
    if(src->pData==(BYTE *)&src->Data)
        dest->pData = (BYTE *)&dest->Data;
}

/******************************************************************************
*   Push item to the operand stack
*******************************************************************************/
static void Push( TStack *Stack, TExItem *pItem )
{
    if( Stack->Top < MAX_STACK )
        CopyItem(&Stack->Item[Stack->Top++], pItem);
    else
        PostError(ERR_TOO_COMPLEX, 0);
}

/******************************************************************************
*   Pop item from the operand stack
*******************************************************************************/
static void Pop( TStack *Stack, TExItem *pItem )
{
    if( Stack->Top == 0 )
    {
        PostError(ERR_SYNTAX, 0);
    }

    CopyItem(pItem, &Stack->Item[ --Stack->Top ]);
}

/******************************************************************************
*   Peek an item from the operand stack
*******************************************************************************/
static TExItem *Peek(TStack *Stack)
{
    if( Stack->Top )
        return( &Stack->Item[ Stack->Top-1 ] );
    else
        return( NULL );
}

/******************************************************************************
*   Push an operator
*******************************************************************************/
static void PushOp(TOpStack *Stack, BYTE Op)
{
    if( Stack->Top < MAX_STACK )
        Stack->Op[ Stack->Top++ ] = Op;
    else
        PostError(ERR_TOO_COMPLEX, 0);
}

/******************************************************************************
*   Pop an operator
*******************************************************************************/
static BYTE PopOp(TOpStack *Stack)
{
    if( Stack->Top == 0 )
    {
        PostError(ERR_SYNTAX, 0);
        return 0;                       // Return something
    }

    return( Stack->Op[ --Stack->Top ] );
}

/******************************************************************************
*   Peek an operator
*******************************************************************************/
static BYTE PeekOp(TOpStack *Stack)
{
    if( Stack->Top )
        return( Stack->Op[ Stack->Top-1 ] );
    return( OP_NULL );
}

/******************************************************************************
*   Are there any more operators on the operator stack?
*******************************************************************************/
static BOOL IsEmptyOp(TOpStack *Stack)
{
    return( !Stack->Top );
}


/******************************************************************************
*
*   BOOL CheckHex(char *pToken, int nTokenLen)
*
*******************************************************************************
*
*   Checks if a string is a valid hex number.
*
*   Where:
*       pToken is the pointer to a string
*       nTokenLen is the number of characters to check
*
*   Returns:
*       TRUE - the string is a valid hex number
*       FALSE - the string is not a valid hex number
*
******************************************************************************/
static BOOL CheckHex(char *pToken, int nTokenLen)
{
    int i;                              // Counter

    if( nTokenLen && nTokenLen<=8 )
    {
        for(i=0; i<nTokenLen; i++ )
        {
            if( !isxdigit(*(pToken+i)) )
                return( FALSE );
        }

        return( TRUE );
    }

    return( FALSE );
}

/******************************************************************************
*
*   BOOL GetHex(UINT *pValue, char **ppString)
*
*******************************************************************************
*
*   Converts a string into a number. Expect a binary number.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       ppString is the address of the pointer to string
*
*   Returns:
*       FALSE - we did not read any hex digits
*       TRUE - hex digit is read into pValue, *ppString is updated
*
******************************************************************************/
static BOOL GetHex(UINT *pValue, char **ppString)
{
    char *pChar = *ppString;            // Running pointer to string
    char nibble;
    int count = 8;
    UINT value = 0;

    while(isxdigit(nibble = *pChar) && count--)
    {
        nibble = tolower(nibble);
        value <<= 4;
        value |= (nibble > '9')? nibble - 'a' + 10: nibble - '0';
        pChar++;
    }

    if( count<0 )                       // Set error if the number if too big
        PostError(ERR_TOO_BIG_HEX, 0);

    if( pChar==*ppString || deb.errorCode ) // Return FALSE if no number was read or error
        return( FALSE );

    *pValue = value;                    // Store the decimal digit final value
    *ppString = pChar;                  // Store the updated string pointer

    return(TRUE);
}

/******************************************************************************
*
*   BOOL GetDecB(UINT *pValue, char **ppString)
*
*******************************************************************************
*
*   Converts string to a decimal number, returns boolean.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       ppString is the address of the pointer to string
*
*   Returns:
*       FALSE - we did not read any decimal digits, or error
*       TRUE - decimal digit is read into pValue, *ppString is updated
*
******************************************************************************/
BOOL GetDecB(UINT *pValue, char **ppString)
{
    char *pChar = *ppString;            // Running pointer to string
    char digit;
    UINT value = 0;

    while(isdigit(digit = *pChar))
    {
        // Check for overflow (number too big)
        // Max digit is 4294967295
        if( value>429496729 )
            PostError(ERR_TOO_BIG_DEC, 0);

        value *= 10;
        value += digit - '0';
        pChar++;
    }

    if( pChar==*ppString || deb.errorCode )
        return( FALSE );

    *pValue = value;                    // Store the decimal digit final value
    *ppString = pChar;                  // Store the updated string pointer

    return(TRUE);
}

/******************************************************************************
*
*   DWORD GetDec(char **ppString)
*
*******************************************************************************
*
*   Converts string to a decimal number, without checking the error.
*
*   Where:
*       ppString is the address of the pointer to string
*
*   Returns:
*       Decimal number
*
******************************************************************************/
DWORD GetDec(char **ppString)
{
    DWORD value;                        // Store value here and return it

    GetDecB(&value, ppString);

    return( value );
}

/******************************************************************************
*
*   BOOL GetOct(UINT *pValue, char **ppString)
*
*******************************************************************************
*
*   Converts a string into a number. Expect an octal number.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       ppString is the address of the pointer to string
*
*   Returns:
*       FALSE - we did not read any octal digits
*       TRUE - hex digit is read into pValue, *ppString is updated
*
******************************************************************************/
static BOOL GetOct(UINT *pValue, char **ppString)
{
    char *pChar = *ppString;            // Running pointer to string
    UINT oct;                           // max number: 37777777777
    int count = 11;
    UINT value = 0;

    while( (oct = *pChar) && (oct>='0' && oct<='7') && count-- )
    {
        value <<= 3;
        value += oct - '0';
        pChar++;
    }

    if( count<0 )                       // Set error if the number if too big
        PostError(ERR_TOO_BIG_OCT, 0);

    if( pChar==*ppString || deb.errorCode ) // Return FALSE if no number was read or error
        return( FALSE );

    *pValue = value;                    // Store the decimal digit final value
    *ppString = pChar;                  // Store the updated string pointer

    return(TRUE);
}

/******************************************************************************
*
*   BOOL GetBin(UINT *pValue, char **ppString)
*
*******************************************************************************
*
*   Converts a string into a number. Expect a binary number.
*
*   Where:
*       pValue is the address of the variable to receive a number
*       ppString is the address of the pointer to string
*
*   Returns:
*       FALSE - we did not read any binary digits
*       TRUE - hex digit is read into pValue, *ppString is updated
*
******************************************************************************/
static BOOL GetBin(UINT *pValue, char **ppString)
{
    char *pChar = *ppString;            // Running pointer to string
    UINT bit;
    int count = 32;
    UINT value = 0;

    while( (bit = *pChar) && (bit=='0' || bit=='1') && count-- )
    {
        value <<= 1;
        value += bit - '0';
        pChar++;
    }

    if( count<0 )                       // Set error if the number if too big
        PostError(ERR_TOO_BIG_BIN, 0);

    if( pChar==*ppString || deb.errorCode ) // Return FALSE if no number was read or error
        return( FALSE );

    *pValue = value;                    // Store the decimal digit final value
    *ppString = pChar;                  // Store the updated string pointer

    return(TRUE);
}

/******************************************************************************
*
*   void scan2dec(char *pBuf, int *p1, int *p2)
*
*******************************************************************************
*
*   Scans 2 decimal numbers. This is a handy shortcut since at several places
*   we need to read type definition numbers etc.
*
*   Where:
*       pBuf is the address of the input buffer
*       p1, p2 are the destination variables to store the numbers
*
******************************************************************************/
void scan2dec(char *pBuf, int *p1, int *p2)
{
    GetDecB(p1, &pBuf);
    pBuf++;
    GetDecB(p2, &pBuf);
}

/******************************************************************************
*   TRegister *IsRegister(char *ptr, int nTokenLen)                           *
*******************************************************************************
*
*   Returns the register structure of the CPU register that match.
*
******************************************************************************/
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

/******************************************************************************
*   int EvalGetFunc( char *pToken, int nTokenLen, int params )                *
*******************************************************************************
*
*   Evaluates a string token into a function name, only for functions that
*   match the number of parameters, since we process them differently.
*
*   Where:
*       pToken is an address to the function name string
*       nTokenLen is the length of the function name string
*       params is the required number of parameters (0 or 1)
*
*   Returns:
*       Ordinal index into the function array
*       0 if no function name match
*
******************************************************************************/
static int EvalGetFunc( char *pToken, int nTokenLen, int params )
{
    int i = 0;                          // Index into function array

    while(Func[i].sName!=NULL)
    {
        if( Func[i].nArgs==params )
        {
            if(Func[i].nameLen==nTokenLen && !strnicmp(Func[i].sName, pToken, nTokenLen))
                return(i + 1);          // Return one more than the index
        }
        i++;
    }

    return( 0 );                        // Return 0 for failure
}

/******************************************************************************
*   int GetTokenLen(char *pToken)                                             *
*******************************************************************************
*
*   Returns the length of the pointed string with respect to the alphanumerical
*   test + underscore.
*
******************************************************************************/
int GetTokenLen(char *pToken)
{
    char *pTmp = pToken;

    while( *pTmp && (isalnum(*pTmp) || *pTmp=='_') ) pTmp++;

    return(pTmp - pToken);
}

/******************************************************************************
*   BYTE TableMatch( char *sTable, char **sToken )                            *
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
*   BOOL NextToken(char **ppNext)
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
*                                                                             *
*   BOOL GetValue( TExItem *item, char **sExpr, int nTokenLen )               *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string token into an item structure. Advances the pointer.
*
*   Where:
*       item is the item to fill in with the value token
*       sExpr is an address of a pointer to a string containing expression
*       nTokenLen is the number of characters that the token has
*
*   Returns:
*       TRUE - token is evaluated into item structure, *sExpr is advanced
*       FALSE - error evaluating a token, (item may be changed)
*
******************************************************************************/
static BOOL GetValue( TExItem *item, char **sExpr, int nTokenLen )
{
    TRegister *pReg;                    // Register structure for CPU register compares
    TADDRDESC Addr;                     // Address structure for breakpoint evaluation
    char *pToken = *sExpr;              // Start of the token pointer
    int n;                              // Temp variable to use locally

    // Zero out the item to avoid stale data and set up the default type to integer
    memset(item, 0, sizeof(TExItem));
    item->Type.pName = "";
    item->Type.pDef = "\1";

    //----------------------------------------------------------------------------------
    // If we switched to decimal radix, try a decimal number first
    //----------------------------------------------------------------------------------
    if( fDecimal==TRUE && GetDecB(&item->Data, &pToken) )   // pToken may be updated
    {
        // Read in a decimal literal number
        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check if the first two characters represent a hex number
    //----------------------------------------------------------------------------------
    if( *pToken=='0' && tolower(*(pToken+1))=='x' )
    {
        pToken += 2;                    // Skip the 0x prefix

        GetHex(&item->Data, &pToken);   // Ignore exit value - it has to be hex

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check if the first two characters represent a binary number
    //----------------------------------------------------------------------------------
    if( *pToken=='0' && tolower(*(pToken+1))=='b' )
    {
        pToken += 2;                    // Skip the 0b prefix

        GetBin(&item->Data, &pToken);   // Ignore exit value - it has to be binary

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check if the first two characters represent a octal number
    //----------------------------------------------------------------------------------
    if( *pToken=='0' && tolower(*(pToken+1))=='o' )
    {
        pToken += 2;                    // Skip the 0o prefix

        GetOct(&item->Data, &pToken);   // Ignore exit value - it has to be octal

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check for character constants: '1', '12', '123', '1234'
    //----------------------------------------------------------------------------------
    if( *pToken=='\'' )
    {
        n = 4;
        pToken++;

        while(*pToken && *pToken!='\'' && n--)
        {
            item->Data <<= 8;
            item->Data |= *pToken++;
        }
        if( *pToken!='\'' )             // Non-terminated character constant 'abc'
            PostError(ERR_SYNTAX, 0);

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check if the first two characters represent a character literal
    //----------------------------------------------------------------------------------
    if( *pToken=='\\' )                                 // \DEC or \xHEX
    {
        pToken++;

        if( *pToken=='x' || *pToken=='X' )              // \xHEX or \XHEX
        {
            pToken++;
            GetHex(&item->Data, &pToken);
        }
        else                                            // Else it is a decimal number
        {
            GetDecB(&item->Data, &pToken);
        }

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // The literal value may be the explicit CPU register
    //----------------------------------------------------------------------------------
    if( (pReg = IsRegister(pToken, nTokenLen)) != 0 )
    {
        item->Data = *(UINT *)((UINT) deb.r + pReg->offset);
        item->Data = (item->Data & pReg->Mask) >> pReg->rShift;
        pToken += pReg->nameLen;

        item->bType = EXTYPE_REGISTER;   // This was a register type
        item->pData = (BYTE *)((UINT) deb.r + pReg->offset);
    }
    else
    //----------------------------------------------------------------------------------
    // Check if it is a breakpoint token 'bpN' or 'bpNN'
    //----------------------------------------------------------------------------------
    if( tolower(*pToken)=='b' && tolower(*(pToken+1))=='p' && isxdigit(*(pToken+2))
      && (nTokenLen==3 || (nTokenLen==4 && isxdigit(*(pToken+3)))))
    {
        pToken += 2;                                // Advance the pointer to the HEX bp number
        GetHex(&n, &pToken);                        // Read in the breakpoint number

        if( EvalBreakpointAddress(&Addr, n) )
        {
            // Store the resulting value as a literal number
            item->bType = EXTYPE_LITERAL;           // This was a literal type
            item->Data  = Addr.offset;              // Store only offset value
            item->pData = (BYTE *)&item->Data;
        }
        else
            PostError(ERR_BPNUM, 0);
    }
    else
    //----------------------------------------------------------------------------------
    // The literal value may be a built-in function with 0 parameters
    //----------------------------------------------------------------------------------
    if( (n = EvalGetFunc(pToken, nTokenLen, 0)) != 0 )
    {
        // Functons with 0 parameters are treated differently from functions with 1 parameter

        // Call the function directly and return the value
        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->Data  = (*Func[n-1].funct)(0);
        item->pData = (BYTE *)&item->Data;

        pToken += nTokenLen;
    }
    else
    //----------------------------------------------------------------------------------
    // Check for the symbol name
    //----------------------------------------------------------------------------------
    if( FindSymbol(item, pToken, &nTokenLen) )
    {
        // Symbol was found. Get the canonical version of its type

        TypedefCanonical(&item->Type);

        pToken += nTokenLen;
    }
    else
    //----------------------------------------------------------------------------------
    // Check for the hex value
    //----------------------------------------------------------------------------------
    if( CheckHex(pToken, nTokenLen) )
    {
        GetHex(&item->Data, &pToken);

        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;
    }
    else
    //----------------------------------------------------------------------------------
    // Check the user variable
    //----------------------------------------------------------------------------------
    if( GetUserVar(&item->Data, pToken, nTokenLen) )
    {
        // Found and evaluated a user variable
        item->bType = EXTYPE_LITERAL;   // This was a literal type
        item->pData = (BYTE *)&item->Data;

        pToken += nTokenLen;
    }
    else
    {
        // If everything else fails, check if it is a token handled by DOT-extension
        if( QueryExtToken(&item->Data, &pToken, nTokenLen)==TRUE )
        {
            item->bType = EXTYPE_LITERAL;   // This was a literal type
            item->pData = (BYTE *)&item->Data;
        }
        else
        {
            // Token is nothing that we can recognize at this point
            return( FALSE );
        }
    }

    fDecimal = FALSE;
    *sExpr = pToken;

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL EvalGetTypeCast( TExItem *item, char **sExpr, int nTokenLen )        *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string token into a typedef cast. This is used for type casting
*   (overloading) of one symbol type or a value with another.
*
*   Where:
*       item is the item to fill in with the value token
*       sExpr is an address of a pointer to a string containing expression
*
*   Returns:
*       TRUE - token is evaluated into item structure, *sExpr is advanced
*       FALSE - error evaluating a token, (item may be changed)
*
******************************************************************************/
static BOOL EvalGetTypeCast( TExItem *Item, char **sExpr, int nTokenLen )
{
    TSYMTYPEDEF1 *pType1;

    // We can type cast only if we have active symbols and scope loaded
    if( deb.pFnScope )
    {
        // Get the type of the given token
        if( (pType1 = Type2Typedef(*sExpr, nTokenLen, deb.pFnScope->file_id)) )
        {
            // Type was found. Copy its descriptor into the item and get the canonical version of it
            memcpy(&Item->Type, pType1, sizeof(TSYMTYPEDEF1));

            TypedefCanonical(&Item->Type);

            Item->bType = EXTYPE_SYMBOL;    // This item is a symbol type now...

            return( TRUE );
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL EvalGetArray( TStack *Values, TOpStack *Operators, TExItem *Item )   *
*                                                                             *
*******************************************************************************
*
*   This function is used when closing on an array brackets []
*   The resulting item has to represent an array type, which is then evaluated
*   and the array member that is indexed is placed instead of the original
*   symbol.
*
*   Where:
*       Values - operands stack
*       Operators - operators stack
*       Item - Final item that is to be generated
*
*   Returns:
*       TRUE - We successfully evalated an array type
*       FALSE - We could not evaluate the array type
*
******************************************************************************/
static BOOL EvalGetArray( TStack *Values, TOpStack *Operators, TExItem *Item )
{
    TSYMTYPEDEF1 *pType1;               // Type of the array element
    TExItem item1, item2;               // Array item and the index item
    UINT index, nSize;                  // Actual index of an array and its size
    char *pDef;                         // Pointer to the type definition

    // The last operand is the index (item2); the one before is the array (item1)
    Pop(Values, &item2);
    Pop(Values, &item1);

    // Check that the operand 1 is a valid symbol array
    if( item1.bType==EXTYPE_SYMBOL && item1.Type.pDef && *item1.Type.pDef=='a' )
    {
        // Check that the operand 2 is a valid literal or register type
        if( item2.bType==EXTYPE_LITERAL || item2.bType==EXTYPE_REGISTER )
        {
            GlobalReadDword(&index, (DWORD) item2.pData);   // Read the actual index

            // Get the type of the array element
            if( (pDef = strrchr(item1.Type.pDef, '(')) )
            {
                // Find the type of the element
                if( (pType1 = Type2Typedef(pDef, 0, deb.pFnScope->file_id)) )
                {
                    nSize = GetTypeSize(pType1);

                    // Advance the pointer (address) by index * size bytes

                    item1.pData = item1.pData + index * nSize;

                    // Change the final type to that of the element

                    memcpy(&item1.Type, pType1, sizeof(TSYMTYPEDEF1));

                    TypedefCanonical(&item1.Type);

                    // Push the final resulting, coalesced array subtype

                    Push(Values, &item1);

                    return( TRUE );
                }
            }
        }
    }
    PostError(ERR_NOTARRAY, 0);

    return( FALSE );
}

/******************************************************************************
*
*   BOOL GetValueStructureMember(TStack *Values, TOpStack *Operators, TExItem *Item, char **sExpr, int nTokenLen )
*
*******************************************************************************
*
*   This function is used to try to evaluate a structure member before the
*   operation -> or . is being pushed on the stack. The member can not be
*   recognized later since it is not in the list of root symbols.
*
*   Where:
*       Values - stack for operands
*       Operators - stack for operators
*       Item - resulting item that needs to be filled up
*       sExpr - points to the address of the structure member
*       nTokenLen - length of the structure member token string
*
*   Returns:
*       TRUE - New Item is a coalesced structure and its member
*       FALSE - Did not evaluate into the structure and its member
*
******************************************************************************/
static BOOL GetValueStructureMember(TStack *Values, TOpStack *Operators, TExItem *Item, char **sExpr, int nTokenLen )
{
    TExItem *pLastItem;
    TExItem LastItem;
    char *pType;
    BYTE bOp;                           // Last operator value
    UINT nOffset;                       // Offset of the element within that structure

    // Peek the data type of the last value - it has to be a symbol type with a structure definiton
    if( (pLastItem = Peek(Values)) )
    {
        if( pLastItem->bType==EXTYPE_SYMBOL )
        {
            if( pLastItem->Type.pDef && *pLastItem->Type.pDef=='s' )
            {
                // We got the base type to be a symbol with a structure, compare the token with each of the
                // elements of that structure to see if we can resolve this indirection in place

                if( (pType = Type2Element(&pLastItem->Type, *sExpr, nTokenLen)) )
                {
                    // Found the structure element. Pop the base type since we will be adjusting few things
                    Pop(Values, &LastItem);

                    bOp = PopOp(Operators);             // Pop the operator -> or .

                    // Fill the resulting type, that is the type of the structure element
                    EvalGetTypeCast(Item, &pType, nTokenLen);

                    // Read the offset of this element from the definition string, and add it to the structure address
                    pType = strchr(pType, ')') + 2;
                    GetDecB(&nOffset, &pType); pType = NULL;

                    Item->pData = LastItem.pData + nOffset/8;

                    // The resulting item has the new name and new type - resolve new type to its canonical definition

                    Item->bType = EXTYPE_SYMBOL;        // The resulting item is a symbol

                    *sExpr += nTokenLen;                // Adjust the pointer to the expression string to past this token

                    return( TRUE );
                }
            }
        }
    }

    return( FALSE );
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
    DWORD Data1, Data2;                 // Actual data from 2 operands

    // Most operations require 2 parameters. Just some are unary (one operand)
    switch( Operation )
    {
        case OP_UNARY_AT:               // @symbol
        case OP_UNARY_AND:              // &symbol
        case OP_UNARY_PTR:              // *symbol

        case OP_NOT:                    // !
        case OP_BITWISE_NOT:            // ~
        case OP_UNARY_MINUS:            // -(num)
        case OP_UNARY_PLUS:             // +(num)
        case OP_LINE_NUMBER:            // .(num)

            Pop(Values, &item1);

            // Get the data value to simplyfy the expressions
            GlobalReadDword(&Data1, (DWORD)item1.pData);

            break;

        default:
            Pop(Values, &item2);
            Pop(Values, &item1);

            // Get the data values to simplyfy the expressions
            GlobalReadDword(&Data1, (DWORD)item1.pData);
            GlobalReadDword(&Data2, (DWORD)item2.pData);
    }

    // By default, use the data type of the first operand
    memcpy(&itemTop.Type, &item1.Type, sizeof(TSYMTYPEDEF1));

    // If the left side is symbol containing the complex definition,
    // there are only certain operations allowed on it:

    if( item1.bType==EXTYPE_SYMBOL && item1.Type.pDef && *item1.Type.pDef>TYPEDEF__LAST )
    {
        switch( Operation )
        {
            // ------------------------- UNARY OPERATIONS -------------------------

            case OP_UNARY_AT:           // @symbol -> makes an address type
                itemTop.bType = EXTYPE_ADDRESS;
                itemTop.wSel  = deb.r->ds;              // Get the current data selector
                itemTop.Data  = (DWORD) item1.pData;
                itemTop.pData = (BYTE*) &itemTop.Data;
                break;

            case OP_UNARY_AND:          // &symbol -> makes a literal type
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = (DWORD) item1.pData;
                itemTop.pData = (BYTE*) &itemTop.Data;
                break;

            case OP_UNARY_PTR:          // *symbol -> increases the pointer level of the symbol (canonical form)
                CopyItem(&itemTop, &item1);
                itemTop.Type.maj++;
                break;

            // ------------------------- BINARY OPERATIONS -------------------------

            case OP_TYPECAST:           // Assign type of a symbol
                // Data is held in the item2, and the type in item1; form the final item
                memcpy(&itemTop.Type, &item1.Type, sizeof(TSYMTYPEDEF1));   // Just use the type from the casting item
                itemTop.bType = EXTYPE_SYMBOL;                              // Making it a symbol type
                if( item2.bType==EXTYPE_SYMBOL )                            // If the casted item is a symbol
                    itemTop.pData = item2.pData;                            // Just take the pointer to it
                else                                                        // Othervise, for all other types
                    itemTop.pData = (BYTE *)Data2;                          // take the address of a symbol from the data
                break;

            case OP_PLUS:               // symbol + 1
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = (DWORD) item1.pData + Data2;
                itemTop.pData = (BYTE*) &itemTop.Data;
                break;

            case OP_MINUS:              // Symbol - 1
                itemTop.bType = EXTYPE_LITERAL;
                itemTop.Data  = (DWORD) item1.pData - Data2;
                itemTop.pData = (BYTE*) &itemTop.Data;
                break;

            default:
                PostError(ERR_INVALIDOP, 0);
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
            itemTop.Data = Data1 || Data2;
            break;
        //--------------------------------------------------------------------
        case OP_BOOL_AND:
            itemTop.Data = Data1 && Data2;
            break;
        //--------------------------------------------------------------------
        case OP_OR:
            itemTop.Data = Data1 | Data2;
            break;
        //--------------------------------------------------------------------
        case OP_XOR:
            itemTop.Data = Data1 ^ Data2;
            break;
        //--------------------------------------------------------------------
        case OP_AND:
            itemTop.Data = Data1 & Data2;
            break;
        //--------------------------------------------------------------------
        case OP_EQ:
            itemTop.Data = Data1 == Data2;
            break;
        //--------------------------------------------------------------------
        case OP_NE:
            itemTop.Data = Data1 != Data2;
            break;
        //--------------------------------------------------------------------
        case OP_L:
            itemTop.Data = (DWORD)((signed)Data1 < (signed)Data2);
            break;
        //--------------------------------------------------------------------
        case OP_LE:
            itemTop.Data = (DWORD)((signed)Data1 <= (signed)Data2);
            break;
        //--------------------------------------------------------------------
        case OP_G:
            itemTop.Data = (DWORD)((signed)Data1 > (signed)Data2);
            break;
        //--------------------------------------------------------------------
        case OP_GE:
            itemTop.Data = (DWORD)((signed)Data1 >= (signed)Data2);
            break;
        //--------------------------------------------------------------------
        case OP_SHL:
            itemTop.Data = Data1 << Data2;
            break;
        //--------------------------------------------------------------------
        case OP_SHR:
            itemTop.Data = Data1 >> Data2;
            break;
        //--------------------------------------------------------------------
        case OP_PLUS:
            itemTop.Data = Data1 + Data2;
            break;
        //--------------------------------------------------------------------
        case OP_MINUS:
            itemTop.Data = Data1 - Data2;
            break;
        //--------------------------------------------------------------------
        case OP_TIMES:
            itemTop.Data = Data1 * Data2;
            break;
        //--------------------------------------------------------------------
        case OP_DIV:
            if( Data2 )
                itemTop.Data = Data1 / Data2;
            else
                PostError(ERR_DIV0, 0);
            break;
        //--------------------------------------------------------------------
        case OP_MOD:
            if( Data2 )
                itemTop.Data = Data1 % Data2;
            else
                PostError(ERR_DIV0, 0);
            break;
        //--------------------------------------------------------------------
        case OP_NOT:
            itemTop.Data = (DWORD)(!(signed)Data1);
            break;
        //--------------------------------------------------------------------
        case OP_BITWISE_NOT:
            itemTop.Data = ~Data1;
            break;
        //--------------------------------------------------------------------
        case OP_UNARY_MINUS:
            itemTop.Data = (DWORD)(-(signed)Data1);
            break;
        //--------------------------------------------------------------------
        case OP_UNARY_PLUS:
            // We dont do anything special for unary plus
            itemTop.Data = Data1;
            break;
        //--------------------------------------------------------------------
        case OP_DOT:
            itemTop.Data = Data1;          // TEST  type.element or eax.4
            break;
        //--------------------------------------------------------------------
        case OP_PTR:
            itemTop.Data = fnPtr(Data1);    // TEST type->element or eax->4
            break;
        //--------------------------------------------------------------------
        case OP_LINE_NUMBER:
            // Line number has to have the corresponding address, report error otherwise
            if( (itemTop.Data = SymLinNum2Address(Data1))==0 )
                PostError(ERR_BPLINE, 0);
            break;
        //--------------------------------------------------------------------
        case OP_SELECTOR:   // Selector:offset
            // The left side contains the selector token, the right side offset
            // We merge them into one address-type token
            itemTop.bType = EXTYPE_ADDRESS;
            itemTop.wSel  = Data1;                      // Selector
            itemTop.Data  = Data2;                      // Offset
            itemTop.pData = (BYTE*) &itemTop.Data;
            if(Data1 > 0xFFFF)                          // Selector has to be the valid size
                PostError(ERR_SELECTOR, (UINT) Data1);

            evalSel = Data1;                            // Selector

            // Do a separate ending since we changed the item type
            Push(Values, &itemTop);

            return;
        //--------------------------------------------------------------------
        case OP_FUNCTION1:
            // Function with 1 parameter; however, it uses the 2 parameter path here
            // since the bottom (first) parameter is the function index
            if( Data1<=MAX_FUNCTION )
            {
                // Call the function with the second parameter as argument
                itemTop.Data = (*Func[Data1-1].funct)(Data2);
            }
            break;
    }

    // The final type is a literal item
    itemTop.bType = EXTYPE_LITERAL;
    itemTop.pData = (BYTE*) &itemTop.Data;

    // At the end, we always push the resulting value onto the stack

    Push(Values, &itemTop);
}

/******************************************************************************
*
*   BOOL Evaluate(TExItem *pItem, char *pExpr, char **ppNext, BOOL fSymbolsValueOf)
*
*******************************************************************************
*
*   Evaluates an expression.
*
*   Where:
*       pItem - the expression item to fill in with the result of evaluation
*       pExpr - expression string
*       ppNext - optional variable to store the end of the expression
*       fSymbolsValueOf - if set to TRUE, symbols will tend to evaluate as value-of instead of address-of
*
*   Returns:
*       TRUE - expression evaluated ok and the item is stored
*       FALSE - error evalauting expression; deb.errorCode has the error code
*
******************************************************************************/
BOOL Evaluate(TExItem *pItem, char *pExpr, char **ppNext, BOOL fSymbolsValueOf)
{
    TOpStack Operators = { 0, };        // Operators stack, initialized to 0 elements
    TStack   Values    = { 0, };        // Values stack, initialized to 0 elements
    TExItem Item;
    char *myNext;                       // This pointer is used if ppNext==NULL
    BYTE NewOp;
    BOOL fExpectValue = TRUE;           // Expect operand value (as opposed to operator)
    int nTokenLen;                      // Length of the input token
    int nFunc1;                         // Temp index of the function

//  deb.error = NOERROR;                // Reset the error variable to no error (TODO: ???)
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
    while( NextToken(&pExpr) && !deb.errorCode )
    {
        if( fExpectValue )
        {
            // We are expecting a operand (value) or an unary operator

            // Check for the unary operator
            if( (NewOp = TableMatch( &pExpr)) )
            {
                // Modify some operators that are unary at this point
                // Left bracket is simply pushed onto the stack as a placeholder
                switch(NewOp)
                {
                // Appropriately for this application, +/- also signals a decimal number
                case OP_PLUS:           NewOp = OP_UNARY_PLUS,  fDecimal = TRUE; break;
                case OP_MINUS:          NewOp = OP_UNARY_MINUS, fDecimal = TRUE; break;
                case OP_DOT:            NewOp = OP_LINE_NUMBER, fDecimal = TRUE; break;
                case OP_AND:            NewOp = OP_UNARY_AND;                    break;
                case OP_TIMES:          NewOp = OP_UNARY_PTR;                    break;
                case OP_BRACKET_START:  NewOp = OP_BRACKET_START;                break;
                };

                // Push the new operator on the operator stack
                PushOp(&Operators, NewOp);

                continue;
            }
            else
            {
                // It was not unary value or a left bracket, so it has to be an operand

                // Find the length of the token in the input buffer (We assume it is a token)
                nTokenLen = GetTokenLen(pExpr);

                // Special case are structure elements as in X->Y or X.Y, where we wont be able to find
                // Y in a symbol database since it is just a part of X's definition string.
                // So, we peek the operator and do slightly more checking for them.
                if( !((PeekOp(&Operators)==OP_PTR || PeekOp(&Operators)==OP_DOT)
                   && GetValueStructureMember(&Values, &Operators, &Item, &pExpr, nTokenLen)) )
                {
                    // Get the next token - it should be an operand
                    if( !GetValue(&Item, &pExpr, nTokenLen) )
                    {
                        // If the token was not an operand, check if it is a type case
                        if( EvalGetTypeCast(&Item, &pExpr, nTokenLen) )
                        {
                            // The token was a type cast. Insert the type case operand and we'll proceed to push the item
                            PushOp(&Operators, OP_TYPECAST);

                            pExpr += nTokenLen;
                        }
                        else
                        {
                            // If the token was not a type cast, check if it is an internal function with 1 argument
                            if( (nFunc1 = EvalGetFunc(pExpr, nTokenLen, 1)) )
                            {
                                // The token was a function that takes 1 parameter. Insert the function token,
                                // then push the index of the function
                                PushOp(&Operators, OP_FUNCTION1);

                                Item.bType = EXTYPE_LITERAL;        // Function index is a literal number
                                Item.Data  = nFunc1;                // Store the index
                                Item.pData = (BYTE*) &Item.Data;    // and the pointer to it

                                pExpr += nTokenLen;
                            }
                            else
                                break;
                        }

                        Push(&Values, &Item);

                        continue;
                    }
                }

                Push(&Values, &Item);

                fExpectValue = FALSE;   // Next token has to be an operator
            }
        }
        else
        {
            // We are expecting only an operator, but not unary, and not left paren
            // but it could be a closing bracket

            if( (NewOp = TableMatch( &pExpr)) )
            {
                // Right bracket needs stack unwinding: ]
                if( NewOp==OP_BRACKET_END )
                {
                    while( PeekOp(&Operators)!=OP_BRACKET_START && !deb.errorCode)
                    {
                        Execute(&Values, PopOp(&Operators));
                    }

                    // Here we have to find left bracket, otherwise it's syntax error
                    if( PopOp(&Operators)!=OP_BRACKET_START )
                        PostError(ERR_SYNTAX, 0);

                    // Evaluate the array index
                    EvalGetArray(&Values, &Operators, &Item);

                    continue;
                }

                // Right parenthesis needs stack unwinding
                if( NewOp==OP_PAREN_END )
                {
                    while( PeekOp(&Operators)!=OP_PAREN_START && !deb.errorCode)
                    {
                        Execute(&Values, PopOp(&Operators));
                    }

                    // Here we have to find left paren, otherwise it's syntax error
                    if( PopOp(&Operators)!=OP_PAREN_START )
                        PostError(ERR_SYNTAX, 0);

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

                    fExpectValue = TRUE;    // Next token has to be a value (operand)

                    continue;
                }
            }

            // If we reached this point, the token was not legal
            break;
        }
    }

    // Evaluate what's left on the stack
    while( !IsEmptyOp(&Operators) )
    {
        Execute(&Values, PopOp(&Operators));
    }

    // At this point the opeator stack should be empty, and the value stack should have only value in it
    if( IsEmptyOp(&Operators) && Values.Top==1 && !deb.errorCode)
    {
        // Success!
        *ppNext = pExpr;

        Pop(&Values, pItem);

        return( TRUE );
    }

    // If we did not return due to the previous clause, post a syntax error if no other error was logged
    PostError(ERR_SYNTAX, 0);

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL Expression(DWORD *pValue, char *pExpr, char **ppNext)                *
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
BOOL Expression(DWORD *pValue, char *pExpr, char **ppNext)
{
    TExItem Item;                       // Store the result of the evaluation

    if( Evaluate(&Item, pExpr, ppNext, FALSE) )
    {
        // Literal contains the value in the *pData
        // Register contains the register value in the *pData
        // Symbol contains the address of the symbol in *pData
        // Address contains the offset in *pData

        // Assign the value from the item pointer
        if( GlobalReadDword(pValue, (DWORD)Item.pData) )
            return( TRUE );

        // Final pointer error
    }

    return( FALSE );
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
BOOL cmdEvaluate(char *args, int subClass)
{
    static char buf[MAX_STRING];        // Temp output buffer
    TExItem Item;                       // Store the result of the evaluation
    char *pEnd;                         // Pointer to the end of expression string
    DWORD Data;                         // Temp data store
    int i;                              // Temp store

    if( *args )
    {
        if( Evaluate(&Item, args, &pEnd, TRUE) && !*pEnd )
        {
            switch( Item.bType )
            {
                //-----------------------------------------------------------------
                // Symbol is expanded
                //-----------------------------------------------------------------
                case EXTYPE_SYMBOL:
                {
                    // Print the expanded symbol
                    ExpandPrintSymbol(&Item, args);
                }
                break;

                //-----------------------------------------------------------------
                // Address contains the offset in the Data field
                //  pData is the selector value
                //  pType is the name of the selector (or NULL)
                //-----------------------------------------------------------------
                case EXTYPE_ADDRESS:
                {
                    i = sprintf(buf, " Address: %04X:%08X", (int)Item.pData, Item.Data);

                    dprinth(1, "%s", buf);
                }
                break;

                //-----------------------------------------------------------------
                // Literal contains the value in the *pData
                // Register contains the register value in the *pData
                //-----------------------------------------------------------------
                // case EXTYPE_LITERAL:
                // case EXTYPE_REGISTER:
                default:
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
            }
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
        PostError(ERR_EXP_WHAT, 0);

    return( TRUE );
}

