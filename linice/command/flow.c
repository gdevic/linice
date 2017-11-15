/******************************************************************************
*                                                                             *
*   Module:     flow.c                                                        *
*                                                                             *
*   Date:       10/16/00                                                      *
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

extern DWORD StackExtraBuffer;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char *pSrcEipLine = NULL;        // Cache pointer to source line for repeated src step and trace

#define MAGIC_CALL_SIG      0x55AA00CC  // Signature dword for the CALL frame
#define MAGIC_CALL_MASK     0xFFFF00FF  // Valid bits in the magic word

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
    int b0;                             // Temp store current byte so we dont fetch it twice

    // Make sure the address is valid and that we want to zap a real INT3
    Addr.sel = deb.r->cs;
    Addr.offset = deb.r->eip - 1;

    if( (b0 = AddrGetByte(&Addr))==0xCC )
    {
        // Zap the INT3
        AddrSetByte(&Addr, 0x90, TRUE);
    }
    else
    if( b0==0x01 || b0==0x03 )          // "INT 1" or "INT 3" two-byte opcodes
    {
        Addr.offset = deb.r->eip - 2;

        if( AddrGetByte(&Addr)==0xCD )
        {
            // Zap the 2-byte INT 1 or INT 3
            // Addr.offset = deb.r->eip - 2;    // We already have this offset in .offset
            AddrSetByte(&Addr, 0x90, TRUE);     // zap with NOP

            Addr.offset = deb.r->eip - 1;       // 2-byte opcode
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


/******************************************************************************
*                                                                             *
*   void UserStackPush(DWORD value)                                           *
*                                                                             *
*******************************************************************************
*
*   Pushes a DWORD value onto the user stack. If the current stack is ring-0,
*   moves our register structure as well.
*
******************************************************************************/
void UserStackPush(DWORD value)
{
    TADDRDESC TOS;                      // Top of Stack address

    // If the SS is ring-0, we need to move register structure to make space
    if( (deb.r->ss & 3)==0 )
    {
        // Move the register structure down to free up one DWORD of stack space (overlapping)
        memmove((void *)(deb.r->esp-4-sizeof(TREGS)), (void *)(deb.r->esp-sizeof(TREGS)), sizeof(TREGS));

        // Adjust the new register structure pointer
        StackExtraBuffer -= 4;

        deb.r = (TREGS *)((DWORD)deb.r - 4);
    }

    deb.r->esp -= 4;                    // Decrement the stack pointer

    TOS.sel = deb.r->ss;
    TOS.offset = deb.r->esp;

    AddrSetDword(&TOS, value);          // Push the value
}

/******************************************************************************
*                                                                             *
*   DWORD UserStackPop(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Pops a DWORD from the user stack. If the current stack is ring-0, moves
*   our register structure as well.
*
******************************************************************************/
DWORD UserStackPop(void)
{
    TADDRDESC TOS;                      // Top of Stack address
    DWORD value;                        // Value that we pop

    TOS.sel = deb.r->ss;
    TOS.offset = deb.r->esp;

    value = AddrGetDword(&TOS);         // Pop the value

    // If the SS is ring-0, we need to move register structure back
    if( (deb.r->ss & 3)==0 )
    {
        // Move the register structure up to reclaim one DWORD of stack space (overlapping)
        memmove((void *)(deb.r->esp+4-sizeof(TREGS)), (void *)(deb.r->esp-sizeof(TREGS)), sizeof(TREGS));

        // Adjust the new register structure pointer
        StackExtraBuffer += 4;

        deb.r = (TREGS *)((DWORD)deb.r + 4);
    }

    deb.r->esp += 4;                    // Increment the stack pointer

    return( value );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdCall(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Executes a function call. This is an added function to Linice only.
*
*   Where:
*       Symtax of the function is:
*       CALL <address> ( {[arg],[arg]} )
*
******************************************************************************/
BOOL cmdCall(char *args, int subClass)
{
    char buf[MAX_STRING], *pBuf = buf;  // Buffer to print final command
    DWORD Args[MAX_CALL_ARGS];          // Array to hold function arguments
    DWORD Addr, RetPtr;                 // Function address
    UINT i, nArg = 0;                   // Index of the argument
    char *pOpen, *pClose;               // Temp pointer to parenthesis

    // For this function to work, we do need arguments
    if( *args )
    {
        // It is rather peculiar syntax where the opening bracket would be
        // eaten by our expression evaluator, so we zap it
        pOpen = strchr(args, '(');
        pClose = strrchr(args, ')');

        // Also zap the trailing bracket, which has to be specified
        if( pOpen && pClose )
        {
            // Zap the set of brackets, that also means we cannot have any
            // expression involving the parenthesis to calculate the address part
            *pOpen = 0;
            *pClose = 0;

            if( Expression(&Addr, args, &args) )
            {
                // Reading the list of arguments now
                args = pOpen;

                do
                {
                    while( *++args==' ' );

                    // If we are done reading, do a call
                    if( !*args )
                    {
                        // Print the final command so the user knows what is being executed
                        pBuf += sprintf(buf, "CALL %08X ( ", Addr);

                        for(i=0; i<nArg; i++)
                            pBuf += sprintf(pBuf, "%X%c", Args[i], (i+1)==nArg?' ':',');

                        dprinth(1, "%s)", buf);

                        // Issue the actual call by setting the debugee's stack frame
                        // and 'returning' to that address

                        // Set up the stack frame to look like this:
                        //    [  ???  ]       previous ESP
                        //    Saved EIP
                        //    MAGIC_CALL_SIG     <--+   contains CC byte (INT3) + number of arguments [15:8]
                        //    [ args ]              |
                        //    ret ------------------+

                        UserStackPush(deb.r->eip);
                        UserStackPush(MAGIC_CALL_SIG | (nArg << 8));

                        RetPtr = deb.r->esp;            // This is where we set the return address to

                        // Build up the stack frame from right to left ("C" convention)
                        for(i=nArg; i; i--)
                            UserStackPush(Args[i-1]);

                        // Push the address to return to
                        UserStackPush(RetPtr);

                        deb.r->eip = Addr;      // Set the new EIP and continue into it

                        return( FALSE );
                    }

                    if( Expression(&Args[nArg], args, &args) )
                    {
                        // We got a function argument, carefully increment the index
                        if( nArg++ == MAX_CALL_ARGS )
                            break;

                        // If there is a next character, it should be argument separator
                        if( *args && *args!=',' )
                            break;
                    }
                    else
                        break;
                }
                while( TRUE );
            }
        }
    }   // Any path to out of this function here is a syntax error

    PostError(ERR_SYNTAX, 0);

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   void FixupUserCallFrame(void)                                             *
*                                                                             *
*******************************************************************************
*
*   Called from the interrupt routine to detect the case where we are returning
*   from the call frame. It fixes it up by popping the parameters that we
*   pushed and displays the result of a call (EAX register)
*
******************************************************************************/
void FixupUserCallFrame(void)
{
    TADDRDESC Addr;
    UINT i;

    Addr.sel = deb.r->cs;
    Addr.offset = deb.r->eip - 1;       // After INT3 (CC), EIP points to the _next_ instruction...

    // Get the DWORD from the current CS:EIP and compare it to our magic signature
    i = AddrGetDword(&Addr);

    if( (i & MAGIC_CALL_MASK)==MAGIC_CALL_SIG )
    {
        // We need to clean up the call stack frame that we created in the cmdCall()
        // Get the number of dwords to pop them ("i" is already fetched)
        i = (i >> 8) & 0xFF;
        i++;                            // Add one for the MAGIC_CALL_SIG parameter
        while( i-- )                    // Pop so many arguments
            UserStackPop();

        // We do this assignment on 2 separate lines because gcc was optimizing it
        // into an incorrect access (it did not know deb.r moved). I really dont
        // want to declare it volatile at this point.
        i = UserStackPop();             // Get the previous EIP from the user stack
        deb.r->eip = i;

        dprinth(1, "Return from CALL = %08X", deb.r->eax);
    }
}
