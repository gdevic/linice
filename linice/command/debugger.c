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
extern void BreakpointsPlace(void);
extern void BreakpointsRemove(void);
extern int  BreakpointCheck(WORD sel, DWORD offset);
extern BOOL BreakpointCondition(int index);

/******************************************************************************
*                                                                             *
*   void DebuggerEnter(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop
*
******************************************************************************/
/*
    INT 1
            * Single step trap (TR)
            * BP due to hardware debug DR0-DR3
            * User INT 1 in kernel
            * User INT 1 in app

    INT 3
            * BPX breakpoint
            * Single-shot breakpoint (F7)
            * User INT3  (1-byte opcode) in kernel/app
            * User INT 3 (2-byte opcode) in kernel/app

    - Single step; Uses Trace Flag
        Break

    - Single shot bp INT3
        Decrement eip
        Restore byte
        Break

    - BPX breakpoints
        Decrement eip
        Restore byte
        If BP has condition
            If Bp(condition) == TRUE
                If BP has DO statement
                    Command eval DO statements
                Endif
            Endif
        Else
            Break
        Endif

    - User INT3
        If Int3Here = On
            If Int3Here == kernel && eip != kernel
                continue user code
            Else
                Breakpoint due to INT3
        Else
            continue user code
        Endif

    - User INT 1
        If Int1Here = On
            If Int1Here == kernel && eip != kernel
                continue user code
            Else
                Breakpoint due to INT1
        Else
            continue user code
        Endif

    ==========================================================================

    When placing INT3 breakpoints:
    If eip == BP
        Set TF flag to trap after the current instruction
        Set BP_eip flag
    Endif

    On INT3,
    If BP_eip == TRUE
        Store INT3 at the previous eip
        continue
    Endif

*/
void DebuggerEnter(void)
{
    BOOL fContinue = TRUE;

    // Reset the current breakpoint index to none
    deb.bpIndex = -1;

    // If we are back from the trace, we will evaluate that case later..
    // For now, do some things in non-trace case:
    if( !deb.fTrace )
    {
        // Check INT3
        if( deb.nInterrupt==3 )
        {
            // Decrement eip since INT3 is an exception where eip points to the next instruction
            deb.r->eip -= 1;

            // Check for single one-time bp (command P, command G with break-address)


            // Check for breakpoint that we placed over the debugee code;
            // If found, return a positive breakpoint index that we store
            if( (deb.bpIndex = BreakpointCheck(deb.r->cs, deb.r->eip)) >= 0 )
            {
                // Check the IF condition and return to debugee if we dont need to break
                if( !BreakpointCondition(deb.bpIndex) )
                {
                    dprinth(1, "Breakpoint BP%X", deb.bpIndex);

                    goto ReturnToDebugee2;
                }
            }
            else
            {
                if( deb.fStep )
                {
                    ;
                }

                // ---- User placed INT 3 -----
                // We need to revert eip to an instruction following the user INT3
                if( deb.bpIndex>=0 )
                    deb.r->eip += 1;

                // If we dont want to stop on user INT3..
                if( deb.fI3Here==FALSE )
                    goto ReturnToDebugee2;

                // If we stopped in the user code, but we want to break in kernel only..
                if( deb.fI3Kernel && deb.r->eip < PAGE_OFFSET )
                    goto ReturnToDebugee2;

                dprinth(1, "Breakpoint due to INT3");
            }
        }
        else
        if( deb.nInterrupt==1 )     // Check INT1
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

    // Read in all the system state registers
    GetSysreg(&deb.sysReg);

    // Adjust system registers to running the debugger:
    //  CR0[16] Write Protect -> 0   so we can write to user pages
    SET_CR0( deb.sysReg.cr0 & ~BITMASK(WP_BIT));

    // Remove all breakpoints that we placed in the previous run
    BreakpointsRemove();

    //========================================================================
    while( fContinue )
    {
        EdLin( sCmd );

        fContinue = CommandExecute( sCmd+1 );   // Skip the prompt
    }
    //========================================================================

    // Set all breakpoints back so we can trap on them
    BreakpointsPlace();

    // Set the DE (Debugging Extensions) bit in CR4 so we can trap IO accesses
    deb.sysReg.cr4 |= BITMASK(DE_BIT);

    // Restore system registers
    SetSysreg(&deb.sysReg);

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

    // Set RESUME flag on the eflags and return to the client
    deb.r->eflags |= RF_MASK;
}

