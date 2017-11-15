/******************************************************************************
*                                                                             *
*   Module:     Eval.c                                                        *
*                                                                             *
*   Date:       5/15/97                                                       *
*                                                                             *
*   Copyright (c) 1997, 2000 Goran Devic                                      *
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
            * user variable name (prefix $)
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

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Predefined default base is hex and there are no literal handlers.

WORD evalSel = 0x0000;                  // Selector result of the expression (optional)
int nEvalDefaultBase = 16;
int (*pfnEvalLiteralHandler)( char *sName ) = NULL;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

BOOL fDecimal = FALSE;      // Prefer decimal number

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


// Define internal functions that can be performed on a subexpression

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
{ "bpIndex", 7, 0, fnBpIndex },
{ "bplog",   5, 0, fnBpLog },

{ "ptr",     3, 1, fnPtr },
{ NULL }
};

#define MAX_RECURSE     5               // Max literal reentrancy depth

#define MAX_STACK       10              // Max depth of a stack structure

#define BOTTOM_STACK    0               // Stack empty code
#define OP_SELECTOR     1               // Selector:offset
#define OP_PAREN_START  2               // Priority codes and also indices
#define OP_PAREN_END    3               //   into the sTokens array
#define OP_POINTER1     4
#define OP_DOT          5
#define OP_PTR          6
#define OP_BOOL_OR      7
#define OP_BOOL_AND     8
#define OP_OR           9
#define OP_XOR          10
#define OP_AND          11
#define OP_EQ           12
#define OP_NE           13
#define OP_SHL          14
#define OP_SHR          15
#define OP_LE           16
#define OP_L            17
#define OP_GE           18
#define OP_G            19
#define OP_PLUS         20
#define OP_MINUS        21
#define OP_TIMES        22
#define OP_DIV          23
#define OP_MOD          24
#define OP_NOT          25
#define OP_LINE         254             // Unary .dot is the line number
#define OP_NEG          255             // Unary minus has highest priority

static char *sTokens[] =
{
    ":",
    "(", ")",
    "->",
    ".",
    "@",
    "||",
    "&&",
    "|", "^", "&",
    "==", "!=",
    "<<", ">>",
    "<=", "<", ">=", ">",
    "+", "-",
    "*", "/", "%",
    "!",
    NULL
};


// Define stack structure

typedef struct
{
    int Data[ MAX_STACK ];              // Stack data
    int Top;                            // Top of stack index
} TStack;

static const char sDelim[] = ",;\"";    // Expressions delimiters - break chars
static const char sLiteral[] = "@!_";   // These are allowed in literal names

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern BOOL GetUserVar(DWORD *pValue, char *sStart, int nLen);
extern BOOL EvalBreakpointAddress(TADDRDESC *pAddr, int index);


int Evaluate( char *sExpr, char **psNext );

static DWORD fnByte(DWORD arg) { return(arg & 0xFF); }
static DWORD fnWord(DWORD arg) { return(arg & 0xFFFF);}
static DWORD fnDword(DWORD arg) { return(arg);}
static DWORD fnHiword(DWORD arg) { return(arg >> 16);}

/******************************************************************************
*                                                                             *
*   void Push( TStack *, int Value )                                          *
*                                                                             *
*******************************************************************************
*
*   Pushes a value into a given stack structure.
*
*   Where:
*       Stack is a pointer to a stack structure
*       Value is a number to push
*
*   Returns:
*       void
*
******************************************************************************/
static void Push( TStack *Stack, int Value )
{
    if( Stack->Top < MAX_STACK )
        Stack->Data[ Stack->Top++ ] = Value;
}


/******************************************************************************
*                                                                             *
*   int Pop( TStack * )                                                       *
*                                                                             *
*******************************************************************************
*
*   Returns a value from the top of a given stack.
*
*   Where:
*       Stack is a pointer to a stack structure
*
*   Returns:
*       Value from the top of a stack.  The value is removed from the stack.
*
******************************************************************************/
static int Pop( TStack *Stack )
{
    if( Stack->Top == 0 )               // If the stack starved, report
        deb.error = ERR_SYNTAX;         // syntax error

    if( Stack->Top == 0 )
        return( BOTTOM_STACK );
    else
        return( Stack->Data[ --Stack->Top ] );
}


/******************************************************************************
*                                                                             *
*   int PopAny( TStack *Stack )                                               *
*                                                                             *
*******************************************************************************
*
*   Returns a value from the top of a given stack.
*
*   Where:
*       Stack is a pointer to a stack structure
*
*   Returns:
*       Value from the top of a stack.  The value is removed from the stack.
*
******************************************************************************/
static int PopAny( TStack *Stack )
{
    if( Stack->Top == 0 )
        return( BOTTOM_STACK );
    else
        return( Stack->Data[ --Stack->Top ] );
}


/******************************************************************************
*                                                                             *
*   int GetValue( char **sExpr )                                              *
*                                                                             *
*******************************************************************************
*
*   Returns a numerical value of a string.  Advances the given pointer.
*
*   Where:
*       sExpr is an address of a pointer to a string containing expression.
*
*   Returns:
*       Numerical value that the string contains.
*
******************************************************************************/

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


/******************************************************************************
*                                                                             *
*   DWORD GetDec(char **psString)                                             *
*                                                                             *
*******************************************************************************
*
*   Converts string to a decimal number.
*
*   Where:
*       psString is the address of the string pointer - will be modified
*
*   Returns:
*       Decimal number
*       string pointer advanced to the next non-decimal character
*
******************************************************************************/
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

TRegister *IsRegister(char *ptr, int nTokenLen)
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


TFunction *IsFunc(char *ptr, int nTokenLen)
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

/******************************************************************************
*                                                                             *
*   int GetValue( char **sExpr )                                              *
*                                                                             *
*******************************************************************************
*
*   Returns a numerical value of a string.  Advances the given pointer.
*
*   Where:
*       sExpr is an address of a pointer to a string containing expression.
*
*   Returns:
*       Numerical value that the string contains.
*
******************************************************************************/
static int GetValue( char **sExpr )
{
    TADDRDESC Addr;
    int value, n;
    char *sStart = *sExpr, *sTmp;
    TRegister *pReg;
    TFunction *pFunc;
    int nTokenLen;                      // Length of the input token

    // Find the length of the token in the input buffer (We assume it is a token)
    sTmp = sStart;

    if( *sTmp=='$' ) sTmp++;            // Allow special case of '$'

    while( *sTmp && (isalnum(*sTmp) || *sTmp=='_') ) sTmp++;

    nTokenLen = sTmp - sStart;

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
            value = GetHex(&sStart);
        }
        else
        {
            sStart += 1;
            value = GetDec(&sStart);
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
    }
    else
    // The first precedence literal value is the symbol name
    if( SymbolName2Value(deb.pSymTabCur, (DWORD *)&value, sStart, nTokenLen) )
    {
        // Symbol name is found and assigned to value in the function above
        sStart += nTokenLen;
    }
    else
    // Check if it is a breakpoint token 'bpN' or 'bpNN'
    if( tolower(*sStart)=='b' && tolower(*(sStart+1))=='p' && isxdigit(*(sStart+2)) )
    {
        sStart += 2;

        n = (int) GetHex(&sStart);

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
                value = Evaluate(sStart, &sStart);
                value = (pFunc->funct)(value);
            break;
        }
    }
    else
    // The literal value may be the user variable
    if( *sStart=='$' )
    {
        sStart += 1;

        if( GetUserVar((DWORD *)&value, sStart, nTokenLen)==FALSE )
        {
            dprinth(1, "Variable \"%s\" not found", substr(sStart, 0, nTokenLen) );
        }
    }
    else
    // If everything else fails, it's gotta be a hex number
    {
        value = GetHex(&sStart);
    }
End:
    fDecimal = FALSE;
    *sExpr = sStart;

    return( value );
}


/******************************************************************************
*                                                                             *
*   int TableMatch( char *sTable, char **sToken )                             *
*                                                                             *
*******************************************************************************
*
*   Returns a matching string from a token table or 0 if there was no match.
*   sToken pointer is advanced accordingly.  Spaces are ignored.
*
*   Where:
*       sTable is an array of pointers pointing to tokens.  Last entry is NULL.
*       sToken is an address of a pointer to string to be examined.
*
*   Returns:
*       Token number from an array or 0 if failed to match a token.
*
******************************************************************************/
static int TableMatch( char **sTable, char **sToken )
{
    char *sRef = sTable[0];
    int index = 0;

    // Skip over spaces
    while( *(*sToken)==' ' )
        *sToken += 1;

    // Find the matching substring in a table
    while( sRef != NULL )
    {
        if( !strncmp( sRef, *sToken, strlen(sRef) ) )
        {
            *sToken += strlen( sRef );

            return( index + 1 );
        }

        sRef = sTable[ ++index ];
    }

    return( 0 );
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
    int top;

    // Perform the operation
    switch( Operation )
    {
        case OP_BOOL_OR:Push( Values, Pop( Values ) || Pop( Values ) );  break;
        case OP_BOOL_AND:Push(Values, Pop( Values ) && Pop( Values ) );  break;
        case OP_OR:     Push( Values, Pop( Values ) |  Pop( Values ) );  break;
        case OP_XOR:    Push( Values, Pop( Values ) ^  Pop( Values ) );  break;
        case OP_AND:    Push( Values, Pop( Values ) &  Pop( Values ) );  break;
        case OP_EQ:     Push( Values, Pop( Values ) == Pop( Values ) );  break;
        case OP_NE:     Push( Values, Pop( Values ) != Pop( Values ) );  break;
        case OP_L:      top = Pop( Values ); Push( Values, Pop( Values ) <  top );  break;
        case OP_LE:     top = Pop( Values ); Push( Values, Pop( Values ) <= top );  break;
        case OP_G:      top = Pop( Values ); Push( Values, Pop( Values ) >  top );  break;
        case OP_GE:     top = Pop( Values ); Push( Values, Pop( Values ) >= top );  break;
        case OP_SHL:    top = Pop( Values ); Push( Values, Pop( Values ) << top );  break;
        case OP_SHR:    top = Pop( Values ); Push( Values, Pop( Values ) >> top );  break;
        case OP_PLUS:   Push( Values, Pop( Values ) +  Pop( Values ) );  break;
        case OP_MINUS:  top = Pop( Values );  Push( Values, Pop( Values ) - top );  break;
        case OP_TIMES:  Push( Values, Pop( Values ) * Pop( Values ) );   break;

        case OP_DIV:    top = Pop( Values );
                        if( top != 0 )
                            Push( Values, Pop( Values ) / top );
                        else
                            deb.error = ERR_DIV0;
            break;
        case OP_MOD:    top = Pop( Values );
                        if( top != 0 )
                            Push( Values, Pop( Values ) % top );
            break;
        case OP_NOT:    Push( Values, ! Pop( Values ) );  break;
        case OP_NEG:    Push( Values, -Pop( Values ) );   break;
        case OP_POINTER1:
        case OP_DOT:    Push( Values, Pop( Values ) + Pop( Values ) );   break;  // TEST
        case OP_PTR:    Push( Values, fnPtr(Pop( Values )) );             break;
        case OP_LINE:   Push( Values, SymLinNum2Address(Pop( Values )) ); break;  // TEST

        case OP_SELECTOR:   // Selector:offset
                // Whatever we did so far was to find the selector part.
                // We store it in the global variable and keep the offset on the stack
                top = Pop( Values );
                evalSel = Pop( Values );
                Push( Values, top );
            break;
    }
}


/******************************************************************************
*                                                                             *
*   int Evaluate2( char *sExpr, char **psNext )                               *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string expression and returns its numerical value. Since this
*   function can be called recursively from within itself, we limit the
*   recursion level.
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
int Evaluate2( char *sExpr, char **psNext )
{
    static int nDeep = 0;               // Recursion depth count
    TStack Values, Operators;
    int NewOp, OldOp, ExpectValue;

    Values.Top = Operators.Top = ExpectValue = 0;
    fDecimal = FALSE;

    // Just in the case that the argument was NULL - return the result of 0.
    if( sExpr==NULL )
    {
        if( psNext!=NULL ) *psNext = NULL;
        return( 0 );
    }

    if( nDeep >= MAX_EVAL_RECURSE )
        return(0);
    nDeep++;                            // Inside the function, recursion count

    // Loop for any new term and stop when hit one of delimiter characters
    while( strchr(sDelim,*sExpr)==NULL )
    {
        NewOp = TableMatch( sTokens, &sExpr);

        // Special cases are the unary operands:
        //    -/+ sets the default radix to 10
        //    * @ are the pointers
        //    . is the decimal line number
        if(ExpectValue==0)
        {
            switch(NewOp)
            {
            case OP_MINUS:      NewOp = OP_NEG, fDecimal = TRUE; break;
            case OP_PLUS:       fDecimal = TRUE; continue;
            case OP_TIMES:
            case OP_PTR:        NewOp = OP_PTR; break;
            case OP_DOT:        NewOp = OP_LINE, fDecimal = TRUE; break;
            }
        }

        // If any operator was in front of a value, push it
        if( NewOp )
        {
            Push( &Operators, NewOp );
            ExpectValue = 0;

            // If the operator was not -/+, reset decimal radix hint
//            if(NewOp!=OP_MINUS && NewOp!=OP_PLUS && NewOp!=OP_LINE)
//                fDecimal = FALSE;

            continue;
        }

        Push( &Values, GetValue( &sExpr ) );
        ExpectValue = 1;
        NewOp = TableMatch( sTokens, &sExpr);

        // If there are no more operators, break out and clean the stack
        if( ! NewOp )
            break;

        OldOp = PopAny( &Operators );

        // If the new op priority is less than the one on the stack, we
        // need to go down and evaluate terms
        if( OldOp != BOTTOM_STACK )
        {
            if( NewOp < OldOp )
                Execute( &Values, OldOp );
            else
                Push( &Operators, OldOp );
        }

        // Push new operation
        Push( &Operators, NewOp );
    }

    // Clean the stack by the means of evaluating expression in RPN
    // This is the only place where we can expect stack to starve,
    // so we can pop anything without a check
    while( (NewOp = PopAny(&Operators)) != BOTTOM_STACK )
    {
        Execute( &Values, NewOp );
    }

    // Store the logical end of the expression
    if( psNext!=NULL ) *psNext = sExpr;

    nDeep--;                            // Leaving the function, recursion count
    // Return the last value on the stack
    return( Pop( &Values ) );
}


/******************************************************************************
*                                                                             *
*   BOOL Expression(DWORD *value, char *sExpr, char **psNext )                *
*                                                                             *
*******************************************************************************
*
*   Evaluates an expression.
*
*   Returns:
*       TRUE - expression ok: *value = result
*       FALSE - expression was invalid, deb.error contains the expression error code
*
******************************************************************************/
BOOL Expression2(DWORD *value, char *sExpr, char **psNext )
{
    if( sExpr && *sExpr )
    {
        *value = Evaluate2( sExpr, psNext );

        if( deb.error != NOERROR )
            return( FALSE );
    }
    else
        return( FALSE );

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdEvaluate2(char *args, int subClass)                               *
*                                                                             *
*******************************************************************************
*
*   Debuger command to evaluate an expression
*
******************************************************************************/
BOOL cmdEvaluate2(char *args, int subClass)
{
    static char buf[MAX_STRING];        // Temp output buffer
    int i;
    DWORD value;

    if( *args )
    {
        if( Expression2(&value, args, &args) && (deb.error==NOERROR) )
        {
            if( !*args )
            {
                i = sprintf(buf, " Hex=%08X  Dec=%010u  ", value, value );

                // Print negative decimal only if it is a negative number
                if( (signed)value<0 )
                    i += sprintf(buf+i, "(%d)  ", (signed)value);

                // Print ASCII representation of that number
                i += sprintf(buf+i, "\"%c%c%c%c\"",
                        ((value>>24) & 0xFF) >= ' '? ((value>>24) & 0xFF) : '.',
                        ((value>>16) & 0xFF) >= ' '? ((value>>16) & 0xFF) : '.',
                        ((value>> 8) & 0xFF) >= ' '? ((value>> 8) & 0xFF) : '.',
                        ((value>> 0) & 0xFF) >= ' '? ((value>> 0) & 0xFF) : '.' );

                dprinth(1, "%s", buf);
            }
            else
                dprinth(1, "Syntax error at ->\"%s\"", args);
        }
    }
    else
        deb.error = ERR_EXP_WHAT;

    return( TRUE );
}

