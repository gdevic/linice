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

extern void EdLin( char *sCmdLine );
extern void GetSysreg( TSysreg * pSys );
extern void SetSysreg( TSysreg * pSys );
extern BOOL NonStickyBreakpointCheck(TADDRDESC Addr);
extern void ClearNonStickyBreakpoint();
extern void ArmBreakpoints(void);
extern void *DisarmBreakpoints(void);
extern BOOL EvalBreakpoint(void *pBp);
extern int  BreakpointCheck(TADDRDESC Addr);
extern void SetSymbolContext(WORD wSel, DWORD dwOffset);
extern char *Index2String(DWORD index);
extern BOOL cmdStep(char *args, int subClass);
extern BOOL MultiTrace(void);


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
    void *pBp = NULL;                   // Pointer to a breakpoint that hit
    BOOL fAcceptNext;                   // Flag to continue looping inside debugger
    
    // Abort possible single step trace state
    deb.r->eflags &= ~TF_MASK;

    //-----------------------------------------------------------------------
    {
        // Read in all the system state registers
        GetSysreg(&deb.sysReg);

        // Adjust system registers to running the debugger:
        //  CR0[16] Write Protect -> 0   so we can write to user pages
        SET_CR0( deb.sysReg.cr0 & ~BITMASK(WP_BIT));

        // Disarm all breakpoints and adjust counters.
        // We dont do this if we are in the single step mode
        if( !deb.fTrace )
            pBp = DisarmBreakpoints();

        {
            // If we are in the "P RET" cycling-trace mode, call the P command handler
            // to manage the exit from this kind of loop (hitting RET instruction)
            // This command will normally return FALSE and we will not enter the inner block code
            if( deb.fStepRet && cmdStep("", NULL)==FALSE )
                goto P_RET_Continuation;

            // If the multiple-step trace is on, we wanted more than a single instruction,
            // (because a trace count could be specified) so we will count down a number of
            // instructions - MultiTrace() will return TRUE if we need to keep looping
            if( MultiTrace() )
                goto T_count_continuation;

            {
                // Reset the trace state and the multi-trace state, just in case
                deb.fTrace = FALSE;
                deb.TraceCount = 0;

                {
                    // Set the content variables used in debugging with symbols
                    SetSymbolContext(deb.r->cs, deb.r->eip);

                    // Enable output driver and save background
                    dputc(DP_ENABLE_OUTPUT);
                    dputc(DP_SAVEBACKGROUND);

                    // Recalculate window locations based on visibility and number of lines
                    // and repaint all windows
                    RecalculateDrawWindows();

                    // This function will cause evaluation of a breakpoint condition, and
                    // possibly the action. The action taken may end in continuation of
                    // execution, in which case we jump forward
                    if( EvalBreakpoint(pBp)==FALSE )
                    {
                        //========================================================================
                        // MAIN COMMAND PROMPT LOOP
                        //========================================================================
                        do
                        {
                            EdLin( sCmd );

                            deb.error = NOERROR;                        // Clear the error code
                            fAcceptNext = CommandExecute( sCmd+1 );     // Skip the prompt

                            // Redraw the debugger screen if we were requested to do so
                            if( deb.fRedraw )
                            {
                                RecalculateDrawWindows();
                                deb.fRedraw = FALSE;
                            }

                            // If there was an error processing the commands, print it
                            if( deb.error )
                                dprinth(1, "%s", Index2String(deb.error) );

                        } while( fAcceptNext );

                        //========================================================================
                    }

                    // Restore background and disable output driver
                    dputc(DP_RESTOREBACKGROUND);
                    dputc(DP_DISABLE_OUTPUT);
                }
            }
        }

        // Copy the content of the general registers in the prev buffer
        // so the next time when we enter the debugger we will be able
        // to tell what registers had changed
        memcpy(&deb.r_prev, deb.r, sizeof(TREGS));

P_RET_Continuation:
T_count_continuation:

        // Arm all breakpoints by inserting INT3 opcode
        // We dont arm them if we are in single step (Trace) mode!
        if( !deb.fTrace )
            ArmBreakpoints();

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

