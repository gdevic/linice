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
extern BOOL BreakpointCheckSpecial(TADDRDESC Addr);
extern BOOL BreakpointCondition(int index);
extern void SetCurrentSymbolContext(void);

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
#if 0
void DebuggerEnter(void)
{
    static BOOL fReArm = FALSE;         // Rearming of the breakpoints turn
    BOOL fContinue;
    TADDRDESC Addr;                     // Current cs:eip
    BYTE bCsEip;                        // Byte at the current cs:eip

    // Reset the current breakpoint index to none
    deb.bpIndex = -1;

    SetCurrentSymbolContext();

    // If we are back from the trace, we will evaluate that case later..
    // For now, do some things in non-trace case:
    if( !deb.fTrace )
    {
        // Check INT3
        if( deb.nInterrupt==3 )
        {
            // First thing to check is the rearm state
            if( fReArm==TRUE )
            {
                // Arm all the breakpoints and continue with the execution of the debugee
                ArmBreakpoints();

                fReArm = FALSE;                 // Make sure we have this flag cleared

                deb.r->eflags &= ~TF_MASK;      // Abort single step trace
            
                return;
            }
        
            // Get a byte from the current cs:eip - we use it to distinguish
            // a single byte INT3 from a two-byte opcode.
            Addr.sel = deb.r->cs;
            Addr.offset = deb.r->eip;
            bCsEip = AddrGetByte(&Addr);

            // If it is a two-byte INT3 opcode, it can only be user embedded "int 3"
            if( bCsEip!=0xCC )
            {
                goto UserInt3;
            }
            else
            {
                // Decrement eip since INT3 is an exception where eip points to the next instruction
                deb.r->eip -= 1;
                
            }

            // Check for single one-time bp (command P, command G with break-address)
            if( NonStickyBreakpointCheck(Addr)==TRUE )
            {
                goto EnterDebugger;
            }

            // Check breakpoints that we placed in the debugee code;
            // If found, return a positive breakpoint index that we store
            if( (deb.bpIndex = BreakpointCheck(Addr)) >= 0 )
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
UserInt3:
                // ---- User placed INT 3 -----
                // We need to revert eip to an instruction following the user INT3
                if( deb.bpIndex>=0 )
                    deb.r->eip = Addr.offset;

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

EnterDebugger:

    // This is opposite from down below:
    // If the FLASH is off and the command was P or T, do NOT save screen
    if( !(deb.fFlash==0 && (deb.fTrace || deb.fStep)) )
    {
        // Enable windowing and save background
        pWin->fEnable = TRUE;
        dputc(DP_SAVEBACKGROUND);
    }

    // Set the new CS:EIP for code disasembly
    deb.codeTopAddr.sel = deb.r->cs;
    deb.codeTopAddr.offset = deb.r->eip;

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
    DisarmBreakpoints();
    ClearNonStickyBreakpoint();

    //========================================================================
    // MAIN COMMAND PROMPT LOOP
    //========================================================================
    do
    {
        EdLin( sCmd );

        fContinue = CommandExecute( sCmd+1 );   // Skip the prompt

    } while( fContinue );

    //========================================================================

    // Arm all breakpoints
    ArmBreakpoints();

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

    // What if the current cs:eip is sitting on a breakpoint???
    Addr.sel = deb.r->cs;
    Addr.offset = deb.r->eip;
    bCsEip = AddrGetByte(&Addr);

    if( bCsEip==0xCC )
    {
        // Check all BPX-type breakpoints and a non-sticky bp if the address
        // match to cs:eip, and if so, we'll need to play a hack and set up
        // a state machine to arm all the breakpoint only after a single step
        // is executed

        if( BreakpointCheckSpecial(Addr)==TRUE || NonStickyBreakpointCheck(Addr)==TRUE )
        {
            // Remove all the breakpoints :(
            DisarmBreakpoints();
            
            // Set up the single step
            deb.r->eflags |= TF_MASK;

            // Set up the re-arm flag
            fReArm = TRUE;
        }
        else
            fReArm = FALSE;             // Make sure we keep this flag cleared
    }

    // If the fTrap signal flag is set, set it in the eflags register
    if( deb.fTrace )
        deb.r->eflags |= TF_MASK;

    // Set RESUME flag on the eflags and return to the client
    deb.r->eflags |= RF_MASK;

}

#endif

void DebuggerEnter(void)
{
    BOOL fContinue;
    
    // Abort possible single step trace state
    deb.r->eflags &= ~TF_MASK;

    // Reset the trace state
    deb.fTrace = FALSE;

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
            DisarmBreakpoints();

            {
                // Enable output driver and save background
                dputc(DP_ENABLE_OUTPUT);
                dputc(DP_SAVEBACKGROUND);

                // Set the content variables used in debugging with symbols
                SetCurrentSymbolContext();

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

