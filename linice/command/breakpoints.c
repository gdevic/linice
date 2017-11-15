/******************************************************************************
*                                                                             *
*   Module:     breakpoints.c                                                 *
*                                                                             *
*   Date:       06/26/01                                                      *
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

// Flags: Any bit can be specified independently:

#define BP_USED         0x01            // Breakpoint is used (not an empty slot)
#define BP_ENABLED      0x02            // Breakpoint is enabled (active)
#define BP_ONETIME      0x04            // One-time breakpoint
#define BP_INTERNAL     0x08            // Internal breakpoint that is not reported

// Types (types and subclasses are the same number): One and only one value can be specified:

#define BP_TYPE_BPX     0x01            // Breakpoint is BPX   (subClass 1)
#define BP_TYPE_BPIO    0x03            // Breakpoint is BPIO  (subClass 3)
#define BP_TYPE_BPMB    0x04            // Breakpoint is BPMB  (subClass 4)
#define BP_TYPE_BPMW    0x05            // Breakpoint is BPMW  (subClass 5)
#define BP_TYPE_BPMD    0x07            // Breakpoint is BPMD  (subClass 7)

typedef struct
{
    BYTE Flags;                         // Breakpoint flags
    BYTE Type;                          // Breakpoint type

    char *pCmd;                         // Pointer to a full bp command line (alloc buffer)
    char *pIF;                          // Pointer to an optional IF expression within that buffer
    char *pDO;                          // Pointer to an optional DO "statements" within that buffer

    BYTE Size;                          // Size of the memory access (B, W, D)
    BYTE Access;                        // Access type (R/W/RW/X)
    BYTE DrRequest;                     // Debug register that the user explicitly requested (bitmask)
    BYTE DrUse;                         // Debug register that is actually used (bitmask)
                                        // Also specifies the use of HW breakpoint as opposed to embedded INT3
    TADDRDESC address;                  // Breakpoint address (sel:offset)
    WORD file_id;                       // Source code file_id
    WORD line;                          // Source code line number
    BYTE origValue;                     // Original value that was there before INT3

    // Breakpoint statistics
    DWORD Hits;                         // Number of times we evaluated a bp
    DWORD Breaks;                       // Number of times a bp has evaluated to TRUE
    DWORD Popups;                       // Number of times it caused Ice to popup
    DWORD Misses;                       // Number of times a bp has evaluated to FALSE
    DWORD Errors;                       // Number of times evaluation resulted in error
    DWORD CurHits;                      // Number of timer we hit it before it popup (clear after popup)
    DWORD CurMisses;                    // Number of times we evaluated to FALSE before it popup (clear after popup)

} TBP;

/*
    BPX   - address.sel, address.offset contain user specified address to break
            file_id, line, if non-zero, index the source code line of that bpx
            Note: In code edit mode, no arguments is allowed.

    BPIO  - address.offset is the IO port to trap (0x00 - 0xFFFF)
            Access: 3 (RW), 2 (R), 1 (W)
            DrRequest register use: 1 (DR0), 2 (DR1), 4 (DR2), 8 (DR3)

    BPM?  - address.sel, address.offset contains the memory address to break
            Access: 3 (RW), 2 (R), 1 (W)
            Size: 0 (B), 1 (W), 3 (D)
            DrRequest register use: 1 (DR0), 2 (DR1), 4 (DR2), 8 (DR3)

*/

static TBP bp[MAX_BREAKPOINTS];         // Array of breakpoint structures

static int hint = -1;                   // No hints to edit breakpoint number
static int hwbpinuse = 0;               // Number of hw breakpoints in use (0 - 4)
static BYTE GlRequest = 0;              // Global hw breakpoint request bitmask

static char Buf[MAX_STRING];            // Temp line buffer to send line to edit and to use as a code edit mode address line
static char *pBuf;

static DWORD dr[8];                     // Temp stored verification values

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

int BreakpointQuery(TADDRDESC Addr);


/******************************************************************************
*                                                                             *
*   BOOL IsNeedHwBp(BYTE Type)                                                *
*                                                                             *
*******************************************************************************
*
*   Returns true if a breakpoint with that type needs a hardware resource (DR)
*
******************************************************************************/
BOOL IsNeedHwBp(BYTE Type)
{
    return( Type>=BP_TYPE_BPIO );
}

/******************************************************************************
*                                                                             *
*   void InitBreakpoints()                                                    *
*                                                                             *
*******************************************************************************
*
*   Call once to initialize breakpoints (clear).
*
******************************************************************************/
void InitBreakpoints()
{
    memset(bp, 0, sizeof(bp));
}

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
*   list needs to be given as a non-zero sequence of hex numbers
*
******************************************************************************/
BOOL cmdBp(char *args, int subClass)
{
    static BYTE select[MAX_BREAKPOINTS];        // Selected breakpoints
    int index;                                  // Temp breakpoint index

    if( *args=='*' )
    {
        // Argument * specifies every (all) breakpoints
        memset(select, 1, sizeof(select));
    }
    else
    {
        // Clear the select array to not selected
        memset(select, 0, sizeof(select));

        // The arguments are a list of breakpoint indices separated by space or comma

        while( Expression(&index, args, &args) )
        {
            if( index>=0 && index<MAX_BREAKPOINTS )
            {
                select[index] = 1;      // Make it selected
            }
            else
            {
                deb.error = ERR_BPNUM;  // Invalid breakpoint number

                return(TRUE);
            }

            // Allow commas as the list delimiters
            if( *args==',' )
                args++;
        }

        // We should have cleaned the argument list
        if( *args )
        {
            deb.error = ERR_SYNTAX;

            return( TRUE );
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
                        // Clear goes via disable path to release resource, then does a clear...

                case 1:                 // subClass = 1, BD, disable a breakpoint
                        // Dont clear breakpoints that are not enabled
                        if( bp[index].Flags & BP_ENABLED )
                        {
                            bp[index].Flags &= ~BP_ENABLED;

                            // Free the hardware bp resource
                            if( bp[index].DrRequest )
                                GlRequest ^= bp[index].DrRequest, hwbpinuse--;
                            else
                                if( IsNeedHwBp(bp[index].Type) )
                                    hwbpinuse--;
                        }

                        // Now we will do actual clear for those cases
                        if( subClass==0 )
                        {
                            // Free the command line of a breakpoint and IF/DO statements
                            freeHeap(deb.hHeap, bp[index].pCmd);

                            // Clear the breakpoint entry
                            memset(&bp[index], 0, sizeof(TBP));
                        }
                    break;

                case 2:                 // subClass = 2, BE, enable a breakpoint
                        // Dont re-enable breakpoints that are already enabled
                        if( !(bp[index].Flags & BP_ENABLED) )
                        {
                            // Reaquire the hw resource if possible
                            if( bp[index].DrRequest )
                            {
                                if( bp[index].DrRequest & GlRequest )
                                {
                                    // Conflict - we cannot use the requested DR register
                                    deb.error = ERR_DRUSED;
                                    break;
                                }

                                GlRequest |= bp[index].DrRequest;
                                hwbpinuse++;
                            }
                            else
                            {
                                if( IsNeedHwBp(bp[index].Type) )
                                {
                                    if( hwbpinuse>4 )
                                    {
                                        // Not enough DR registers, so we cannot enable this bp
                                        deb.error = ERR_BP_TOO_MANY;
                                        break;
                                    }
                                    else
                                        hwbpinuse++;
                                }
                            }

                            bp[index].Flags |= BP_ENABLED;
                        }
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
*   char *GetBpFormatString(TBP *pBp)                                         *
*                                                                             *
*******************************************************************************
*
*   Forms a string with the given breakpoint description for listing it.
*   It uses the temp static buffer.
*
******************************************************************************/
static char *GetBpFormatString(TBP *pBp)
{
    // BPIO types (stored in the Access field)
    static const char *sBpio[] = { "", "R ", "W ", "RW " };

    // BPM sizes (encoded in the type)
    static const char *sBpm[] = { "B ", "W ", "", "D " };

    // DR register name from a bitfield
    static const char *sDr[] =  { "", "DR0 ", "DR1 ", "", "DR2 ", "", "", "", "DR3 " };

    // Reset the string buffer pointer
    pBuf = Buf;

    // Check the breakpoint type flag and print accordingly
    switch( pBp->Type )
    {
        case BP_TYPE_BPX:
            pBuf += sprintf(pBuf, "BPX %04X:%08X %s%s",
                pBp->address.sel, pBp->address.offset,
                sDr[pBp->DrRequest],
                (pBp->Flags & BP_ONETIME)?"O ":"");
            break;

        case BP_TYPE_BPIO:
            pBuf += sprintf(pBuf, "BPIO %X %s%s%s",
                pBp->address.offset,
                sBpio[pBp->Access],
                sDr[pBp->DrRequest],
                (pBp->Flags & BP_ONETIME)?"O ":"");
            break;

        case BP_TYPE_BPMB:
        case BP_TYPE_BPMW:
        case BP_TYPE_BPMD:
            pBuf += sprintf(pBuf, "BPM%s %04X:%08X %s%s%s",
                sBpm[pBp->Type - BP_TYPE_BPMB],
                pBp->address.sel, pBp->address.offset,
                sBpio[pBp->Access],
                sDr[pBp->DrRequest],
                (pBp->Flags & BP_ONETIME)?"O ":"");
            break;
    }

    // In all cases append optional IF and DO statements
    if( pBp->pIF )
        pBuf += sprintf(pBuf, "IF %s ", pBp->pIF);

    if( pBp->pDO )
        pBuf += sprintf(pBuf, "DO %s", pBp->pDO);

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
    int index;                          // Temp breakpoint index

    for(index=0; index<MAX_BREAKPOINTS; index++ )
    {
        // Naturally, only list used breakpoint slots

        if( bp[index].Flags & BP_USED )
        {
            // print the string describing a breakpoint using a different color
            // if the breakpoint is indexed as current

            if(dprinth(nLine++, "%c%c%02X) %s %s",
                DP_SETCOLINDEX, (index==deb.bpIndex)? COL_BOLD : COL_NORMAL,
                index,
                (bp[index].Flags & BP_ENABLED)? " ":"*",
                GetBpFormatString(&bp[index]) )==FALSE)
                break;
        }
    }

    // Advanced list will list all internal structures
    if( *args && *args=='#' )
    {
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( bp[index].Flags & BP_USED )
            {
                dprinth(nLine++, "%02X f=%d t=%d rq=%X du=%X a=%d s=%d",
                    index,
                    bp[index].Flags,
                    bp[index].Type,
                    bp[index].DrRequest,
                    bp[index].DrUse,
                    bp[index].Access,
                    bp[index].Size );
            }
        }

        dprinth(nLine++, "hint=%d inuse=%d GR=%X ind=%d", hint, hwbpinuse, GlRequest, deb.bpIndex);
        dprinth(nLine++, "%08X %08X %08X %08X 6=%08X 7=%08X", dr[0], dr[1], dr[2], dr[3], dr[6], dr[7]);
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

                    pCmdEdit = GetBpFormatString(&bp[index]);

                    return(TRUE);
                }
            }

            // From now on it is an invalid breakpoint
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
*   BOOL BPstat(int index, int n)                                             *
*                                                                             *
*******************************************************************************
*
*   Display statistic of a single (used) breakpoint
*   Return: FALSE if the printing should stop
*
******************************************************************************/
BOOL BPstat(int index, int n)
{
    static int nLine;                   // Local line counter
    TBP *p = &bp[index];                // Get the pointer to a current bp

    if( n==1 ) nLine = 1;               // Reset the line counter in the first call

    if(dprinth(nLine++, "Breakpoint Statistics for #%02X %s", index, (p->Flags & BP_ENABLED)?"" : "(disabled)")
    && dprinth(nLine++, "   %s", GetBpFormatString(p) )
    && dprinth(nLine++, "   Cond    %s", p->pIF? p->pIF : "No" )
    && dprinth(nLine++, "   Action  %s", p->pDO? p->pDO : "No" )
    && dprinth(nLine++, "Totals")
    && dprinth(nLine++, "   Hits    %X", p->Hits )
    && dprinth(nLine++, "   Breaks  %X", p->Breaks )
    && dprinth(nLine++, "   Popups  %X", p->Popups )
    && dprinth(nLine++, "   Misses  %X", p->Misses )
    && dprinth(nLine++, "   Errors  %X", p->Errors )
    && dprinth(nLine++, "Current")
    && dprinth(nLine++, "   Hits    %X", p->CurHits )
    && dprinth(nLine++, "   Misses  %X", p->CurMisses ))
        return( TRUE );

    return(FALSE);
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
    int nFirst = 1;
    int index;

    if( *args )
    {
        // A specific breakpoint
        if( Expression(&index, args, &args))
        {
            if( index>=0 && index<MAX_BREAKPOINTS && (bp[index].Flags & BP_USED) )
            {
                // Display individual statistics
                BPstat(index, nFirst);
            }
            else
                deb.error = ERR_BPNUM;      // Invalid breakpoint number
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
                if( BPstat(index, nFirst++)==FALSE )
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
*   When we define a new breakpoint, we need to check that it is valid with
*   regards to other breakpoints already defined.
*   That is, no duplicate breakpoint and some parameter validity.
*
*   Where:
*       pBp is the breakpoint that is to be checked against the rest
*           This pointer may address a breakpoint from the array.
*
*   Returns:
*       TRUE - Given breakpoint is ok
*       FALSE - There was some error with the given breakpoint
*
******************************************************************************/
static BOOL VerifyBreakpoint(TBP *pBp)
{
    int index;                                  // Temp breakpoint index

    // With all breakpoints types, we can compare address and selector for the same values
    // If we found, those are duplicate breakpoints (of the same class)

    // TODO - do we need to compare only to BP_USED bps?

    for(index=0; index<MAX_BREAKPOINTS; index++)
    {
        if( pBp != &bp[index] )
        {
            // The breakpoint type has to be the same to compare
            if( bp[index].Type == pBp->Type )
            {
                if( bp[index].address.offset==pBp->address.offset && bp[index].address.sel==pBp->address.sel)
                {
                    deb.error = ERR_BPDUP;      // Duplicate breakpoint

                    return( FALSE );
                }
            }
        }
    }

    // Type specific checks --

    switch( pBp->Type )
    {
        //====================================================================
        case BP_TYPE_BPIO:
            // Port number has to be in the range 0000 - FFFF
            if( pBp->address.offset > 0xFFFF )
            {
                deb.error = ERR_BPIO;           // Invalid port number in a breakpoint

                return( FALSE );
            }
            break;

        //====================================================================
        case BP_TYPE_BPMW:
            // Memory alignment: BPW must be aligned on a word boundary
            if( (pBp->address.offset & 1)!=0 )
            {
                deb.error = ERR_BPMWALIGN;      // Data alignment error

                return( FALSE );
            }
            break;

        //====================================================================
        case BP_TYPE_BPMD:
            // Memory alignment: BPD must be aligned on a dword boundary
            if( (pBp->address.offset & 3)!=0 )
            {
                deb.error = ERR_BPMDALIGN;      // Data alignment error

                return( FALSE );
            }
            break;
    }

    // Do this check last since it actually assigns resources

    if( pBp->DrRequest )
    {
        if( pBp->DrRequest & GlRequest )
        {
            // Conflict - we cannot use the requested DR register
            deb.error = ERR_DRUSED;
            return( FALSE );
        }
        else
        {
            // Assign the resource
            GlRequest |= pBp->DrRequest;
            hwbpinuse++;
        }
    }
    else
    if( IsNeedHwBp(pBp->Type) )
    {
        if( hwbpinuse>4 )
        {
            // Not enough DR registers, so we cannot enable this bp
            deb.error = ERR_BP_TOO_MANY;
            return( FALSE );
        }
        else
            hwbpinuse++;
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
*       BPX [address] [DRx] [O] [IF expression] [DO "command1; command2;..."]
*       BPIO  port [access: R W RW] [DRx] [O] [IF expression] [DO "command1; command2;..."]
*       BPM[B|W|D] address [access: R W RW] [DRx] [O] [IF expression] [DO "command1; command2;..."]
*
*       DRx = DR0 or DR1 or DR2 or DR3 - request a specific DR register to use
*       O   = One time breakpoint - it will clear after hitting
*
*   Note: BPX command allows no arguments if we are in the code edit mode.
*         In this mode, a breakpoint is toggled at the currently selected line.
*
*   BPX   = subClass 1
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
    TBP *pBp;                           // Pointer to the current breakpoint
    int index, dummy;

    // Special case if we are in the code edit mode and no arguments are given,
    // toggle the breakpoint at the selected line
    if( deb.fCodeEdit && !*args )
    {
        // Only if the valid address had beed selected, we can proceed
        if( deb.CodeEditAddress.sel==0 )
            return( TRUE );

        // Search for an existing breakpoint to know should we set a new one or clear existing one
        if( (index = BreakpointQuery(deb.CodeEditAddress))>=0 )
        {
            // Brakpoint already exists. Clear it.
            sprintf(Buf, "%02X", index);

            cmdBp(Buf, 0);      // Subclass 0 is BC, clear breakpoint

            return( TRUE );
        }

        // We did not find an existing breakpoint, so we will create a new one.
        // Form the address in the temp buffer and use that buffer as a argument

        sprintf(Buf, "%04X:%08X", deb.CodeEditAddress.sel, deb.CodeEditAddress.offset);

        args = Buf;
    }


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
            freeHeap(deb.hHeap, pBp->pCmd);
    }
    else
    {
        // Find the first available slot for the new breakpoint
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

        // Clear the breakpoint entry since we will rebuild it
        memset(pBp, 0, sizeof(TBP));

        // Allocate space to copy the command line string
        if( (pBp->pCmd = mallocHeap(deb.hHeap, strlen(args)+1)) )
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

                    // User can always assign an optional HW breakpoint register as his choice, DR0..DR3
                    if( !strnicmp(args, "DR", 2) )
                    {
                        if( *(args+2)=='0' ) pBp->DrRequest = 1 << 0;
                        else
                        if( *(args+2)=='1' ) pBp->DrRequest = 1 << 1;
                        else
                        if( *(args+2)=='2' ) pBp->DrRequest = 1 << 2;
                        else
                        if( *(args+2)=='3' ) pBp->DrRequest = 1 << 3;
                        else
                        {
                            deb.error = ERR_DRINVALID;
                            goto bpx_failed;
                        }

                        args += 3;                                  // Skip the DRx token

                        while( *args==' ' ) args++;                 // Skip spaces
                    }

                    //------------------------------------------------------------------
                    // If the next character is "O", accept it as a flag for One-time breakpoint
                    if( !strnicmp(args, "O", 1) )
                    {
                        args += 1;                                  // Skip the flag token
                        while( *args==' ' ) args++;                 // Skip spaces

                        pBp->Flags |= BP_ONETIME;                   // Set a one time breakpoint flag
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

                        // Finalize the breakpoint as valid and active
                        pBp->Flags |= BP_USED | BP_ENABLED;

                        return(TRUE);
                    }
                }

                // Else do not define a new breakpoint, but clean up the slot
bpx_failed:;
            }
            else
                deb.error = ERR_SYNTAX;     // Syntax error evaluating

            // Free the buffer since setting a bp failed
            freeHeap(deb.hHeap, pBp->pCmd);

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
*   void SetOneTimeBreakpoint(TADDRDESC Addr)                                 *
*                                                                             *
*******************************************************************************
*
*   Set a one-time execute breakpoint to the current address. Note that we
*   allow duplicate address here since the Linice needs to set it without any
*   possible conditions.
*
*   It is called from the G=xxx command and from the step functionality.
*   It marks the breakpoint as internal since we dont report it as an user
*   breakpoint.
*
*   Where:
*       Addr is the address to set it to
*
******************************************************************************/
void SetOneTimeBreakpoint(TADDRDESC Addr)
{
    int index;                          // Temp breakpoint index
    TBP *pBp;                           // Pointer to a breakpoint to use

    // Find the first available slot for the new breakpoint
    // This call cannot fail - if there is no free bp slots, we just
    // steal the last one, oh well...

    for(index=0; index<MAX_BREAKPOINTS-1; index++ )
    {
        if( !(bp[index].Flags & BP_USED) )
            break;
    }

    // `index' now selects a breakpoint slot
    pBp = &bp[index];               // Assign the pointer to a new bp

    // If we had to steal the last slot, need to free it
    if( pBp->Flags & BP_USED )
    {
        // Free the command line of a breakpoint and IF/DO statements
        freeHeap(deb.hHeap, pBp->pCmd);
    }

    // Clear the breakpoint entry since we will rebuild it
    memset(pBp, 0, sizeof(TBP));

    pBp->Flags = BP_USED | BP_ENABLED | BP_ONETIME | BP_INTERNAL;
    pBp->Type  = BP_TYPE_BPX;
    pBp->address = Addr;
    // We dont allocate the command buffer for internal breakpoints
}


/******************************************************************************
*                                                                             *
*   BOOL IsBreakpointAtCSIP(void)                                             *
*                                                                             *
*******************************************************************************
*
*   This function supports a peculiar case where a breakpoint might be
*   placed at the address of the current cs:eip and the running it would
*   break immediately. We check all the BPX-type breakpoints.
*
*   TODO - This is really only needed for the INT3 embedded style BPX since the HW breakpoints are supressed by the RF flag.
*
*
*   Returns:
*       TRUE - a bpx-type breakpoint was found at the cs:eip
*       FALSE - no bpx-type breakpoints at this address
*
******************************************************************************/
BOOL IsBreakpointAtCSIP(void)
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
*   void ArmBreakpoints(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   This function arms all breakpoints. This is done before returning
*   control to the debugee.
*
*   Special case is if a breakpoint exists at the current cs:eip, and we cannot
*   use hardware DR register, in that case we enter the state of a delayed arm
*   (we do a single step after which we arm all breakpoints and continue)
*
******************************************************************************/
void ArmBreakpoints(void)
{
    // Translation from DR bitfield into DR register number
    static const UINT bDr[] = {  0, 0, 1, 0, 2, 0, 0, 0, 3  };

    int index;                          // Temp breakpoint index
    BYTE avail;                         // Available hw breakpoints
    UINT dr;                            // Temp debug register


    // Reset debug registers
    deb.sysReg.dr7 = 3 << 8;            // LE GE set recommended

    // Before returning control to the debugee, need to clear the current breakpoint
    // values of hits and misses (only for the breakpoint that actually hit this time)
    if( deb.bpIndex>=0 )
    {
        bp[deb.bpIndex].CurHits = 0;
        bp[deb.bpIndex].CurMisses = 0;
    }

    // If the breakpoint would be placed at the current cs:eip..
    if( IsBreakpointAtCSIP() )
    {
        // .. enter the delayed arm mode

        deb.fDelayedArm = TRUE;

        // Set the single-step trace mode
        deb.r->eflags |= TF_MASK;
    }
    else
    {
        avail = 0xF;                    // All 4 DR registers are available

        //=================================================================================
        // Resource allocation
        //=================================================================================

        // Pass I: Clear all in-use masks since we will start allocation
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( (bp[index].Flags & BP_ENABLED) )
            {
                bp[index].DrUse = 0;
            }
        }


        // Pass II: Arm all breakpoints that have a hardware resource already assigned
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( (bp[index].Flags & BP_ENABLED) && (bp[index].DrRequest) )
            {
                bp[index].DrUse = bp[index].DrRequest;
                avail ^= bp[index].DrUse;
            }
        }

        // Pass III: Allocate available to those that need it, if they did not get it yet
        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( (bp[index].Flags & BP_ENABLED) && IsNeedHwBp(bp[index].Type) && (bp[index].DrUse==0) )
            {
                // Assign the first available

                if( avail & 1 ) bp[index].DrUse = 1;
                else
                if( avail & 2 ) bp[index].DrUse = 2;
                else
                if( avail & 4 ) bp[index].DrUse = 4;
                else
                if( avail & 8 ) bp[index].DrUse = 8;

                avail ^= bp[index].DrUse;
            }
        }

        // Pass IV: While there is more availables, distribute them out
        index = 0;
        while( index<MAX_BREAKPOINTS && avail )
        {
            if( (bp[index].Flags & BP_ENABLED) && (bp[index].DrUse==0) )
            {
                // Assign the first available

                if( avail & 1 ) bp[index].DrUse = 1;
                else
                if( avail & 2 ) bp[index].DrUse = 2;
                else
                if( avail & 4 ) bp[index].DrUse = 4;
                else
                if( avail & 8 ) bp[index].DrUse = 8;

                avail ^= bp[index].DrUse;
            }

            index++;
        }

        //=================================================================================
        // Setting the breakpoints
        //=================================================================================

        for(index=0; index<MAX_BREAKPOINTS; index++ )
        {
            if( bp[index].Flags & BP_ENABLED )
            {
                switch( bp[index].Type )
                {
                    case BP_TYPE_BPX:
                        if( bp[index].DrUse )
                        {
                            // This BPX breakpoint is using a hw resource

                            dr = bDr[bp[index].DrUse];      // Debug register number

                            // Set the address to trap on into a debug register
                            deb.sysReg.dr[ dr ] = bp[index].address.offset;

                            // Enable that breakpoint
                            // We dont need to do anything special for execute since DR7
                            // fields should be set to 0
                            //deb.sysReg.dr7 |= 0 << (dr*4 + 16); // Execute Breakpoint

                            deb.sysReg.dr7 |= 3 << (dr*2);  // Global + Local flag
                        }
                        else
                        {
                            // This BPX breakpoint is using embedded INT3

                            // Save the original byte and place INT3 opcode
                            bp[index].origValue = AddrGetByte(&bp[index].address);
                            AddrSetByte(&bp[index].address, 0xCC, TRUE);
                        }
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
*   void DisarmBreakpoints(void)                                              *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugger gets control to remove all
*   breakpoints that it had placed, so they dont obstruct the view.
*
*   It is also called at the point of unloading Linice in order to unhook the
*   running kernel from all debugger triggers.
*
*   The other major function is to store the index of the breakpoint that may
*   hit by examining effective address and hw debug register state.
*
*   The index of such breakpoint is stored in deb.bpIndex,
*       if no hit is detected, -1 is stored.
*
*                       -  -  -
*
*   If we break due to our breakpoint INT3, decrement eip to position it on the
*   valid instruction start that has been overwritten with 0xCC.
*
*   Debug registers will be cleared.
*
******************************************************************************/
void DisarmBreakpoints(void)
{
    int index;

    // Disarm the opposite way to allow possible duplicate breakpoints

    for(index=MAX_BREAKPOINTS-1; index>=0; index-- )
    {
        if( bp[index].Flags & BP_ENABLED )
        {
            switch( bp[index].Type )
            {
                case BP_TYPE_BPX:
                    // Execute breakpoint may use INT3 or hardware resource
                    if( bp[index].DrUse )
                    {
                        if( (deb.sysReg.dr6 & 0xF) & bp[index].DrUse )
                        {
                            // If the current cs:eip matches the bpx address, set the index
                            // We do that additional step for the cases where we simply used a HW BP
                            // which aliases with a regular DR-assigned breakpoint (like module loading)

                            if( deb.r->cs==bp[index].address.sel && deb.r->eip==bp[index].address.offset )
                                if( !(bp[index].Flags & BP_INTERNAL) )  // Dont set index on internal bps!
                                    deb.bpIndex = index;
                        }
                    }
                    else
                    {
                        // Restore original value only if we found INT3 there
                        if( AddrGetByte(&bp[index].address)==0xCC )  // Was it INT3?
                        {
                            // Restore original value
                            AddrSetByte(&bp[index].address, bp[index].origValue, TRUE);

                            // If the current cs:eip-1 matches the bpx address, set the index
                            if( deb.r->cs==bp[index].address.sel && deb.r->eip-1==bp[index].address.offset )
                                if( !(bp[index].Flags & BP_INTERNAL) )  // Dont set index on internal bps!
                                    deb.bpIndex = index;

                            // Since we restored original byte from INT3 opcode, need to rewind the EIP
                            deb.r->eip -= 1;
                        }
                    }

                    // As we loop over all enabled breakpoints, clear all internal ones since every
                    // time we hit a bp, we dont want them active any more. Those are step call-bypass
                    // and G=xxx command.
                    // If we hit a one-time, internal breakpoint, we will clear it here, so it will be

                    if( bp[index].Flags & BP_INTERNAL )
                    {
                        // Free the hardware bp resource

                        if( bp[index].DrRequest )
                            GlRequest ^= bp[index].DrRequest, hwbpinuse--;
                        else
                            if( IsNeedHwBp(bp[index].Type) )
                                hwbpinuse--;

                        // Clear the breakpoint entry - internal breakpoints dont have the command string
                        memset(&bp[index], 0, sizeof(TBP));
                    }

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

    // For verification purposes, save debug registers away
    dr[0] = deb.sysReg.dr[0];
    dr[1] = deb.sysReg.dr[1];
    dr[2] = deb.sysReg.dr[2];
    dr[3] = deb.sysReg.dr[3];
    dr[6] = deb.sysReg.dr6;
    dr[7] = deb.sysReg.dr7;

    // Clear the DR6 register since CPU never does it
    deb.sysReg.dr6 = 0;

    // Clear the DR7 register since it needs to be rebuilt anyways.
    // Also, we dont want to trigger hardware breakpoints while we are in the debugger
    deb.sysReg.dr7 = 0;

    SetSysreg(&deb.sysReg);             // Write out DR7
}


/******************************************************************************
*                                                                             *
*   BOOL EvalBreakpoint(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   This function is called before debugger gets control to evaluate a single
*   breakpoint that hit. All breakpoint counters will be adjusted.
*
*   The breakpoint that hit is read from deb.bpIndex variable.
*
*   Returns:
*       TRUE - evaluation requires to continue execution
*       FALSE - stop in the debugger
*
******************************************************************************/
BOOL EvalBreakpoint(void)
{
    TBP *p;                             // Pointer to the breakpoint that hit
    DWORD result;                       // Result of evaluation
    BOOL fPopup;                        // Result of breakpoint DO evaluation

    // Evaluate only if we hit a breakpoint
    if( deb.bpIndex>=0 )
    {
        p = &bp[deb.bpIndex];

        // If we hit a one-time breakpoint, clear it
        if( p->Flags & BP_ONETIME )
        {
            dprinth(1, "Breakpoint due to one-time BPX %02X, cleared.", deb.bpIndex);

            // Free the hardware bp resource
            if( p->DrRequest )
                GlRequest ^= p->DrRequest, hwbpinuse--;
            else
                if( IsNeedHwBp(p->Type) )
                    hwbpinuse--;

            // Free the command line of a breakpoint and IF/DO statements
            freeHeap(deb.hHeap, p->pCmd);

            // Clear the breakpoint entry
            memset(p, 0, sizeof(TBP));
        }
        else
        {
            dprinth(1, "Breakpoint due to BPX %02X", deb.bpIndex);

            p->Hits++;
            p->CurHits++;

            // If this breakpoint has IF condition, evaluate it now
            if( p->pIF )
            {
                if( Expression(&result, p->pIF, NULL) )
                {
                    if( result )
                    {
                        // Result is non-zero, means TRUE.
                        // If the DO part was given, evaluate it as a command stream
                        if( p->pDO )
                        {
                            fPopup = CommandExecute(p->pDO);

                            if( fPopup==FALSE )
                            {
                                // Command evaluation required to continue execution

                                p->CurMisses++;

                                return( TRUE );
                            }
                        }
                        else
                        {
                            // No DO portion and it evaluated to TRUE, so break and popup

                            p->Popups++;
                            p->Breaks++;
                        }
                    }
                    else
                    {
                        // Result is zero, means FALSE, we will continue
                        p->Misses++;        // Evaluated to FALSE and miss
                        p->CurMisses++;     // Current misses

                        return( TRUE );     // Return to debugee
                    }
                }
                else
                {
                    p->Errors++;            // Expression resulted in error in evaluation
                }
            }
            else
            {
                // The breakpoint does not have IF condition; if it has DO portion, execute it unconditionally

                if( p->pDO )
                {
                    fPopup = CommandExecute(p->pDO);

                    if( fPopup==FALSE )
                    {
                        // Command evaluation required to continue execution

                        p->CurMisses++;

                        return( TRUE );     // Return to debugee
                    }
                }
                else
                {
                    // No DO portion and it evaluated to TRUE, so break and popup

                    p->Popups++;                    // We will popup now
                    p->Breaks++;
                }
            }
        }
    }

    return( FALSE );        // Popup in the debugger
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
*   This function is called from the code source listing to signal a breakpoint
*   within a source code line or address.
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

            // BPIO does not have selector part set valid, so dont copy it
            if( bp[index].Type!=BP_TYPE_BPIO )
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
*   void ArmDebugReg(int nDr, TADDRDESC Addr)                                 *
*                                                                             *
*******************************************************************************
*
*   Arms a specified debug register for execution only.
*
*   This function is called from the symbol loading module to set a quick
*   breakpoint at the init_module address, which should trigger immediately.
*
*   Where:
*       nDr (0,1,2,3) selects a particular debug reg
*       Addr is the sel:offset of the address to arm
*
******************************************************************************/
void ArmDebugReg(int nDr, TADDRDESC Addr)
{
    DWORD Linear;

    // Calculate the linear address of the breakpoint
    // TODO: We will again assume our selector is zero-based
    Linear = Addr.offset;

    // BPX breakpoint
    deb.sysReg.dr[nDr] = Linear;

    // Set breakpoint on execution only (value 0)
    deb.sysReg.dr7 &= ~(3 << ((nDr*4)+16));

    // Set global and local flag to actually enable a breakpoint
    deb.sysReg.dr7 |= 3 << (nDr*2);
}

