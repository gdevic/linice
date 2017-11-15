/******************************************************************************
*                                                                             *
*   Module:     types.c                                                       *
*                                                                             *
*   Date:       07/23/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2004 Goran Devic                                       *
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

};


// Size (memory footprints) of the simple, built-in types

static UINT nSimpleTypes[TYPEDEF__LAST] =
{
    sizeof(NULL),
    sizeof(int),                    //  4
    sizeof(char),                   //  1
    sizeof(long int),               //  4
    sizeof(unsigned int),           //  4
    sizeof(long unsigned int),      //  4
    8,   //     sizeof(long long int)
    8,   //     sizeof(long long unsigned int)
    sizeof(short int),              //  2
    sizeof(short unsigned int),     //  2
    sizeof(signed char),            //  1
    sizeof(unsigned char),          //  1
    4,   //     sizeof(float)
    8,   //     sizeof(double)
    12,  //     sizeof(long double)
    8,   //     sizeof(complex int)
    8,   //     sizeof(complex float)
    16,  //     sizeof(complex double)
    24,  //     sizeof(complex long double)
};

typedef union
{
    int _int;
    unsigned char _char;
    long int _long_int;
    unsigned int _unsigned_int;
    long unsigned int _long_unsigned_int;
    //  int _long_long_int;
    //  int _long_long_unsigned_int;
    short int _short_int;
    short unsigned int _short_unsigned_int;
    signed char _signed_char;
    unsigned char _unsigned_char;
    //  int _float;
    //  int _double;
    //  int _long_double;
    //  int _complex_int;
    //  int _complex_float;
    //  int _complex_double;
    //  int _complex_long_double;
    BYTE padding[24];                   // The footprint has to match the largest data type: sizeof(complex long double)

} TTYPEFOOTPRINT;


#define MAX_CHAR_PTR_SAMPLE  16         // How much of a sample string to get?


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen, WORD file_id);

extern BOOL GlobalReadDword(DWORD *pDword, DWORD dwAddress);
extern BOOL GlobalReadBYTE(BYTE *pByte, DWORD dwAddress);
extern BOOL GlobalReadMem(BYTE *pBuf, DWORD dwAddress, UINT nLen);
extern void scan2dec(char *pBuf, int *p1, int *p2);


/******************************************************************************
*                                                                             *
*   TSYMTYPEDEF1 *Type2Typedef(char *pType, int nLen, WORD file_id)           *
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
*   In order to resolve the type, a reference type array is first consulted
*   for the file_id. Adjustment is applied to the major type number and then
*   that file_id is searched for the type.
*
*   Where:
*       pType is the type name or number pair
*       nLen is the size of the input token len (only for type name strings)
*       file_id is the file ID of the source where this type is defined
*
*   Returns:
*       Typedef token
*       NULL if the typedef describing that type name could not be found
*
******************************************************************************/
TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen, WORD file_id)
{
//    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pType;                 // Type structure header
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    int maj=0, min=0;                   // Major and minor type number
    WORD new_file_id;                   // New file ID where the type is defined

    // Sanity check that we have symbols and pTypeName is properly given
    if( deb.pSymTabCur && deb.pFnScope && pTypeName && *pTypeName )
    {
        // If we are searching for a type number, read the (maj,min)
        if( nLen==0 )
        {
            // The alternate form is when a pointer addresses the type qualifier "x(maj,min)" which we ignore
            if( *(pTypeName+1)=='(' )
                pTypeName++;

            if( *pTypeName=='(' )
            {
                pTypeName++;                        // Advance past '('
                scan2dec(pTypeName, &maj, &min);    // Scan 2 decimal numbers "%d,%d"
            }
        }

        // Find the typedef record of the file_id whose type we are looking for
        pType = SymTabFindTypedef(deb.pSymTabCur, file_id);

        // Found the type descriptor of the given file_id
        if( pType )
        {
            // Apply the reference adjustment value to the major type number and get the
            // resulting source file_id where this type is actually defined

            new_file_id = pType->pRel[maj].file_id;
            maj         = maj + pType->pRel[maj].adjust;

            // Find the type descriptor of the new file_id
            // To optimize the search, dont search if the type is in the same file ID
            if( new_file_id != file_id )
            {
                pType = SymTabFindTypedef(deb.pSymTabCur, new_file_id);
                if( !pType )
                {
                    // We should always be able to resolve the type using the reference lookup array
                    // If we dont, the symbol table is bad, or I did not understand how the types are laid out...
                    dprinth(1, "Internal error %s:%d", __FILE__, __LINE__);

                    // Well, return something - hopefully the above error message will ring the bell.
                    pType = SymTabFindTypedef(deb.pSymTabCur, file_id);
                    return( pType->list );
                }
            }

            // Search for the new (major, minor) through the new file ID type definition record

            nTypedefs = pType->nTypedefs;
            pType1 = pType->list;

            // We will do a search in two different ways, depending on the input
            if( nLen==0 )           // Input was "(a,b)"
            {
                // Search by the type number
                for(; nTypedefs>0; nTypedefs--)
                {
                    if( pType1->min==(WORD)min && pType1->maj==(WORD)maj )
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
    TSYMTYPEDEF1 *pTypeNext2;           // Next type down the definition chain
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
            if((pTypeNext2 = Type2Typedef(pTypeNext->pDef, 0, pTypeNext->file_id))==NULL)
                break;

            // We do this to detect circular type definition: Sometimes a type is defined as
            // itself, so in this case we break out
            if( pTypeNext==pTypeNext2 )
            {
                break;
            }
            else
                pTypeNext = pTypeNext2;

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
*   UINT GetTypeSize(TSYMTYPEDEF1 *pType1)                                    *
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
UINT GetTypeSize(TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 Type1;                 // Local type storage
    UINT nSize = 0;                     // Size variable
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

                pDef = strchr(pDef, ';');           // Find the first ';' to get to the bounds
                scan2dec(pDef+1, &lower, &upper);   // Scan 2 decimal numbers "%d,%d"
                pDef = strchr(pDef, '(');           // Find the trailing '(' to get to the child element

                // This will call itself recursively to get the size of one array element
                pType1 = Type2Typedef(pDef, 0, Type1.file_id);
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

/******************************************************************************
*   int PrintBasicTypeValue(char *buf, TTYPEFOOTPRINT *pValue, TSYMTYPEDEF1 *pType1)
*******************************************************************************
*
*   This function decodes the basic type and prints its value appropriately.
*
*   Where:
*       buf - the destination buffer
*       pValue - the address of the footprint variable for various types
*       pType1 - the type descriptor to use
*
******************************************************************************/
static int PrintBasicTypeValue(char *buf, TTYPEFOOTPRINT *pValue, TSYMTYPEDEF1 *pType1)
{
    static char sChar[] = "<\' \'>";    // Buffer that prints a printable character
    int written = 0;                    // Number of characters written

    if( pType1 && pType1->pDef && *pType1->pDef <= TYPEDEF__LAST )
    {
        // Printing a character is extra step that we may want to do in some cases
        // It is simpler to store a byte and use it as a character if we do those cases
        sChar[2] = pValue->_char;

        switch( *pType1->pDef )
        {
            case TYPE_INT:                      written = sprintf(buf, "0x%X", pValue->_int);
                break;
            case TYPE_CHAR:                     written = sprintf(buf, "0x%02X %s", pValue->_char, isprint(pValue->_char)? sChar:"");
                break;
            case TYPE_LONG_INT:                 written = sprintf(buf, "0x%X", (UINT) pValue->_long_int);
                break;
            case TYPE_UNSIGNED_INT:             written = sprintf(buf, "0x%08X", pValue->_unsigned_int);
                break;
            case TYPE_LONG_UNSIGNED_INT:        written = sprintf(buf, "0x%08X", pValue->_unsigned_int);   // pValue->_long_unsigned_int
                break;
            case TYPE_LONG_LONG_INT:            written = sprintf(buf, "0x%08X%08X", *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int));
                break;
            case TYPE_LONG_LONG_UNSIGNED_INT:   written = sprintf(buf, "0x%08X%08X", *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int));
                break;
            case TYPE_SHORT_INT:                written = sprintf(buf, "0x%hX", pValue->_short_int);
                break;
            case TYPE_SHORT_UNSIGNED_INT:       written = sprintf(buf, "0x%04hX", pValue->_short_unsigned_int);
                break;
            case TYPE_SIGNED_CHAR:              written = sprintf(buf, "0x%X %s", pValue->_signed_char, isprint(pValue->_signed_char)? sChar:"");
                break;
            case TYPE_UNSIGNED_CHAR:            written = sprintf(buf, "0x%02X %s", pValue->_unsigned_char, isprint(pValue->_unsigned_char)? sChar:"");
                break;
            case TYPE_FLOAT:                    written = sprintf(buf, "(float)%08X", *(int *)((UINT)&pValue->_int));
                break;
            case TYPE_DOUBLE:                   written = sprintf(buf, "(double)%08X%08X", *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int));
                break;
            case TYPE_LONG_DOUBLE:              written = sprintf(buf, "(long double)%08X%08X%08X", *(int *)((UINT)&pValue->_int)+sizeof(int)*2, *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int));
                break;
            case TYPE_COMPLEX_INT:              written = sprintf(buf, "complex(%d,%d)", *(int *)((UINT)&pValue->_int), *(int *)((UINT)&pValue->_int)+sizeof(int));
                break;
            case TYPE_COMPLEX_FLOAT:            written = sprintf(buf, "complex(%08X,%08X)", *(int *)((UINT)&pValue->_int), *(int *)((UINT)&pValue->_int)+sizeof(int));
                break;
            case TYPE_COMPLEX_DOUBLE:           written = sprintf(buf, "complex(%08X%08X,%08X%08X)", *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int), *(int *)((UINT)&pValue->_int)+sizeof(int)*3, *(int *)((UINT)&pValue->_int)+sizeof(int)*2);
                break;
            case TYPE_COMPLEX_LONG_DOUBLE:      written = sprintf(buf, "complex(%08X%08X%08X,%08X%08X%08X)", *(int *)((UINT)&pValue->_int)+sizeof(int)*2, *(int *)((UINT)&pValue->_int)+sizeof(int), *(int *)((UINT)&pValue->_int), *(int *)((UINT)&pValue->_int)+sizeof(int)*5, *(int *)((UINT)&pValue->_int)+sizeof(int)*4, *(int *)((UINT)&pValue->_int)+sizeof(int)*3);
                break;
        }
    }

    return( written );
}

/******************************************************************************
*   int PrintEnumTypeValue(char *buf, char *pEnum, BYTE *pVar)
*******************************************************************************
*
*   Since enums are a bit odd to decode (complex type yet simple integer),
*   this function prints the enum literal string from its numerical value.
*
*   Where:
*       buf - the destination buffer
*       pVar - the address of the enum integer
*
*   Returns:
*       Number of characters printed
*
******************************************************************************/
static int PrintEnumTypeValue(char *buf, char *pEnum, BYTE *pVar)
{
    int Value;                          // Enums are always ints
    char *p;                            // Walking pointer for the enum definition string
    UINT nNameLen;                      // Length of the name portion
    int written = 0;                    // Number of characters written

    // Walk the enum definition list and find the literal value that corresponds to the variable value

    if( GlobalReadDword(&Value, (DWORD) pVar) )
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

                    written = sprintf(buf, "%d <\"%s\">", Value, substr(pEnum, 0, nNameLen));

                    return( written );
                }

                pEnum = p + 1;
            }
            else
            {
                // Could not find the colon, so the enum value does not match
                written = sprintf(buf, "%d", Value);

                return( written );
            }
        }
        while( TRUE );
    }
    else
    {
        // Could not read the value
        written = sprintf(buf, "<?>");
    }

    return( written );
}

/******************************************************************************
*   void PrintExpandCharPtr(char *buf, BYTE *pPointee)
*******************************************************************************
*
*   Print a certain number of character string being decoded from the
*   target address.
*
*   Where:
*       buf - the destination buffer
*       pPointee - the address to start reading the string
*
*   Returns:
*       Number of characters printed
*
******************************************************************************/
static int PrintExpandCharPtr(char *buf, BYTE *pPointee)
{
    static char String[MAX_CHAR_PTR_SAMPLE+1];  // String to store sample
    int i;                                      // String counter
    int written = 0;                    // Number of characters written

    // Try to read several values from the pointee and print them if they make up a string
    for(i=0; i<MAX_CHAR_PTR_SAMPLE; i++)
    {
        if( GlobalReadBYTE(&String[i], (DWORD) pPointee) )
        {
            // If this is end-of-string, break
            if( String[i]==0 )
                break;

            // If the character is not printable, this is not a string
            if( !isprint(String[i]) )
                return( written );

            pPointee++;
        }
        else
            return( written );
    }

    // Print as much of a string as we got
    String[MAX_CHAR_PTR_SAMPLE] = 0;

    written = sprintf(buf, " <\"%s\">", String);

    return( written );
}


/******************************************************************************
*
*   void PrintTypeValue(char *buf, TExItem *pItem, BYTE *pVar, UINT delta, UINT width)
*
*******************************************************************************
*
*   Main type print function. It looks up in the memory for the data that is
*   addressed to by the pItem token.
*
*   This function handles invalid data type where the pointers are NULL.
*
*   Where:
*       buf - buffer to print effective value of an item descriptor
*             this function will zero-terminate the buffer
*             ASSUMPTION: Buffer is large enough - no length checks are done!
*       pItem - descriptor for the data
*       pVar - the effective memory address of the data structure
*       delta - TBD
*       width - TBD
*
*   Returns:
*       void
*
******************************************************************************/
void PrintTypeValue(char *buf, TExItem *pItem, BYTE *pVar, UINT delta, UINT width)
{
    TSYMTYPEDEF1 *pType1;               // Pointer to the type information
    TTYPEFOOTPRINT Value;               // Value of the variable

    // pVar is the address of the variable or a memory footprint

    // Break up depending on the pointer indirection level to:
    //   direct type (no indirection)       example:   char
    //   one-level pointer redirections:    example:   int *
    //   multi-level pointer redirections:  example:   int **  or  double ***

    pType1 = &pItem->Type;

    // Take care of invalid data pointers (used for illegal watch variables)
    if( pType1->pDef && pType1->pName )
    {
        if( pItem->Type.maj==0 )
        {
            // Immediate data

            if( *pType1->pDef <= TYPEDEF__LAST )
            {
                // Simple data types

                // Read in the value of the variable, exactly the number of bytes that we need

                GlobalReadMem((BYTE *) &Value, (DWORD) pVar, nSimpleTypes[(int)*pType1->pDef]);

                buf += PrintBasicTypeValue(buf, &Value, pType1);
            }
            else
            {
                // Complex data types: structure, union, array, ... and enum

                // If this is an enum, process it differently -> decode it
                if( *pType1->pDef=='e' )
                {
                    buf += PrintEnumTypeValue(buf, pType1->pDef + 1, pVar);
                }
                else
                {
                    buf += sprintf(buf, "%08X", (DWORD) pVar);
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
                buf += sprintf(buf, "NULL");
            }
            else
            {
                // Read the effective address of that pointer: "void *" is unsigned int

                if( GlobalReadDword(&Value._unsigned_int, (DWORD) pVar) )
                {
                    buf += sprintf(buf, "%08X", (DWORD) pVar);

                    // Try to get one item from the pointee to print out
                    // but process "char *" separately since it is a string

                    if( *pType1->pDef==TYPE_CHAR || *pType1->pDef==TYPE_SIGNED_CHAR || *pType1->pDef==TYPE_UNSIGNED_CHAR )
                    {
                        buf += PrintExpandCharPtr(buf, (BYTE *) Value._unsigned_int);
                    }
                    else
                    {
                        if( *pType1->pDef <= TYPEDEF__LAST )
                        {
                            // Read in the value of the variable, exactly the number of bytes that we need
                            GlobalReadMem((BYTE *) &Value, Value._unsigned_int, nSimpleTypes[(int)*pType1->pDef]);

                            // Read the data from the memory being pointed to, but enclose it in brackets
                            *buf++ = ' ';
                            *buf++ = '<';
                            buf += PrintBasicTypeValue(buf, &Value, pType1);
                            *buf++ = '>';
                        }
                        else
                        {
                            // Pointer to a complex type

                            buf += sprintf(buf, " {...}");
                        }
                    }
                }
                else
                {
                    buf += sprintf(buf, "%08X <illegal>", (DWORD) pVar);
                }
            }
        }
        else
        {
            // Multi-level pointer indirection

            // Verify the common case of a NULL pointer first
            if( pVar==0 )
            {
                buf += sprintf(buf, "NULL");
            }
            else
            {
                buf += sprintf(buf, "%08X {{...}}", (DWORD) pVar);
            }
        }
    }
    else
    {
        // Pointers to data type or name are NULL
        buf += sprintf(buf, "<unknown>");
    }

    // Zero-terminate the output buffer
    *buf = '\0';
}
