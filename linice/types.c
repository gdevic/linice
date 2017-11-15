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

char *GetTypeString(TSYMTYPEDEF1 *pType1)
{
    static char buf[MAX_STRING+1];      // Static buffer to return the string info
    char *pName;                        // Pointer to name string
    char *pDef;                         // Pointer to definition string

    // Parse type definition string to return the proper type description

    pName = pIce->pSymTabCur->pPriv->pStrings + pType1->dName;
    pDef = pIce->pSymTabCur->pPriv->pStrings + pType1->dDef;

    // If it is a basic type, use the type name as a definition instead and we are done
    if( pType1->dDef <= TYPEDEF__LAST )
    {
        // Type is one of the basic types

        strcpy(buf, pName);
    }
    else
    {
        // If the name type was 'T', prepend "typedef"
        if( *pName=='T' )
            strcpy(buf, "typedef ");
        else
            buf[0] = 0;

        switch( *pDef )
        {
            case '*':   // Type is a pointer to another type, type number follows the '*'
                strcat(buf, "* ");
                strncpy(buf, pDef, MAX_STRING-3);
                buf[MAX_STRING] = 0;
                break;

            case 's':   // Type is a structure
                strcat(buf, "struct ");
                strcat(buf, pName+1);
                break;

            case 'e':   // Type is an enum
                strcat(buf, "enum ");
                strcat(buf, pName+1);
                break;

            case 'u':   // Type is an union
                strcat(buf, "union ");
                strcat(buf, pName+1);
                break;

            default:    // Whatever appears here, we ought to support!
                strncpy(buf, pDef, MAX_STRING);
                buf[MAX_STRING] = 0;
                break;
        }
    }

    return( buf );
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
*       TYPES type      - display type information for the given symbol type
*
******************************************************************************/
BOOL cmdTypes(char *args, int subClass)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMTYPEDEF *pType;                 // Type structure header
    TSYMTYPEDEF1 *pType1;               // Pointer to a single type definition
    WORD nTypedefs;                     // How many types are in the current definition block
    char *pStr;                         // Pointer to name string
    int nLine = 1;

    if( pIce->pSymTabCur )
    {
        if( *args && *args!='*' )
        {
            ;
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

                        if( dprinth(nLine++, "%-30s%s", pStr+1, GetTypeString(pType1) )==FALSE)
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

