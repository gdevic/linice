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

// Flags:

#define BP_USED         0x01            // Breakpoint is used (not empty slot)
#define BP_ENABLED      0x02            // Breakpoint is enabled (active)

// Types (types and subclasses are the same number):

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
    BYTE DrHint;                        // Debug register hint bitfield (user spec)
    BYTE DrUse;                         // Debug register actual use bitfield
    char *pCmd;                         // Pointer to a full bp command line
    char *pIF;                          // Pointer to an optional IF expression
    char *pDO;                          // Pointer to an optional DO "statements"

    TADDRDESC address;                  // Breakpoint address (sel:offset)
    WORD file_id;                       // Source code file_id
    WORD line;                          // Source code line number
    BYTE origValue;                     // Original value that was there before INT3
//  DWORD dwLinear;                     // Virtual linear address of a breakpoint

    // Breakpoint statistics
    DWORD Hits;                         // Number of times we evaluated a bp
    DWORD Breaks;                       // Number of times a bp has evaluated to TRUE
    DWORD Popups;                       // Number of times it caused Ice to popup
    DWORD Misses;                       // Number of times a bp has evaluated to FALSE
    DWORD Errors;                       // Number of times evaluation resulted in error
    DWORD CurHits;                      // Current hits
    DWORD CurMisses;                    // Current misses

} TBP;

/*
    BPX   - address.sel, address.offset contain user specified address to break
            file_id, line, if non-zero, index the source code line of that bpx

    BPINT - address.offset is the interrupt number (0x00 - 0xFF)

    BPIO  - address.offset is the IO port to trap (0x00 - 0xFFFF)
            Access: 3 (RW), 2 (R), 1 (W)
            DrHint register use: 1 (DR0), 2 (DR1), 4 (DR2), 8 (DR3)

    BPM?  - address.sel, address.offset contains the memory address to break
            Access: 3 (RW), 2 (R), 1 (W)
            Size: 0 (B), 1 (W), 3 (D)
            DrHint register use: 1 (DR0), 2 (DR1), 4 (DR2), 8 (DR3)

*/

static TBP bp[MAX_BREAKPOINTS];         // Array of breakpoint structures

static TBP nsbp;                        // Single non-sticky breakpoint

static int hint = -1;                   // No hints to edit breakpoint number

static char Buf[MAX_STRING];            // Temp line buffer to send line to edit
static char *pBuf;

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern DWORD GetHex(char **psString);
extern void SetSysreg( TSysreg * pSys );


DWORD fnBpCount(DWORD arg) { return( deb.bpIndex>=0? bp[deb.bpIndex].CurHits : 0 ); };
DWORD fnBpMiss(DWORD arg) { return( deb.bpIndex>=0? bp[deb.bpIndex].CurMisses : 0 ); };
DWORD fnBpTotal(DWORD arg) { return( deb.bpIndex>=0? bp[deb.bpIndex].Hits : 0 ); };
DWORD fnBpIndex(DWORD arg) { return( deb.bpIndex ); };
DWORD fnBpLog(DWORD arg)
{
    // Log the breakpoint into the history buffer
    dprinth(1, "BPLOG Not implemented yet");
    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL AssignDebugReg(TBP *pBp);
BOOL VerifyBreakpoint(TBP *pBp);
BOOL BreakpointCheckSpecial(void);

void InitBreakpoints()
{
    memset(bp, 0, sizeof(bp));
    memset(&nsbp, 0, sizeof(nsbp));
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
                deb.error = ERR_BPNUM;

                return(TRUE);
            }
        }
    }

    // Loop over selected breakpoints and perform operation
    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        if( select[index] )
        {
            switch( subClass )
            {
                case 0:                 // subClass = 0, BC, clear a breakpoint
                        // Free the command line of a breakpoint and IF/DO statements
                        _kFree(pIce->hHeap, bp[index].pCmd);

                        // Clear the breakpoint entry
                        memset(&bp[index], 0, sizeof(TBP));
                    break;

                case 1:                 // subClass = 1, BD, disable a breakpoint
                        bp[index].Flags &= ~BP_ENABLED;
                    break;

                case 2:                 // subClass = 2, BE, enable a breakpoint
                    // Before we enable a breakpoint, we need to verify if it is still a valid
                    // That includes use of debug register and possible new duplicate breakpoints

                    // For breakpoints that use debug registers, reassign the register number
                    if( bp[index].Type==BP_TYPE_BPIO || bp[index].Type==BP_TYPE_BPMB || bp[index].Type==BP_TYPE_BPMW || bp[index].Type==BP_TYPE_BPMD )
                    {
                        if( bp[index].DrHint )
                            bp[index].DrUse = bp[index].DrHint;
                        else
                        if( AssignDebugReg(&bp[index])==FALSE )
                        {
                            // Failed to assign a debug register, which is error
                            deb.error = ERR_DRUSEDUP;           // All debug registers used
                            return( TRUE );
                        }
                    }

                    if( VerifyBreakpoint(&bp[index])==FALSE )
                        return( TRUE );         // Breakpoint failed in some way to be enabled. Return.

                    // All tests passed - enable this breakpoint
                    bp[index].Flags |= BP_ENABLED;

                    break;
            }

            // We made a change in the state of a breakpoint. Redraw the display to reflect that.
            deb.fRedraw = TRUE;
        }
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   static char *GetBpString(TBP *pBp)                                        *
*                                                                             *
*******************************************************************************
*
*   Forms a string with the given breakpoint description for listing it.
*
******************************************************************************/
static char *GetBpString(TBP *pBp)
{
    // BPIO types (stored in the Access field)
    static const char *sBpio[] = { "", "R", "W", "RW" };

    // BPM sizes (encoded in the type)
    static const char *sBpm[] = { "B", "W", "", "D" };

    // DR register name from a bitfield
    static const char *sDr[] =  { "", "DR0", "DR1", "", "DR2", "", "", "", "DR3" };

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
            pBuf += sprintf(pBuf, "BPIO %X %s %s", pBp->address.offset, sDr[pBp->DrUse], sBpio[pBp->Access] );
            break;

        case BP_TYPE_BPMB:
        case BP_TYPE_BPMW:
        case BP_TYPE_BPMD:
            pBuf += sprintf(pBuf, "BPM%s %04X:%08X %s %s",
                sBpm[pBp->Type - BP_TYPE_BPMB],
                pBp->address.sel, pBp->address.offset,
                sDr[pBp->DrUse],
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
            deb.error = ERR_BPNUM;

            return(TRUE);
        }
    }

    // Everything else is a syntax error
    deb.error = ERR_SYNTAX;

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL BPstat(int index, int *pLine)                                        *
*                                                                             *
*******************************************************************************
*
*   Display statistic of a single (used) breakpoint
*   Return: FALSE if the printing should stop
*
******************************************************************************/
BOOL BPstat(int index, int *pLine)
{
    TBP *p = &bp[index];            // Get the pointer to a current bp

    if(dprinth(*pLine++, "Breakpoint Statistics for #%02X %s", index, (p->Flags & BP_ENABLED)?"" : "(disabled)")==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   %s", GetBpString(p) )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Cond    %s", p->pIF? p->pIF : "No" )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Action  %s", p->pDO? p->pDO : "No" )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "Totals")==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Hits    %X", p->Hits )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Breaks  %X", p->Breaks )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Popups  %X", p->Popups )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Misses  %X", p->Misses )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Errors  %X", p->Errors )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "Current")==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Hits    %X", p->CurHits )==FALSE) return(FALSE);
    if(dprinth(*pLine++, "   Misses  %X", p->CurMisses )==FALSE) return(FALSE);

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

    if( *args )
    {
        // A specific breakpoint
        if( Expression(&index, args, &args))
        {
            if( index>=0 && index<MAX_BREAKPOINTS && (bp[index].Flags & BP_USED) )
            {
                // Display individual statistics
                BPstat(index, &nLine);
            }
            else
                dprinth(1, "Invalid breakpoint number %d", index);
        }
        else
            deb.error = ERR_SYNTAX;         // Syntax error at this point
    }
    else
    {
        // Display statistics for every defined breakpoint
        for(index=0; index<MAX_BREAKPOINTS; index++)
        {
            if( bp[index].Flags & BP_USED )
            {
                if( BPstat(index, &nLine)==FALSE )
                    break;
            }
        }
    }

    return(TRUE);
}

/******************************************************************************
*                                                                             *
*   BOOL VerifyBreakpoint(TBP *pBp)                                           *
*                                                                             *
*******************************************************************************
*
*   When we define a new breakpoint and when we enable an old breakpoint, we
*   need to check that it is valid with regards to other breakpoints already
*   defined. That is, no duplicate breakpoint and some parameter validity.
*
*   Where:
*       pBp is the breakpoint that is to be checked against the rest
*
******************************************************************************/
BOOL VerifyBreakpoint(TBP *pBp)
{
    int index;

    switch( pBp->Type )
    {
        case BP_TYPE_BPX:
            // 1. No duplicate breakpoint with the same address
            for(index=0; index<MAX_BREAKPOINTS; index++)
            {
                if( bp[index].Flags & BP_ENABLED && bp[index].Type==BP_TYPE_BPX
                && bp[index].address.offset==pBp->address.offset && bp[index].address.sel==pBp->address.sel)
                {
                    deb.error = ERR_BPDUP;      // Duplicate breakpoint

                    return( FALSE );
                }
            }
            break;

        case BP_TYPE_BPINT:
            // 1. Interrupt number has to be in the range 0x00 - 0xFF
            // 2. No duplicate breakpoints with the same interrupt number
            if( pBp->address.offset > 0xFF )
            {
                dprinth(1, "Invalid interrupt number");
                deb.error = ERR_BPINT;          // Invalid interrupt number in a breakpoint

                return( FALSE );
            }

            for(index=0; index<MAX_BREAKPOINTS; index++)
            {
                if( bp[index].Flags & BP_ENABLED && bp[index].Type==BP_TYPE_BPINT && bp[index].address.offset==pBp->address.offset )
                {
                    deb.error = ERR_BPDUP;      // Duplicate breakpoint

                    return( FALSE );
                }
            }
            break;

        case BP_TYPE_BPIO:
            // 1. Port number has to be in the range 0000 - FFFF
            // 2. Debug register should not be already in use
            // 3. No duplicate interrupt with the same port
            if( pBp->address.offset > 0xFFFF )
            {
                dprinth(1, "Invalid IO port number");
                deb.error = ERR_BPIO;           // Invalid port number in a breakpoint

                return( FALSE );
            }

            if( pBp->DrUse )
            {
                for(index=0; index<MAX_BREAKPOINTS; index++)
                {
                    if( bp[index].Flags & BP_ENABLED && bp[index].DrUse==pBp->DrUse)
                    {
                        deb.error = ERR_DRUSED; // Debug register is already being used

                        return( FALSE );
                    }
                }
            }

            for(index=0; index<MAX_BREAKPOINTS; index++)
            {
                if( bp[index].Flags & BP_ENABLED && bp[index].Type==BP_TYPE_BPIO && bp[index].address.offset==pBp->address.offset )
                {
                    deb.error = ERR_BPDUP;      // Duplicate breakpoint

                    return( FALSE );
                }
            }
            break;

        case BP_TYPE_BPMB:
        case BP_TYPE_BPMW:
        case BP_TYPE_BPMD:
            // 1. Debug register should not be already in use
            // 2. No duplicate breakpoint with the same address
            if( pBp->DrUse )
            {
                for(index=0; index<MAX_BREAKPOINTS; index++)
                {
                    if( bp[index].Flags & BP_ENABLED && bp[index].DrUse==pBp->DrUse)
                    {
                        deb.error = ERR_DRUSED; // Debug register is already being used

                        return( FALSE );
                    }
                }
            }

            for(index=0; index<MAX_BREAKPOINTS; index++)
            {
                if( bp[index].Flags & BP_ENABLED &&
                (bp[index].Type==BP_TYPE_BPMB || bp[index].Type==BP_TYPE_BPMW || bp[index].Type==BP_TYPE_BPMD)
                && bp[index].address.offset==pBp->address.offset && bp[index].address.sel==pBp->address.sel)
                {
                    deb.error = ERR_BPDUP;      // Duplicate breakpoint

                    return( FALSE );
                }
            }
            break;
    }

    // Additional checks for BPM memory breakpoints:
    // 1. Memory alignment: BPW must be aligned on a word boundary, BPD on a dword boundary

    if( pBp->Type==BP_TYPE_BPMW && (pBp->address.offset & 1)!=0 )
    {
        dprinth(1, "BPMW address must be on WORD boundary");
        deb.error = ERR_BPMWALIGN;

        return( FALSE );
    }

    if( pBp->Type==BP_TYPE_BPMD && (pBp->address.offset & 3)!=0 )
    {
        dprinth(1, "BPMD address must be on DWORD boundary");
        deb.error = ERR_BPMDALIGN;

        return( FALSE );
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL AssignDebugReg(TBP *pBp)                                             *
*                                                                             *
*******************************************************************************
*
*   Assigns a first free debug register to a given breakpoint descriptor.
*
*   Returns:
*       TRUE - DrUse is assigned
*       FALSE - No more available debug registers
*
******************************************************************************/
BOOL AssignDebugReg(TBP *pBp)
{
    int index;
    BYTE available = 0xF;                   // Assume all 4 are available

    // We need to find a not used debug register to assign
    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Clear the bit of a debug register that is already in use
        if( bp[index].Flags & BP_USED )
            available &= ~bp[index].DrUse;
    }

    if( available )
    {
        // Assign the first one available
        if( available & 1 ) pBp->DrUse = 1; else    // DR0
        if( available & 2 ) pBp->DrUse = 2; else    // DR1
        if( available & 4 ) pBp->DrUse = 4; else    // DR2
        pBp->DrUse = 8;                             // DR3
    }
    else
    {
        return( FALSE );
    }

    return( TRUE );
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
*   Note: Global variable 'hint' is normally kept at -1 unless the option
*         BPE selects an existing breakpoint to be modified, in which case
*         it temporarily indexes selected slot to be used.
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

        pBp = &bp[index];               // Assign the pointer to the old bp

        // If we are reusing a breakpoint slot, clear some variables

        if( pBp->pCmd )
            _kFree(pIce->hHeap, pBp->pCmd);

        // Clear the breakpoint entry since we will rebuild it
        memset(pBp, 0, sizeof(TBP));
    }
    else
    {
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( !(bp[index].Flags & BP_USED) )
                break;
        }
    }

    // `index' now selects a breakpoint slot

    if( index<MAX_BREAKPOINTS )
    {
        pBp = &bp[index];               // Assign the pointer to a new bp

        // Allocate space to copy the command line string
        if( (pBp->pCmd = _kMalloc(pIce->hHeap, strlen(args)+1)) )
        {
            // Copy the breakpoint line into the new buffer
            strcpy(pBp->pCmd, args);

            // Start parsing a copy of the command line
            args = pBp->pCmd;

            // Set the breakpoint type
            pBp->Type = subClass;

            // Get the mandatory address portion: selector portion is CS by default
            evalSel = deb.r->cs;

            // Evaluate expression for the address portion
            if( Expression(&pBp->address.offset, args, &args))
            {
                // Verify that the selector is readable and valid
                if( VerifySelector(evalSel) )
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
                            pBp->Access = 3;        // default is "RW"
                    }

                    while( *args==' ' ) args++;     // Skip spaces

                    // Specify the size parameter that is valid for memory breakpoints
                    if( subClass==BP_TYPE_BPMB || subClass==BP_TYPE_BPMW || subClass==BP_TYPE_BPMD )
                    {
                        pBp->Size = subClass - BP_TYPE_BPMB;        // 0 (B), 1 (W), 3 (D)
                    }

                    // Memory and IO breakpoints can specify an optional debug register DR0..DR3
                    // In any case, we need to assign a debug register here
                    if( subClass==BP_TYPE_BPIO || subClass==BP_TYPE_BPMB || subClass==BP_TYPE_BPMW || subClass==BP_TYPE_BPMD )
                    {
                        if( !strnicmp(args, "DR", 2) )
                        {
                            args += 2;                              // Point to the DR register number

                            pBp->DrHint = 1 << GetHex(&args);       // Get the DR register bit

                            pBp->DrUse = pBp->DrHint;               // Use the one that is hinted

                            while( *args==' ' ) args++;                 // Skip spaces
                        }
                        else
                        {
                            // Assign a debug register
                            if( AssignDebugReg(pBp)==FALSE )
                            {
                                // Failed to assign a debug register, which is error
                                deb.error = ERR_DRUSEDUP;           // All debug registers used
                                goto bpx_failed;
                            }
                        }
                    }

                    //------------------------------------------------------------------
                    // If the next statement is "IF", accept it
                    if( !strnicmp(args, "IF", 2) )
                    {
                        // Store a pointer to the IF expression statement
                        args += 2;
                        pBp->pIF = args;

                        // Massage the pointer to IF<expression> to skip heading spaces
                        while( *pBp->pIF==' ' ) pBp->pIF++;

                        // Dummy evalute the expression to skip it
                        Expression(&dummy, args, &args);

                        // ("DO" will sometimes evaluate as 0xD + 'O' by our eval. Fix it)
                        if( !strnicmp(args-1, "DO", 2) ) args--;

                        // Terminate "IF" expression substring
                        *(args-1) = 0;
                    }

                    //------------------------------------------------------------------
                    // If the next statement is "DO", accept it
                    if( !strnicmp(args, "DO", 2) )
                    {
                        // Store a pointer to DO statements
                        args += 2;
                        pBp->pDO = args;

                        // Massage the pointer to DO<cmd> to skip heading spaces
                        while( *pBp->pDO==' ' ) pBp->pDO++;
                    }

                    // For BPX breakpoints, set a possible file_id and line number
                    if( subClass==BP_TYPE_BPX )
                    {
                        TSYMFNSCOPE *pFnScope;
                        TSYMFNLIN *pFnLin;

                        pFnScope = SymAddress2FnScope(pBp->address.sel, pBp->address.offset);
                        if( pFnScope )
                        {
                            pBp->file_id = pFnScope->file_id;

                            pFnLin = SymAddress2FnLin(pBp->address.sel, pBp->address.offset);
                            SymFnLin2Line(&pBp->line, pFnLin, pBp->address.offset);
                        }

                        // Also, for BPX breakpoints, redraw the screen since we may want to
                        // highlight differently the code pane (source or machine)

                        deb.fRedraw = TRUE;
                    }

                    //------------------------------------------------------------------
                    // Perform validity check for all types of breakpoints
                    //------------------------------------------------------------------
                    if( VerifyBreakpoint(pBp)==TRUE )
                    {
                        // If everything went OK, we activate a new breakpoint
                        if( !deb.error )
                        {
                            // Finalize the breakpoint as valid and active
                            pBp->Flags |= BP_USED | BP_ENABLED;

                            return(TRUE);
                        }
                    }
                }

                // Else do not define a new breakpoint, but clean up the slot
bpx_failed:;
            }
            else
                deb.error = ERR_SYNTAX;     // Syntax error evaluating

            // Free the buffer since setting a bp failed
            _kFree(pIce->hHeap, pBp->pCmd);

            // Clear the breakpoint entry
            memset(pBp, 0, sizeof(TBP));
        }
        else
            deb.error = ERR_INT_OUTOFMEM;   // Internal error: out of memory
    }
    else
        deb.error = ERR_BP_TOO_MANY;    // No more breakpoints available

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   B A C K   E N D   F U N C T I O N S                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void SetNonStickyBreakpoint(TADDRDESC Addr)                               *
*                                                                             *
*******************************************************************************
*
*   Call this function to set a non-sticky breakpoint which will be armed
*   before debugee runs.
*
******************************************************************************/
void SetNonStickyBreakpoint(TADDRDESC Addr)
{
    // Enable breakpoint and set the address

    nsbp.Flags |= BP_ENABLED;
    nsbp.address = Addr;
}

#if 0
/******************************************************************************
*                                                                             *
*   void ClearNonStickyBreakpoint(void)                                       *
*                                                                             *
*******************************************************************************
*
*   Clears the record of a non-sticky breakpoint.
*
******************************************************************************/
void ClearNonStickyBreakpoint(void)
{
    nsbp.Flags = 0;
}
#endif

/******************************************************************************
*                                                                             *
*   void ArmBreakpoints(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   This function arms all breakpoints. This is done before returning
*   control to the debugee.
*
*   Special case is if a breakpoint exists at the current cs:eip, in which
*   case we enter the state of a delayed arm (we do a single step after
*   which we arm all breakpoints and continue)
*
*   In addition to all sticky breakpoints, a single non-sticky breakpoint will
*   be placed. This one is used as a single-shot breakpoint.
*
******************************************************************************/
void ArmBreakpoints(void)
{
    int index, dr;

    // Translation from DR bitfield into DR register number
    static const int bDr[] = {  0, 0, 1, 0, 2, 0, 0, 0, 3  };

    // Reset debug registers
    deb.sysReg.dr7 = 3 << 8;            // LE GE set recommended

    // If the breakpoint would be placed at the current cs:eip..
    if( BreakpointCheckSpecial() )
    {
        // .. enter the delayed arm mode

        pIce->eDebuggerState = DEB_STATE_DELAYED_ARM;

        // Set the single-step trace mode
        deb.r->eflags |= TF_MASK;
    }
    else
    {
        // Arm a single non-sticky breakpoint, if required
        if( nsbp.Flags & BP_ENABLED )
        {
            // Save the original byte and place int3
            nsbp.origValue = AddrGetByte(&nsbp.address);
            AddrSetByte(&nsbp.address, 0xCC, TRUE);
        }

        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            // Place only enabled breakpoints

            if( bp[index].Flags & BP_ENABLED )
            {
                switch( bp[index].Type )
                {
                    case BP_TYPE_BPX:
                        // Save the original byte and place int3
                        bp[index].origValue = AddrGetByte(&bp[index].address);
                        AddrSetByte(&bp[index].address, 0xCC, TRUE);
                        break;

                    case BP_TYPE_BPINT:
                        break;

                    case BP_TYPE_BPIO:
                        // Set the DE (Debugging Extensions) bit in CR4 so we can trap IO accesses
                        deb.sysReg.cr4 |= BITMASK(DE_BIT);

                        dr = bDr[bp[index].DrUse];      // Debug register number

                        // Set the IO port to trap on into a debug register
                        deb.sysReg.dr[ dr ] = bp[index].address.offset;

                        // Enable that breakpoint
                        deb.sysReg.dr7 |= 2 << (dr*4 + 16); // IO Breakpoint

                        deb.sysReg.dr7 |= 3 << (dr*2);  // Global + Local flag

                        break;

                    case BP_TYPE_BPMB:
                    case BP_TYPE_BPMW:
                    case BP_TYPE_BPMD:

                        dr = bDr[bp[index].DrUse];      // Debug register number

                        // Set the address to trap the access to
                        deb.sysReg.dr[ dr ] = bp[index].address.offset;

                        // Enable that breakpoint
                        deb.sysReg.dr7 |= bp[index].Access << (dr*4 + 16);

                        // Set the access size
                        deb.sysReg.dr7 |= bp[index].Size << (dr*4 + 16 + 2);

                        deb.sysReg.dr7 |= 3 << (dr*2);  // Global + Local flag

                        break;
                }
            }
        }
    }
}

/******************************************************************************
*                                                                             *
*   void *DisarmBreakpoints(void)                                             *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugger gets control to remove all
*   breakpoints that it had placed, so they dont obstruct the view.
*
*   In addition to all sticky breakpoints, a single non-sticky breakpoint
*   will be disarmed (but not cleared!)
*
*   If we break due to our breakpoint INT3, decrement eip to position it on the
*   valid instruction start that has been overwritten with 0xCC.
*   We define 'our breakpoint INT3' as DR6=0 && cs:eip-1==BPx
*   DR6=0 means we did not stop here due to any hardware or trap fault.
*
*   Debug registers will be cleared.
*
*   This function also sets or clears the breakpoint index in deb structure.
*
*   Returns:
*       pointer to a breakpoint record that hit
*       NULL if no breakpoint matched
*
******************************************************************************/
void *DisarmBreakpoints(void)
{
    int index;
    TBP *pBp;                           // Pointer to a breakpoint that hit

    deb.bpIndex = -1;                   // Reset the index to signal no breakpoint hit

    for(index=MAX_BREAKPOINTS-1; index>=0; index-- )
    {
        // Restore only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            switch( bp[index].Type )
            {
                case BP_TYPE_BPX:
                    // Restore original value only if we found INT3 there
                    if( AddrGetByte(&bp[index].address)==0xCC )  // Was it INT3?
                    {
                        // Restore original value
                        AddrSetByte(&bp[index].address, bp[index].origValue, TRUE);

                        // If the current cs:eip-1 matches the bpx address, set the index
                        if( deb.r->cs==bp[index].address.sel && deb.r->eip-1==bp[index].address.offset )
                            deb.bpIndex = index;
                    }
                    break;

                case BP_TYPE_BPINT:
                    break;

                // All of these are using only debug registers
                case BP_TYPE_BPIO:

                case BP_TYPE_BPMB:
                case BP_TYPE_BPMW:
                case BP_TYPE_BPMD:

                    if( (deb.sysReg.dr6 & 0xF) & bp[index].DrUse )
                        deb.bpIndex = index;
                    break;
            }
        }
    }

    if( deb.bpIndex>=0 )
        pBp = &bp[deb.bpIndex];
    else
        pBp = NULL;

    // Disarm the non-sticky breakpoint, if enabled, and disable it
    if( nsbp.Flags & BP_ENABLED )
    {
        if( AddrGetByte(&nsbp.address)==0xCC )  // Was it INT3?
        {
            AddrSetByte(&nsbp.address, nsbp.origValue, TRUE);

            // If the current cs:eip-1 matches the non-sticky bp address, do not reset the nbsp.Flags
            // to signal later that we indeed hit one INT3
            if( deb.r->cs!=nsbp.address.sel || deb.r->eip-1!=nsbp.address.offset )
                nsbp.Flags = 0;                         // Disable non-sticky breakpoint
        }
    }

    // If the DR6=0 (no hardware breakpoint hit or a trap fault) and cs:eip-1 was a
    // breakpoint, decrement eip
    if( (pBp || nsbp.Flags) && (deb.sysReg.dr6 & (DR6_BS_BIT|DR6_BT_BIT|0xF))==0 )
    {
        deb.r->eip -= 1;
    }

    // Disable non-sticky breakpoint
    nsbp.Flags = 0;

    // Clear the DR6 register since CPU never does it
    deb.sysReg.dr6 = 0;

    // Clear the DR7 register since it needs to be rebuilt anyways. This allows us to have temp
    // non-sticky hardware breakpoints. Also, we dont want to trigger hardware breakpoints
    // while we are in the debugger
    deb.sysReg.dr7 = 0;

    SetSysreg(&deb.sysReg);             // Write out DR7

    return( (void *)pBp );
}


/******************************************************************************
*                                                                             *
*   BOOL EvalBreakpoint(void *pBp)                                            *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugger gets control to evaluate a single
*   breakpoint that hit. All breakpoint counters will be adjusted.
*
*   Where:
*       pBp is the pointer to a breakpoint that hit
*
*   Returns:
*       TRUE - evaluation requires to continue execution
*       FALSE - stop in the debugger
*
******************************************************************************/
BOOL EvalBreakpoint(void *_pBp)
{
    TBP *pBp;                           // Pointer to a breakpoint that hit
    DWORD result;                       // Result of evaluation
    BOOL fPopup;                        // Result of breakpoint DO evaluation

    // Only if we are passed a valid breakpoint address...
    if( _pBp )
    {
        pBp = (TBP *)_pBp;              // Get the pointer, but type casted

        // Print the message so we know why did we stop
        dprinth(1, "Breakpoint due to BPX %02X", pBp - bp);

        pBp->Hits++;                    // Increment the breakpoint hit counter
        pBp->CurHits++;

        // If this breakpoint has IF condition, evaluate it now
        if( pBp->pIF )
        {
            if( Expression(&result, pBp->pIF, NULL) )
            {
                if( result )
                {
                    // Result is non-zero, means TRUE.
                    // If the DO part was given, evaluate it as a command stream
                    if( pBp->pDO )
                    {
                        fPopup = CommandExecute(pBp->pDO);

                        if( fPopup==FALSE )
                        {
                            // Command evaluation required to continue execution

                            return( TRUE );
                        }
                    }
                    else
                    {
                        // No DO portion and it evaluated to TRUE, so break and popup
//                      pBp->Breaks++;
                    }
                }
                else
                {
                    // Result is zero, means FALSE, we will continue
                    pBp->Misses++;      // Evaluated to FALSE and miss
                    pBp->CurMisses++;   // Current misses

                    return( TRUE );
                }
            }
            else
            {
                pBp->Errors++;          // Expression resulted in error in evaluation
            }
        }

        pBp->Popups++;                  // We will popup now
        pBp->Breaks++;
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   int BreakpointCheck(TADDRDESC Addr)                                       *
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
int BreakpointCheck(TADDRDESC Addr)
{
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Check only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            switch( bp[index].Type )
            {
                case BP_TYPE_BPX:
                    if( bp[index].address.sel==Addr.sel && bp[index].address.offset==Addr.offset )
                    {
                        // Found a matching address!

                        bp[index].Hits++;       // Total number of times this was hit

                        return(index);
                    }
                    break;

                case BP_TYPE_BPINT:
                    break;

                case BP_TYPE_BPIO:
                    break;

                case BP_TYPE_BPMB:
                    break;

                case BP_TYPE_BPMW:
                    break;

                case BP_TYPE_BPMD:
                    break;
            }
        }
    }

    return(-1);
}


/******************************************************************************
*                                                                             *
*   int BreakpointQuery(TADDRDESC Addr)                                       *
*                                                                             *
*******************************************************************************
*
*   Checks if any breakpoint address matches given sel:offset. This is valid
*   only for BPX breakpoints.
*
*   Returns:
*       -1 if no breakpoint addresses match
*       bp index if match is found
*
******************************************************************************/
int BreakpointQuery(TADDRDESC Addr)
{
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Check only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            if( bp[index].Type==BP_TYPE_BPX )
            {
                if( bp[index].address.sel==Addr.sel && bp[index].address.offset==Addr.offset )
                {
                    // Found a matching address!
                    return(index);
                }
            }
        }
    }

    return(-1);
}


/******************************************************************************
*                                                                             *
*   int BreakpointQueryFileLine(WORD file_id, WORD line)                      *
*                                                                             *
*******************************************************************************
*
*   Checks if any breakpoint matches given file_id and line number
*
*   Returns:
*       -1 if no breakpoint addresses match
*       bp index if match is found
*
******************************************************************************/
int BreakpointQueryFileLine(WORD file_id, WORD line)
{
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Check only enabled breakpoints

        if( bp[index].Flags & BP_ENABLED )
        {
            if( bp[index].Type==BP_TYPE_BPX )
            {
                if( bp[index].file_id==file_id && bp[index].line==line )
                {
                    // Found a matching source file and line number!
                    return(index);
                }
            }
        }
    }

    return(-1);
}


/******************************************************************************
*                                                                             *
*   BOOL EvalBreakpointAddress(TADDRDESC *pAddr, int index)                   *
*                                                                             *
*******************************************************************************
*
*   Returns breakpoint address for expression evaluation.
*
*   Where:
*       pAddr is where to store the result
*       index is the breakpoint index
*
*   Returns:
*       TRUE - valid breakpoint data returned
*       FALSE - index had invalid brekpoint
*
******************************************************************************/
BOOL EvalBreakpointAddress(TADDRDESC *pAddr, int index)
{
    // Consider only valid indices
    if( index>=0 && index<MAX_BREAKPOINTS )
    {
        // Breakpoint has to be used (defined)
        if( bp[index].Flags & BP_USED )
        {
            pAddr->offset = bp[index].address.offset;

            // BPIO and BPINT do not have selector part set valid, so dont copy it
            if( bp[index].Type!=BP_TYPE_BPINT && bp[index].Type!=BP_TYPE_BPIO )
            {
                pAddr->sel = bp[index].address.sel;
            }

            return( TRUE );
        }
    }

    return(FALSE);
}


/******************************************************************************
*                                                                             *
*   BOOL BreakpointCheckSpecial(void)                                         *
*                                                                             *
*******************************************************************************
*
*   This function supports a peculiar case where a breakpoint might be
*   placed at the address of the current cs:eip and the running it would
*   break immediately. We check all the BPX-type breakpoints.
*
*   Returns:
*       TRUE - a bpx-type breakpoint was found at the cs:eip
*       FALSE - no bpx-type breakpoints at this address
*
******************************************************************************/
BOOL BreakpointCheckSpecial(void)
{
    int index;

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Check only enabled breakpoints of the BPX-type

        if( bp[index].Flags & BP_ENABLED )
        {
            if( bp[index].Type==BP_TYPE_BPX )
            {
                if( bp[index].address.sel==deb.r->cs && bp[index].address.offset==deb.r->eip )
                {
                    // Found a matching breakpoint!

                    return( TRUE );
                }
            }
        }
    }

    return(FALSE);
}

/******************************************************************************
*                                                                             *
*   BOOL NonStickyBreakpointCheck(TADDRDESC Addr)                             *
*                                                                             *
*******************************************************************************
*
*   Checks if the given address matches a non-sticky breakpoint.
*
*   Returns:
*       FALSE if the address does not match a non-sticky breakpoint
*       TRUE if the address matches a non-sticky breakpoint
*
******************************************************************************/
BOOL NonStickyBreakpointCheck(TADDRDESC Addr)
{
    if( nsbp.Flags & BP_ENABLED )
    {
        // Compare the sel and offset of the address
        if( nsbp.address.sel==Addr.sel && nsbp.address.offset==Addr.offset )
            return( TRUE );
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   int ArmDebugReg(int nDr, TADDRDESC Addr)                                  *
*                                                                             *
*******************************************************************************
*
*   Arms a specified debug register for execution only
*
*   Where:
*       nDr (0,1,2,3) selects a particular debug reg
*       Addr is the sel:offset of the address to arm
*
*   Returns:
*       0
*
******************************************************************************/
int ArmDebugReg(int nDr, TADDRDESC Addr)
{
    DWORD Linear;

    // Calculate the linear address of the breakpoint
    // TODO: We will again assume our selector is zero-based
    Linear = Addr.offset;

    // BPX breakpoint
    deb.sysReg.dr[nDr] = Linear;

    // Set breakpoint on execution only (value 0)
    deb.sysReg.dr7 &= ~3 << ((nDr*4)+16);

    // Set global and local flag to actually enable a breakpoint
    deb.sysReg.dr7 |= 3 << (nDr*2);

    return(0);
}


