/******************************************************************************
*                                                                             *
*   Module:     types.c                                                       *
*                                                                             *
*   Date:       07/23/02                                                      *
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

        This module contains code for managing symbolic type information.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 07/23/02   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "lists.h"                      // Include lists support header file

#include "debug.h"                      // Include our dprintk()

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

// Names of the simple, built-in types

static char *sSimpleTypes[TYPEDEF__LAST] =
{
    "<invalid>",
    "int ",
    "char ",
    "long int ",
    "unsigned int ",
    "long unsigned int ",
    "long long int ",
    "long long unsigned int ",
    "short int ",
    "short unsigned int ",
    "signed char ",
    "unsigned char ",
    "float ",
    "double ",
    "long double ",
    "complex int ",
    "complex float ",
    "complex double ",
    "complex long double "
};

// Enum values of the simple, built-in types

enum
{
    TYPE_INT = 1,
    TYPE_CHAR,
    TYPE_LONG_INT,
    TYPE_UNSIGNED_INT,
    TYPE_LONG_UNSIGNED_INT,
    TYPE_LONG_LONG_INT,
    TYPE_LONG_LONG_UNSIGNED_INT,
    TYPE_SHORT_INT,
    TYPE_SHORT_UNSIGNED_INT,
    TYPE_SIGNED_CHAR,
    TYPE_UNSIGNED_CHAR,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_LONG_DOUBLE,
    TYPE_COMPLEX_INT,
    TYPE_COMPLEX_FLOAT,
    TYPE_COMPLEX_DOUBLE,
    TYPE_COMPLEX_LONG_DOUBLE,

} TENUMBUILTINTYPES;


// Size (memory footprints) of the simple, built-in types

//*****************************************************************************
#ifdef WIN32
//*****************************************************************************
static int nSimpleTypes[TYPEDEF__LAST] =
{
    sizeof(NULL),
    sizeof(int),
    sizeof(char),
    sizeof(long int),
    sizeof(unsigned int),
    sizeof(long unsigned int),
    sizeof(long int),
    sizeof(long unsigned int),
    sizeof(short int),
    sizeof(short unsigned int),
    sizeof(signed char),
    sizeof(unsigned char),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL)
};

typedef union
{
    int _int;
    char _char;
    long int _long_int;
    unsigned int _unsigned_int;
    long unsigned int _long_unsigned_int;
  int _long_long_int;
  int _long_long_unsigned_int;
    short int _short_int;
    short unsigned int _short_unsigned_int;
    signed char _signed_char;
    unsigned char _unsigned_char;
  int _float;
  int _double;
  int _long_double;
  int _complex_int;
  int _complex_float;
  int _complex_double;
  int _complex_long_double;

} TTYPEFOOTPRINT;

//*****************************************************************************
#else /*                          LINUX                                       */
//*****************************************************************************
static int nSimpleTypes[TYPEDEF__LAST] =
{
    sizeof(NULL),
    sizeof(int),
    sizeof(char),
    sizeof(long int),
    sizeof(unsigned int),
    sizeof(long unsigned int),
    sizeof(long long int),
    sizeof(long long unsigned int),
    sizeof(short int),
    sizeof(short unsigned int),
    sizeof(signed char),
    sizeof(unsigned char),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL),
    sizeof(NULL)
};

typedef union
{
    int _int;
    char _char;
    long int _long_int;
    unsigned int _unsigned_int;
    long unsigned int _long_unsigned_int;
    long long int _long_long_int;
    long long unsigned int _long_long_unsigned_int;
    short int _short_int;
    short unsigned int _short_unsigned_int;
    signed char _signed_char;
    unsigned char _unsigned_char;
    float _float;
    double _double;
    long double _long_double;
    int _complex_int;
    float _complex_float;
    double _complex_double;
    long double _complex_long_double;

} TTYPEFOOTPRINT;

#endif


static char *sPtr = "*****";

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen);

extern BOOL GlobalReadDword(DWORD *pDword, DWORD dwAddress);
extern BOOL GlobalReadBYTE(BYTE *pByte, DWORD dwAddress);
extern BOOL GlobalReadMem(BYTE *pBuf, DWORD dwAddress, UINT nLen);


/******************************************************************************
*                                                                             *
*   TSYMTYPEDEF1 *Type2Typedef(char *pType, int nLen)                         *
*                                                                             *
*******************************************************************************
*
*   Searches the context-sensitive type definition list and returns a typedef
*   that matches search criteria:
*       1) Type number - pType points to a string in the form "(a,b)"
*       2) Type number - pType points to a string in the form "x(a,b)" where char x is ignored
*       3) Type name   - pType points to a type name string (nLen is its length)
*
*       For forms 1 and 2, nLen should be set to 0.
*
*   Where:
*       pType is the type name or number pair
*       nLen is the size of the input token len (only for type name strings)
*
*   Returns:
*       Typedef token
*       NULL if the typedef describing that type name could not be found
*
******************************************************************************/
TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pType;                 // Type structure header
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    WORD maj=0, min=0;                  // Major and minor type number

    // Sanity check that we have symbols and pTypeName is properly given
    if( deb.pSymTabCur && deb.pFnScope && pTypeName && *pTypeName )
    {
        pHead = deb.pSymTabCur->header;

        // If we are searching for a type number, read the (maj,min)
        if( nLen==0 )
        {
            // The alternate form is when a pointer addresses the type qualifier "x(maj,min)" which we ignore
            if( *(pTypeName+1)=='(' )
                pTypeName++;

            if( *pTypeName=='(' )
            {
                pTypeName++;                // Advance past '('
                maj = GetDec(&pTypeName);   // Get the major type number
                pTypeName++;                // Advance past delimiting comma
                min = GetDec(&pTypeName);   // Get the minor type number
            }
        }

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == HTYPE_TYPEDEF )
            {
                pType = (TSYMTYPEDEF*)pHead;

                // Got a type header, make sure that the source contex match

                if( pType->file_id == deb.pFnScope->file_id )
                {
                    nTypedefs = pType->nTypedefs;
                    pType1 = pType->list;

                    // We will do a search in two different ways, depending on the input
                    if( nLen==0 )           // Input was "(a,b)"
                    {
                        // Search by the type number
                        for(; nTypedefs>0; nTypedefs--)
                        {
                            if( pType1->min==min && pType1->maj==maj )
                                return( pType1 );

                            pType1++;
                        }
                    }
                    else
                    {
                        // Search by the type name
                        for(; nTypedefs>0; nTypedefs--)
                        {
                            pStr = pType1->pName;

                            if( !strnicmp(pStr, pTypeName, nLen) && strlen(pStr)==nLen )
                                return( pType1 );

                            pType1++;
                        }
                    }
                }
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return(NULL);
}

/******************************************************************************
*                                                                             *
*   void TypedefCanonical(TSYMTYPEDEF1 *pType1)                               *
*                                                                             *
*******************************************************************************
*
*   Modifies the type descriptor pType1 to be a canonical type descriptor,
*   that is:
*       min - set to 0 (only canonical types can have 0 in this field)
*       maj - number of pointer indirection
*       pDef - points to the terminating type node (either a built-in type or a complex type definition
*       pName - points to the last non-empty type name
*
*   Where:
*       pType1 is a pointer to the type descriptor to modify
*
*   Returns:
*       pType1 is modified
*
******************************************************************************/
void TypedefCanonical(TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 *pTypeNext;            // Next type down the definition chain
    int nPtr = 0;                       // Level of pointer redirections

    // If a type is already in canonical form, don't do it again
    if( pType1->min )
    {
        pTypeNext = pType1;                 // Use this pointer to walk down the type chain

        // Resolve data type by dereferencing the pointers until we find the type name
        do
        {
            // Check if we have a pointer redirection
            if( *pTypeNext->pDef=='*' )
            {
                nPtr++;                     // Increment the pointer redirection

                // After the pointer redirection, we have to have a reference to another typedef
                pTypeNext->pDef++;          // This is always "("
            }

            // If we dont have a pointer redirection or another type '(', we are done
            if( *pTypeNext->pDef!='(' )
                break;

            // Otherwise, follow the def chain
            pTypeNext = Type2Typedef(pTypeNext->pDef, 0);

            // Assign the new name, if the next type has it defined as non-zero string
            if(*pTypeNext->pName)
                pType1->pName = pTypeNext->pName;

        } while( TRUE );

        pType1->maj = nPtr;                 // Pointer indirection level
        pType1->min = 0;                    // Signature of the canonical type format
        pType1->pDef = pTypeNext->pDef;     // New type definition
    }
}

/******************************************************************************
*                                                                             *
*   int GetTypeSize(TSYMTYPEDEF1 *pType1)                                     *
*                                                                             *
*******************************************************************************
*
*   Returns the size (in bytes) of the type's memory footprint. The type can
*   be any kind of type.
*
*   Where:
*       pType1 is the type descriptor to get the memory footprint
*
*   Returns:
*       Type memory footprint in bytes
*       0 if the type is invalid for some reasons
*
******************************************************************************/
int GetTypeSize(TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 Type1;                 // Local type storage
    int nSize = 0;                      // Size variable
    char *pDef;                         // Pointer to the type definition

    // Make sure the given type descriptor is valid
    if( pType1 )
    {
        // Copy the input type into the local store so we may modify it
        memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));

        // Get the local type into the canonical form - this will modify it
        TypedefCanonical(&Type1);

        // Simple - if there is any pointer redirection level, the final size is just the pointer size
        if( Type1.maj )
            return( sizeof(void *) );

        // Depending on the base type, find the size - built in types:
        if( *pType1->pDef <= TYPEDEF__LAST )
            return( nSimpleTypes[*(BYTE *)pType1->pDef] );

        // Get the new type definition...
        pDef = Type1.pDef;

        // We know we have a terminal type here - one of the complex types:
        switch( *pDef )
        {
            case 'u':       // Unions always keep the size in bits
            case 's':       // Structures always keep the size in bytes
                pDef++;
                nSize = GetDec(&pDef);
            break;

            case 'e':       // Consider enum an integer size
            case 'r':       // A type that is a subrange of itself - right now we assume "int"
                nSize = sizeof(int);
            break;

            case 'a':       // Array is most complicated since we need the size of a child * the number of elements
            {
                int lower, upper;           // Array bounds

                // Find the first ';' to get to the bounds
                pDef = strchr(pDef, ';');
                sscanf(pDef, ";%d;%d;", &lower, &upper);

                // Find the trailing '(' to get to the child element
                pDef = strchr(pDef, '(');

                // This will call itself recursively to get the size of one array element
                pType1 = Type2Typedef(pDef, 0);
                nSize = (upper-lower+1) * GetTypeSize(pType1);
            }
            break;

            default:
                ;           // We should not be here...
                break;
        }
    }

    return( nSize );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTypes(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display all types in a current symbol file or display a specific type
*   structure information.
*
*   Symtax:
*       TYPES           - lists all data types in the current symbol table
*       TYPES *         - alternate way to list all types
*       TYPES type      - display type information for a given symbol type
*
******************************************************************************/
BOOL cmdTypes(char *args, int subClass)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pType;                 // Type structure header
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition
    TSYMSOURCE *pSource;                // Source file that a type belongs to
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    int nLine = 1;
    int nPtrLevel;
    BOOL fPrinted;                      // Header is already printed flag

    if( deb.pSymTabCur )
    {
        if( *args && *args!='*' )
        {
            // Argument is a specific type name - find it and list its members
#if 0
            pType1 = Type2Typedef(args, strlen(args));
            pType1 = Type2BaseType(&nPtrLevel, pType1);
            if( pType1 )
            {
                dprinth(nLine++, "(%d,%d) %s *%d", pType1->maj, pType1->min, GetTypeString(pType1), nPtrLevel);

                // If the type is a complex type (structure etc.), print the complete typedef
                if( pType1->pDef > TYPEDEF__LAST )
                {
// TODO -
//                  PrettyPrintType(pType1);
                }
            }
#endif
#if 0
            pHead = deb.pSymTabCur->header;

            while( pHead->hType != HTYPE__END )
            {
                if( pHead->hType == HTYPE_TYPEDEF )
                {
                    pType = (TSYMTYPEDEF*)pHead;

                    // Got a type header, show the source file name

                    // TODO - sources do not seem to get assigned right...

                    pSource = SymTabFindSource(deb.pSymTabCur, pType->file_id);
                    fPrinted = FALSE;

                    nTypedefs = pType->nTypedefs;
                    pType1 = pType->list;

                    for(; nTypedefs>0; nTypedefs--)
                    {
                        pStr = GET_STRING( pType1->dName );

                        if( !stricmp(pStr+1, args) )
                        {
                            if( pSource && !fPrinted )
                            {
                                fPrinted = TRUE;
#if 0
                                if( dprinth(nLine++, "Defined in %s:",
                                        GET_STRING( pSource->dSourcePath ),
                                        GET_STRING( pSource->dSourceName ) )==FALSE)
                                    return( TRUE );
#endif
                            }

                            if( dprinth(nLine++, "(%d,%d) %-30s%s", pType1->maj, pType1->min, pStr+1, GetTypeString(pType1) )==FALSE)
                                return( TRUE );
                        }

                        pType1++;
                    }
                }

                pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
            }
#endif
        }
        else
        {
            // No arguments given or argument was '*' - lists all symbol types

            pHead = deb.pSymTabCur->header;

            while( pHead->hType != HTYPE__END )
            {
                if( pHead->hType == HTYPE_TYPEDEF )
                {
                    pType = (TSYMTYPEDEF*)pHead;

                    // Got a type header, list all types defined there

                    nTypedefs = pType->nTypedefs;
                    pType1 = pType->list;

                    dprinth(nLine++, "Type Name                     Typedef");

                    for(; nTypedefs>0; nTypedefs--)
                    {
                        pStr = pType1->pName;

//                        if( dprinth(nLine++, "(%d,%d) %-30s%s", pType1->maj, pType1->min, pStr, GetTypeString(pType1) )==FALSE)
//                            return( TRUE );

                        pType1++;
                    }
                }

                pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
            }
        }
    }
    else
        dprinth(1, "No symbol table loaded.");

    return( TRUE );
}


/******************************************************************************
*
*   char *Type2Element(TSYMTYPEDEF1 *pType, char *pName, int nLen)
*
*******************************************************************************
*
*   Given the complex type descriptor and a string with the element name which
*   belongs to that type, returns the pointer to that element descriptor (a,b),...
*
*   Where:
*       pType is the complex type descriptor
*       pName is the name of one element to look for
*       nLen is the length of the pName string
*
*   Returns:
*       pointer to the element descriptor string, "(x,y);offset,size"
*       NULL if the element could not be found
*
******************************************************************************/
char *Type2Element(TSYMTYPEDEF1 *pType, char *pName, int nLen)
{
    char *pDef;                         // Pointer to the type definition string
    int len;

    pDef = pType->pDef;

    // Scan the type string in search of the element name
    // Skip the structure designator and the total size
    if( *pDef=='s' || *pDef=='u' )
    {
        pDef++;
        while( isdigit(*pDef) ) pDef++;

        do
        {
            // Get the size of the element name in the type string
            len = strchr(pDef, ':') - pDef;
            if( len==nLen && !strnicmp(pName, pDef, nLen) )
            {
                // Found it!
                return( pDef + nLen + 1 );
            }

            if( (pDef = strchr(pDef, ';'))==NULL)
                break;

            // Structure terminates its definition string with two successive ;;
        } while( *(++pDef)!=';' );
    }

    return( NULL );
}


static void PrintBasicTypeValue(char *buf, TTYPEFOOTPRINT *pValue, TSYMTYPEDEF1 *pType1)
{
    if( pType1 && pType1->pDef && *pType1->pDef <= TYPEDEF__LAST )
    {
        switch( *pType1->pDef )
        {
            // TODO: Need to rewrite sprintf() to process all these new data types to print


            case TYPE_INT:                      sprintf(buf, "%d", pValue->_int);
                break;
            case TYPE_CHAR:                     sprintf(buf, "0x%02X %c", pValue->_char, isascii(pValue->_char)? pValue->_char : '?');
                break;
            case TYPE_LONG_INT:                 sprintf(buf, "%ld", pValue->_long_int);
                break;
            case TYPE_UNSIGNED_INT:             sprintf(buf, "0x%08X", pValue->_unsigned_int);
                break;
            case TYPE_LONG_UNSIGNED_INT:        sprintf(buf, "0x%08lX", pValue->_long_unsigned_int);
                break;
            case TYPE_LONG_LONG_INT:            sprintf(buf, "%ll", pValue->_long_long_int);
                break;
            case TYPE_LONG_LONG_UNSIGNED_INT:   sprintf(buf, "%llX", pValue->_long_long_unsigned_int);
                break;
            case TYPE_SHORT_INT:                sprintf(buf, "%hd", pValue->_short_int);
                break;
            case TYPE_SHORT_UNSIGNED_INT:       sprintf(buf, "%hX", pValue->_short_unsigned_int);
                break;
            case TYPE_SIGNED_CHAR:              sprintf(buf, "%d %c", pValue->_signed_char, isascii(pValue->_signed_char)? pValue->_signed_char : '?');
                break;
            case TYPE_UNSIGNED_CHAR:            sprintf(buf, "%X %c", pValue->_signed_char, isascii(pValue->_signed_char)? pValue->_signed_char : '?');
                break;
            case TYPE_FLOAT:                    sprintf(buf, "%f", pValue->_float);
                break;
            case TYPE_DOUBLE:                   sprintf(buf, "%e", pValue->_double);
                break;
            case TYPE_LONG_DOUBLE:              sprintf(buf, "%Le", pValue->_long_double);
                break;
            case TYPE_COMPLEX_INT:              sprintf(buf, "(%d,%d)", *(int *)&pValue->_complex_int, *(int *)((BYTE)&pValue->_complex_int)+sizeof(int));
                break;
            case TYPE_COMPLEX_FLOAT:            sprintf(buf, "(%f,%f)", *(float *)&pValue->_complex_float, *(float *)((BYTE)&pValue->_complex_float)+sizeof(float));
                break;
            case TYPE_COMPLEX_DOUBLE:           sprintf(buf, "(%e,%e)", *(double *)&pValue->_complex_double, *(double *)((BYTE)&pValue->_complex_double)+sizeof(double));
                break;
            case TYPE_COMPLEX_LONG_DOUBLE:      sprintf(buf, "(%Le,%Le)", *(long double *)&pValue->_complex_long_double, *(long double *)((BYTE)&pValue->_complex_long_double)+sizeof(long double));
                break;
        }
    }
}

void PrintEnumTypeValue(char *buf, char *pEnum, BYTE *pVar)
{
    int Value;                          // Enums are always ints
    char *p;                            // Walking pointer for the enum definition string
    UINT nNameLen;                      // Length of the name portion

    // Walk the enum definition list and find the literal value that corresponds to the variable value

    if( GlobalReadDword(&Value, pVar) )
    {
        // emon:1,tue:2,wed:3,thr:4,fri:5,sat:6,sun:7,;
        p = pEnum;                      // Start with the first name

        do
        {
            p = strchr(pEnum, ':');     // Find the colon
            if( p )
            {
                nNameLen = p - pEnum;
                p = p+1;                // Increment to the actual decimal value

                if( GetDec(&p)==Value )
                {
                    // Found the matching enum value - print it and return

                    sprintf(buf, "%d <\"%s\">", Value, substr(pEnum, 0, nNameLen));

                    return;
                }

                pEnum = p + 1;
            }
            else
            {
                // Could not find the colon, so the enum value does not match
                sprintf(buf, "%d", Value);

                return;
            }
        }
        while( TRUE );
    }
    else
    {
        // Could not read the value
        sprintf(buf, "<?>");
    }
}

#define MAX_CHAR_PTR_SAMPLE  16                       // How much of a sample string to get?

void PrintExpandCharPtr(char *buf, BYTE *pPointee)
{
    static char String[MAX_CHAR_PTR_SAMPLE+1];  // String to store sample
    int i;                                      // String counter

    // Try to read several values from the pointee and print them if they make up a string
    for(i=0; i<MAX_CHAR_PTR_SAMPLE; i++)
    {
        if( GlobalReadBYTE(&String[i], pPointee) )
        {
            // If this is end-of-string, break
            if( String[i]==0 )
                break;

            // If the character is not ascii, this is not a string
            if( !isascii(String[i]) )
                return;

            pPointee++;
        }
        else
            return;
    }

    // Print as much of a string as we got
    String[MAX_CHAR_PTR_SAMPLE] = 0;

    sprintf(buf, "<\"%s\">", String);
}


void PrintTypeValue(char *buf, TExItem *pItem, BYTE *pVar, UINT delta, UINT width)
{
    TSYMTYPEDEF1 *pType1;               // Pointer to the type information
    TTYPEFOOTPRINT Value;               // Value of the variable
    BYTE *pPointee;                     // Address of the target variable

    // pVar is the address of the variable or a memory footprint

    // Break up depending on the pointer indirection level to:
    //   direct type (no indirection)       example:   char
    //   one-level pointer redirections:    example:   int *
    //   multi-level pointer redirections:  example:   int **  or  double ***

    pType1 = &pItem->Type;

    if( pItem->Type.maj==0 )
    {
        // Immediate data

        if( *pType1->pDef <= TYPEDEF__LAST )
        {
            // Simple data types

            // Read in the value of the variable, exactly the number of bytes that we need

            GlobalReadMem(&Value, pVar, nSimpleTypes[*pType1->pDef]);

            PrintBasicTypeValue(buf, &Value, pType1);
        }
        else
        {
            // Complex data types: structure, union, array, ... and enum

            // If this is an enum, process it differently -> decode it
            if( *pType1->pDef=='e' )
            {
                PrintEnumTypeValue(buf, pType1->pDef + 1, pVar);
            }
            else
            {
                sprintf(buf, "%08X", pVar);
            }
        }
    }
    else
    if( pItem->Type.maj==1 )
    {
        // One-level pointer indirection

        // Verify the common case of a NULL pointer first
        if( pVar==0 )
        {
            sprintf(buf, "NULL");
        }
        else
        {
            // Read the effective address of that pointer

            if( GlobalReadDword(&pPointee, pVar) )
            {
                buf += sprintf(buf, "%08X ", pVar);

                // Try to get one item from the pointee to print out
                // but process "char *" separately since it is a string

                if( *pType1->pDef==TYPE_CHAR || *pType1->pDef==TYPE_SIGNED_CHAR || *pType1->pDef==TYPE_UNSIGNED_CHAR )
                {
                    PrintExpandCharPtr(buf, pPointee);
                }
                else
                {
                    PrintBasicTypeValue(buf, &Value, pType1);
                }
            }
            else
            {
                sprintf(buf, "%08X <illegal>", pVar);
            }
        }
    }
    else
    {
        // Multi-level pointer indirection

        // Verify the common case of a NULL pointer first
        if( pVar==0 )
        {
            sprintf(buf, "NULL");
        }
        else
        {
            sprintf(buf, "%08X", pVar);
        }
    }
}
