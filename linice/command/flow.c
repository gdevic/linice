/******************************************************************************
*                                                                             *
*   Module:     flow.c                                                        *
*                                                                             *
*   Date:       10/16/00                                                      *
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

        Flow control commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/16/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

#include "disassembler.h"               // Include disassembler

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

static char *pSrcEipLine = NULL;        // Cache pointer to source line for repeated src step and trace

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void SetOneTimeBreakpoint(TADDRDESC Addr);

// From linux/reboot.h

extern void machine_restart(char *cmd);
extern void machine_halt(void);
extern void machine_power_off(void);


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdXit(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Returns to the debugee
*
******************************************************************************/
BOOL cmdXit(char *args, int subClass)
{
    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdGo(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   Runs the interrupted program
*
*   G [=start-address] [break-address]
*
*   If you specify start-address, eip is set to it before executing
*   If you spcify break-address, a one-time breakpoint is set
*
******************************************************************************/
BOOL cmdGo(char *args, int subClass)
{
    TADDRDESC StartAddr, BpAddr;        // Actual start and break address
    BOOL fStart, fBreak;                // We have start, break address

    fStart = fBreak = FALSE;

    if( *args )
    {
        // Arguments present. Is it start-address?
        if( *args=='=' )
        {
            args++;                     // Skip the '=' character

            // Set the default selector to current CS
            evalSel = deb.r->cs;

            if( Expression(&StartAddr.offset, args, &args) )
            {
                // Verify that the selector is readable and valid
                if( VerifySelector(evalSel) )
                {
                    // Assign the complete start address
                    StartAddr.sel = evalSel;
                    fStart = TRUE;
                }
                else
                    return( TRUE );
            }
            else
            {
                // Something was bad with the address
                dprinth(1, "Syntax error");
                return( TRUE );
            }
        }

        // See if we have the break address
        if( *args )
        {
            // Set the default selector to current CS
            evalSel = deb.r->cs;

            if( Expression(&BpAddr.offset, args, &args) )
            {
                // Verify that the selector is readable and valid
                if( VerifySelector(evalSel) )
                {
                    // Assign the complete bp address
                    BpAddr.sel = evalSel;
                    fBreak = TRUE;
                }
                else
                    return( TRUE );
            }
            else
            {
                // Something was bad with the address
                dprinth(1, "Syntax error");
                return( TRUE );
            }
        }

        // If we specified start address, modify cs:eip
        if( fStart )
        {
            deb.r->cs = StartAddr.sel;
            deb.r->eip = StartAddr.offset;
        }

        // If we specified break address, set a one-time bp
        if( fBreak )
        {
            SetOneTimeBreakpoint(BpAddr);
        }
    }

    // On the help line, print that the Linux is running...
    dprint("%c%c%c%c%c%c    Linux is running... Press %s + %c to break\r%c",
        DP_SAVEXY,
        DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
        DP_SETCOLINDEX, COL_HELP,
        deb.BreakKey & CHAR_CTRL? "CTRL" : "ALT", deb.BreakKey & 0x7F,
        DP_RESTOREXY);

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL RepeatSrcTrace(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Check if we need to continue execution or break on a source trace command.
*
*   Returns:
*       TRUE - Do repeat trace command
*       FALSE - Break into the debugger
*
******************************************************************************/
BOOL RepeatSrcTrace(void)
{
    // If the new context is in the same function line as the cached originator,
    // continue running, otherwise break.
    // Also break if we left the legal context (pFnLin is NULL in that case)
    if( deb.pFnLin && deb.pSrcEipLine==pSrcEipLine )
    {
        deb.fTrace = TRUE;              // Do another trace step

        return( TRUE );                 // Continue looping
    }

    pSrcEipLine = NULL;                 // Reset the cached variable

    return( FALSE );                    // Exit breaking into the debugger
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTrace(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Trace one instruction or source line, optionally following into subroutines.
*   Optional start address may be specified (prefixed by "="), or
*   a count giving the number of instructions to be traced.
*
*   T [=start_address] [count]
*
*   Keyboard mapped to F8
*
******************************************************************************/
BOOL cmdTrace(char *args, int subClass)
{
    DWORD count;                        // Temporary count store
    DWORD dwAddress;                    // Current scanning address

    if( *args != 0 )
    {
        // Arguments present. Is it start-address?
        if( *args=='=' )
        {
            args++;                     // Skip the '=' character

            // Set the default selector to current CS
            evalSel = deb.r->cs;

            if( Expression(&dwAddress, args, &args) )
            {
                // Verify that the selector is readable and valid
                if( VerifySelector(evalSel) )
                {
                    // Assign new start address to cs:eip
                    deb.r->cs  = evalSel;
                    deb.r->eip = dwAddress;
                }
                else
                    return( TRUE );
            }
            else
            {
                // Something was bad with the address
                dprinth(1, "Syntax error");
                return( TRUE );
            }
        }

        // Another optional parameter is the number of instructions to single step
        if( *args )
        {
            if( Expression(&count, args, &args) && !*args && count )
            {
                deb.nTraceCount = count;
            }
            else
            {
                // Invalid expression - abort trace
                dprinth(1, "Syntax error");
                return( TRUE );
            }
        }
    }

    if( (deb.eSrc==SRC_ON) && deb.pFnScope && deb.pFnLin )
    {
        // SOURCE CODE ACTIVE

        pSrcEipLine = deb.pSrcEipLine;  // Set the pointer cache of the current source line
        deb.fSrcTrace = TRUE;           // We are doing source trace
    }

    // Common case - Use CPU Trace Flag - We are executing a trace command

    deb.fTrace = TRUE;

    return( FALSE );                    // Exit into debugee...
}

/******************************************************************************
*                                                                             *
*   BOOL MultiTrace(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Helper function that is called from the main debugger interrupt handler
*   to query if another loop of a multiple trace iteration should be performed.
*
*   Returns:
*       TRUE - skip debugger block and continue tracing
*       FALSE - break into debugger, no multi-tracing, or it is counted to 0
*
*
*
******************************************************************************/
BOOL MultiTrace(void)
{
    // Assume deb.nTraceCount > 0
    // If the counter is not yet 0 (or yet 0), we will most likely loop
    if( --deb.nTraceCount )
    {
        // Trace again without stopping, but first we need to check if the user has pressed
        // ESC key to stop tracing

        if( GetKey(FALSE)==ESC )
        {
            dprinth(1, "Break. 0x%X trace iterations left.", deb.nTraceCount );

            deb.nTraceCount = 0;     // Multi-trace off
        }
        else
        {
            // Call regular Trace command to correctly set up trace parameters...

            cmdTrace("", NULL);

            return( TRUE );             // Do not break, but execute trace command
        }
    }

    return( FALSE );                    // No need to trace more, reached the final count
}


/******************************************************************************
*                                                                             *
*   BOOL void ArmStep()                                                       *
*                                                                             *
*******************************************************************************
*
*   Arms the step command. This is not as trivial as CPU trace command, since
*   we need to disassemble the current command in order to find out if it is
*   a call or interrupt, which are not traced, but a non-sticky breakpoint is
*   placed immediately after it.
*
*   Return:
*       TRUE - We armed a step command
*       FALSE - We did not arm the step command because we hit return in the "P RET" mode
*
******************************************************************************/
static BOOL ArmStep()
{
    TDISASM Dis;                        // Disassembler interface structure
    TADDRDESC BpAddr;                   // Breakpoint address descriptor

    // Get the size in bytes of the current instruction and its flags
    Dis.bState   = DIS_DATA32 | DIS_ADDRESS32;
    Dis.wSel     = deb.r->cs;
    Dis.dwOffset = deb.r->eip;
    DisassemblerLen(&Dis);

    Dis.bFlags &= SCAN_MASK;            // Mask the scan bits

    if( Dis.bFlags==SCAN_CALL || Dis.bFlags==SCAN_INT )
    {
        // Call and INT instructions we skip

        // Set a non-sticky breakpoint at the next instruction
        BpAddr.sel    = deb.r->cs;
        BpAddr.offset = deb.r->eip + Dis.bInstrLen;
        SetOneTimeBreakpoint(BpAddr);

        deb.fTrace = FALSE;             // This time we are not using CPU trace facility
    }
    else
    {
        // If we are in the "P RET" mode ("step until return"), and the current
        // instruction _is_ a return, clear that mode and quit stepping

        if( deb.fStepRet && Dis.bFlags==SCAN_RET )
        {
            deb.fStepRet = FALSE;           // Dont loop any more

            return( FALSE );                // Signal that we did not arm it
        }
        // All other instructions we single step

        deb.fTrace = TRUE;              // Use CPU Trace Flag
    }

    // Common case - We are executing a step command

    deb.fStep = TRUE;

    return( TRUE );                     // Signal that we armed the step
}


/******************************************************************************
*                                                                             *
*   BOOL RepeatSrcStep(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Check if we need to continue execution or break on a source step command.
*
*   Returns:
*       TRUE - Do repeat step command
*       FALSE - Break into the debugger
*
******************************************************************************/
BOOL RepeatSrcStep(void)
{
    // If the new context is in the same function line as the cached originator,
    // continue running, otherwise break.
    // Also break if we left the legal context (pFnLin is NULL in that case)
    if( deb.pFnLin && deb.pSrcEipLine==pSrcEipLine )
    {
        // Continue looping unless we hit a return while in the "P RET" mode
        return( ArmStep() );
    }

    pSrcEipLine = NULL;                 // Reset the cached variable

    return( FALSE );                    // Exit breaking into the debugger
}

/******************************************************************************
*                                                                             *
*   BOOL cmdStep(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Execute one logical program step. Command P.  This skips over the calls,
*   interrupts, loops and repeated string instructions.
*
*   Keyboard mapped to F10
*
*   Optional parameter [RET] will single step until a RET or IRET instruction
*   is hit.
*
******************************************************************************/
BOOL cmdStep(char *args, int subClass)
{
    if( *args != 0 )
    {
        // Argument must be 'RET' - step until the ret instruction
        if( !stricmp(args, "ret") )
        {
            deb.fStepRet = TRUE;
        }
        else
        {
            dprinth(1, "Syntax error");
            return( TRUE );
        }
    }

    // First, set the current symbol content to make sure our cs:eip is at the
    // right place, since we may have it set differenly
    SetSymbolContext(deb.r->cs, deb.r->eip);

    if( (deb.eSrc==SRC_ON) && deb.pFnScope && deb.pFnLin )
    {
        // SOURCE CODE ACTIVE - initiate repeated step cycles

        pSrcEipLine = deb.pSrcEipLine;  // Set the pointer cache of the current source line
        deb.fSrcStep = TRUE;            // We are doing source step
    }

    // Common case - Use CPU Trace Flag or skip calls and ints

    // Arm the step command by either using a CPU trace flag for single step or
    // non-sticky breakpoint in order to skip over a call or interrupt
    ArmStep();

    return( FALSE );                    // Exit into debugee...
}


/******************************************************************************
*                                                                             *
*   BOOL cmdZap(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Replaces embedded INT1 or INT3 instruction with a NOP
*
******************************************************************************/
BOOL cmdZap(char *args, int subClass)
{
    TADDRDESC Addr;                     // Address to zap
    int b0;

    // Make sure the address is valid
    b0 = GetByte(deb.r->cs, deb.r->eip - 1);
    if( b0==0xCC )
    {
        Addr.sel = deb.r->cs;
        Addr.offset = deb.r->eip - 1;

        // Zap the INT3
        AddrSetByte(&Addr, 0x90, TRUE);
    }
    else
    if( b0==0x01 || b0==0x03 )
    {
        if( GetByte(deb.r->cs, deb.r->eip - 2)==0xCD )
        {
            Addr.sel = deb.r->cs;

            // Zap the 2-byte INT 1 or INT 3
            Addr.offset = deb.r->eip - 1;
            AddrSetByte(&Addr, 0x90, TRUE);

            Addr.offset = deb.r->eip - 2;
            AddrSetByte(&Addr, 0x90, TRUE);
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdI1here(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Pop up on embedded INT1 instruction.
*
******************************************************************************/
BOOL cmdI1here(char *args, int subClass)
{
    switch( GetOnOff(args) )
    {
        case 1:         // On
            deb.fI1Here = TRUE;
            deb.fI1Kernel = FALSE;
        break;

        case 2:         // Off
            deb.fI1Here = FALSE;
            deb.fI1Kernel = FALSE;
        break;

        case 3:         // Display the state of the variable
            dprinth(1, "I1Here is %s", deb.fI1Here? (deb.fI1Kernel? "kernel" : "on") : "off");
        break;

        case 4:         // Only on KERNEL code
            deb.fI1Here = TRUE;
            deb.fI1Kernel = TRUE;
        break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdI3here(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Pop up on embedded INT3 instruction.
*
******************************************************************************/
BOOL cmdI3here(char *args, int subClass)
{
    switch( GetOnOff(args) )
    {
        case 1:         // On
            deb.fI3Here = TRUE;
            deb.fI3Kernel = FALSE;
        break;

        case 2:         // Off
            deb.fI3Here = FALSE;
        break;

        case 3:         // Display the state of the variable
            dprinth(1, "I3Here is %s", deb.fI3Here? (deb.fI3Kernel? "kernel" : "on") : "off");
        break;

        case 4:         // Only on KERNEL code
            deb.fI3Here = TRUE;
            deb.fI3Kernel = TRUE;
        break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdHboot(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Total computer reset
*
******************************************************************************/
BOOL cmdHboot(char *args, int subClass)
{
    // Linux: process.c

    machine_restart(NULL);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdHalt(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Powers computer off
*
******************************************************************************/
BOOL cmdHalt(char *args, int subClass)
{
    // Use APM power off:
    machine_power_off();

    // If that fails, we hboot-it
    machine_restart(NULL);

    return( TRUE );
}

