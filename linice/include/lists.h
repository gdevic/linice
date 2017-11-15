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

#include "eval.h"                       // Include expression evaluator

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Define a list item containing information about the symbol item in a list
typedef struct
{
    struct TLISTITEM *pNext;            // Pointer to the next variable item
    struct TLISTITEM *pElement;         // Pointer to the (expanded) element
    BOOL fCanDelete;                    // This list item can be deleted by the user
    UINT nLevel;                        // Level of expansion for right shift
    //-----------------------------------------------------------------------
    // Element information
    //-----------------------------------------------------------------------
    char Name[MAX_STRING];              // Variable name or expression string
    char Value[MAX_STRING];             // Resulting value string
    char String[MAX_STRING];            // Space to print the resulting ASCII string

    TExItem Item;                       // Evaluated expression item

    //-----------------------------------------------------------------------
    // Specifiers used for memory access of expanded structures, unions and arrays
    //-----------------------------------------------------------------------
    UINT delta;                         // Distance in bits from the parent address
    UINT width;                         // Width of the element in bits

} TLISTITEM;

// Define a structure TLIST, that holds complete information about a list set,
// that is, locals or watch window contents
typedef struct
{
    UINT ID;                            // ID of this list
    TLISTITEM *pList;                   // First element of the list
    TLISTITEM *pSelected;               // Currently selected item
    TLISTITEM *pWinTop;                 // Current windows top item
    DWORD nXOffset;                     // Printing offset to the right
    BOOL fInFocus;                      // Highlight the pSelected line

} TLIST;

#define LIST_ID_WATCH       0x00        // Watch list
#define LIST_ID_LOCALS      0x01        // Locals list
#define LIST_ID_EXPRESSION  0x02        // Expression query
#define LIST_ID_TYPE        0x03        // Typedef query (no evaluation needed)

/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/


#endif //  _LISTS_H_

