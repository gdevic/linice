/******************************************************************************
*                                                                             *
*   Module:     init.c                                                        *
*                                                                             *
*   Date:       03/09/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 03/09/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "ice-ioctl.h"                  // Include our own IOCTL numbers

#include "debug.h"                      // Include our dprintk()
#include "intel.h"                      // Include processor specific stuff

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

typedef struct
{
    BYTE code;
    char *string;
} TDEFAULTFKEY, *PTDEFAULTFKEY;

TDEFAULTFKEY defaultFKEY[] = {
    { F2,  "wr" },
    { F3,  "src" },
    { F4,  "rs" },
    { F5,  "x" },
    { F6,  "ec" },
    { F7,  "here" },
    { F8,  "t" },
    { F9,  "bpx" },
    { F10, "p" },

    { SF3, "format" },

    { CF8, "xt" },
    { CF9, "trace" },
    { CF10,"xp" },
    { CF11,"show" },

    { AF2, "wd" },
    { AF3, "wc" },
    { AF4, "ww" },
    { AF5, "cls" },

    { 0, NULL }
};


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
extern void HookDebugger();

extern void RegDraw();
extern void DataDraw();
extern void CodeDraw();
extern void HistoryDraw();

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
    PTDEFAULTFKEY pKey = defaultFKEY;
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

                // Init window drawing functions
                pWin->r.draw = RegDraw;
                pWin->d.draw = DataDraw;
                pWin->c.draw = CodeDraw;
                pWin->h.draw = HistoryDraw;

                // Set default values for initial windows:
                // Visible: registers, data and code windows

                pWin->r.fVisible = TRUE;
                pWin->r.nLines   = 3;
                pWin->d.fVisible = TRUE;
                pWin->d.nLines   = 5;
                pWin->c.fVisible = TRUE;
                pWin->c.nLines   = 5;

                // From now on, we can print !!!

                dprint("Allocated %d Kb for history buffer\n", pInit->nHistorySize / 1024);

                // Initialize interrupt handling subsystem

                InterruptInit();

                // Allocate heap for the symbol table

                if( pIce->hSymbolBuffer == NULL )
                {
                    if( (pIce->hSymbolBuffer = ice_init_heap(pInit->nSymbolSize)) != NULL )
                    {
                        INFO(("Allocated %d Kb for symbol pool\n", pInit->nSymbolSize / 1024));
                        dprint("Allocated %d Kb for symbol pool\n", pInit->nSymbolSize / 1024);

                        // Set the default keyboard layout to English

                        pIce->layout = LAYOUT_US;

                        pIce->fLowercase = pInit->fLowercase;

                        // Copy keyboard F-key assignments

                        memcpy(pIce->keyFn , pInit->keyFn , sizeof(pIce->keyFn));

                        // Set the default F-strings for those that are not supplied with init packet

                        while( pKey->code )
                        {
                            if( *pIce->keyFn[pKey->code - F1]==0 )  // If still unassigned...
                                strcpy((void *)pIce->keyFn[pKey->code - F1], pKey->string);
                            pKey++;
                        }

                        // Now we can hook our master IDT so all the faults will route to
                        // debugger.  This effectively makes it active.

                        HookDebugger();

                        // Interpret init command string and execute it

                        if( CommandExecute(pInit->sInit)==TRUE )
                        {
                            // Enter the debugger if the init string did not end with command 'X'

                            INT3();
                        }

                        retval = 0;
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
