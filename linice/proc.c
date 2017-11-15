/******************************************************************************
*                                                                             *
*   Module:     proc.c                                                        *
*                                                                             *
*   Date:       10/14/00                                                      *
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

        Simple read-only /proc virtual file implementation

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/14/00   Initial version                                      Goran Devic *
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
    len = 0;

    // Print the number of interrupts that we trapped

    len += sprintf(buf+len, "Host ints:      Linice ints:\n");
    len += sprintf(buf+len, " int1:  %5d     %5d\n", pIce->nIntsPass[0x01], pIce->nIntsIce[0x01]);
    len += sprintf(buf+len, " int3:  %5d     %5d\n", pIce->nIntsPass[0x03], pIce->nIntsIce[0x03]);
    len += sprintf(buf+len, " timer: %5d     %5d\n", pIce->nIntsPass[0x20], pIce->nIntsIce[0x20]);
    len += sprintf(buf+len, " kbd:   %5d     %5d\n", pIce->nIntsPass[0x21], pIce->nIntsIce[0x21]);
    len += sprintf(buf+len, " com2:  %5d     %5d\n", pIce->nIntsPass[0x23], pIce->nIntsIce[0x23]);
    len += sprintf(buf+len, " com1:  %5d     %5d\n", pIce->nIntsPass[0x24], pIce->nIntsIce[0x24]);
    len += sprintf(buf+len, " ps/2:  %5d     %5d\n", pIce->nIntsPass[0x2C], pIce->nIntsIce[0x2C]);
    len += sprintf(buf+len, " PF:    %5d     %5d\n", pIce->nIntsPass[0x0E], pIce->nIntsIce[0x0E]);

    return( len );
}

