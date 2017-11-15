/******************************************************************************
*                                                                             *
*   Module:     unassemble.c                                                  *
*                                                                             *
*   Date:       11/16/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains unassemble functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/16/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

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

static char sLine[160];


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BYTE Disassembler( TDisassembler *pDis );


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static DWORD GetDisLine(DWORD addr)
{
    static char *hex = "0123456789abcdef";
    TDisassembler dis;
    int i;
    char *p;
    
    dis.dwFlags  = DIS_DATA32 | DIS_ADDRESS32;
    dis.bpTarget = (BYTE *) addr;
    dis.szDisasm = sLine;

    // If "set code on"
    if( 1 )
    {
        memset(sLine, ' ', 2*CODE_BYTES + 2);

        // Disassemble and store into the line buffer leaving some space for codes
        dis.szDisasm += 2*CODE_BYTES + 2;
        Disassembler( &dis );

        // Print so many code bytes into the code byte buffer
        p = sLine;
        for( i=0; i<dis.bInstrLen && i<CODE_BYTES; i++ )
        {
            *p++ = hex[TOPNIBBLE(dis.Codes[i])];
            *p++ = hex[(dis.Codes[i])&0xF];
        }
    }
    else
    {
        // Disassemble and store into the line buffer
        Disassembler( &dis );
    }

    return( dis.bInstrLen );
}    


void PrintCode()
{
    DWORD lines = deb.wc.nLines;        // Get the number of lines to draw
    DWORD nLen, addr;

    addr = deb.codeOffset;

    // Print the code window header
    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_LINE]);
    dprint("-Code---------------------------------------------------------------------------\n");
    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_NORMAL]);
    
    while( lines-- > 0 )
    {
        nLen = GetDisLine(deb.codeOffset);
        dprint("%04X:%08X %s\n", 0x10, deb.codeOffset, sLine);
        deb.codeOffset += nLen;
    }
}    


BOOL CmdUnassemble(char *args)
{
    DWORD nLen;

    nLen = GetDisLine(deb.codeOffset);
    dprint("%04X:%08X %s\n", 0x10, deb.codeOffset, sLine);
    deb.codeOffset += nLen;

    return( TRUE );
}    

