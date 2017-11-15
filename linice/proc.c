/******************************************************************************
*                                                                             *
*   Module:     proc.c                                                        *
*                                                                             *
*   Date:       10/14/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
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

static int ProcRead(char *buf, char **start, off_t offset, int len, int *unused, int *data);
static int ProcWrite(void *file, char *buf, unsigned long count, void *data);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

int InitProcFs()
{
    return( ice_init_proc((int) ProcRead, (int) ProcWrite) );
}


int CloseProcFs()
{
    return( ice_close_proc() );
}


/******************************************************************************
*                                                                             *
*   int ProcRead(char *buf, char **start, off_t offset, int len, int *unused) *
*                                                                             *
*******************************************************************************
*
*   Called when /proc/linice is read.
*
*   Where:
*       Standard procfs parameters :)
*
*   Returns:
*       Right now we simply return some collected statistics.
*
******************************************************************************/
static int ProcRead(char *buf, char **start, off_t offset, int len, int *unused, int *data)
{
    len = 0;

    ice_mod_inc_use_count();

    // Print the number of interrupts that we trapped

    len += sprintf(buf+len, "Host ints:      Linice ints:\n");
    len += sprintf(buf+len, " int1:  %5d     %5d\n", deb.nIntsPass[0x01], deb.nIntsIce[0x01]);
    len += sprintf(buf+len, " int3:  %5d     %5d\n", deb.nIntsPass[0x03], deb.nIntsIce[0x03]);
    len += sprintf(buf+len, " timer: %5d     %5d\n", deb.nIntsPass[0x20], deb.nIntsIce[0x20]);
    len += sprintf(buf+len, " kbd:   %5d     %5d\n", deb.nIntsPass[0x21], deb.nIntsIce[0x21]);
    len += sprintf(buf+len, " com2:  %5d     %5d\n", deb.nIntsPass[0x23], deb.nIntsIce[0x23]);
    len += sprintf(buf+len, " com1:  %5d     %5d\n", deb.nIntsPass[0x24], deb.nIntsIce[0x24]);
    len += sprintf(buf+len, " ps/2:  %5d     %5d\n", deb.nIntsPass[0x2C], deb.nIntsIce[0x2C]);
    len += sprintf(buf+len, " PF:    %5d     %5d\n", deb.nIntsPass[0x0E], deb.nIntsIce[0x0E]);

    ice_mod_dec_use_count();

    return( len );
}


/******************************************************************************
*                                                                             *
*   int ProcWrite(struct file *file, char *buf, unsigned long count, void *data)
*                                                                             *
*******************************************************************************
*
*   Called when /proc/linice is written.
*
*   Where:
*       Standard procfs parameters :)
*
*   Returns:
*       We eat all writes for now.
*
******************************************************************************/
static int ProcWrite(void *file, char *buf, unsigned long count, void *data)
{
    // Return the count of data.
    return( count );
}

