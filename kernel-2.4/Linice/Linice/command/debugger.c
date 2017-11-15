/******************************************************************************
*                                                                             *
*   Module:     debugger.c                                                    *
*                                                                             *
*   Date:       04/31/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        This module contains debugger main loop.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/31/00   Original                                             Goran Devic *
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

static char sCmd[MAX_STRING];


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void RecalculateDrawWindows();
extern void EdLin( char *sCmdLine );

/******************************************************************************
*                                                                             *
*   void DebuggerEnter(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop
*
******************************************************************************/
void DebuggerEnter(void)
{
    BOOL fContinue = TRUE;

    // If we are back from the trace, we evaluate that case later..
    if( !deb.fTrace )
    {
        // Check INT3
        if( deb.nInterrupt==3 )
        {
            // Check for user breakpoint addresses


            // Check for single one-time bp (command P, command G with break-address)


            if( deb.fStep )
            {
                ;
            }

            // If we dont want to stop on INT3..
            if( deb.fI3Here==FALSE )
                goto ReturnToDebugee2;

            // If we stopped in the user code, but we want to break in kernel only..
            if( deb.fI3Kernel && deb.r->eip < PAGE_OFFSET )
                goto ReturnToDebugee2;

            dprinth(1, "Breakpoint due to INT3");
        }

        // Check INT1
        if( deb.nInterrupt==1 )
        {
            // If we dont want to stop on INT1..
            if( deb.fI1Here==FALSE )
                goto ReturnToDebugee2;

            // If we stopped in the user code, but we want to break in kernel only..
            if( deb.fI1Kernel && deb.r->eip < PAGE_OFFSET )
                goto ReturnToDebugee2;

            dprinth(1, "Breakpoint due to INT1");
        }
    }

    // This is opposite from down below:
    // If the FLASH is off and the command was P or T, do NOT save screen
    if( !(deb.fFlash==0 && (deb.fTrace || deb.fStep)) )
    {
        // Enable windowing and save background
        pWin->fEnable = TRUE;
        dputc(DP_SAVEBACKGROUND);
    }

    // Set the new CS:EIP for code disasembly
    deb.codeAddr.sel = deb.r->cs;
    deb.codeAddr.offset = deb.r->eip;

    // Recalculate window locations based on visibility and number of lines
    // and repaint all windows
    RecalculateDrawWindows();

    // If the Trace bit was set, finish with the trace command
    if( deb.fTrace )
    {
        // What is the instruction trace count?
        if( --deb.TraceCount )
        {
            // Single step another instruction - may be aborted by pressing ESC
            // Get non-blocking key and check if it is ESC
            if( GetKey(FALSE) != ESC )
                goto ReturnToDebugee;
        }
        // Abort tracing and enter debugger
        deb.r->eflags &= ~TF_MASK;

        // Reset the trace state
        deb.fTrace = FALSE;
    }

    //========================================================================
    while( fContinue )
    {
        EdLin( sCmd );

        fContinue = CommandExecute( sCmd+1 );   // Skip the prompt
    }
    //========================================================================

ReturnToDebugee:

    // Copy the content of the general registers in the prev buffer
    // so the next time when we enter the debugger we will be able
    // to tell what registers had changed
    memcpy(&deb.r_prev, deb.r, sizeof(TREGS));

    // If FLASH is off and the command is P or T, do NOT restore screen
    if( !(deb.fFlash==0 && (deb.fTrace || deb.fStep)) )
    {
        // Disable windowing and restore background
        pWin->fEnable = FALSE;
        dputc(DP_RESTOREBACKGROUND);
    }

ReturnToDebugee2:
}

