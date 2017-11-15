/******************************************************************************
*                                                                             *
*   Module:     typesprint.c                                                  *
*                                                                             *
*   Date:       02/09/04                                                      *
*                                                                             *
*   Copyright (c) 2004 Goran Devic                                            *
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

        This module contains various utility functions to print types

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 02/09/04   Initial version                                      Goran Devic *
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

};


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL GlobalReadDword(DWORD *pDword, DWORD dwAddress);
extern BOOL GlobalReadBYTE(BYTE *pByte, DWORD dwAddress);

extern TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame);
extern void ListDel(TLIST *pList, TLISTITEM *pItem, BOOL fDelRoot);
extern void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce);
extern TLISTITEM *ListGetNewItem();
extern TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen, WORD file_id);
extern void TypedefCanonical(TSYMTYPEDEF1 *pType1);
extern void scan2dec(char *pBuf, int *p1, int *p2);

extern UINT GetTypeSize(TSYMTYPEDEF1 *pType1);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void PrintTypeListExpand(TLISTITEM *pListItem);

static int PrintTypeName(char *buf, TSYMTYPEDEF1 *pType1);


/******************************************************************************
*   int PrintStructTypeName(char *buf, TSYMTYPEDEF1 *pType1)                  *
*******************************************************************************
*
*   Prints the structure type name.
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintStructTypeName(char *buf, TSYMTYPEDEF1 *pType1)
{
    int written = 0;                    // Number of characters written

    written += sprintf(buf, "struct %s ", pType1->pName);

    return( written );
}

/******************************************************************************
*   int PrintUnionTypeName(char *buf, TSYMTYPEDEF1 *pType1)                   *
*******************************************************************************
*
*   Prints the union type name.
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintUnionTypeName(char *buf, TSYMTYPEDEF1 *pType1)
{
    int written = 0;                    // Number of characters written

    written += sprintf(buf, "union %s ", pType1->pName);

    return( written );
}

/******************************************************************************
*   int PrintEnumTypeName(char *buf, TSYMTYPEDEF1 *pType1)                    *
*******************************************************************************
*
*   Prints the enum type name.
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintEnumTypeName(char *buf, TSYMTYPEDEF1 *pType1)
{
    int written = 0;                    // Number of characters written

    written += sprintf(buf, "enum %s ", pType1->pName);

    return( written );
}

/******************************************************************************
*   int PrintArrayTypeName(char *buf, TSYMTYPEDEF1 *pType1)                   *
*******************************************************************************
*
*   Prints the array type name.
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintArrayTypeName(char *buf, TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 Type1;                 // Temporary type descriptor
    int written = 0;                    // Number of characters written
    char *p;                            // Pointer to various sections of typedef

    // Find the base type name of the array element
    // ar(1,17);0;9;(1,7)
MultiDimArray:
    p = pType1->pDef;
    p = strchr(p, '(');                 // Look for the first set of parents '('
    if( p )
    {
        p = strchr(p+1, '(');           // Look for the second set of parents '('
        if( p )
        {
            // Get the child element type and resolve it into a new typedef
            pType1 = Type2Typedef(p, 0, pType1->file_id);

            // Make the new type a canonical
            memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));
            TypedefCanonical(&Type1);

            // See if we have another dimention of array without a pointer redirection
            if( Type1.maj==0 && *Type1.pDef=='a' )
            {
                goto MultiDimArray;
            }

            // Print the element type name

            written = sprintf(buf, "array ");

            written += PrintTypeName(buf+written, &Type1);
        }
    }

    return( written );
}

/******************************************************************************
*   int PrintTypeName(char *buf, TSYMTYPEDEF1 *pType1)                        *
*******************************************************************************
*
*   Prints the type name into the given buffer.
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintTypeName(char *buf, TSYMTYPEDEF1 *pType1)
{
    static char *sPtr = "*****";        // Levels of pointer redirection
    int written = 0;                    // Number of characters written

    // Type has to be a valid data type pointer
    if( pType1 && pType1->pDef )
    {
        if( *pType1->pDef <= TYPEDEF__LAST )
        {
            // The type is one of the base types

            strcpy(buf, sSimpleTypes[(UINT)*pType1->pDef]);

            written = strlen(sSimpleTypes[(UINT)*pType1->pDef]);
        }
        else
        {
            // The type is a complex type or a specific descriptor

            switch( *pType1->pDef )
            {
                case 's':   written = PrintStructTypeName(buf, pType1);  break;
                case 'u':   written = PrintUnionTypeName(buf, pType1);  break;
                case 'e':   written = PrintEnumTypeName(buf, pType1);  break;
                case 'a':   written = PrintArrayTypeName(buf, pType1);  break;
                case 'f':   written = sprintf(buf, "function ");  break;
                case 'r':   written = sprintf(buf, "int ");  break;
                case 'B':   written = PrintTypeName(buf, Type2Typedef(pType1->pDef, 0, pType1->file_id)); break;     // volatile variable descriptor
                    break;
                default:
                    written = sprintf(buf, "<unknown> %s...", substr(pType1->pDef, 0, 47));
            }
        }

        // Follow with a number of optional pointer redirectors (only for canonical types)

        if( pType1->min==0 && pType1->maj )
        {
            written += sprintf(buf + written, "%s", sPtr + 5 - pType1->maj);
        }
    }

    return( written );
}

/******************************************************************************
*   int PrintArrayIndexList(char *buf, TSYMTYPEDEF1 *pType1)                  *
*******************************************************************************
*
*   Expands a list of arrays in the form [x][y]... by walking the typedef chain
*
*   Where:
*       buf - outout buffer to print to
*       pType1 - Type descriptor
*
*   Returns:
*       The number of characters written
*
******************************************************************************/
static int PrintArrayIndexList(char *buf, TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 Type1;                 // Temporary type descriptor
    int written = 0;                    // Number of characters written
    char *pDef;                         // Pointer to various sections of typedef
    int lower, upper;                   // Array bounds variables

ArrayNextDimension:
    // Copy the type descriptor into local structure so we can make it canonical

    memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));
    TypedefCanonical(&Type1);

    // If the type is an array, process it
    if( *Type1.pDef=='a' )
    {
        pDef = Type1.pDef;

        // Read the array bounds
        pDef = strchr(pDef, ';');           // Get to the bounds part
        scan2dec(pDef+1, &lower, &upper);   // Scan 2 decimal numbers "%d,%d"
        pDef = strchr(pDef, '(');           // Get to the typedef of a child element

        // Print the current array dimension

        written += sprintf(buf+written, "[%d]", upper-lower+1);

        // Read the typedef of a child element and loop if it is an array without redirection

        pType1 = Type2Typedef(pDef, 0, pType1->file_id);

        goto ArrayNextDimension;
    }

    return( written );
}

/******************************************************************************
*                                                                             *
*   void PrettyPrintVariableName(char *pString, char *pName, TSYMTYPEDEF1 *pType1)
*                                                                             *
*******************************************************************************
*
*   Pretty-Prints the type name into the given buffer.
*
*
******************************************************************************/
void PrettyPrintVariableName(char *pString, char *pName, TSYMTYPEDEF1 *pType1)
{
    // Print the type name first

    pString += PrintTypeName(pString, pType1);

    // Follow with the variable name

    pString += sprintf(pString, "%s", pName);

    // Expand with a list of possible array indices

    pString += PrintArrayIndexList(pString, pType1);
}


/******************************************************************************
*                                                                             *
*   void ExpandPrintSymbol(TExItem *Item, char *pName)                        *
*                                                                             *
*******************************************************************************
*
*   Expands symbol one level and pretty-prints it
*
******************************************************************************/
void ExpandPrintSymbol(TExItem *Item, char *pName)
{
#if 0
    static char buf[MAX_STRING+1], *pStr = buf;
    static char indir[] = "*****";
    DWORD dwSymbol;                     // Symbol content

// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
    dprinth(1, " bType = %d", Item->bType);
    dprinth(1, " pData = %08X", Item->pData);
    dprinth(1, " Data  =%c%08X", Item->pData==&Item->Data? '*': ' ', Item->Data);
    dprinth(1, " Type.maj = %d", Item->Type.maj);
    dprinth(1, " Type.min = %d", Item->Type.min);
    dprinth(1, " Type.pName = %08X %s", Item->Type.pName, Item->Type.pName? Item->Type.pName : "");
    if( Item->Type.pDef )
    {
        if( *Item->Type.pDef<32 )
            dprinth(1, " Type.pDef  = %08X [%d]", Item->Type.pDef, *Item->Type.pDef);
        else
            dprinth(1, " Type.pDef  = %08X %s", Item->Type.pDef, Item->Type.pDef);
    }
    else
        dprinth(1, " Type.pDef  = %08X", Item->Type.pDef);

    return;
// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST
#endif

    TLIST List;                         // List node
    TLISTITEM *pItem;                   // Root item to be added to the list

    // Create a temporary list with the symbol in the root

    memset(&List, 0, sizeof(TLIST));

    List.ID = LIST_ID_EXPRESSION;

    if((pItem = ListAdd(&List, &pWin->h)))
    {
        // Copy the expression Item into the root list node and format the root list node
        memcpy(&pItem->Item, Item, sizeof(TExItem));
        strcpy(pItem->Name, pName);

        PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);

        // Expand that root symbol if possible

        PrintTypeListExpand(pItem);

        // Print the expanded item - at this point we need to set the history window to
        // not visible, so the list draw function will not try to print it from the
        // top of that window
        pWin->h.fVisible = FALSE;

        ListDraw(&List, &pWin->h, TRUE);

        pWin->h.fVisible = TRUE;        // Of course, history window is always visible :)

        // Now we are done, delete the list
        ListDel(&List, pItem, TRUE);
    }
}



/******************************************************************************
*
*   TLISTITEM *TypeListExpandStruct(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)

*******************************************************************************
*
*   Expands a structure.
*
******************************************************************************/
static TLISTITEM *TypeListExpandStruct(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)
{
    TLISTITEM *pFirstItem = NULL;       // First item that is added
    TLISTITEM *pPrev = NULL;            // Previous item so we can link them
    TLISTITEM *pItem;                   // New item to add
    PSTR pDef;                          // Type definition string
    int nStart, nSize;                  // Element start offset and size

    // Parse the structure type descriptor and keep adding new items to the list
    pDef = pType1->pDef + 1;

    // Get the total structure size in bytes (and discard since we dont need that information)
    GetDec(&pDef);          // This will advance iterator past the decimal number

    do
    {
        if( (pItem = ListGetNewItem()) )
        {
            // Remember the first item so we can return it; also link the new one to the previous one
            if( pFirstItem==NULL )
                pFirstItem = pPrev = pItem;
            else
                pPrev->pNext = (struct TLISTITEM *) pItem, pPrev = pItem;

            // Expanded items are one level higher than the parent item
            pItem->nLevel = pListItem->nLevel + 1;

            // Copy the element name until the ":" delimiter into the name field
            pDef += strccpy(pItem->Name, pDef, ':');

            // No need to terminate name string since ListGetNewItem() will zero out all its fields
            pDef++;

            // Get the type of that element and format the TExItem structure
            pType1 = Type2Typedef(pDef, 0, pType1->file_id);

            pItem->Item.bType = EXTYPE_SYMBOL;
            memcpy(&pItem->Item.Type, pType1, sizeof(TSYMTYPEDEF1));

            // Make the stored type canonical
            TypedefCanonical(&pItem->Item.Type);

            // Calculate the offset of that element from the start of the structure
            pDef = strchr(pDef, ')') + 1;       // Advance past the type definition
            scan2dec(pDef+1, &nStart, &nSize);  // Scan 2 decimal numbers "%d,%d"
            pDef = strchr(pDef, ';') + 1;       // Find the end of the element description

            pItem->delta = nStart;              // Assign the starting offset in bits
            pItem->width = nSize;               // Assign the size (width) in bits

            // Finally, set up the root string for this element

            PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);
        }
    }while(pItem && *pDef!=';');

    return( pFirstItem );
}

/******************************************************************************
*
*   TLISTITEM *TypeListExpandUnion(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)
*
*******************************************************************************
*
*   Expands an union.
*
******************************************************************************/
static TLISTITEM *TypeListExpandUnion(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)
{
    // Unions are processed the same as structures

    return( TypeListExpandStruct(pListItem, pType1) );
}

/******************************************************************************
*
*   TLISTITEM *TypeListExpandArray(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)
*
*******************************************************************************
*
*   Expands an array.
*
******************************************************************************/
static TLISTITEM *TypeListExpandArray(TLISTITEM *pListItem, TSYMTYPEDEF1 *pType1)
{
    TSYMTYPEDEF1 Type1;                 // Actual type descriptor
    TLISTITEM *pFirstItem = NULL;       // First item that is added
    TLISTITEM *pPrev = NULL;            // Previous item so we can link them
    TLISTITEM *pItem;                   // New item to add
    int lower, upper;                   // Array bounds variables
    UINT nDelta, nSize;                 // Offset and the Size of a child element
    PSTR pDef;                          // Type definition string

    // Parse the structure type descriptor and keep adding new items to the list
    pDef = pType1->pDef + 1;

    // Array: ar(%d,%d);%d;%d;(%d,%d)
    // (1,16) = ar(1,17);0;9;(0,2)
    //                        \ type of the child element
    //                    \ array bounds

    pDef = strchr(pDef, ';');           // Get to the bounds part
    scan2dec(pDef+1, &lower, &upper);   // Scan 2 decimal numbers "%d,%d"
    pDef = strchr(pDef, '(');           // Get to the typedef of a child element

    // If the array bounds are not explicitly set, assume 1 element; do this also
    // if there are too many elements
    if(lower>upper)
        lower = upper = 0;
    if(upper-lower>MAX_ARRAY_EXPAND)
        lower = 0, upper = MAX_ARRAY_EXPAND;

    // Get the type of the array element and copy it into our local structure
    pType1 = Type2Typedef(pDef, 0, pType1->file_id);
    memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));

    // Make the stored type canonical
    TypedefCanonical(&Type1);

    nSize = GetTypeSize(&Type1);        // Get the size in bytes of the child type
    nDelta = lower * nSize;             // We start with this element, although in C that will always be 0

    for(; lower<=upper; lower++ )
    {
        if( (pItem = ListGetNewItem()) )
        {
            // Remember the first item so we can return it; also link the new one to the previous one
            if( pFirstItem==NULL )
                pFirstItem = pPrev = pItem;
            else
                pPrev->pNext = (struct TLISTITEM *) pItem, pPrev = pItem;

            // Expanded items are one level higher than the parent item
            pItem->nLevel = pListItem->nLevel + 1;

            pItem->Item.bType = EXTYPE_SYMBOL;

            // The element name is the array counter in [brackets]
            sprintf(pItem->Name, "[%d]", lower);

            // Copy the child element type node
            memcpy(&pItem->Item.Type, &Type1, sizeof(TSYMTYPEDEF1));

            // Calculate the offset of that element from the start of the structure
            pItem->delta = nDelta * 8;      // Assign the starting offset in bits
            nDelta += nSize;                // Add the counter to the next element

            // Finally, set up the root string for this element

            PrettyPrintVariableName(pItem->String, pItem->Name, &pItem->Item.Type);
        }
    }

    return( pFirstItem );
}


/******************************************************************************
*                                                                             *
*   void PrintTypeListExpand(TLISTITEM *pListItem)                            *
*                                                                             *
*******************************************************************************
*
*   Expands a given list item into a linked list of its elements.
*
*   Where:
*       pListItem - item (root node or parent variable) to expand
*
******************************************************************************/
void PrintTypeListExpand(TLISTITEM *pListItem)
{
    TSYMTYPEDEF1 *pType1;               // Shortcut pointer to the root type

    // If a type of that list item can be expanded, expand it
    if( pListItem->Item.bType==EXTYPE_SYMBOL )
    {
        pType1 = &pListItem->Item.Type;

        switch( *pType1->pDef )
        {
            case 's':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandStruct(pListItem, pType1);  break;
            case 'u':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandUnion( pListItem, pType1);  break;
            case 'a':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandArray( pListItem, pType1);  break;

//        case 's':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandStruct(pListItem, pType1->pDef + 1);   break;
//        case 'u':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandUnion(pListItem, pType1->pDef + 1);    break;
//        case 'a':   pListItem->pElement = (struct TLISTITEM *) TypeListExpandArray(pListItem, pType1->pDef + 1);    break;
        }
    }
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
*   There is a slight difference between first two - '*' will cause all
*   types to be printed out, it is more of a dumping option.
*
******************************************************************************/
BOOL cmdTypes(char *args, int subClass)
{
    char buf[MAX_STRING+1];             // Temp string to print a line to
    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pType;                 // Type structure header
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    int nLine = 1;                      // Standard line counter
    BOOL fPrint;                        // Whether we print a type of skip it
    TSYMTYPEDEF1 Type1;                 // Type descriptor to make canonical
    TLIST List;                         // List node
    TLISTITEM *pItem;                   // Root item to be added to the list

    buf[MAX_STRING] = 0;

    if( deb.pSymTabCur )
    {
        if( deb.pFnScope )
        {
            if( *args && *args!='*' )
            {
                // Argument is a specific type name - find it and list its members

                pType1 = Type2Typedef(args, strlen(args), deb.pFnScope->file_id);

                if( pType1 )
                {
                    // Get the type name portion
                    pStr = pType1->pName;

                    // Copy the type descriptor since we will make it canonical
                    memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));

                    TypedefCanonical(&Type1);

                    // Create a temporary list with the symbol in the root
                    memset(&List, 0, sizeof(TLIST));

                    List.ID = LIST_ID_TYPE;

                    if((pItem = ListAdd(&List, &pWin->h)))
                    {
                        PrettyPrintVariableName(pItem->String, "TYPE", pType1);

                        // Expand that root symbol if possible

                        memcpy(&pItem->Item.Type, pType1, sizeof(TSYMTYPEDEF1));
                        pItem->Item.bType = EXTYPE_SYMBOL;

                        PrintTypeListExpand(pItem);

                        // Print the expanded item - at this point we need to set the history window to
                        // not visible, so the list draw function will not try to print it from the
                        // top of that window
                        pWin->h.fVisible = FALSE;

                        ListDraw(&List, &pWin->h, TRUE);

                        pWin->h.fVisible = TRUE;        // Of course, history window is always visible :)

                        // Now we are done, delete the list
                        ListDel(&List, pItem, TRUE);
                    }
                }
                else
                    dprinth(1, "Unrecognized type name.");
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

                        dprinth(nLine++, "Size  Type Name                      Typedef");

                        for(; nTypedefs>0; nTypedefs--)
                        {
                            fPrint = TRUE;  // Assume we will print every type

                            // Get the type name portion
                            pStr = pType1->pName;

                            // Dont print if the type is not named or it is a basic type
                            if( pStr==NULL || *pStr=='\0' )
                                fPrint = FALSE;

                            if( *pType1->pDef <= TYPEDEF__LAST )
                                fPrint = FALSE;

                            // Copy the type descriptor since we will make it canonical
                            memcpy(&Type1, pType1, sizeof(TSYMTYPEDEF1));

                            TypedefCanonical(&Type1);

                            PrintTypeName(buf, &Type1);

                            // If the argument was not '*', limit the type that we print

                            if( !(*args!='*' && fPrint==FALSE) )
                                if( dprinth(nLine++, "%04X  %-30s %s", GetTypeSize(&Type1), pStr, buf)==FALSE)
                                    return( TRUE );

                            pType1++;
                        }
                    }

                    pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
                }
            }
        }
        else
            dprinth(1, "Unknown context.");
    }
    else
        dprinth(1, "No symbol table loaded.");

    return( TRUE );
}

