/******************************************************************************
*                                                                             *
*   Module:     proc.c                                                        *
*                                                                             *
*   Date:       04/14/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        Simple read-only /proc virtual file implementation

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/14/01   Initial version                                      Goran Devic *
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

/******************************************************************************
*                                                                             *
*   int ProcLinice(char *buf, char **start, off_t offset, int len, int unused)*
*                                                                             *
*******************************************************************************
*
*   Called when /proc/linice is read.
*
*   Where:
*
*   Returns:
*
******************************************************************************/
int ProcLinice(char *buf, char **start, off_t offset, int len, int unused)
{
    int i;

    len = 0;

    // Print the number of interrupts that we trapped

    len += sprintf(buf+len, "Host ints:           Linice ints:\n");
    len += sprintf(buf+len, " timer: %5d         timer: %5d\n", pIce->nIntsPass[0x20], pIce->nIntsIce[0x20]);
    len += sprintf(buf+len, " kbd:   %5d         kbd:   %5d\n", pIce->nIntsPass[0x21], pIce->nIntsIce[0x21]);
    len += sprintf(buf+len, " com2:  %5d         com2:  %5d\n", pIce->nIntsPass[0x22], pIce->nIntsIce[0x22]);
    len += sprintf(buf+len, " com1:  %5d         com1:  %5d\n", pIce->nIntsPass[0x23], pIce->nIntsIce[0x23]);
    len += sprintf(buf+len, " ps/2:  %5d         ps/2:  %5d\n", pIce->nIntsPass[0x2C], pIce->nIntsIce[0x2C]);
    len += sprintf(buf+len, " PF:    %5d         PF:    %5d\n", pIce->nIntsPass[0x0E], pIce->nIntsIce[0x0E]);

    return( len );
}
