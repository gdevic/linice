/*****************************************************************************
*                                                                             *
*   Module:     breakpoints.c                                                 *
*                                                                             *
*   Date:       06/26/01                                                      *
*                                                                             *
*   Copyright (c) 2001 - 2001 Goran Devic                                     *
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

        This module contains code for managing breakpoints and associated
        commands.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 06/26/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "intel.h"
#include "ice.h"                        // Include main debugger structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern char *pCmdEdit;                  // Push edit line string

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// Define a structure that holds a breakpoint info

#define BP_USED         0x01            // Breakpoint is used
#define BP_ENABLED      0x02            // Breakpoint is enabled (active)

#define BP_TYPE_BPX     0x01            // Breakpoint is BPX   (subClass 1)
#define BP_TYPE_BPINT   0x02            // Breakpoint is BPINT (subClass 2)
#define BP_TYPE_BPIO    0x03            // Breakpoint is BPIO  (subClass 3)
#define BP_TYPE_BPMB    0x04            // Breakpoint is BPMB  (subClass 4)
#define BP_TYPE_BPMW    0x05            // Breakpoint is BPMW  (subClass 5)
#define BP_TYPE_BPMD    0x07            // Breakpoint is BPMD  (subClass 7)

typedef struct
{
    BYTE Flags;                         // Breakpoint flags
    BYTE Type;                          // Breakpoint type
    BYTE Size;                          // Size of the memory access (B, W, D)
    BYTE Access;                        // Access type (R/W/RW/X)
    char *pCmd;                         // Pointer to a full bp command line
    char *pIF;                          // Pointer to an optional IF expression
    char *pDO;                          // Pointer to an optional DO "statements"

    TADDRDESC address;                  // Breakpoint address (sel:offset)
    BYTE origValue;                     // Original value that was there before INT3
    DWORD dwLinear;                     // Linear address to store into debug register

    // Breakpoint statistics
    DWORD Hits;                         // Number of times we evaluated a bp
    DWORD Breaks;                       // Number of times a bp has evaluated to TRUE
    DWORD Popups;                       // Number of times it caused Ice to popup
    DWORD Misses;                       // Number of times a bp has evaluated to FALSE
    DWORD Errors;                       // Number of times evaluation resulted in error
    DWORD CurHits;                      // Current hits
    DWORD CurMisses;                    // Current misses

} TBP;

// Array of breakpoint structures

static TBP bp[MAX_BREAKPOINTS];

static int hint = -1;                   // No hints to edit breakpoint number

static char Buf[MAX_STRING];            // Temp line buffer to send line to edit
static char *pBuf;

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void InitBreakpoints()
{
    memset(bp, 0, sizeof(bp));
}

/******************************************************************************
*                                                                             *
*   F R O N T   E N D   F U N C T I O N S                                     *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL cmdBp(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   Breakpoint commands: BC, BD and BE
*       BC - clear breakpoint(s)
*       BD - disable breakpoint(s)
*       BE - enable breakpoint(s)
*   Sysntax: list | *
*
******************************************************************************/
BOOL cmdBp(char *args, int subClass)
{
    int nLine = 1;                      // Standard line counter
    BYTE select[MAX_BREAKPOINTS];       // Selected breakpoints
    int index;

    if( *args=='*' )
    {
        // Argument * specifies every (all) breakpoints
        memset(select, 1, sizeof(select));
    }
    else
    {
        // Clear the select array to not selected
        memset(select, 0, sizeof(select));

        // The arguments are a list of breakpoint indices

        while( Expression(&index, args, &args) )
        {
            if( index>=0 && index<MAX_BREAKPOINTS )
            {
                select[index] = 1;      // Make it selected
            }
            else
            {
                dprinth(nLine++, "Invalid breakpoint number %d", index);

                return(TRUE);
            }
        }
    }

    // Loop over the selected breakpoints and perform operation
    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        if( select[index] )
        {
            switch( subClass )
            {
                case 0:                 // subClass = 0, BC, clear a breakpoint
                        bp[index].Flags = 0;

                        // Free the command line of a breakpoint and IF/DO statements
                        _kFree(pIce->hHeap, bp[index].pCmd);

                        // Clear the breakpoint entry
                        memset(&bp[index], 0, sizeof(TBP));
                    break;

                case 1:                 // subClass = 1, BD, disable a breakpoint
                        bp[index].Flags &= ~BP_ENABLED;
                    break;

                case 2:                 // subClass = 2, BE, enable a breakpoint
                        bp[index].Flags |= BP_ENABLED;
                    break;
            }
        }
    }

    return(TRUE);
}


static char *GetBpString(TBP *pBp)
{
    // BPIO types (stored in the Access field)
    static const char *sBpio[] = { "", "R", "W", "RW" };

    // BPM sizes (encoded in the type)
    static const char *sBpm[] = { "B", "W", "", "D" };

    // Reset the string buffer pointer
    pBuf = Buf;

    // Check the breakpoint type flag and print accordingly
    switch( pBp->Type )
    {
        case BP_TYPE_BPX:
            pBuf += sprintf(pBuf, "BPX %04X:%08X", pBp->address.sel, pBp->address.offset );
            break;

        case BP_TYPE_BPINT:
            pBuf += sprintf(pBuf, "BPINT %X", pBp->address.offset );
            break;

        case BP_TYPE_BPIO:
            pBuf += sprintf(pBuf, "BPIO %X %s", pBp->address.offset, sBpio[pBp->Access] );
            break;

        case BP_TYPE_BPMB:
        case BP_TYPE_BPMW:
        case BP_TYPE_BPMD:
            pBuf += sprintf(pBuf, "BPM%c %04X:%08X %s",
                sBpm[pBp->Type - BP_TYPE_BPMB],
                pBp->address.sel, pBp->address.offset,
                sBpio[pBp->Access] );
            break;
    }

    // Finish with the optional IF and DO statements
    if( pBp->pIF )
    {
        pBuf += sprintf(pBuf, " IF %s", pBp->pIF);
        if( pBp->pDO )
        {
            pBuf += sprintf(pBuf, " DO %s", pBp->pDO);
        }
    }

    return(Buf);
}

/******************************************************************************
*                                                                             *
*   BOOL cmdBl(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   List all breakpoints
*
******************************************************************************/
BOOL cmdBl(char *args, int subClass)
{
    int nLine = 1;                      // Standard line counter
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Only list used breakpoint slots, of course
        if( bp[index].Flags & BP_USED )
        {
            // print the string describing a breakpoint using a different color
            // if the breakpoint is indexed as current
            if(dprinth(nLine++, "%c%c%02X) %c %s",
                DP_SETCOLINDEX, (index==deb.bpIndex)? COL_BOLD : COL_NORMAL,
                index,
                (bp[index].Flags & BP_ENABLED)? ' ':'*',
                GetBpString(&bp[index]) )==FALSE)
                break;
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdBpet(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Edit a breakpoint description, also do breakpoint template since it is
*   the same function, but it creates a new breakpoint
*
******************************************************************************/
BOOL cmdBpet(char *args, int subClass)
{
    int index;

    if( *args )
    {
        if( Expression(&index, args, &args))
        {
            if( index>=0 && index<MAX_BREAKPOINTS )
            {
                if( bp[index].Flags & BP_USED )
                {
                    // If the command was BPT (template), we do the same as with
                    // edit, but do not hint to overwrite a template breakpoint

                    if( subClass==0 )
                    {
                        // Arrange for this breakpoint to be edited *and* stored
                        // at the same index. We use variable 'hint' to hint next
                        // breakpoint creating instruction to use that index
                        hint = index;
                    }

                    // Prepare the command line to be edited

                    pCmdEdit = GetBpString(&bp[index]);

                    return(TRUE);
                }
            }

            // From now on it is an invalid breakpoint
            dprinth(1, "Invalid breakpoint number %d", index);

            return(TRUE);
        }
    }

    // Everything else is a syntax error
    dprinth(1, "Syntax error");

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdBstat(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display statistics for one or all breakpoints
*
******************************************************************************/
BOOL cmdBstat(char *args, int subClass)
{
    int nLine = 1;                      // Standard line counter
    int index;

    // Display statistic of a single (used) breakpoint
    // Return: FALSE if the printing should stop
    BOOL BPstat(int index)
    {
        TBP *p = &bp[index];            // Get the pointer to a current bp

        if(dprinth(nLine++, "Breakpoint Statistics for #%02X %s", index, (p->Flags & BP_ENABLED)?"" : "(disabled)")==FALSE) return(FALSE);
        if(dprinth(nLine++, "   %s", GetBpString(p) )==FALSE) return(FALSE);
#if 1
        if(dprinth(nLine++, "   Cond    %s", p->pIF? p->pIF : "No" )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Action  %s", p->pDO? p->pDO : "No" )==FALSE) return(FALSE);
#endif
        if(dprinth(nLine++, "Totals")==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Hits    %X", p->Hits )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Breaks  %X", p->Breaks )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Popups  %X", p->Popups )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Misses  %X", p->Misses )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Errors  %X", p->Errors )==FALSE) return(FALSE);
        if(dprinth(nLine++, "Current")==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Hits    %X", p->CurHits )==FALSE) return(FALSE);
        if(dprinth(nLine++, "   Misses  %X", p->CurMisses )==FALSE) return(FALSE);

        return(TRUE);
    }

    if( *args )
    {
        // A specific breakpoint
        if( Expression(&index, args, &args))
        {
            if( index>=0 && index<MAX_BREAKPOINTS && (bp[index].Flags & BP_USED) )
            {
                // Display individual statistics
                BPstat(index);

                return(TRUE);
            }
            else
                dprinth(1, "Invalid breakpoint number %d", index);
        }
        else
            dprinth(1, "Syntax error");
    }
    else
    {
        // Display statistics for every defined breakpoint
        for(index=0; index<MAX_BREAKPOINTS; index++)
        {
            if( bp[index].Flags & BP_USED )
            {
                if( BPstat(index) )
                    break;
            }
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdBpx(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Set a breakpoint:
*       BPX [address] [IF expression] [DO "command1; command2;..."]
*       BPINT int-number [IF expression] [DO "command1; command2;..."]
*       BPIO  port [access] [IF expression] [DO "command1; command2;..."]
*       BPM[B|W|D] address [access] [IF expression] [DO "command1; command2;..."]
*
*   BPX   = subClass 1
*   BPINT = subClass 2
*   BPIO  = subClass 3
*   BPMB  = subClass 4
*   BPMW  = subClass 5
*   BPMD  = subClass 7
*
******************************************************************************/
BOOL cmdBpx(char *args, int subClass)
{
    TBP *pBp;                           // Pointer to current breakpoint
    int index, dummy;

    // Find the first empty slot that we will use. If the variable 'hint' was
    // set, we override and use that particular index. That is done with the
    // BPE edit breakpoint where we want to store into the same index
    if( hint >= 0 )
    {
        index = hint;                   // Use that breakpoint
        hint = -1;                      // Reset the hint
    }
    else
    {
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( !(bp[index].Flags & BP_USED) )
                break;
        }
    }

    if( index<MAX_BREAKPOINTS )
    {
        pBp = &bp[index];               // Assign the pointer to current bp

        if( *args )
        {
            // Allocate space to copy the command line string
            if( (pBp->pCmd = _kMalloc(pIce->hHeap, strlen(args)+1)) )
            {
                // Copy the command line on the memory heap
                strcpy(pBp->pCmd, args);

                // Start parsing the copy of the command line
                args = pBp->pCmd;

                // Get the mandatory address portion: selector portion is CS by default
                evalSel = deb.r->cs;

                // Evaluate expression for the address portion. If the command was
                // BPINT, we keep the interrupt numeber in the address.offset field
                // BPIO,  we keep the IO port in the address.offset field
                if( Expression(&pBp->address.offset, args, &args))
                {
                    pBp->address.sel = evalSel;

                    // With BPIO and BPM, we can specify an optional access verb here
                    if( subClass==BP_TYPE_BPIO || subClass==BP_TYPE_BPMB
                     || subClass==BP_TYPE_BPMW || subClass==BP_TYPE_BPMD )
                    {
                        if( !strnicmp(args, "RW", 2) )
                        {
                            args += 2;
                            pBp->Access = 3;
                        }
                        else
                        if( !strnicmp(args, "R", 1) )
                        {
                            args += 1;
                            pBp->Access = 1;
                        }
                        else
                        if( !strnicmp(args, "W", 1) )
                        {
                            args += 1;
                            pBp->Access = 2;
                        }
                        else
                            pBp->Access = 3;       // default is "RW"
                    }

                    // If the next statement is "IF", accept it
                    if( *args )
                    {
                        dprinth(1, ">%s<", args);
                        if( !strnicmp(args, "IF", 2) )
                        {
                            // Store a pointer to the IF expression statement
                            args += 2;
                            pBp->pIF = args;

                            // Dummy evalute the expression to skip it
                            Expression(&dummy, args, &args);

                            // If the next statement is "DO", accept it
                            dprinth(1, ">%s<", args);
                            if( *args )
                            {
                                // "DO" will sometimes evaluate as 0xD + 'O'
                                // so step back a character
                                if( !strnicmp(args-1, "DO", 2) )
                                    args--;

                                if( !strnicmp(args, "DO", 2) )
                                {
                                    // Store a pointer to DO statements
                                    args += 2;
                                    pBp->pDO = args;

                                    // Terminate "IF" expression
                                    *(pBp->pDO-2) = 0;
                                }
                                else
                                    goto SyntaxError;
                            }
                        }
                        else
                            goto SyntaxError;
                    }

                    // Finalize the breakpoint as valid and active and activate type
                    pBp->Flags |= BP_USED | BP_ENABLED;
                    pBp->Type = subClass;

                    return(TRUE);
                }
SyntaxError:
                // Free the buffer since setting a bp failed
                _kFree(pIce->hHeap, pBp->pCmd);

                // Clear the breakpoint entry
                memset(pBp, 0, sizeof(TBP));
            }
            else
                dprinth(1, "Should never happen");
        }

        dprinth(1, "Syntax error");
    }
    else
        dprinth(1, "Too many breakpoints");

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   B A C K   E N D   F U N C T I O N S                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void BreakpointsPlace(void)                                               *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugee gets control under linice, so
*   we can trap on any breakpoint that was enabled.
*
******************************************************************************/
void BreakpointsPlace(void)
{
    TADDRDESC Addr;                     // Address to set a breakpoint at
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Place only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            switch( bp[index].Type )
            {
                case BP_TYPE_BPX:
                    Addr = bp[index].address;       // Store the original value
                    bp[index].origValue = AddrGetByte(&Addr);
                    AddrSetByte(&Addr, 0xCC);       // Place INT3
                    break;

                default:
                    dprinth(1, "BP TYPE %d not supported", bp[index].Type);
                    break;
            }
        }
    }
}

/******************************************************************************
*                                                                             *
*   void BreakpointsRemove(void)                                              *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugger gets control to remove all
*   breakpoints that it had placed, so they dont obstruct the view.
*
******************************************************************************/
void BreakpointsRemove(void)
{
    TADDRDESC Addr;                     // Address to reset breakpoint
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Restore only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            switch( bp[index].Type )
            {
                case BP_TYPE_BPX:
                    // Restore original value only if we find INT3 there
                    Addr = bp[index].address;
                    if( AddrGetByte(&Addr)==0xCC )  // Was it INT3?
                    {
                        // Restore original value
                        AddrSetByte(&Addr, bp[index].origValue);
                    }
                    break;

                default:
                    dprinth(1, "BP TYPE %d not supported", bp[index].Type);
                    break;
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   int BreakpointCheck(WORD sel, DWORD offset)                               *
*                                                                             *
*******************************************************************************
*
*   This function is called only from the debugger entry point.
*   Checks if any breakpoint address matches given sel:offset
*   If so, marks it as current, increments statistic counters and returns TRUE.
*
*   Returns:
*       -1 if no breakpoint addresses match
*       bp index if match is found
*
******************************************************************************/
BOOL BreakpointCheck(WORD sel, DWORD offset)
{
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Check only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            if( bp[index].address.sel==sel && bp[index].address.offset==offset )
            {
                // Found a matching address!

                bp[index].Hits++;       // Total number of times this was hit

                return(index);
            }
        }
    }

    return(-1);
}


/******************************************************************************
*                                                                             *
*   BOOL BreakpointCondition(int index)                                       *
*                                                                             *
*******************************************************************************
*
*   This function is called only from the debugger entry point.
*   Checks if an optional breakpoint condition evaluates to true.
*
*   Returns:
*       TRUE - condition exists and evaluates to non-zero
*       FALSE - condition either does not exist, or evaluates to zero
*
******************************************************************************/
BOOL BreakpointCondition(int index)
{
    DWORD value;

    if( bp[index].pIF )
    {
        // Breakpoint condition exists, we need to evaluate it
        if( Expression(&value, bp[index].pIF, NULL) )
        {
            if( value )
            {
                bp[index].Breaks++;     // Number of times we break due to it

                return(TRUE);
            }
        }
        else
        {
            // Expressio is invalid

            bp[index].Errors++;         // Evaluation resulted in error
        }
    }

    bp[index].Misses++;                 // Number of times we dont break from it

    return(FALSE);
}


/******************************************************************************
*                                                                             *
*   int IsDebugRegAvail(int which)                                            *
*                                                                             *
*******************************************************************************
*
*   Checks if a specified DR is available for use. If which is (-1), any debug
*   reg is checked.
*
*   Where:
*       which (0,1,2,3) checks a particular debug reg
*       which <0 checks for any debug reg
*
*   Returns:
*       DR # that is available
*       -1 if no DR is available or a particular DR is not available
*
******************************************************************************/
int IsDebugRegAvail(int which)
{
    if( which>=0 && which<=3 )
    {
        // Check a particular debug register: DR7 LE/GE bits
        if( (deb.sysReg.dr7 & (3 << (which*2)))==0 )
        {
            // Debug register is disabled, therefore free
            return(which);
        }
    }
    else
    {
        // Search all 4 debug registers from last to first for the free one
        for(which=3; which>=0; which-- )
        {
            if( (deb.sysReg.dr7 & (3 << (which*2)))==0 )
            {
                // Debug register is disabled, therefore free
                return(which);
            }
        }
    }

    // No available debug registers
    return(-1);
}


/******************************************************************************
*                                                                             *
*   int ArmDebugReg(int which, TADDRDESC Addr, BYTE type)                     *
*                                                                             *
*******************************************************************************
*
*   Arms a specified debug register.
*
*   Where:
*       which (0,1,2,3) checks a particular debug reg
*       Addr is the sel:offset of the address to arm
*       type is one of the DR7_* types (eg. DR7_EXEC,...)
*
*   Returns:
*       0
*
******************************************************************************/
int ArmDebugReg(int which, TADDRDESC Addr, BYTE type)
{
    DWORD Linear;

    // Calculate the linear address of the breakpoint
    // TODO: We will again assume our selector is zero-based
    Linear = Addr.offset;

    deb.sysReg.dr[which] = Linear;

    // Set the type of breakpoint
    deb.sysReg.dr7 |= type << (which*4 + 16);

    // Arm the debug register with both LE and GE bits
    deb.sysReg.dr7 |= 3 << (which*2);

    return(0);
}


/******************************************************************************
*                                                                             *
*   void ArmSingleShotInt3(TADDRDESC Addr)                                    *
*                                                                             *
*******************************************************************************
*
*   Arms a single shot INT3 breakpoint
*
*   Where:
*       Addr is the sel:offset of the address to arm
*
******************************************************************************/
void ArmSingleShotInt3(TADDRDESC Addr)
{
    ;
}

