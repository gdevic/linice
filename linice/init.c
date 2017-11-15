/******************************************************************************
*                                                                             *
*   Module:     init.c                                                        *
*                                                                             *
*   Date:       09/09/00                                                      *
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

#include "module-header.h"              // Versatile module header file

#include <asm/uaccess.h>                // User space memory access functions

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include hardware defines

#include "ice-ioctl.h"                  // Include our own IOCTL numbers

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
extern BOOL InitUserVars(int num);
extern BOOL InitMacros(int num);
extern void InitEdit();

extern BYTE *ice_malloc(DWORD size);
extern void ice_free_heap(BYTE *pHeap);

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
        if( pIce->pHistoryBuffer == NULL )
        {
            // Allocate heap for the history buffer

            if( (pIce->pHistoryBuffer = ice_malloc(pInit->nHistorySize)) != NULL)
            {
                INFO(("Allocated %d Kb for history pool\n", pInit->nHistorySize / 1024));

                CommandBuildHelpIndex();

                // Initialize history buffer so we can start using it

                pIce->nHistorySize = pInit->nHistorySize;
                ClearHistory();

                // Link the VGA to be the initial output device

                VgaInit();
                pOut = &outVga;

                // Set default values for initial windows:
                // Visible: registers, data and code windows and, of course, history

                pWin->r.fVisible = TRUE;
                pWin->r.nLines   = 3;
                pWin->d.fVisible = TRUE;
                pWin->d.nLines   = 5;
                pWin->c.fVisible = TRUE;
                pWin->c.nLines   = 5;
                pWin->h.fVisible = TRUE;

                // Windowing is disabled at first (until we break into the debugger)
                pWin->fEnable = FALSE;

                // Initialize default data pointer
                deb.dataAddr.sel = __KERNEL_DS;
                deb.dataAddr.offset = 0;
                deb.DumpSize = 1;           // Dump bytes

                // Initialize default code pointer
                deb.codeTopAddr.sel = __KERNEL_CS;
                deb.codeTopAddr.offset = 0;
                deb.fCode = FALSE;
                deb.eSrc = 1;                       // Default Source ON

                // Initialize the default break key
                deb.BreakKey = CHAR_CTRL | 'Q';

                HistoryAdd("LinIce (C) 2000-2001 by Goran Devic\n");

                pIce->nXDrawSize = pInit->nDrawSize;

                // Initialize interrupt handling subsystem

                InterruptInit();

                // Allocate heap for the symbol table

                if( pIce->hSymbolBuffer == NULL )
                {
                    if( (pIce->hSymbolBuffer = ice_init_heap(pInit->nSymbolSize)) != NULL )
                    {
                        INFO(("Allocated %d Kb for symbol pool\n", pInit->nSymbolSize / 1024));

                        pIce->nSymbolBufferSize = pIce->nSymbolBufferAvail = pInit->nSymbolSize;

                        // Allocate heap for debuggers internal use

                        if( pIce->hHeap == NULL )
                        {
                            if( (pIce->hHeap = ice_init_heap(MAX_HEAP)) != NULL )
                            {
                                // Allocate space for user variables and macros

                                if( InitUserVars(pInit->nVars) )
                                {
                                    pIce->nVars = pInit->nVars;

                                    if( InitMacros(pInit->nMacros) )
                                    {
                                        pIce->nMacros = pInit->nMacros;

                                        // Initialize command line editor

                                        InitEdit();

                                        // Set the default keyboard layout to English

                                        pIce->layout = LAYOUT_US;

                                        deb.bpIndex = -1;

                                        deb.fLowercase = pInit->fLowercase;
                                        deb.fPause = TRUE;

                                        // Set up default output colors

                                        pIce->col[COL_NORMAL]  = 0x07;
                                        pIce->col[COL_BOLD]    = 0x0B;
                                        pIce->col[COL_REVERSE] = 0x71;
                                        pIce->col[COL_HELP]    = 0x30;
                                        pIce->col[COL_LINE]    = 0x02;

                                        // Copy keyboard F-key assignments

                                        memcpy(pIce->keyFn , pInit->keyFn , sizeof(pIce->keyFn));

                                        // Now we can hook our master IDT so all the faults will route to
                                        // debugger.  This effectively makes it active.

                                        HookDebuger();

                                        // Interpret init command string and execute it

                                        INFO(("INIT: ""%s""\n", pInit->sInit));

                                        if( CommandExecute(pInit->sInit)==TRUE )
                                        {
                                            // Enter the debugger if the init string did not end with command 'X'
                                            // Schedule the debugger entry the same way hotkey does:

                                            pIce->fKbdBreak = TRUE;
                                        }
                                        retval = 0;
                                    }
                                    else
                                    {
                                        ERROR(("INIT: Heap too small for MACROS"));
                                        retval = -ENOMEM;
                                    }
                                }
                                else
                                {
                                    ERROR(("INIT: Heap too small for VARS"));
                                    retval = -ENOMEM;
                                }
                            }
                            else
                            {
                                ERROR(("Unable to allocate %d for memory heap!\n", MAX_HEAP));
                                retval = -ENOMEM;
                            }
                        }
                        else
                        {
                            ERROR(("pIce->hHeap != NULL\n"));
                        }
                    }
                    else
                    {
                        ERROR(("Unable to allocate %d for symbol buffer!\n", pInit->nSymbolSize));
                        retval = -ENOMEM;
                    }
                }
                else
                {
                    ERROR(("pIce->hSymbolBuffer != NULL\n"));
                }
            }
            else
            {
                ERROR(("Unable to allocate %d for history buffer!\n", pInit->nHistorySize));
                retval = -ENOMEM;
            }
        }
        else
        {
            ERROR(("pIce->hHistoryBuffer != NULL\n"));
        }
    }

    return( retval );
}

