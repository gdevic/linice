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
extern void ArmBreakpoints(void);
extern void DisarmBreakpoints(void);
extern BOOL EvalBreakpoint(void);
extern BOOL cmdStep(char *args, int subClass);
extern BOOL MultiTrace(void);
extern BOOL RepeatSrcTrace(void);
extern BOOL RepeatSrcStep(void);
extern void DebPrintErrorString();


/******************************************************************************
*                                                                             *
*   void DebuggerEnterBreak(void)                                             *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop: Functions running at different debugger states
*
******************************************************************************/
/*
    Interrupt 1
            * Single step trap (TR)
            * BP due to hardware debug DR0-DR3
            * INT 1 (Two-byte opcode)

    Interrupt 3
            * INT3  (1-byte opcode 0xCC)
            * INT 3 (2-byte opcode)
*/

void DebuggerEnterBreak(void)
{
    BOOL fAcceptNext;                   // Flag to continue looping inside debugger

    // Abort possible single step trace state
    deb.r->eflags &= ~TF_MASK;

    //-----------------------------------------------------------------------
    {
        deb.bpIndex = -1;                   // Reset the index to signal no breakpoint hit

        // Disarm all breakpoints and adjust counters.
        // We dont need to disarm breakpoints if we are in the CPU trace mode

        if( !deb.fTrace )
            DisarmBreakpoints();

        {
            // Set the content variables used in debugging with symbols. We need this
            // context for the next few step and trace tests
            SetSymbolContext(deb.r->cs, deb.r->eip);

            // We can check for repeating instructions only if we did not hit any official breakpoint
            if( deb.bpIndex==-1 )
            {
                // If we are in the "P RET" cycling-step mode, call the P command handler
                // to manage the exit from this kind of loop (hitting RET instruction).
                // We exit if we actually hit RET command in this mode.
                if( deb.fStepRet && cmdStep("", NULL)==FALSE )
                    goto P_RET_Continuation;

                // If we are in the SRC mode, and doing a step or trace over the source lines,
                // we may need to repeatedly issue trace to get to the next line or a function
                if( deb.fSrcTrace && RepeatSrcTrace() )
                    goto P_RET_Continuation;

                if( deb.fSrcStep && RepeatSrcStep() )
                    goto P_RET_Continuation;

                // If the multiple-step trace is on, we wanted more than a single instruction,
                // (because a trace count is specified) so we will count down a number of instructions
                if( deb.nTraceCount && MultiTrace() )
                    goto T_count_continuation;
            }
            // ===================================================================================
            {
                // This function will cause evaluation of a breakpoint condition, and
                // possibly the action. The action taken may end in a continuation of
                // execution, in which case we jump forward
                if( EvalBreakpoint()==FALSE )
                {
                    // Enable output driver and save background if flash is on or we are not in step or trace
                    if( deb.fFlash || !(deb.fStep || deb.fTrace) )
                    {
                        dputc(DP_ENABLE_OUTPUT);
                        dputc(DP_SAVEBACKGROUND);
                    }
                    // Recalculate window locations based on visibility and number of lines
                    // and repaint all windows
                    RecalculateDrawWindows();
                    {
                        // Reset various trace state flags
                        deb.fTrace = FALSE;
                        deb.nTraceCount = 0;
                        deb.fSrcTrace = FALSE;

                        deb.fStep = FALSE;
                        deb.fSrcStep = FALSE;

                        //========================================================================
                        // MAIN COMMAND PROMPT LOOP
                        //========================================================================
                        do
                        {
                            EdLin( sCmd );

                            fAcceptNext = CommandExecute( sCmd+1 );     // Skip the prompt

                            // Redraw the debugger screen if we were requested to do so
                            if( deb.fRedraw )
                            {
                                RecalculateDrawWindows();
                                deb.fRedraw = FALSE;
                            }

                            // Print the possible error message that occurred during the command run
                            if( deb.error )
                            {
                                DebPrintErrorString();
                                deb.error = NOERROR;
                            }

                        } while( fAcceptNext );

                        //========================================================================
                    }

                    // Restore background and disable output driver if flash is on or we are not in step or trace
                    if( deb.fFlash || !(deb.fStep || deb.fTrace) )
                    {
                        dputc(DP_RESTOREBACKGROUND);
                        dputc(DP_DISABLE_OUTPUT);
                    }
                }
            }
        }

        // Copy the content of the general registers in the prev buffer
        // so the next time when we enter the debugger we will be able
        // to tell what registers had changed
        memcpy(&deb.r_prev, deb.r, sizeof(TREGS));

P_RET_Continuation:
T_count_continuation:

        // Arm all breakpoints
        // We dont arm breakpoints if we are in the CPU trace mode

        if( !deb.fTrace )
            ArmBreakpoints();
    }

    // If the fTrace signal flag is set, set it in the eflags register. This
    // signals a single step over one machine instruction
    if( deb.fTrace )
        deb.r->eflags |= TF_MASK;

    // We can not execute HLT instruction in trace mode, so if the instruction is HLT, advance the EIP
    if( deb.fTrace )
    {
        // Make sure the address is valid
        if( GetByte(deb.r->cs, deb.r->eip)==0xF4)
        {
            deb.r->eip++;                   // Advance EIP if the current instruction is HLT (0xF4)
        }
    }

    // Set RESUME flag on the eflags and return to the client. This way we dont
    // break on the same condition, if a hardware bp would trigger it at this address
    deb.r->eflags |= RF_MASK;

    return;
}


/******************************************************************************
*                                                                             *
*   void DebuggerEnterDelayedArm(void)                                        *
*                                                                             *
*******************************************************************************
*
*   Alternate entry point - this state will restart the debugee execution
*   but it will restore all the breakpoints first.
*
******************************************************************************/
void DebuggerEnterDelayedArm(void)
{
    // Delayed arm - arm breakpoints and continue

    ArmBreakpoints();

    // Clear the trace flag that was set in order for us to be here
    deb.r->eflags &= ~TF_MASK;

    // If the fTrace signal flag is set, set it in the eflags register. This
    // signals a single step over one machine instruction
    if( deb.fTrace )
        deb.r->eflags |= TF_MASK;

    deb.fDelayedArm = FALSE;

    return;
}

