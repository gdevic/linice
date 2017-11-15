/******************************************************************************
*                                                                             *
*   Module:     printk.c                                                      *
*                                                                             *
*   Date:       01/12/05                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
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

        This module contains the printk hook and supports the functionality
        of capturing the kernel printk() into Linice command buffer.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 01/12/05   Initial version                                      Goran Devic *
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

static BYTE PrintkKernelCodeLine[5];    // Buffer to store original code bytes
static BYTE *PrintkAddress = NULL;      // Address of the printk() function

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   asmlinkage void PrintkHandler(char *format, ...)                          *
*                                                                             *
*******************************************************************************
*
*   This function gets called instead of kernel printk() after we are hooked.
*
******************************************************************************/
asmlinkage void PrintkHandler(char *format, ...)
{
    static char printBuf[1024];
    char *p;
    va_list arg;

    // TODO: It is possible to overflow the buffer by printk()-ing a large string. Fix it.

    // Print the line into our local buffer so we can clean it up and print
    va_start( arg, format );
    ivsprintf(printBuf, format, arg);
    va_end(arg);

    // Since we are using our own print driver which has support for more
    // output options, we need to clean the incoming string from all
    // non-ASCII characters to avoid the string messing up the output.
    for(p=printBuf; *p ; p++ )
    {
        if( !isprint(*p) )
            *p = ' ';
    }

    dprinth(1, "%s", printBuf);
}

/******************************************************************************
*                                                                             *
*   void HookPrintk(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Hooks the kernel printk() function so we can capture the debug stream.
*
******************************************************************************/
void HookPrintk(void)
{
    INFO("HookPrintk()\n");

    if( !PrintkAddress )
    {
        PrintkAddress = (BYTE *) ice_get_printk();

        if( PrintkAddress )
        {
            // Store the original code bytes into our buffer
            memcpy(PrintkKernelCodeLine, PrintkAddress, sizeof(PrintkKernelCodeLine));

            // Form the hook jump into our own function
            *(BYTE *) (PrintkAddress+0) = 0xE9;      // JMP <rel32>
            *(DWORD *)(PrintkAddress+1) = (DWORD) PrintkHandler - ((DWORD) PrintkAddress + 5);
        }
        else
        {
            INFO("printk() NOT hooked!\n");
        }
    }
    else
    {
        dprinth(1, "kernel printk() already hooked!");
    }
}

/******************************************************************************
*                                                                             *
*   void UnhookPrintk(void)                                                   *
*                                                                             *
*******************************************************************************
*
*   Unhooks the kernel printk() function.
*
******************************************************************************/
void UnhookPrintk(void)
{
    INFO("PrintkUnhook()\n");

    if( PrintkAddress )
    {
        // Restore the original code bytes from our buffer
        memcpy(PrintkAddress, PrintkKernelCodeLine, sizeof(PrintkKernelCodeLine));

        PrintkAddress = NULL;
    }
}

