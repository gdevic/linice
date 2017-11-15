/******************************************************************************
*                                                                             *
*   Module:     memaccess.c                                                   *
*                                                                             *
*   Date:       03/30/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 03/30/01   Original                                             Goran Devic *
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

