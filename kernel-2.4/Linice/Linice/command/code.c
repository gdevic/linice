/******************************************************************************
*                                                                             *
*   Module:     code.c                                                        *
*                                                                             *
*   Date:       05/16/00                                                      *
*                                                                             *
*   Copyright (c) 2000 - 2001 Goran Devic                                     *
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

static char buf[MAX_STRING];
static char disasm[MAX_STRING];

// We do ourselves a favor and keep the last disassembled address here
// so on the next call we simply use that one
static TADDRDESC Addr = { 0, 0 };

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

static DWORD GetCodeLine(PTADDRDESC pAddr)
{
    TDISASM dis;
    int i, pos;

    pos = sprintf(buf, "%04X:%08X ", pAddr->sel, pAddr->offset);

    dis.dwFlags  = DIS_DATA32 | DIS_ADDRESS32;
    dis.wSel = pAddr->sel;
    dis.dwOffset = pAddr->offset;
    dis.szDisasm = disasm;

    // Disassemble and store into the line buffer
    Disassembler( &dis );

    // If CODE was ON, print the code bytes
    if( deb.fCode )
    {
        for( i=0; i<dis.bInstrLen && i<CODE_BYTES; i++ )
        {
            pos += sprintf(buf+pos, "%02X", dis.bCodes[i]);
        }

        // Append spaces, if necessary
        while( i++ < CODE_BYTES )
        {
            pos += sprintf(buf+pos, "  ");
        }
    }

    // Make the buffers lowercased if the variable was set so
    if( deb.fLowercase==TRUE )
    {
        strtolower(buf);
        strtolower(disasm);
    }

    return( dis.bInstrLen );
}


static DWORD GetCodeLines()
{
    // If data frame is visible, we will advance so many lines of code
    if( pWin->c.fVisible )
        return( pWin->c.nLines - 1 );

    // Code window is not visible, so advance 8 or (history height-1) code lines
    if( pWin->h.nLines > 8 )
        return( 8 );

    return( pWin->h.nLines - 1 );
}


void CodeDraw(BOOL fForce, DWORD newOffset)
{
    int maxLines;
    int nLen, nLine=1;
    char col;

    if( pWin->c.fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pWin->c.Top+1);
        PrintLine(" Code");
    }
    else
        if( fForce==FALSE )
            return;

    deb.codeAddr.offset = newOffset;    // Store the new offset to disassemble
    Addr = deb.codeAddr;                // Copy the current code address
    maxLines = GetCodeLines();

    while( nLine <= maxLines )
    {
        nLen = GetCodeLine(&Addr);

        // If the address is the current CS:EIP, get the line color inverted
        if( Addr.sel==deb.r->cs && Addr.offset==deb.r->eip )
            col = COL_REVERSE;
        else
            col = COL_NORMAL;

        if(dprinth(nLine++, "%c%c%s%s\r", DP_SETCOLINDEX, col, buf, disasm)==FALSE)
            break;

        // Advance code offset for the next line
        Addr.offset += nLen;
    }

    if( pWin->c.fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}


/******************************************************************************
*                                                                             *
*   void CodeScroll(int direction)                                            *
*                                                                             *
*******************************************************************************
*
*   This function is called from the edlim module to scroll code window
*   one frame forward (1) or one frame backward (-1).
*
******************************************************************************/
void CodeScroll(int direction)
{
//    TADDRDESC Guess;
//    DWORD offset;
//    int maxLines = GetCodeLines();

    if( pWin->c.fVisible )
    {
        if( direction==1 )
        {
            // Forward is easy since we already saved the address of the next
            // insteruction to disassemble
            deb.codeAddr.offset = Addr.offset;
            CodeDraw(FALSE, deb.codeAddr.offset);
        }
        else
        {
            // Backward is much trickier.. We need to estimate how many bytes
            // to backtrack and then attempt until we get the right code alignment...

            // <TODO>

            CodeDraw(FALSE, deb.codeAddr.offset - pWin->c.nLines);
        }
    }
}

/******************************************************************************
*                                                                             *
*   BOOL cmdUnasm(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Disassemble command
*
*   U <address> [L len]
*
******************************************************************************/
BOOL cmdUnasm(char *args, int subClass)
{
    if( *args!=0 )
    {
        // Argument present: U <address> [L <len>]
        evalSel = deb.codeAddr.sel;
        deb.codeAddr.offset = Evaluate(args, &args);
        deb.codeAddr.sel = evalSel;
    }
    else
    {
        // No arguments - advance current address one screenful
        // We saved the offset at which previous disassembly ended up
        deb.codeAddr.offset = Addr.offset;
    }

    CodeDraw(TRUE, deb.codeAddr.offset);

    return( TRUE );
}

