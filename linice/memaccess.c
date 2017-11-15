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
    // Since we are writing a byte and the selector value may be of any privilege,
    // we use kernel ds that has base=0 so we recalculate offset.

    // TODO: Right now we support only user/kernel selectors with base 0
    //       In order to provide more generic support, we need to recompute
    //       offset based on the base address of the given selector

    SetByte(__KERNEL_DS, pAddr->offset, value);
}
