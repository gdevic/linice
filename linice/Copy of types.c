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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

char *GetTypeString(TSYMTYPEDEF1 *pType1, BOOL fTypedef)
{
    static char buf[MAX_STRING+1];      // Static buffer to return the string info
    char *pName;                        // Pointer to name string
    char *pDef;                         // Pointer to definition string

    if( pType1 )
    {
        // Parse type definition string to return the proper type description

        pName = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;
        pDef = pIce->pSymTabCur->pPriv->pStrings + pType1->dDef;

        // If it is a basic type, use the type name as a definition instead and we are done
        if( pType1->dDef <= TYPEDEF__LAST )
        {
            // Type is one of the basic types

            strcpy(buf, pName+1);
        }
        else
        {
            memset(buf, 0, sizeof(buf));

            // If the name type was 'T', prepend "typedef", if we asked for it..
            if( *pName=='T' && fTypedef )
                strcpy(buf, "typedef ");

            // If it is a forward-defined type, it has form of xs, xu, xe


            switch( *pDef )
            {
                case 's':   // Type is a structure
                    strcat(buf, "struct ");
                    strncat(buf, pName+1, 80);
                    break;

                case 'e':   // Type is an enum
                    strcat(buf, "enum ");
                    strncat(buf, pName+1, 80);
                    break;

                case 'u':   // Type is an union
                    strcat(buf, "union ");
                    strncat(buf, pName+1, 80);
                    break;

                default:    // Whatever appears here, we ought to support!
                    strncpy(buf, pDef, 80);
                    break;
            }
        }
    }
    else
    {
        strcpy(buf, "unknown");
    }

    return( buf );
}

char *GetTypeStringItem(TSYMTYPEDEF1 *pType1, char *pTypeName)
{
    static char buf[MAX_STRING+1];      // Static buffer to return the string info
    char *pName;                        // Pointer to name string
    char *pDef;                         // Pointer to definition string
    BOOL fForward = FALSE;              // Forward declaration type?

    if( pType1 )
    {
        // Parse type definition string to return the proper type description

        pName = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;
        pDef = pIce->pSymTabCur->pPriv->pStrings + pType1->dDef;

        // If it is a basic type, use the type name as a definition instead and we are done
        if( pType1->dDef <= TYPEDEF__LAST )
        {
            // Type is one of the basic types

            strcpy(buf, pName+1);
        }
        else
        {
            buf[0] = 0;             // Zero-out the buffer

            // If it is a forward-defined type, it has form of xs, xu, xe
            if( *pDef=='x' )
            {
                pDef++;             // Advance to the type character

                switch( *pDef )
                {
                    case 's':       // Type is a structure
                        strcpy(buf, "struct ");
                        strcat(buf, pDef+1);
                        break;
                }
            }
            else
            {
                switch( *pDef )
                {
                    case 's':   // Type is a structure
                        strncat(buf, pName+1, MAX_SYMBOL_LEN);
                        break;

                    case 'e':   // Type is an enum
                        strncat(buf, pName+1, MAX_SYMBOL_LEN);
                        break;

                    case 'u':   // Type is an union
                        strncat(buf, pName+1, MAX_SYMBOL_LEN);
                        break;

                    case 'a':   // Type is an array
                        sprintf(buf, ">>%s<<%s>>", pName, pDef);
                        break;

                    default:    // Whatever appears here, we ought to support!
                        strncpy(buf, pDef, MAX_SYMBOL_LEN);
                        break;
                }
            }
        }
    }
    else
    {
        strcpy(buf, "unknown");
    }

    return( buf );
}


TSYMTYPEDEF1 *Type2Typedef(char *pTypeName);
TSYMTYPEDEF1 *Typedef2TypedefLeaf(int *pPtrLevel, TSYMTYPEDEF1 *pType1);


void PrettyPrintType(TSYMTYPEDEF1 *pType1)
{
    static char buf[MAX_STRING+1];      // Static buffer to print one line
    TSYMTYPEDEF1 *pSubType1;
    char *pDef;                         // Pointer to definition string
    char *pName;                        // Pointer to type name
    DWORD size;                         // Item size
    int nLine = 1;
    int nTokenLen;                      // Token length
    int nPtrLevel = 0;

    pName = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;
    pDef = pIce->pSymTabCur->pPriv->pStrings + pType1->dDef;
    
    switch( *pDef )
    {
        //--------------------------------------------------------------------
        case 's':       // Structure
        //--------------------------------------------------------------------

            if( dprinth(nLine++, "%s {", GetTypeString(pType1, TRUE) )==FALSE) break;

            pDef++;                 // Advance past 's'
            size = GetDec(&pDef);   // Get the structure size in bytes

            do
            {
                buf[0] = 0;             // Reset the buffer
                pName = pDef;           // Field name starts here

                // Find the size of the name token and new def address
                pDef = strchr(pName, ':');
                pDef++;                 // Advance to the subtype type string
                nTokenLen = pDef - pName;

                // Get the leaf type of that particular field
                pSubType1 = Type2Typedef(pDef);
                pSubType1 = Typedef2TypedefLeaf(&nPtrLevel, pSubType1);
                if( pSubType1 )
                {
                    // Print the subtype string
                    strcpy(buf, GetTypeStringItem(pSubType1), NULL);
                    strcat(buf, " ");

                    // Print a number of levels of pointer indirection
                    while( nPtrLevel-- )
                    {
                        strcat(buf, "*");
                    }
                }

                if( dprinth(nLine++, "    %s%s;", buf, substr(pName, 0, nTokenLen-1))==FALSE ) return;

                // Look for the next subitem entry, separated by ';'
                pDef = strchr(pDef, ';');
                pDef++;                     // Advance to the name or end ';'

                // We are at the end of the definition string when hitting ';;'
            } while( *pDef != ';' );

            if( dprinth(nLine++, "};")==FALSE ) break;

            break;

        //--------------------------------------------------------------------
        case 'a':       // Array
//        194  TYPEDEF(53, 3)  = ar(22,4);0;5;(0,3)
            
            {
                int low, high;         // Lower and upper bounds of the array

                pDef = strchr(pDef, ';');
                pDef++;

                low = GetDec(&pDef);
                pDef++;
                high = GetDec(&pDef);
                pDef++;

                // Get the leaf type of that particular field
                pSubType1 = Type2Typedef(pDef);
                pSubType1 = Typedef2TypedefLeaf(&nPtrLevel, pSubType1);
                if( pSubType1 )
                {
                    // Print the subtype string
                    strcpy(buf, GetTypeStringItem(pSubType1), NULL);
                    strcat(buf, " ");

                    // Print a number of levels of pointer indirection
                    while( nPtrLevel-- )
                    {
                        strcat(buf, "*");
                    }
                }

                dprinth(nLine++, "%s [%d];", pName, high-low+1);
            }

            break;
        //--------------------------------------------------------------------

        //--------------------------------------------------------------------
        case 'e':       // Enum
            break;
        //--------------------------------------------------------------------
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
            pType1 = Type2Typedef(args);
            pType1 = Typedef2TypedefLeaf(&nPtrLevel, pType1);
            if( pType1 )
            {

                // If the type is a complex type (structure etc.), print the complete typedef
                if( pType1->dDef > TYPEDEF__LAST )
                {
                    PrettyPrintType(pType1);
                }
                else
                {
                    dprinth(1, "%s *%d", GetTypeString(pType1, TRUE), nPtrLevel);
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
                        pStr = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;

                        if( !stricmp(pStr+1, args) )
                        {
                            if( pSource && !fPrinted )
                            {
                                fPrinted = TRUE;
#if 0
                                if( dprinth(nLine++, "Defined in %s:", 
                                        pIce->pSymTabCur->pPriv->pStrings + pSource->dSourcePath, 
                                        pIce->pSymTabCur->pPriv->pStrings + pSource->dSourceName )==FALSE)
                                    return( TRUE );
#endif
                            }
        
                            if( dprinth(nLine++, "(%d,%d) %-30s%s", pType1->maj, pType1->min, pStr+1, GetTypeString(pType1, TRUE) )==FALSE)
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
                        pStr = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;

                        if( dprinth(nLine++, "(%d,%d) %-30s%s", pType1->maj, pType1->min, pStr+1, GetTypeString(pType1, TRUE) )==FALSE)
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
*                                                                             *
*   TSYMTYPEDEF1 *Type2Typedef(char *pType)                                   *
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
*
*   Returns:
*       Typedef token
*       NULL if the typedef describing that type name could not be found
*
******************************************************************************/
TSYMTYPEDEF1 *Type2Typedef(char *pTypeName)
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
    if( pIce->pSymTabCur && pTypeName && *pTypeName )
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

                // Got a type header, show the source file name

                // TODO - sources do not seem to get assigned right...

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
                        pStr = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;

                        // Compare only type names
                        if( (*pStr=='T' || *pStr=='t') && !stricmp(pStr+1, pTypeName) )
                            return( pType1 );

                        pType1++;
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
*   TSYMTYPEDEF1 *Typedef2TypedefLeaf(int *pPtrLevel, TSYMTYPEDEF1 *pType1)   *
*                                                                             *
*******************************************************************************
*
*   Typedef token to its typedef leaf node, counting the pointer indirection
*   level. Context sensitive to deb.r->eip.
*
*   Where:
*       pPtrLevel is the pointer redirection level
*       pType1 is the input type token
*
*   Returns:
*       Typedef token
*       NULL if the typedef describing that type name could not be found
*
******************************************************************************/
TSYMTYPEDEF1 *Typedef2TypedefLeaf(int *pPtrLevel, TSYMTYPEDEF1 *pType1)
{
    char *pName;                        // Pointer to name string
    char *pDef;                         // Pointer to definition string

    // Sanity check that we have symbols and pType1 is properly given
    if( pIce->pSymTabCur && pType1 && pPtrLevel )
    {
        *pPtrLevel = 0;

        do
        {
            // If we reached a basic type, exit
            if( pType1->dDef <= TYPEDEF__LAST )
                return( pType1 );

            // Parse type definition string to return the proper type description
    
            pName = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;
            pDef = pIce->pSymTabCur->pPriv->pStrings + pType1->dDef;

            // If we have a pointer type, increase the pointer level count
            if( *pDef=='*' )
            {
                *pPtrLevel = *pPtrLevel + 1;

                pDef += 1;
            }

            // Return if we dont have any more type reference in the form "(a,b)"
            if( *pDef != '(' )
                return( pType1 );

            pType1 = Type2Typedef(pDef);

        } while( pType1 );
    }

    return( NULL );
}

