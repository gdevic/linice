/******************************************************************************
*                                                                             *
*   Module:     watch.c                                                       *
*                                                                             *
*   Date:       09/11/2002                                                    *
*                                                                             *
*   Copyright (c) 2000 - 2002 Goran Devic                                     *
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

        This module contans code to display and manage watch window and
        associated operations.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

static char buf[MAX_STRING];

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern TLISTITEM *ListAdd(TLIST *pList, TFRAME *pFrame, char *pVar);
extern void ListDraw(TLIST *pList, TFRAME *pFrame, BOOL fForce);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdWatch(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Adds a new expression / variable watch to the list of watches
*
******************************************************************************/
BOOL cmdWatch(char *args, int subClass)
{
    TLISTITEM *pItem;                   // Item to add to watch list

    // Right now we can only watch variables, not expressions

    // New item will be marked as selected
    if(pItem = ListAdd(&deb.Watch, &pWin->w, args))
    {
        // Successfully added a watch item, redraw the window
        WatchDraw(TRUE);
    }
    else
    {
        dprinth(1, "Unable to add a watch.");
    }

    return( TRUE );
}


void WatchDraw(BOOL fForce)
{
    ListDraw(&deb.Watch, &pWin->w, fForce);
}

