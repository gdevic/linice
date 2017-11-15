/******************************************************************************
*                                                                             *
*   Module:     code.c                                                        *
*                                                                             *
*   Date:       05/16/00                                                      *
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

        This module contains disassembly command

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/16/00   Original                                             Goran Devic *
* 09/11/00   Second edition                                       Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
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

#define CODE_BYTES         8

static char buf[MAX_STRING+1];
static char sCodeLine[MAX_STRING+1];

// We do ourselves a favor and keep the last disassembled address here
// so on the next call we simply use that one
static TADDRDESC Addr = { 0, 0 };

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int BreakpointQuery(TADDRDESC Addr);
extern BOOL cmdGo           (char *args, int subClass);      // flow.c

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BYTE GetCodeLine(PTADDRDESC pAddr, BOOL fDecodeExtra, BOOL fTarget)       *
*                                                                             *
*******************************************************************************
*
*   Prints a disassembly line into the sCodeLine buffer, a global string buffer.
*
*   Where:
*       pAddr is the address of the requested code
*       fDecodeExtra if we are able to print some extra info (jump/no jump...)
*       fTarget if the current address is a jump target
*
*   Returns:
*       size in bytes of the addressed instruction opcode
*
*       sCodeLine contains the disassembly of the pAddr-addressed instruction
*
******************************************************************************/
static BYTE GetCodeLine(PTADDRDESC pAddr, BOOL fDecodeExtra, BOOL fTarget)
{
    TDISASM dis;
    int i, pos;
    BYTE bLen;
    int eCondJump;                      // Conditional jump condition index
    BOOL fJump;                         // Conditional jump, jump decision
    DWORD eflags;                       // Temp deb.r->eflags
    DWORD ecx;                          // Temp cx or ecx for conditional jump

    // Disassemble the current instruction

    dis.bState  = DIS_DATA32 | DIS_ADDRESS32;
    dis.wSel = pAddr->sel;
    dis.dwOffset = pAddr->offset;
    dis.szDisasm = buf;

    // Disassemble and store into the line buffer
    Disassembler( &dis );

    bLen = dis.bInstrLen;

    // If a current instruction is a jump target, print "==>" instead of a selector value
    if( fTarget )
        pos = sprintf(sCodeLine, " ==> %08X  ",             pAddr->offset);
    else
        pos = sprintf(sCodeLine, "%04X:%08X  ", pAddr->sel, pAddr->offset);

    // If CODE was ON, print the code bytes
    if( deb.fCode )
    {
        for( i=0; i<bLen && i<CODE_BYTES; i++ )
        {
            pos += sprintf(sCodeLine+pos, "%02X", dis.bCodes[i]);
        }

        // If we had more code bytes to print, but not enough space,
        // print trailing '+'
        if( bLen > CODE_BYTES )
            pos += sprintf(sCodeLine+pos, "+ ");
        else
            pos += sprintf(sCodeLine+pos, "  ");

        // Append spaces, if necessary
        while( i++ < CODE_BYTES )
        {
            pos += sprintf(sCodeLine+pos, "  ");
        }
    }

    // Append the disassembly buffer into the final sCodeLine buffer
    strcat(sCodeLine, buf);

    // If we are allowed to print some extra information, do it here:
    if( fDecodeExtra )
    {
        // If the instruction is a conditional jump, decipher if we will jump or not
        if( (dis.bFlags & SCAN_MASK) == SCAN_COND_JUMP )
        {
            // There are 3 blocks of instructions with SCAN_COND_JUMP:
            //  1) Opcodes 0x70 - 0x7F                       |
            //  2) Opcode 0x0F followed by 0x80 - 0x8F       / same pattern
            //  3) 4 Opcodes at 0xE0 - 0xE4

            if( dis.bCodes[0]>=0x70 && dis.bCodes[0]<=0x7F )
                eCondJump = dis.bCodes[0]-0x70;
            else
            if( dis.bCodes[0]==0x0F && dis.bCodes[1]>=0x80 && dis.bCodes[1]<=0x8F )
                eCondJump = dis.bCodes[1]-0x80;
            else
            if( dis.bCodes[0]>=0xE0 && dis.bCodes[0]<=0xE3 )
                eCondJump = dis.bCodes[0]-0xE0 + 16;
            else
                eCondJump = -1;         // We have a bug??

            fJump = FALSE;              // Assume we will not jump
            eflags = deb.r->eflags;

            // Assign the proper width of the ecx register to compare
            if( dis.bState & DIS_DATA32 )
                ecx = deb.r->ecx;
            else
                ecx = deb.r->ecx & 0xFFFF;

            switch( eCondJump )
            {
                case 0x00: /* jo   */ if(  eflags&OF_MASK     ) fJump = TRUE;  break;
                case 0x01: /* jno  */ if( (eflags&OF_MASK)==0 ) fJump = TRUE;  break;
                case 0x02: /* jb   */ if(  eflags&CF_MASK     ) fJump = TRUE;  break;
                case 0x03: /* jnb  */ if( (eflags&CF_MASK)==0 ) fJump = TRUE;  break;
                case 0x04: /* jz   */ if(  eflags&ZF_MASK     ) fJump = TRUE;  break;
                case 0x05: /* jnz  */ if( (eflags&ZF_MASK)==0 ) fJump = TRUE;  break;
                case 0x06: /* jbe  */ if(  eflags&CF_MASK    ||   eflags&ZF_MASK     ) fJump = TRUE;  break;
                case 0x07: /* jnbe */ if( (eflags&CF_MASK)==0 && (eflags&ZF_MASK)==0 ) fJump = TRUE;  break;
                case 0x08: /* js   */ if(  eflags&SF_MASK     ) fJump = TRUE;  break;
                case 0x09: /* jns  */ if( (eflags&SF_MASK)==0 ) fJump = TRUE;  break;
                case 0x0A: /* jp   */ if(  eflags&PF_MASK     ) fJump = TRUE;  break;
                case 0x0B: /* jnp  */ if( (eflags&PF_MASK)==0 ) fJump = TRUE;  break;
                case 0x0C: /* jl   */ if( SF_VALUE(eflags) != OF_VALUE(eflags) ) fJump = TRUE;  break;
                case 0x0D: /* jnl  */ if( SF_VALUE(eflags) == OF_VALUE(eflags) ) fJump = TRUE;  break;
                case 0x0E: /* jle  */ if(  eflags&ZF_MASK     || (SF_VALUE(eflags) != OF_VALUE(eflags)) ) fJump = TRUE;  break;
                case 0x0F: /* jnle */ if( (eflags&ZF_MASK)==0 && (SF_VALUE(eflags) == OF_VALUE(eflags)) ) fJump = TRUE;  break;

                case 0x10: /* lpne */ if( (eflags&ZF_MASK)==0 && (ecx!=1) ) fJump = TRUE;  break;
                case 0x11: /* lpe  */ if(  eflags&ZF_MASK     && (ecx!=1) ) fJump = TRUE;  break;
                case 0x12: /* loop */ if( ecx!=1 ) fJump = TRUE;  break;
                case 0x13: /* jcxz */ if( ecx==0 ) fJump = TRUE;  break;
            }

            // Finally, append the resulting string
            if( fJump==FALSE )
                sprintf(buf, "\r%c(no jump)", DP_RIGHTALIGN);
            else
                sprintf(buf, "\r%c(jump %s)", DP_RIGHTALIGN, dis.dwTargetAddress < pAddr->offset? "back" : "ahead" );

            strcat(sCodeLine, buf);
        }
        else
        // If the instruction is a simple jump, just print out the direction of jump
        if( (dis.bFlags & SCAN_MASK) == SCAN_JUMP )
        {
            sprintf(buf, "\r%c(jump %s)", DP_RIGHTALIGN, dis.dwTargetAddress < pAddr->offset? "back" : "ahead" );

            strcat(sCodeLine, buf);
        }
    }

    // Since by default all the strings are lowercased, make the string uppercased
    // if the current option was _not_ lowercase
    if( deb.fLowercase==FALSE )
    {
        strupr(sCodeLine);
    }

    return( bLen );
}


/******************************************************************************
*                                                                             *
*   DWORD GetCodeLines()                                                      *
*                                                                             *
*******************************************************************************
*
*   Returns the number of lines that we need to print out.
*
******************************************************************************/
static DWORD GetCodeLines()
{
    // If code window is visible, we will advance so many lines of code
    if( pWin->c.fVisible )
        return( pWin->c.nLines - 1 );

    // Code window is not visible, so advance 8 or (history height-1) code lines
    // This is in the cases where we just dump source to the history window
    if( pWin->h.nLines > 8 )
        return( 8 );

    return( pWin->h.nLines - 1 );
}


/******************************************************************************
*                                                                             *
*   char *GetSourceLine(WORD *pwLine, PTADDRDESC pAddr)                       *
*                                                                             *
*******************************************************************************
*
*   For the given address, returns the pointer to the source file.
*
*   Where:
*       pwLine is the address of the variable receiving the line number
*       pAddr is the address to query
*
*   Returns:
*       Pointer to the source line; *pwLine is set to the exact line number
*       NULL if the address does not correspond to any source line
*
******************************************************************************/
char *GetSourceLine(WORD *pwLine, PTADDRDESC pAddr)
{
    TSYMFNLIN *pFnLin;
    char *pLine;

    // Search for the source ID of the addressed function
    pFnLin = SymAddress2FnLin(pAddr->sel, pAddr->offset);

    if( pFnLin )
    {
        // Found the function that contains our given address, find the offset
        // inside the function that would reveal the source fild ID and line number

        pLine = SymFnLin2LineExact(pwLine, pFnLin, pAddr->offset);

        return( pLine );
    }

    // No source line found
    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void CodeDraw(BOOL fForce)                                                *
*                                                                             *
*******************************************************************************
*
*   Prints out a block of code lines.
*
*   Where:
*       fForce will force redraw even if source window is not open (dray into history buffer)
*
******************************************************************************/
void CodeDraw(BOOL fForce)
{
    TDISASM dis;                        // Disassembler interface structure
    BOOL fTarget;                       // Current address is a jump target
    int maxLines;
    int nLen, nLine=1;
    char col;
    char *pLine;                        // Pointer to a source line
    int eMode = deb.eSrc;               // Get the code view mode
    WORD wLine;                         // Line number
    BYTE bSpaces;                       // Number of heading spaces in a line

    if( pWin->c.fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pWin->c.Top+1);

        if( deb.pSource )
            PrintLine(deb.pSource->pSourceName);
        else
            PrintLine("Code");
    }
    else
        if( fForce==FALSE )
            return;

    // If code window is not visible and we are forcing the unassembly,
    // set the mode to machine disassemble
    if( fForce && pWin->c.fVisible==FALSE )
        eMode = 0;                      // Machine disassembly

    Addr = deb.codeTopAddr;             // Copy the current code address
    maxLines = GetCodeLines();
    deb.CodeEditAddress.sel = 0;        // Zero out target address for the code edit mode
    deb.CodeEditAddress.offset = 0;

    // Depending on the source mode, we display source file or basically
    // a machine code with optional mixed source lines
    if( eMode==1 && deb.pFnScope && deb.pFnLin && deb.pSource )
    {
        // SOURCE CODE LINES
        //================================================================

        // Our line count wLine is 0-based, while deb.codeFileTopLine is 1-based
        wLine = deb.codeFileTopLine - 1;

        // Loop and print all the source lines
        while( (nLine <= maxLines) && (wLine < deb.pSource->nLines) )
        {
            // Get the address that corresponds to the source at the current line number
            Addr.offset = SymLinNum2Address(wLine+1);

            pLine = deb.pSource->pLineArray[wLine];

            bSpaces = *(BYTE *)pLine;

            // Set a number of heading spaces into the final line buffer
            memset(buf, ' ', bSpaces);

            // Copy the source line into the buffer to be printed out
            strcpy(buf+bSpaces, pLine+1);

            // Finally, reset the pointer to line into our buffer
            pLine = buf;

            // If the current file in the window is the one containing the current function
            // scope, and the line number is the current EIP line number, invert the line color
            if( (deb.pSource->file_id==deb.pFnScope->file_id) && (wLine==(deb.codeFileEipLine-1)) )
                col = COL_REVERSE;
            else
            {
                // If the current file_id and line contains a breakpoint, highlight it
                if( BreakpointQuery(Addr) >= 0 )
                    col = COL_BOLD;
                else
                    col = COL_NORMAL;
            }

            // Make X offset adjustments to the line
            if( deb.codeFileXoffset > strlen(pLine) )
                pLine = "";
            else
                pLine += deb.codeFileXoffset;

            dprinth(nLine++, "%c%c%05d:%s\r", DP_SETCOLINDEX, col, wLine + 1, pLine);

            // For the code edit mode store the address (if valid) if the Y line numbers match
            if( deb.nCodeEditY==nLine-2 && Addr.offset)
                deb.CodeEditAddress = Addr;

            wLine++;
        }

        // If we run out of source lines, fill the rest with empty lines
        if( !(wLine < deb.pSource->nLines) )
        {
            while(nLine <= maxLines)
            {
                dprinth(nLine++, "\r");
            }
        }
    }
    else
    {
        // MACHINE CODE DISASSEMBLY or MIXED SOURCE LINES AND DISASSEMBLY
        //================================================================

        // First run a quick scan over the current instruction in order to pick
        // up jump or conditional jump destination address, so we can print "==>" mark

        dis.bState   = DIS_DATA32 | DIS_ADDRESS32;
        dis.wSel     = deb.r->cs;
        dis.dwOffset = deb.r->eip;
        DisassemblerLen(&dis);
        dis.bFlags &= SCAN_MASK;        // Isolate only jump flags

        // Now we keep possible jump destination address in dis.dwTargetAddress, and
        // the jump state flag in dis.bFlags

        while( nLine <= maxLines )
        {
            // Determine if the current instruction line is a target to a jump or conditional jump
            if( dis.dwTargetAddress==Addr.offset && (dis.bFlags==SCAN_COND_JUMP || dis.bFlags==SCAN_JUMP) )
                fTarget = TRUE;
            else
                fTarget = FALSE;

            // If the address is the current CS:EIP, invert the line color;
            // In the same token, send the argument to signal the current cs:eip
            if( Addr.sel==deb.r->cs && Addr.offset==deb.r->eip )
                col = COL_REVERSE,
                nLen = GetCodeLine(&Addr, TRUE, fTarget);
            else
            {
                // If a current address contains a breakpoint, highlight it
                if( BreakpointQuery(Addr) >= 0 )
                    col = COL_BOLD;
                else
                    col = COL_NORMAL;

                nLen = GetCodeLine(&Addr, FALSE, fTarget);
            }

            pLine = sCodeLine;              // Print the default disassembly

            // If the source mode is mixed machine code and disassembly,
            // insert the source code line before disassembly
            if( eMode==2 )
            {
                // If the source code line is not found, print the default disassembly
                if( (pLine = GetSourceLine(&wLine, &Addr))==NULL )
                    pLine = sCodeLine;
                else
                {
                    pLine = pLine + 1;      // Actual line starts after the number of spaces, remember?

                    // If there is a source line, print it first, and then continue
                    // with disassembly. Add few spaces to align the line somewhat nicer.
                    dprinth(nLine++,"%c%c%05d:       %s\r", DP_SETCOLINDEX, col, wLine, pLine);

                    if( nLine <= maxLines )
                        pLine = sCodeLine;
                    else
                        return;
                }

                // For the code edit mode store the address if the Y line numbers match
                if( deb.nCodeEditY==nLine-2 )
                    deb.CodeEditAddress = Addr;
            }

            if(dprinth(nLine++, "%c%c%s\r", DP_SETCOLINDEX, col, pLine)==FALSE)
                break;

            // For the code edit mode store the address if the Y line numbers match
            if( deb.nCodeEditY==nLine-2 )
                deb.CodeEditAddress = Addr;

            // Keep the running bottom address
            deb.codeBottomAddr = Addr;

            // Advance machine code offset for the next line
            Addr.offset += nLen;
        }
    }

    if( pWin->c.fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}


/******************************************************************************
*                                                                             *
*   void CodeScrollLineUp(TDISASM *pDis)                                      *
*                                                                             *
*******************************************************************************
*
*   Subfunction that scrolls a code window one line up:
*
*   Scrolling up is really tricky with the Intel x86 machine code.
*   We use the assumption that if you disassemble a lot of code, it
*   eventually 'fixes' itself.
*
*   TODO: We could also look up for a symbol in a symbol table that is close
*
******************************************************************************/
void CodeScrollLineUp(TDISASM *pDis)
{
#define MAX_UNASM_BACKTRACE     64      // How many bytes we unassemble to find the start
    BYTE bSizes[MAX_UNASM_BACKTRACE];
    int i;                          // Generic counter

    pDis->bState   = DIS_DATA32 | DIS_ADDRESS32;
    pDis->wSel     = deb.codeTopAddr.sel;
    pDis->dwOffset = deb.codeTopAddr.offset - MAX_UNASM_BACKTRACE;

    i = 0;
    while( pDis->dwOffset < deb.codeTopAddr.offset )
    {
        bSizes[i] = DisassemblerLen(pDis);

        pDis->dwOffset += bSizes[i];

        i++;
    }

    deb.codeTopAddr.offset -= bSizes[i-1];
}

/******************************************************************************
*                                                                             *
*   DWORD fnCodeAddr(DWORD arg)                                               *
*                                                                             *
*******************************************************************************
*
*   Expression evaluator helper function to return the address of the first
*   instruction displayed in the code window.
*
******************************************************************************/
DWORD fnCodeAddr(DWORD arg)
{
    return( deb.codeTopAddr.offset );
}

/******************************************************************************
*                                                                             *
*   void CodeScroll(int Xdir, int Ydir)                                       *
*                                                                             *
*******************************************************************************
*
*   This function is called from the edlim module to scroll code window.
*   It will repaint code window only if visible.
*
*   Where:
*       Xdir =  1 one character to the left
*              -1 one character to the right
*
*       Ydir = -2 one page back
*              -1 one line back
*               1 one line forth
*               2 one page forth
*              -3 top of source
*               3 end of source
*
******************************************************************************/
void CodeScroll(int Xdir, int Ydir)
{
    TDISASM Dis;                        // Disassembler interface structure
    BYTE bLen;                          // Temp instruction len
    int i;                              // Generic counter

    if( pWin->c.fVisible )
    {
        // If we have source code, scrolling is different than machine code
        if( deb.eSrc==SRC_ON && deb.pSource )
        {
            // Code window contains a source file
            //====================================
            switch( Xdir )
            {
                case 1:         // One character to the left
                    if( deb.codeFileXoffset < MAX_STRING-80 )
                        deb.codeFileXoffset += 1;
                    break;

                case -1:        // One character to the right
                    if( deb.codeFileXoffset > 0 )
                        deb.codeFileXoffset -= 1;
                    break;
            }

            switch( Ydir )
            {
                case -3:        // Top of source - both X and Y
                    deb.codeFileTopLine = 1;
                    deb.codeFileXoffset = 0;
                    break;

                case -2:        // One page up
                    if( deb.codeFileTopLine >= pWin->c.nLines-1 )
                        deb.codeFileTopLine -= pWin->c.nLines-1;
                    else
                        deb.codeFileTopLine = 1;
                    break;

                case -1:        // One line up
                    if( deb.codeFileTopLine > 1 )
                        deb.codeFileTopLine -= 1;
                    break;

                case 1:         // One line down
                    if( deb.codeFileTopLine < deb.pSource->nLines )
                        deb.codeFileTopLine += 1;
                    break;

                case 2:         // One page down
                    if( deb.codeFileTopLine+pWin->c.nLines-1 <= deb.pSource->nLines )
                        deb.codeFileTopLine += pWin->c.nLines-1;
                    break;

                case 3:         // End of source
                    if( deb.pSource->nLines < pWin->c.nLines-1 )
                        deb.codeFileTopLine = 1;
                    else
                        deb.codeFileTopLine = deb.pSource->nLines - (pWin->c.nLines-1) + 1;
                    break;
            }
        }
        else
        {
            // Code window contains the machine code or mixed
            //================================================
            switch( Ydir )
            {
                case -3:        // Home
                    break;

                case -2:        // One page up
                    // Scrolling up is really tricky with the Intel x86 machine code.
                    // We use the assumption that if you disassemble a lot of code, it
                    // eventually 'fixes' itself.

                    // We step back a number of bytes and store the instruction sizes
                    for(i=0; i<pWin->c.nLines-1; i++)
                    {
                        CodeScrollLineUp(&Dis);
                    }

                    break;

                case -1:        // One line up
                    // Scrolling up is really tricky with the Intel x86 machine code.
                    // We use the assumption that if you disassemble a lot of code, it
                    // eventually 'fixes' itself.

                    // We step back a number of bytes and store the instruction sizes
                    CodeScrollLineUp(&Dis);

                    break;

                case 1:         // One line down
                    // Add the current instruction len to the offset

                    Dis.wSel     = deb.codeTopAddr.sel;
                    Dis.dwOffset = deb.codeTopAddr.offset;
                    Dis.bState   = DIS_DATA32 | DIS_ADDRESS32;

                    bLen = DisassemblerLen(&Dis);
                    deb.codeTopAddr.offset += bLen;

                    break;

                case 2:         // One page down
                    // Roll and get lengths of all instructions in the code window so
                    // we can continue at the bottom

                    Dis.wSel     = deb.codeTopAddr.sel;
                    Dis.dwOffset = deb.codeTopAddr.offset;
                    Dis.bState   = DIS_DATA32 | DIS_ADDRESS32;

                    for(i=0; i<pWin->c.nLines-1; i++)
                    {
                        Dis.dwOffset += DisassemblerLen(&Dis);
                    }

                    deb.codeTopAddr.offset = Dis.dwOffset;

                    break;

                case 3:         // End
                    break;
            }
        }

        // Finally, redraw the code window
        CodeDraw(FALSE);
    }
}

/******************************************************************************
*                                                                             *
*   void CodeCursorScroll(int Ydir)                                           *
*                                                                             *
*******************************************************************************
*
*   This function is called from the edlim module in the code edit mode,
*   to move cursor one line up or down, possibly scrolling the content of the
*   source window.
*
*   Where:
*       Ydir = -1 one line back
*               1 one line forth
*
******************************************************************************/
void CodeCursorScroll(int Ydir)
{
    switch( Ydir )
    {
        case -1:        // Cursor up, possibly scroll code window one line down
            if( deb.nCodeEditY )
                deb.nCodeEditY--;
            else
                // We are already at the first line, scroll the code window
                CodeScroll(0, -1);
            break;

        case 1:         // Cursor down, possibly scroll code window one line up
            if( deb.nCodeEditY < pWin->c.nLines-2 )
                deb.nCodeEditY++;
            else
                // We are at the last line, scroll the window down
                CodeScroll(0, 1);
            break;
    }

    // Redraw the source window so the address variables are adjusted
    CodeDraw(FALSE);
}

/******************************************************************************
*                                                                             *
*   BOOL cmdEnterCode(char *args, int subClass)                               *
*                                                                             *
*******************************************************************************
*
*   Moves the cursor to the code window. This allows setting a breakpoint
*   in place (with F9 key) or executing up to the line (with F7 key), etc.
*
*   No parameters.
*
******************************************************************************/
BOOL cmdEnterCode(char *args, int subClass)
{
    // Check for no paremeters
    if( !*args )
    {
        // Toggle the code edit window
        if( deb.fCodeEdit )
        {
            // Code edit is already active, deactivate it
            deb.fCodeEdit = FALSE;
        }
        else
        {
            // Code edit is not active, activate it

            // If the code window is not visible, make it visible
            if( pWin->c.fVisible==FALSE )
            {
                // Make a window visible and redraw whole screen
                pWin->c.fVisible = TRUE;
                deb.fRedraw = TRUE;
            }

            deb.fCodeEdit = TRUE;
            deb.nCodeEditY = 0;
        }
    }
    else
        PostError(ERR_SYNTAX, 0);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdHere(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Executes code up to the selected line or address. The code edit mode has to
*   be set (EC / F6) in order to select a line.
*
*   No parameters.
*
******************************************************************************/
BOOL cmdHere(char *args, int subClass)
{
    // Check for no paremeters
    if( !*args )
    {
        // We have to be in the code edit mode for this command to work
        if( deb.fCodeEdit )
        {
            // If the valid address is selected, execute up to that address
            if( deb.CodeEditAddress.sel )
            {
                // Set up a one-time internal breakpoint at the selected address
                // Instead of repeating the code, we use command "G" to run this command.

                sprintf(buf, "%04X:%08X", deb.CodeEditAddress.sel, deb.CodeEditAddress.offset);

                cmdGo(buf, 0);

                return( FALSE );
            }
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
        PostError(ERR_SYNTAX, 0);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdUnasm(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Disassemble command. If the code window is up, it will use whatever mode
*   it is currently active (code, source or mixed). If the code window is not
*   up, it will only machine disassemble into the history window.
*
*   U <address> [L len]
*
******************************************************************************/
BOOL cmdUnasm(char *args, int subClass)
{
    DWORD offset;                       // Temporary offset portion

    if( *args!=0 )
    {
        // Argument present: U <address> [L <len>]
        evalSel = deb.codeTopAddr.sel;
        if( Expression(&offset, args, &args) )
        {
            // Verify the selector value
            if( VerifySelector(evalSel) )
            {
                deb.codeTopAddr.offset = offset;
                deb.codeTopAddr.sel = evalSel;
            }
            else
                return( TRUE );
        }
    }
    else
    {
        // No arguments - advance current address one screenful
        // We saved the offset at which previous disassembly ended up
        deb.codeTopAddr.offset = Addr.offset;
    }

    CodeDraw(TRUE);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdDot(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Display current instruction. Equal to 'U eip' command.
*
******************************************************************************/
BOOL cmdDot(char *args, int subClass)
{
    // Basically reset the code view to what comes by default on break

    SetSymbolContext(deb.r->cs, deb.r->eip);

    CodeDraw(TRUE);

    return( TRUE );
}

