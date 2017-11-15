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
extern void GetSysreg( TSysreg * pSys );
extern void SetSysreg( TSysreg * pSys );
extern BOOL NonStickyBreakpointCheck(TADDRDESC Addr);
extern void ClearNonStickyBreakpoint();
extern void ArmBreakpoints(void);
extern void DisarmBreakpoints(void);
extern int  BreakpointCheck(TADDRDESC Addr);
extern BOOL BreakpointCondition(int index);
extern void SetSymbolContext(WORD wSel, DWORD dwOffset);

/******************************************************************************
*                                                                             *
*   void DebuggerEnter * (void)                                               *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop: Functions running at different debugger states
*
******************************************************************************/
/*
    INT 1
            * Single step trap (TR)
            * BP due to hardware debug DR0-DR3
            * single, non-sticky debugger breakpoint DRx
            * single, non-sticky user breakpoint (HERE command - execute to..)
            * User INT 1 in kernel
            * User INT 1 in app

    INT 3
            * BPX user breakpoint
            * single, non-sticky debugger breakpoint
            * single, non-sticky user breakpoint (HERE)
            * User INT3  (1-byte opcode) in kernel/app
            * User INT 3 (2-byte opcode) in kernel/app

*/

void DebuggerEnterBreak(void)
{
    BOOL fContinue;
    
    // Abort possible single step trace state
    deb.r->eflags &= ~TF_MASK;

    //-----------------------------------------------------------------------
    {
        // Read in all the system state registers
        GetSysreg(&deb.sysReg);

        // Adjust system registers to running the debugger:
        //  CR0[16] Write Protect -> 0   so we can write to user pages
        SET_CR0( deb.sysReg.cr0 & ~BITMASK(WP_BIT));

        {
            // Disarm all breakpoints by resetting original opcodes at places
            // where we inserted INT3
            // We dont need to disarm them if we are in single step (Trace) mode!
            if( !deb.fTrace )
                DisarmBreakpoints();

            // Reset the trace state
            deb.fTrace = FALSE;
    
            {
                // Enable output driver and save background
                dputc(DP_ENABLE_OUTPUT);
                dputc(DP_SAVEBACKGROUND);

                // Set the content variables used in debugging with symbols
                SetSymbolContext(deb.r->cs, deb.r->eip);

                // Recalculate window locations based on visibility and number of lines
                // and repaint all windows
                RecalculateDrawWindows();

                //========================================================================
                // MAIN COMMAND PROMPT LOOP
                //========================================================================
                do
                {
                    EdLin( sCmd );

                    fContinue = CommandExecute( sCmd+1 );   // Skip the prompt

                } while( fContinue );

                //========================================================================

                // Restore background and disable output driver
                dputc(DP_RESTOREBACKGROUND);
                dputc(DP_DISABLE_OUTPUT);
            }

            // Arm all breakpoints by inserting INT3 opcode
            // We dont arm them if we are in single step (Trace) mode!
            if( !deb.fTrace )
                ArmBreakpoints();
        }

        // Copy the content of the general registers in the prev buffer
        // so the next time when we enter the debugger we will be able
        // to tell what registers had changed
        memcpy(&deb.r_prev, deb.r, sizeof(TREGS));

        // Restore system registers
        SetSysreg(&deb.sysReg);
    }

    // If the fTrace signal flag is set, set it in the eflags register. This
    // signals a single step over one machine instruction
    if( deb.fTrace )
        deb.r->eflags |= TF_MASK;

    // Set RESUME flag on the eflags and return to the client. This way we dont
    // break on the same condition, if the hardware bp would trigger it at this address
    deb.r->eflags |= RF_MASK;

    return;
}

void DebuggerEnterDelayedTrace(void)
{
    // Delayed Trace - set the Trace flag for this run
    deb.r->eflags |= TF_MASK;
    deb.fTrace = TRUE;

    // Exit delayed Trace mode on a next turn
    deb.fDelayedTrace = FALSE;

    // Set RESUME flag on the eflags and return to the client. This way we dont
    // break on the same condition, if the hardware bp would trigger it at this address
    deb.r->eflags |= RF_MASK;

    // Set the debugger state back to the normal
    pIce->eDebuggerState = DEB_STATE_BREAK;

    return;
}

void DebuggerEnterDelayedArm(void)
{
    // Delayed arm - arm breakpoints and continue

    {
        // Read in all the system state registers
        GetSysreg(&deb.sysReg);

        // Adjust system registers to running the debugger:
        //  CR0[16] Write Protect -> 0   so we can write to user pages
        SET_CR0( deb.sysReg.cr0 & ~BITMASK(WP_BIT));

        {
            ArmBreakpoints();
        }
    
        // Restore system registers
        SetSysreg(&deb.sysReg);
    }

    // Set the debugger state back to the normal
    pIce->eDebuggerState = DEB_STATE_BREAK;

    // Clear trace flag
    deb.r->eflags &= ~TF_MASK;

    return;
}

