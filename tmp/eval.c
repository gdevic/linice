/******************************************************************************
*                                                                             *
*   Module:     Eval.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/15/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a generic expresion evaluator.  No error checking is
        performed.

        Numbers that are acepted are integers written in the notation with
        default base of 10.

        A new default base can be reset by changing the global variable
        `nEvalDefaultBase' to any base from 2 to 16.

        Numbers may be expressed in:
            binary base, add `b' at the end of a binary number;
            octal base, add `o' at the end of an octal number;
            decimal base, add `d' at the end of a decimal number;
            hexadecimal base, add `h' at the end of a hex number.

        Variables (alphanumerical literals) are supported by registering an
        external function that will handle them to the global variable
        pfnEvalLiteralHandler.  That function will get the zero-terminated
        string containing the name of the literal and should return the
        value associated with it.

        That external function may, in turn, call evaluator.
        The reentrancy limit is defined as MAX_RECURSE.  If that limit is
        reached, the resulting value is undefined.

        Provision is given for multiple expressions in a single string.
-.
*******************************************************************************

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 05/15/97   1.00  Original                                       Goran Devic *
* 05/18/97   1.01  Added bitwise, boolean operators               Goran Devic *
* 05/20/97   1.02  Literal handling                               Goran Devic *
* 09/10/97   1.03  Literal function may call evaluator            Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file
#include <ctype.h>                      // Include character testing
#include <string.h>                     // Include string header

#include "Eval.h"                       // Include its own header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

// Predefined default base is decimal and there are no literal handlers.

int nEvalDefaultBase = 10;
int (*pfnEvalLiteralHandler)( char *sName ) = NULL;


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define MAX_RECURSE     5               // Max literal reentrancy depth

#define MAX_STACK       10              // Max depth of a stack structure

#define BOTTOM_STACK    0               // Stack empty code
#define OP_PAREN_START  1               // Priority codes and also indices
#define OP_PAREN_END    2               //   into the sTokens array
#define OP_BOOL_OR      3
#define OP_BOOL_AND     4
#define OP_OR           5
#define OP_XOR          6
#define OP_AND          7
#define OP_EQ           8
#define OP_NE           9
#define OP_SHL          10
#define OP_SHR          11
#define OP_L            12
#define OP_LE           13
#define OP_G            14
#define OP_GE           15
#define OP_PLUS         16
#define OP_MINUS        17
#define OP_TIMES        18
#define OP_DIV          19
#define OP_MOD          20
#define OP_NOT          21
#define OP_NEG          255             // Unary minus has highest priority

static char *sTokens[] =
{
    "(", ")",
    "||",
    "&&",
    "|", "^", "&",
    "==", "!=",
    "<<", ">>",
    "<", "<=", ">", ">=",
    "+", "-",
    "*", "/", "%",
    "!",
    NULL
};


static struct Stack                     // Defines stack structure(s)
{
    int Data[ MAX_STACK ];              // Stack data
    int Top;                            // Top of stack index
};


static const char sDelim[] = ",;\"";    // Expressions delimiters - break chars
static const char sLiteral[] = "@:_";   // These are allowed in literal names

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void Push( struct *Stack, int Value )                                     *
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
static void Push( struct Stack *Stack, int Value )
{
    if( Stack->Top < MAX_STACK )
        Stack->Data[ Stack->Top++ ] = Value;
}


/******************************************************************************
*                                                                             *
*   int Pop( struct *Stack )                                                  *
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
static int Pop( struct Stack *Stack )
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
static int GetValue( char **sExpr )
{
    static unsigned int nRecurse = 0;
    int value, base, fSkipBaseChar;
    char *sStart = *sExpr;
    char *sEnd = sStart;
    char ch;

    // Check if the first character is a literal
    if( isalpha(*sStart) || strchr(sLiteral,*sEnd)!=NULL )
    {
        // Find the end of a literal
        while( isalnum(*sEnd) || strchr(sLiteral,*sEnd)!=NULL )
            sEnd++;

        // The following line may also increase the level of recursion
        if( pfnEvalLiteralHandler != NULL && nRecurse++ < MAX_RECURSE )
        {
            ch = *sEnd;
            *sEnd = '\0';

            value = pfnEvalLiteralHandler( sStart );

            // Decrease the level of recursion
            nRecurse--;

            // Restore end of literal character and return
            *sEnd = ch;
            *sExpr = sEnd;

            return( value );
        }
        else
        {
            // Not handling literals, return 0
            *sExpr = sEnd;

            return( 0 );
        }
    }

    // Assume a default base 10 and clear the initial value
    base  = nEvalDefaultBase;
    value = 0;
    fSkipBaseChar = 0;

    // Traverse and find the last character comprising a value
    do
    {
        // Break out if the current character is not alphanumeric
        if( !isalnum( *sEnd ) )
            break;

        // Last char can be 'h' to set base to 16 or b to set base to 2
        ch = tolower(*sEnd);
        if( ch=='h' )
        {
            base = 16;
            fSkipBaseChar = 1;

            break;
        }
        else
            // If the last char is `o', the value is octal.
            if( ch=='o' )
            {
                base = 8;
                fSkipBaseChar = 1;

                break;
            }
        else
            // If the last char is `d', the value is decimal.
            if( ch=='d' )
            {
                // If the next character is also alphanumeric, then this
                // 'd' cannot be a decimal designator
                if( !isalnum( *(sEnd+1) ) )
                {
                    base = 10;
                    fSkipBaseChar = 1;

                    break;
                }
            }
        else
            if( ch=='b' )
                // If the next character is also alphanumeric, then this
                // 'b' cannot be a binary designator
                if( !isalnum( *(sEnd+1) ) )
                {
                    base = 2;
                    fSkipBaseChar = 1;

                    break;
                };

        sEnd++;

    }while( 1 );

    // Scan from the start of the numerics and multiply/add them
    while( sStart < sEnd )
    {
        value *= base;
        value += (*sStart > '9')? tolower(*sStart) - 'a' + 10 : *sStart - '0';
        sStart++;
    }

    // Specified bases had extra character that now needs to be skipped
    sEnd += fSkipBaseChar;

    *sExpr = sEnd;

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
*   void Execute( struct Stack *Values, int Operation )                       *
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
static void Execute( struct Stack *Values, int Operation )
{
    int top;

    // Perform the operation
    switch( Operation )
    {
        case OP_BOOL_OR:Push( Values, Pop( Values ) || Pop( Values ) );
            break;

        case OP_BOOL_AND:Push(Values, Pop( Values ) && Pop( Values ) );
            break;

        case OP_OR:     Push( Values, Pop( Values ) | Pop( Values ) );
            break;

        case OP_XOR:    Push( Values, Pop( Values ) ^ Pop( Values ) );
            break;

        case OP_AND:    Push( Values, Pop( Values ) & Pop( Values ) );
            break;

        case OP_EQ:     Push( Values, Pop( Values ) == Pop( Values ) );
            break;

        case OP_NE:     Push( Values, Pop( Values ) != Pop( Values ) );
            break;

        case OP_L:      Push( Values, Pop( Values ) < Pop( Values ) );
            break;

        case OP_LE:     Push( Values, Pop( Values ) <= Pop( Values ) );
            break;

        case OP_G:      Push( Values, Pop( Values ) > Pop( Values ) );
            break;

        case OP_GE:     Push( Values, Pop( Values ) >= Pop( Values ) );
            break;

        case OP_SHL:    Push( Values, Pop( Values ) << Pop( Values ) );
            break;

        case OP_SHR:    Push( Values, Pop( Values ) >> Pop( Values ) );
            break;

        case OP_PLUS:   Push( Values, Pop( Values ) + Pop( Values ) );
            break;

        case OP_MINUS:  top = Pop( Values );
                        Push( Values, Pop( Values ) - top );
            break;

        case OP_TIMES:  Push( Values, Pop( Values ) * Pop( Values ) );
            break;

        case OP_DIV:    top = Pop( Values );
                        if( top != 0 )
                            Push( Values, Pop( Values ) / top );
            break;

        case OP_MOD:    top = Pop( Values );
                        if( top != 0 )
                            Push( Values, Pop( Values ) % top );
            break;

        case OP_NOT:    Push( Values, ! Pop( Values ) );
            break;

        case OP_NEG:    Push( Values, -Pop( Values ) );
            break;
    }
}


/******************************************************************************
*                                                                             *
*   int nEvaluate( char *sExpr, char **psNext )                               *
*                                                                             *
*******************************************************************************
*
*   Evaluates a string expression and returns its numerical value.
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
int nEvaluate( char *sExpr, char **psNext )
{
    struct Stack Values, Operators;
    int NewOp, OldOp, Operator;

    Values.Top = Operators.Top = Operator = 0;

    // Just in the case that the argument was NULL - return the result of 0.
    if( sExpr==NULL )
    {
        if( psNext!=NULL ) *psNext = NULL;
        return( 0 );
    }

    // Loop for any new term and stop when hit one of delimiter characters
    while( strchr(sDelim,*sExpr)==NULL )
    {
        NewOp = TableMatch( sTokens, &sExpr);

        // Special case is unary minus in front of a value
        if( Operator==0 && NewOp==OP_MINUS )
            NewOp = OP_NEG;

        // If any operator was in front of a value, push it
        if( NewOp )
        {
            Push( &Operators, NewOp );
            Operator = 0;               // Expect a value

            continue;
        }

        Push( &Values, GetValue( &sExpr ) );
        Operator = 1;                   // Expect an operator
        NewOp = TableMatch( sTokens, &sExpr);

        // If there are no more operators, break out and clean the stack
        if( ! NewOp )
            break;

        OldOp = Pop( &Operators );

        // If the new op priority is less than the one on the stack, we
        // need to go down and evaluate terms
        if( NewOp < OldOp )
            Execute( &Values, OldOp );
        else
            Push( &Operators, OldOp );

        // Push new operation
        Push( &Operators, NewOp );
    }

    // Clean the stack by the means of evaluating expression in RPN
    while( (NewOp = Pop(&Operators)) != BOTTOM_STACK )
    {
        Execute( &Values, NewOp );
    }

    // Store the logical end of the expression
    if( psNext!=NULL ) *psNext = sExpr;

    // Return the last value on the stack
    return( Pop( &Values ) );
}


#ifdef TEST

char *s1 = "(1)-1";

main()
{
       printf("%d\n", nEvaluate(s1, NULL) );

}

#endif