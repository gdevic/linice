/******************************************************************************
*                                                                             *
*   Module:     lists.h                                                       *
*                                                                             *
*   Date:       02/04/2003                                                    *
*                                                                             *
*   Copyright (c) 2003-2005 Goran Devic                                       *
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
#define LIST_ID_STACK       0x02        // Stack list
#define LIST_ID_EXPRESSION  0x03        // Expression query
#define LIST_ID_TYPE        0x04        // Typedef query (no evaluation needed)


// Define generic purpose previous and next counters

#define PREV(value, max)        (((value)==0)? (max)-1:(value)-1)
#define NEXT(value, max)        ((value)+1>=max? 0:((value)+1))


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

#endif //  _LISTS_H_
