/******************************************************************************
*                                                                             *
*   Module:     code.c                                                        *
*                                                                             *
*   Date:       11/16/00                                                      *
*                                                                             *
*   Copyright (c) 2000 - 2001 Goran Devic                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 11/16/00   Original                                             Goran Devic *
* 03/11/01   Second edition                                       Goran Devic *
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


static void PrintCodeLines(int maxLines)
{
    int nLen, nLine=1;
    TADDRDESC Addr;

    Addr = deb.codeAddr;                // Copy the current code address

    while( nLine < maxLines )
    {
        nLen = GetCodeLine(&Addr);
        if(dprinth(nLine++, "%s%s\n", buf, disasm)==FALSE)
            break;

        // Advance code offset for the next line
        Addr.offset += nLen;
    }
}

void CodeDraw()
{
    PrintLine(" Code");

    PrintCodeLines(pWin->c.nLines);
}


BOOL cmdUnassemble(char *args)
{
    PrintCodeLines(8);

    return( TRUE );
}

