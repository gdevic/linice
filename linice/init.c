/******************************************************************************
*                                                                             *
*   Module:     init.c                                                        *
*                                                                             *
*   Date:       09/09/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This module contains initialization functions

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/09/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "errno.h"                      // Include kernel error numbers

#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "ice-keycode.h"                // Include keyboard codes
#include "ibm-pc.h"                     // Include hardware defines
#include "debug.h"                      // Include our dprintk()
#include "intel.h"                      // Include processor specific stuff

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern PTOUT pOut;                      // Pointer to a current Out class
extern TOUT outVga;

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

extern void CommandBuildHelpIndex();
extern void VgaInit();
extern void VgaSprint(char *s);
extern void InterruptInit();
extern void HookDebuger();
extern void HookSyscall(void);
extern void HookSwitch(void);
extern void HookPrintk(void);
extern BOOL InitUserVars(int num);
extern BOOL InitMacros(int num);
extern void InitEdit();
extern void InitKeyboardLayout(char Layout[3][128]);
extern void InitBreakpoints();
extern BOOL cmdVer(char *args, int subClass);
extern BYTE *memInitHeap(UINT size);
extern void memFreeHeap(BYTE *hHeap);

/******************************************************************************
*                                                                             *
*   int InitPacket(PTINITPACKET pInit)                                        *
*                                                                             *
*******************************************************************************
*
*   This function initializes structures pertinent to init packet data
*
*   Where:
*       pInit is the address of the init packet data
*
*   Returns:
*       0 init ok
*       -EINVAL general failure: abort ice installation
*       -ENOMEM not enough memory: abort ice installation
*       > 0  Invalid init string
*
******************************************************************************/
int InitPacket(PTINITPACKET pInit)
{
    int retval = -EINVAL;

    if( pInit->nSize == sizeof(TINITPACKET) )
    {
        if( deb.hHistoryBufferHeap == NULL )
        {
            // Allocate heap for the history buffer

            if( (deb.hHistoryBufferHeap = memInitHeap(pInit->nHistorySize)) != NULL)
            {
                INFO("Allocated %d Kb for history pool\n", pInit->nHistorySize / 1024);

                if( ice_get_flags() & 1 )
                {
                    // IO APIC is present
                    deb.fIoApic = TRUE;
                }

                if( ice_get_flags() & 2 )
                {
                    // SMP machine
                    deb.fSmp    = TRUE;
                }

                // Build the command help index

                CommandBuildHelpIndex();

                // Initialize history buffer so we can start using it

                deb.nHistorySize = pInit->nHistorySize;
                ClearHistory();

                // Link the VGA to be the initial output device

                VgaInit();
                pOut = &outVga;

                // Save user screen from the VGA display device
                dputc(DP_ENABLE_OUTPUT);
                dputc(DP_SAVEBACKGROUND);

                // Set default values for initial windows:
                // Visible: registers, data and code windows and, of course, history
                // We need to set number of lines even if it is invisible at start

                pWin->r.fVisible = TRUE;    // Registers
                pWin->r.nLines   = 3;
                pWin->l.fVisible = FALSE;   // Locals
                pWin->l.nLines   = 4;
                pWin->w.fVisible = FALSE;   // Watch
                pWin->w.nLines   = 4;
                pWin->s.fVisible = FALSE;   // Stack
                pWin->s.nLines   = 4;
                pWin->d.fVisible = TRUE;    // Data
                pWin->d.nLines   = 5;
                pWin->c.fVisible = TRUE;    // Code
                pWin->c.nLines   = 5;
                pWin->h.fVisible = TRUE;    // History

                // Initialize lists of items
                deb.Watch.ID = LIST_ID_WATCH;
                deb.Local.ID = LIST_ID_LOCALS;
                deb.Stack.ID = LIST_ID_STACK;

                // Adjust all output driver coordinates - we won't print anything since
                // we are not yet in the debugger (deb.fRunningIce is false)
                RecalculateDrawWindows();
                pOut->y = pWin->h.Top + 1;          // Force it into the history buffer

                // Initialize default data pointer
                deb.dataAddr.sel = GetKernelDS();
                deb.dataAddr.offset = 0;
                deb.nDumpSize = 1;                  // Dump bytes

                // Initialize default code pointer
                deb.codeTopAddr.sel = GetKernelCS();
                deb.codeTopAddr.offset = 0;
                deb.fCode = FALSE;
                deb.eSrc = SRC_ON;                  // Default Source ON

                // Initialize the default break key
                deb.BreakKey = CHAR_CTRL | 'Q';

                deb.nTabs = 4;                      // Initial TABS value

                deb.nXDrawSize = pInit->nDrawSize;

                // Initialize interrupt handling subsystem

                InterruptInit();

                // Allocate heap for the symbol table

                if( deb.hSymbolBufferHeap == NULL )
                {
                    if( (deb.hSymbolBufferHeap = memInitHeap(pInit->nSymbolSize)) != NULL )
                    {
                        INFO("Allocated %d Kb for symbol pool\n", pInit->nSymbolSize / 1024);

                        deb.nSymbolBufferSize = deb.nSymbolBufferAvail = pInit->nSymbolSize;

                        // Allocate heap for debuggers internal use

                        if( deb.hHeap == NULL )
                        {
                            if( (deb.hHeap = memInitHeap(MAX_HEAP)) != NULL )
                            {
                                // Allocate space for user variables and macros

                                if( InitUserVars(pInit->nVars) )
                                {
                                    deb.nVars = pInit->nVars;

                                    if( InitMacros(pInit->nMacros) )
                                    {
                                        deb.nMacros = pInit->nMacros;

                                        // Initialize command line editor

                                        InitEdit();

                                        // Copy the keyboard layout overrides

                                        InitKeyboardLayout(pInit->Layout);

                                        // Set different default values

                                        deb.bpIndex = -1;

                                        deb.fLowercase = pInit->fLowercase;
                                        deb.fPause = TRUE;
                                        deb.fTableAutoOn = TRUE;

                                        // Set up default output colors

                                        deb.col[COL_NORMAL]  = 0x07;
                                        deb.col[COL_BOLD]    = 0x0B;
                                        deb.col[COL_REVERSE] = 0x71;
                                        deb.col[COL_HELP]    = 0x30;
                                        deb.col[COL_LINE]    = 0x02;

                                        // Copy keyboard F-key assignments

                                        memcpy(deb.keyFn , pInit->keyFn , sizeof(deb.keyFn));

                                        // Init the breakpoint structures
                                        InitBreakpoints();

                                        // Hook system call table so we can monitor system calls
                                        HookSyscall();

                                        // Now we can hook our master IDT so all the faults will route to
                                        // debugger.  This effectively makes it active.

                                        HookDebuger();

                                        // Hook the task switcher
                                        HookSwitch();

                                        // Hook the kernel printk() output, only if we are not debug build
#                                       ifdef DBG
                                        INFO("Not hooking printk() since this is DEBUG build.\n");
#                                       else
                                        HookPrintk();
#                                       endif

                                        // Linice is now operational
                                        deb.fOperational = TRUE;

                                        // Interpret init command string and execute it

                                        INFO("INIT: ""%s""\n", pInit->sInit);

                                        // Display version information

                                        cmdVer("", 0);

                                        dprinth(1, "BY USING THIS SOFTWARE YOU AGREE WITH THE TERMS OF THE LICENSE AGREEMENT.");
                                        dprinth(1, "");
                                        dprinth(1, "LINICE: Init: %s", pInit->sInit);

                                        if( CommandExecute(pInit->sInit)==TRUE )
                                        {
                                            // Enter the debugger if the init string did not end with command 'X'
                                            // Schedule the debugger entry the same way hotkey does:

                                            deb.nScheduleKbdBreakTimeout = 2;
                                        }

                                        // Restore background and disable output driver
                                        dputc(DP_RESTOREBACKGROUND);
                                        dputc(DP_DISABLE_OUTPUT);

                                        retval = 0;

                                        return( retval );
                                    }
                                    else
                                    {
                                        ERROR("INIT: Heap too small for MACROS");
                                        retval = -ENOMEM;
                                    }
                                }
                                else
                                {
                                    ERROR("INIT: Heap too small for VARS");
                                    retval = -ENOMEM;
                                }

                                memFreeHeap(deb.hHeap);
                            }
                            else
                            {
                                ERROR("Unable to allocate %d for memory heap!\n", MAX_HEAP);
                                retval = -ENOMEM;
                            }
                        }
                        else
                        {
                            ERROR("deb.hHeap != NULL\n");
                        }

                        memFreeHeap(deb.hSymbolBufferHeap);
                    }
                    else
                    {
                        ERROR("Unable to allocate %d for symbol buffer!\n", pInit->nSymbolSize);
                        retval = -ENOMEM;
                    }
                }
                else
                {
                    ERROR("deb.hSymbolBufferHeap != NULL\n");
                }

                // Restore background and disable output driver
                dputc(DP_RESTOREBACKGROUND);
                dputc(DP_DISABLE_OUTPUT);

                memFreeHeap(deb.hHistoryBufferHeap);
            }
            else
            {
                ERROR("Unable to allocate %d for history buffer!\n", pInit->nHistorySize);
                retval = -ENOMEM;
            }
        }
        else
        {
            ERROR("deb.hHistoryBuffer != NULL\n");
        }
    }

    return( retval );
}

