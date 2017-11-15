/******************************************************************************
*                                                                             *
*   Module:     debugger.c                                                    *
*                                                                             *
*   Date:       04/31/00                                                      *
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

extern DWORD Checksum1(DWORD start, DWORD len);
extern void DisplayMessage(void);
extern void EdLin( char *sCmdLine );
extern void ArmBreakpoints(void);
extern void DisarmBreakpoints(void);
extern BOOL EvalBreakpoint(void);
extern BOOL cmdStep(char *args, int subClass);
extern BOOL MultiTrace(void);
extern BOOL RepeatSrcTrace(void);
extern BOOL RepeatSrcStep(void);
extern void DebPrintErrorString();
extern void DispatchExtEnter();
extern void DispatchExtLeave();
extern void FixupUserCallFrame(void);


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
    TADDRDESC Addr;                     // Address descriptor
    BOOL fAcceptNext;                   // Flag to continue looping inside debugger

    // Abort possible single step trace state
    deb.r->eflags &= ~TF_MASK;

    //-----------------------------------------------------------------------
    {
        // If we hit our internal INT3 from the custom function call, restore the ESP and EIP
        // to what it was before we issued a call. This supports cmdCall command.
        FixupUserCallFrame();

        deb.bpIndex = -1;                   // Reset the index to signal no breakpoint hit

        // Disarm all breakpoints and adjust counters.
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
                    // Recalculate the checksum of the linice code to make sure it is not
                    // trashed by a misbehaving debugee
                    if( deb.LiniceChecksum != Checksum1((DWORD)ObjectStart, (DWORD)ObjectEnd - (DWORD)ObjectStart) )
                    {
                        dprinth(1, "MEMORY CORRUPTED - SYSTEM UNSTABLE. It is advisable to reboot.");
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

                        // Dispatch the message that we entered the debugger
                        DispatchExtEnter();

                        //========================================================================
                        // MAIN COMMAND PROMPT LOOP
                        //========================================================================
                        do
                        {
                            // Before the user is given a line, display a custom message
                            // But do it only if the command was not empty, to avoid someone
                            // camping on an Enter key and getting messages
                            if( sCmd[1] )
                                DisplayMessage();

                            EdLin( sCmd );

                            fAcceptNext = CommandExecute( sCmd+1 );     // Skip the prompt

                            // Redraw the debugger screen if we were requested to do so
                            if( deb.fRedraw )
                            {
                                RecalculateDrawWindows();
                                deb.fRedraw = FALSE;
                            }

                            // Print the possible error message that occurred during the command run
                            if( deb.errorCode )
                            {
                                DebPrintErrorString();
                                deb.errorCode = NOERROR;
                            }

                        } while( fAcceptNext );

                        //========================================================================
                        // Dispatch the message that we are leaving the debugger
                        DispatchExtLeave();
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
        Addr.sel    = deb.r->cs;
        Addr.offset = deb.r->eip;

        if( AddrGetByte(&Addr)==0xF4)
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

