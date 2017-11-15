/******************************************************************************
*                                                                             *
*   Module:     memaccess.c                                                   *
*                                                                             *
*   Date:       09/30/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

        This module contains code for memory access

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/30/00   Original                                             Goran Devic *
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

#define MAX_AUXBUF       256            // Maximum fill/search string len

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void RecalculateDrawWindows();


BOOL AddrIsPresent(PTADDRDESC pAddr)
{
    DWORD fPresent;

    fPresent = GetByte(pAddr->sel, pAddr->offset);

    return( fPresent <= 0xFF );
}

BYTE AddrGetByte(PTADDRDESC pAddr)
{
    BYTE value;

    value = (GetByte(pAddr->sel, pAddr->offset) & 0xFF);

    return( value );
}

void AddrSetByte(PTADDRDESC pAddr, BYTE value)
{
    SetByte(pAddr->sel, pAddr->offset, value);
}


