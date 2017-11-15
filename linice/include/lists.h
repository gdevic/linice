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

// Define a list item containing information about an atomic item
typedef struct
{
    char *pExp;                         // Expression which evaluates into an item
    char *pType;                        // Expression typedef string

    BOOL fExpanded;                     // The item is an _expanded_ complex type
    TQueue SubList;                     // List of elements in a complex type
    
} TITEM;

// Define a structure TLIST, that holds complete information about a list set,
// that is, locals, stack or watch window contents
typedef struct
{
    TQueue Item;                        // List of items
    TITEM *pSelected;                   // Currently selected item
    TITEM *pWinTop;                     // Current windows top item
    
} TLIST;


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/


#endif //  _LISTS_H_

