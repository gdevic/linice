/******************************************************************************
*                                                                             *
*   Module:     types.c                                                       *
*                                                                             *
*   Date:       07/23/02                                                      *
*                                                                             *
*   Copyright (c) 2000-2002 Goran Devic                                       *
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

// Size (memory footprints) of the simple, built-in types

#ifdef WIN32
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
#else
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
#endif

static char *sPtr = "*****";

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen);
TSYMTYPEDEF1 *Type2BaseType(int *pnPtr, TSYMTYPEDEF1 *pType1);


char *GetTypeString(TSYMTYPEDEF1 *pType1)
{
    return( "{...}" );
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

    if( pIce->pSymTabCur )
    {
        if( *args && *args!='*' )
        {
            // Argument is a specific type name - find it and list its members
#if 1
            pType1 = Type2Typedef(args, strlen(args));
            pType1 = Type2BaseType(&nPtrLevel, pType1);
            if( pType1 )
            {
                dprinth(nLine++, "(%d,%d) %s *%d", pType1->maj, pType1->min, GetTypeString(pType1), nPtrLevel);

                // If the type is a complex type (structure etc.), print the complete typedef
                if( pType1->dDef > TYPEDEF__LAST )
                {
// TODO -
//                  PrettyPrintType(pType1);
                }
            }
#endif
#if 0
            pHead = pIce->pSymTabCur->header;

            while( pHead->hType != HTYPE__END )
            {
                if( pHead->hType == HTYPE_TYPEDEF )
                {
                    pType = (TSYMTYPEDEF*)pHead;

                    // Got a type header, show the source file name

                    // TODO - sources do not seem to get assigned right...

                    pSource = SymTabFindSource(pIce->pSymTabCur, pType->file_id);
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

            pHead = pIce->pSymTabCur->header;

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
                        pStr = GET_STRING( pType1->dName );

                        if( dprinth(nLine++, "(%d,%d) %-30s%s", pType1->maj, pType1->min, pStr, GetTypeString(pType1) )==FALSE)
                            return( TRUE );

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
*       pointer to the element descriptor string
*       NULL if the element could not be found
*
******************************************************************************/
char *Type2Element(TSYMTYPEDEF1 *pType, char *pName, int nLen)
{
    char *pDef;                         // Pointer to the type definition string
    int len;

    pDef = GET_STRING( pType->dDef );

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
*                                                                             *
*   TSYMTYPEDEF1 *Type2Typedef(char *pType, int nLen)                         *
*                                                                             *
*******************************************************************************
*
*   Searches the context-sensitive type definition list and returns a typedef
*   that matches search criteria:
*       1) Type number - pType points to a string in the form "(a,b)"
*       2) Type name   - pType points to a type name string
*
*   Where:
*       pType is the type name or number couple
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
//  TSYMSOURCE *pSource;                // Source file that a type belongs to
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    WORD maj=0, min=0;                  // Major and minor type number
    BOOL fTypeNumber = FALSE;           // By default we are searching by type name

    // Sanity check that we have symbols and pTypeName is properly given
    if( pIce->pSymTabCur && deb.pFnScope && pTypeName && *pTypeName )
    {
        pHead = pIce->pSymTabCur->header;

        // If we are searching for a type number, read the (maj,min)

        if( *pTypeName=='(' )
        {
            pTypeName++;                // Advance past '('
            maj = GetDec(&pTypeName);   // Get the major type number
            pTypeName++;                // Advance past delimiting comma
            min = GetDec(&pTypeName);   // Get the minor type number

            fTypeNumber = TRUE;         // Signal that we are now looking for the type number
        }

        while( pHead->hType != HTYPE__END )
        {
            if( pHead->hType == HTYPE_TYPEDEF )
            {
                pType = (TSYMTYPEDEF*)pHead;

                // Got a type header, make sure that the source contex match

                if( pType->file_id == deb.pFnScope->file_id )
                {
    //                pSource = SymTabFindSource(pIce->pSymTabCur, pType->file_id);

                    nTypedefs = pType->nTypedefs;
                    pType1 = pType->list;

                    if( fTypeNumber==TRUE )
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
                            pStr = GET_STRING( pType1->dName );

                            // Compare only type names

                            // TODO: Big mess with type versus variables definition

    //                        if( (*pStr=='T' || *pStr=='t') && !strnicmp(pStr+1, pTypeName, nLen) )
    //                            return( pType1 );
                            if( !strnicmp(pStr, pTypeName, nLen) )
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
*   TSYMTYPEDEF1 *Type2BaseType(int *pnPtr, TSYMTYPEDEF1 *pType1)             *
*                                                                             *
*******************************************************************************
*
*   Scans and returns the base type information given the type descriptor string
*
*   Where:
*       pnPtr will be set with the pointer redirection count
*       pType1 is the type descriptor
*
*   Returns:
*       Pointer to the type descriptor, *pnPtr is set to the level of indirection
*       NULL for error
*
******************************************************************************/
TSYMTYPEDEF1 *Type2BaseType(int *pnPtr, TSYMTYPEDEF1 *pType1)
{
    char *pName;                        // Pointer to name string
    char *pDef;                         // Pointer to definition string
    int nPtr = 0;                       // Level of pointer redirections

    // Make sure the given type descriptor is valid
    if( pType1 )
    {
        // Resolve data type by dereferencing the pointers until we find the type name
        do
        {
            if( pType1->dDef <= TYPEDEF__LAST )
                break;

            // Get the new type definition...
            pDef = GET_STRING(pType1->dDef);

            // Check if we have a pointer redirection
            if( *pDef=='*' )
            {
                nPtr++;                     // Increment the pointer redirection

                // After the pointer redirection, we have to have a reference to another typedef
                pDef = pDef + 1;            // This is always "("
            }

            // If we dont have a pointer redirection or another type '(', we are done
            if( *pDef!='(' )
                break;

            // Otherwise, follow the def chain
            pType1 = Type2Typedef(pDef, 0);

        } while( pType1 );
    }

    // Store the pointer redirection level
    *pnPtr = nPtr;

    return( pType1 );
}


/******************************************************************************
*                                                                             *
*   int GetTypeSize(TSYMTYPEDEF1 *pType1)                                     *
*                                                                             *
*******************************************************************************
*
*   Returns the size (in bytes) of the type memory footprint
*
*   Where:
*       pType1 is the type descriptor to get the memory footprint
*
*   Returns:
*       Type memory footprint in bytes
*
******************************************************************************/
int GetTypeSize(TSYMTYPEDEF1 *pType1)
{
    int nSize = 0;                      // Size variable
    int nPtr;                           // Pointer redirection level
    char *pDef;                         // Pointer to the type definition

    // Make sure the given type descriptor is valid
    if( pType1 )
    {
        // First get to the base type
        pType1 = Type2BaseType(&nPtr, pType1);

        // If we had any pointer redirection level, the final size is just the pointer size
        if( nPtr )
            return( sizeof(void *) );

        // Depending on the base type, find the size - built in types:
        if( pType1->dDef <= TYPEDEF__LAST )
            return( nSimpleTypes[(int)pType1->dDef] );

        // Get the new type definition...
        pDef = GET_STRING(pType1->dDef);

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
*   char *GetTypeName(int nPtr, TSYMTYPEDEF1 *pType1)                         *
*                                                                             *
*******************************************************************************
*
*   Forms a type name with regards to pointer redirection and type 'type'
*   (basic or complex)
*
*   Where:
*       nPtr is the desired pointer redirection count
*       pType1 is the type descriptor
*
*   Returns:
*       Pointer to a STATIC buffer where the type is printed
*
******************************************************************************/
char *GetTypeName(int nPtr, TSYMTYPEDEF1 *pType1)
{
    static char buf[MAX_STRING+1];      // Buffer into which print a type name
    char *pDef;                         // Pointer to the type definition string

    buf[0] = 0;                         // Reset the buffer string to zero

    // Make sure the given type descriptor is valid
    if( pType1 )
    {
        // We have two cases - basic type and complex type
        if( pType1->dDef <= TYPEDEF__LAST )
        {
            // Basic type, simply copy the built-in type name
            strcpy(buf, sSimpleTypes[pType1->dDef]);
        }
        else
        {
            // If the type has a name, use that name and finish
            if( !pType1->dName )
            {
                // If a type does not have name, follow it until we get the name or terminate

                // Get the type definition string
                pDef = GET_STRING(pType1->dDef);

                // If a type is a pointer reference, follow it recursively
                if( *pDef=='*' )
                {
                    return( GetTypeName(nPtr+1, Type2Typedef(pDef+1, 0) ));
                }

                // If a type is a simple type redirection, follow it as well
                if( *pDef=='(' )
                {
                    return( GetTypeName(nPtr, Type2Typedef(pDef+1, 0) ));
                }

                // If the type is forward-defined (or without definition, handle it first
                if( *pDef=='x' )
                    pDef++;             // Skip to the type designator xs, xu, xe

                // Complex nameless data type - parse depending on the string
                switch( *pDef )
                {
                case 's':               // Structure
                    strcpy(buf, "struct ");
                break;

                case 'u':               // Union
                    strcpy(buf, "union ");
                break;

                case 'e':               // Enumeration
                    strcpy(buf, "enum ");
                break;

                case 'a':               // Array
                    strcpy(buf, "array ");
                break;

                case 'f':               // Function
                    strcpy(buf, "function ");
                break;

                case 'r':               // A type that is a subrange of itself - right now we assume "int"
                                        // TYPEDEF( 8,28)  = r(8,28);0000000000000;0037777777777;;0;1;(0,1)
                    strcpy(buf, "int ");
                break;
                }
            }

            // Append the type name if the type has a name, otherwise it's a null string anyways
            strcat(buf, GET_STRING( pType1->dName ));
        }

        // Finish with writing the appriprate number of pointer redirections
        strcat(buf, &sPtr[strlen(sPtr)-nPtr]);
    }

    return( buf );
}


int AdjustFinalTypeAddress(TADDRDESC *pAddr, TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 *pBaseType;            // Base or root type
    int nPtr = -1, i;                   // Level of pointer redirection

    // Make sure the given type descriptor is valid
    if( pType1 )
    {
        // First get to the base (or root) type so we know what kind of node we are dealing with
        // This call will also return the number of indirections in nPtr
        pBaseType = Type2BaseType(&nPtr, pType1);

        if( nPtr )
        {
            i = nPtr;

            // Pointer redirection - resolve the address redirection
            while( i-- )
            {
                // Get the value or the next address dereference
                pAddr->offset = AddrGetDword(pAddr);

                // If there was an error accessing the address, return
                if( deb.memaccess & 0x100 )
                {
                    return( -1 );
                }
            }
        }
    }

    // No pointer redirection, so the final address is the same

    return( nPtr );
}

/******************************************************************************
*                                                                             *
*   char *GetTypeValue(PTADDRDESC pAddr, TSYMTYPEDEF1 *pType1)                *
*                                                                             *
*******************************************************************************
*
*   Prints a final alphanumerical value for the specified type that resides
*   at the given address. The symbol target address will be adjusted as
*   necessary to follow the pointer redirection.
*
*   Where:
*       pSymTarget is the address of the variable to start, will adjust
*       pType1 is the type of the variable, does not have to be base
*
*   Returns:
*       Pointer to a STATIC buffer where the result is printed
*
******************************************************************************/
char *GetTypeValue(PTADDRDESC pAddr, TSYMTYPEDEF1 *pType1)
{
    static char buf[MAX_STRING+1];      // Buffer into which print the result value
    TSYMTYPEDEF1 *pBaseType;            // Base or root type
    int nPtr;                           // Level of pointer redirection
    DWORD dwValue;                      // Sample value from the target address
    BOOL fPtr = FALSE;                  // Do we have pointer redirection?

    // Assume unknown type so we can simply return on errors
    sprintf(buf, "<?>");

    // Make sure the given type descriptor is valid
    if( !pType1 )
        return( buf );

    // First get to the base (or root) type so we know what kind of node we are dealing with
    // This call will also return the number of indirections in nPtr
    pBaseType = Type2BaseType(&nPtr, pType1);

    // If there was a pointer redirection, process it differently from basic types
    if( nPtr )
    {
        // Pointer redirection

        // Resolve pointer redirection, exiting if any of the addresses are bad
        while( nPtr-- )
        {
            // Get the value or the next address dereference
            pAddr->offset = AddrGetDword(pAddr);

            // The last access in the line of address pointers may be a pseudo-legal NULL
            if( nPtr==0 && pAddr->offset==NULL )
            {
                sprintf(buf, "NULL");
                return( buf );
            }

            // If there was an error accessing the address, return
            if( deb.memaccess & 0x100 )
                return( buf );
        }

        // Get the final value - Get the DWORD that is most likely value targeted

        dwValue = AddrGetDword(pAddr);

        // If the base type is simple, evaluate its target; otherwise, just print the address
        if( pBaseType->dDef <= TYPEDEF__LAST )
        {
            // Print the variable in the correct format depending on the final variable type
            switch( pBaseType->dDef )
            {
            case 1: // int *        // (0,1): int:t(0,1)=r(0,1);-2147483648;2147483647;
            case 3:                 // (0,3): long int:t(0,3)=r(0,3);-2147483648;2147483647;
                sprintf(buf, "0x%08X, %d", dwValue, (signed) dwValue);
            break;

            case 2: // char *       // (0,2): char:t(0,2)=r(0,2);0;127;
            case 10:                // (0,10): signed char:t(0,10)=@s8;r(0,10);-128;127;
            case 11:                // (0,11): unsigned char:t(0,11)=@s8;r(0,11);0;255;
            {
                // char *

                // See how much of a string can we get from the address
                char *p = AddrGetByte(pAddr);

                if( deb.memaccess & 0x100 )
                {
                    // Error accessing the final address
                    sprintf(buf, "0x%08X <?>", p);
                }
                else
                {
#                   define SAMPLE_MAX  16                       // How much of a sample string to get?
                    // We could access the final address
                    char Sample[SAMPLE_MAX+1];                  // Sample string buffer
                    int i;

                    for(i=0; i<SAMPLE_MAX; i++)
                    {
                        Sample[i] = AddrGetByte(pAddr);
                        pAddr->offset++;

                        // If there was an error accessin the char, exit
                        if( deb.memaccess & 0x100 )
                            break;

                        // If the character is not printable, exit
                        if( Sample[i]<' ' || Sample[i]>127 )
                            break;
                    }

                    // Terminate the sample string
                    Sample[i] = 0;

                    sprintf(buf, "0x%08X <\"%s\">", p, Sample);
                }
            }
            break;

            case 4:                 // (0,4): unsigned int:t(0,4)=r(0,4);0000000000000;0037777777777;
            case 5:                 // (0,5): long unsigned int:t(0,5)=r(0,5);0000000000000;0037777777777;
                sprintf(buf, "%X", (unsigned int) dwValue);
            break;

            case 6:                 // (0,6): long long int:t(0,6)=@s64;r(0,6);01000000000000000000000;0777777777777777777777;
                sprintf(buf, "[long long int]");
            break;

            case 7:                 // (0,7): long long unsigned int:t(0,7)=@s64;r(0,7);0000000000000;01777777777777777777777;
                sprintf(buf, "[long long unsigned int]");
            break;

            case 8:                 // (0,8): short int:t(0,8)=@s16;r(0,8);-32768;32767;
                sprintf(buf, "%X, %d", dwValue & 0xFFFF, (short int) (dwValue & 0xFFFF));
            break;

            case 9:                 // (0,9): short unsigned int:t(0,9)=@s16;r(0,9);0;65535;
                sprintf(buf, "%X", (short unsigned int) (dwValue & 0xFFFF));
            break;

            case 12:                // (0,12): float:t(0,12)=r(0,1);4;0;
                sprintf(buf, "[float]");
            break;

            case 13:                // (0,13): double:t(0,13)=r(0,1);8;0;
                sprintf(buf, "[double]");
            break;

            case 14:                // (0,14): long double:t(0,14)=r(0,1);12;0;
                sprintf(buf, "[long double]");
            break;

            case 15:                // (0,15): complex int:t(0,15)=s8real:(0,1),0,32;imag:(0,1),32,32;;
                sprintf(buf, "[complex int]");
            break;

            case 16:                // (0,16): complex float:t(0,16)=R3;8;0;
                sprintf(buf, "[complex float]");
            break;

            case 17:                // (0,17): complex double:t(0,17)=R4;16;0;
                sprintf(buf, "[complex double]");
            break;

            case 18:                // (0,18): complex long double:t(0,18)=R5;24;0;
                sprintf(buf, "[complex long double]");
            break;
            }
        }
        else
        {
            // Complex type - just return the address of the structure / array
            sprintf(buf, "0x%08X", dwValue);
        }
    }
    else
    {
        // No pointer redirection - Get the DWORD that is most likely value targeted

        dwValue = AddrGetDword(pAddr);

        // If there was an error accessing the address, return
        if( deb.memaccess & 0x100 )
            return( buf );

        // If the base type is simple, evaluate its target; otherwise, just print the address
        if( pBaseType->dDef <= TYPEDEF__LAST )
        {
            // Print the variable in the correct format depending on the final variable type
            switch( pBaseType->dDef )
            {
            case 1:                 // (0,1): int:t(0,1)=r(0,1);-2147483648;2147483647;
            case 3:                 // (0,3): long int:t(0,3)=r(0,3);-2147483648;2147483647;
                sprintf(buf, "0x%08X, %d", dwValue, (signed) dwValue);
            break;

            case 2:                 // (0,2): char:t(0,2)=r(0,2);0;127;
            case 10:                // (0,10): signed char:t(0,10)=@s8;r(0,10);-128;127;
            case 11:                // (0,11): unsigned char:t(0,11)=@s8;r(0,11);0;255;
            {
                // Simple type

                char c = (unsigned char) (dwValue & 0xFF);

                if( c>=' ' )
                    sprintf(buf, "0x%02X '%c'", c, c);      // Printable character
                else
                    sprintf(buf, "0x%02X", c);              // Not printable character
            }
            break;

            case 4:                 // (0,4): unsigned int:t(0,4)=r(0,4);0000000000000;0037777777777;
            case 5:                 // (0,5): long unsigned int:t(0,5)=r(0,5);0000000000000;0037777777777;
                sprintf(buf, "%X", (unsigned int) dwValue);
            break;

            case 6:                 // (0,6): long long int:t(0,6)=@s64;r(0,6);01000000000000000000000;0777777777777777777777;
                sprintf(buf, "[long long int]");
            break;

            case 7:                 // (0,7): long long unsigned int:t(0,7)=@s64;r(0,7);0000000000000;01777777777777777777777;
                sprintf(buf, "[long long unsigned int]");
            break;

            case 8:                 // (0,8): short int:t(0,8)=@s16;r(0,8);-32768;32767;
                sprintf(buf, "%X, %d", dwValue & 0xFFFF, (short int) (dwValue & 0xFFFF));
            break;

            case 9:                 // (0,9): short unsigned int:t(0,9)=@s16;r(0,9);0;65535;
                sprintf(buf, "%X", (short unsigned int) (dwValue & 0xFFFF));
            break;

            case 12:                // (0,12): float:t(0,12)=r(0,1);4;0;
                sprintf(buf, "[float]");
            break;

            case 13:                // (0,13): double:t(0,13)=r(0,1);8;0;
                sprintf(buf, "[double]");
            break;

            case 14:                // (0,14): long double:t(0,14)=r(0,1);12;0;
                sprintf(buf, "[long double]");
            break;

            case 15:                // (0,15): complex int:t(0,15)=s8real:(0,1),0,32;imag:(0,1),32,32;;
                sprintf(buf, "[complex int]");
            break;

            case 16:                // (0,16): complex float:t(0,16)=R3;8;0;
                sprintf(buf, "[complex float]");
            break;

            case 17:                // (0,17): complex double:t(0,17)=R4;16;0;
                sprintf(buf, "[complex double]");
            break;

            case 18:                // (0,18): complex long double:t(0,18)=R5;24;0;
                sprintf(buf, "[complex long double]");
            break;
            }
        }
        else
        {
            // Complex type - just return the address of the structure / array
            sprintf(buf, "0x%08X", dwValue);
        }
    }

    return( buf );
}


void PrintListItem(char *pBuf, TLISTITEM *pItem, char *pSymbol, int nNameLen, int nArrayIndex)
{
    TSYMTYPEDEF1 *pBaseType1;           // Pointer to the base type descriptor
    int nPtr;                           // Level of pointer redirection
    TADDRDESC Addr;                     // Address descriptor
    char *pTypeName;                    // Pointer to type name
    char *pTypeValue;                   // Pointer to type final value
    char sSymbol[MAX_SYMBOL_LEN+1];     // Symbol name

//    Addr.sel = pItem->pSymbol.sel;
//    Addr.sel = KERNEL_DS;
    Addr.offset = pItem->pSymbol;

    pBaseType1 = Type2BaseType(&nPtr, pItem->pType1);

    pTypeName = GetTypeName(0, pItem->pType1);

    pTypeValue = GetTypeValue(&Addr, pItem->pType1);

    // Prepare the optional symbol name to print
    if( pSymbol )
    {
        strncpy(sSymbol, pSymbol, nNameLen);
        sSymbol[nNameLen] = 0;
    }
    else
        sSymbol[0] = 0;

    // If we are printing a root array item, force nArrayIndex to the array bounds of the variable
    if( pItem->Array_Bound )
        nArrayIndex = pItem->Array_Bound;

    // In order to slightly indent each line, progressively as it is expanded,
    // for each level of expansion, prepend two spaces
    while( pItem->Parent )
    {
        pBuf += sprintf(pBuf, "  ");
        pItem = pItem->Parent;
    }

    if( nArrayIndex >= 0 )
        sprintf(pBuf, "%s %s[%d] = %s", pTypeName, sSymbol, nArrayIndex, pTypeValue);
    else
        sprintf(pBuf, "%s %s = %s", pTypeName, sSymbol, pTypeValue);
}

/******************************************************************************
*                                                                             *
*   BOOL SetupForExpansion(TLISTITEM *pItem)                                  *
*                                                                             *
*******************************************************************************
*
*   Prepares a listitem node for expansion. This sets up iterators and pointers
*   for easy access at the later time when we are expanding its children.
*
*   Where:
*       pItem is the parent root node to prepare
*
*   Returns:
*       TRUE - Parent node is expandable
*       FALSE - Parent node is not expandable
*
******************************************************************************/
BOOL SetupForExpansion(TLISTITEM *pItem)
{
    TSYMTYPEDEF1 *pBaseType1;           // Pointer to the base type descriptor
    int nPtr;                           // Level of pointer redirection
    char *pDef;                         // Pointer to the type definition string
    int lower, upper;                   // Array bounds variables

    // Get to the base type that this variable represents
    pBaseType1 = Type2BaseType(&nPtr, pItem->pType1);

    // Depending on the base type, set up the expnandable temp values
    if( pBaseType1 && pBaseType1->dDef > TYPEDEF__LAST )
    {
        // Get to the base type definition string to examine it for complex types
        pDef = GET_STRING(pBaseType1->dDef);

        pItem->pBaseTypeDef = pDef;

        switch( *pDef )
        {
            case 'a':                   // Array: ar(%d,%d);%d;%d;(%d,%d)
            {
                // Arrays are the most complex types. We need to prepare child type descriptor
                // and the array bounds, so we read them from the type descriptor

                pDef = strchr(pDef, ';');   // Get to the bounds part
                sscanf(pDef, ";%d;%d", &lower, &upper);
                pDef = strchr(pDef, '(');   // Get to the typedef of a child element

                // If the array bounds are not explicitly set, assume 1 element; do this also
                // if there are too many elements
                if(lower>upper)
                    lower = upper = 0;
                if(upper-lower>MAX_ARRAY_EXPAND)
                    lower = 0, upper = MAX_ARRAY_EXPAND;

                pItem->Array_pTypeChild = Type2Typedef(pDef,0);
                pItem->Array_Bound = upper-lower+1;
                pItem->nIArray = 0;

//                pItem->IAddr->sel = KERNEL_DS;
                pItem->IAddr.offset = pItem->pSymbol;

                AdjustFinalTypeAddress(&pItem->IAddr, pItem->pType1);

                pItem->fExpandable = TRUE;
                return( TRUE );
            }

            case 's':                   // Structure: s<size>_name...
            case 'u':                   // Union: s<size>_name...
            {
                // Structures and unions have definitions with multiple elements,
                // set up the iterator pointer

                pDef = pDef + 1;        // Advance the iterator to the size value
                GetDec(&pDef);          // This will advance iterator past the decimal number
                pItem->pIStruct = pDef; // Store the iterator pointer

//                pItem->IAddr->sel = KERNEL_DS;
                pItem->IAddr.offset = pItem->pSymbol;

                AdjustFinalTypeAddress(&pItem->IAddr, pItem->pType1);

                pItem->fExpandable = TRUE;
                return( TRUE );
            }
        }
    }
    // Everything else is not expandable !

    pItem->fExpandable = FALSE;         // Not expandable

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL TypeExpandIterator(TLISTITEM *pItem)                                 *
*                                                                             *
*******************************************************************************
*
*   Walks one line off the parent node. Calls PrintListItem() for that node
*   and returns if there are more children to print/expand.
*
*   Where:
*       Pointer to a child node
*
*   Returns:
*       TRUE - There are more elements of that parent
*       FALSE - No more elements
*
******************************************************************************/
BOOL TypeExpandIterator(TLISTITEM *pItem)
{
    TLISTITEM *Parent;                  // Parent list item
    char *pDef;                         // Pointer to the type definition string
    char *pName;                        // Structure: Element name
    int nNameLen;                       // Structure: Element name length
    int nStart, nSize;                  // Structure: Element start offset and size

    Parent = pItem->Parent;

    switch( *Parent->pBaseTypeDef )
    {
        case 'a':                       // Expanding an array element
        {
            pItem->pType1 = Parent->Array_pTypeChild;
            pItem->pSymbol = Parent->IAddr.offset;

            PrintListItem(pItem->sExp, pItem, NULL, 0, Parent->nIArray);

            // Prepare for the next element
            Parent->nIArray++;
            Parent->IAddr.offset += GetTypeSize(pItem->pType1);

            if( Parent->nIArray < Parent->Array_Bound )
                return( TRUE );         // Keep expanding
        }
        break;

        case 'u':                       // Explanding union or structure element
        case 's':
        {
            pDef = Parent->pIStruct;    // Structure definition iterator

            // Get the element information from the structure definition string
            pName = pDef;               // Name is at the current iterator address
            nNameLen = strchr(pDef, ':') - pDef;
            pDef = pDef + nNameLen + 1; // After "name:" there is "(x,y),start, size;"

            pItem->pType1 = Type2Typedef(pDef, 0);
            pDef = strchr(pDef, ')') + 1;
            sscanf(pDef, ",%d,%d", &nStart, &nSize);

            pItem->pSymbol = Parent->IAddr.offset;

            // Move the iterator to the next element (or the end of the definition)
            pDef = strchr(pDef, ';') + 1;

            PrintListItem(pItem->sExp, pItem, pName, nNameLen, -1);

            Parent->IAddr.offset += GetTypeSize(pItem->pType1);

            Parent->pIStruct = pDef;    // Update the iterator

            if( *pDef!=';' )            // If there are more elements, keep expanding
                return( TRUE );
        }
        break;
    }

    // By default stop expanding

    return( FALSE );
}


