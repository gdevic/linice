/******************************************************************************
*                                                                             *
*   Module:     lists.h                                                       *
*                                                                             *
*   Date:       02/04/2003                                                    *
*                                                                             *
*   Copyright (c) 2003 Goran Devic                                            *
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

        This header file contains lists structures and protos

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 02/04/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _LISTS_H_
#define _LISTS_H_

#include "queue.h"                      // Include queue definitions

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Define a list item containing information about the symbol item in a list
typedef struct
{
    char sExp[MAX_STRING+1];            // Symbol name / expression

    TSYMTYPEDEF1 *pType1;               // Pointer to the type descriptor
    DWORD *pSymbol;                     // Address of the symbol

    BOOL fExpandable;                   // The item can be visually expanded
    BOOL fExpanded;                     // The item is visually expanded
    struct TLISTITEM *Parent;           // Expanded, parent item

    //-----------------------------------------------------------------------
    char *pBaseTypeDef;                 // Parent base type definition ptr
    TSYMTYPEDEF1 *Array_pTypeChild;     // Array: Type of the child element
    int Array_Bound;                    // Array: Number of elements
    int nIArray;                        // Array: Current iterator value
    char *pIStruct;                     // Structure: Current iterator pointer
    TADDRDESC IAddr;                    // Both: Address iterator

} TLISTITEM;

// Define a structure TLIST, that holds complete information about a list set,
// that is, locals, stack or watch window contents
typedef struct
{
    int ID;                             // ID of this list
    TQueue Item;                        // List of items
    TLISTITEM *pSelected;               // Currently selected item
    TLISTITEM *pWinTop;                 // Current windows top item
    DWORD nXOffset;                     // Display is shifted to show right excess

} TLIST;

#define LIST_ID_WATCH       0x00        // Watch list
#define LIST_ID_LOCALS      0x01        // Locals list
#define LIST_ID_STACK       0x02        // Stack list

/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/


#endif //  _LISTS_H_

