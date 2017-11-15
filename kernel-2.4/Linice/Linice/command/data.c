/******************************************************************************
*                                                                             *
*   Module:     data.c                                                        *
*                                                                             *
*   Date:       05/15/00                                                      *
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

        This module contains memory dump functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/15/00   Original                                             Goran Devic *
* 09/11/00   Second edition                                       Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

static union
{
    BYTE byte[DATA_BYTES];
    WORD word[DATA_BYTES/2];
    DWORD dword[DATA_BYTES/4];

} MyData;

static BOOL fValid[DATA_BYTES];

static char buf[MAX_STRING];

static char *sSize[4] = { "byte", "word", "", "dword" };

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static int PrintAscii(int pos)
{
    int i;

    buf[pos++] = ' ';

    for( i=0; i<DATA_BYTES; i++, pos++)
    {
        if( (fValid[i]==TRUE) && MyData.byte[i]>=DP_AVAIL )
            buf[pos] = MyData.byte[i];
        else
            buf[pos] = '.';
    }

    return( i + 1 );
}

void GetDataLine(PTADDRDESC pAddr)
{
    int i, pos;

    // Fetch a lineful of bytes at a time and get their present flags
    for( i=0; i<DATA_BYTES; i++)
    {
        fValid[i] = AddrIsPresent(pAddr);
        if( fValid[i]==TRUE )
            MyData.byte[i] = AddrGetByte(pAddr);
        pAddr->offset++;
    }

    pos = sprintf(buf, "%04X:%08X ", pAddr->sel, pAddr->offset - DATA_BYTES);

    switch( deb.DumpSize )
    {
    //=========================================================
        case 1:             // BYTE
    //=========================================================

            for( i=0; i<DATA_BYTES; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%02X ", MyData.byte[i]);
                else
                    pos += sprintf(buf+pos, "?? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;

    //=========================================================
        case 2:             // WORD
    //=========================================================

            for( i=0; i<DATA_BYTES/2; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%04X ", MyData.word[i]);
                else
                    pos += sprintf(buf+pos, "???? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;

    //=========================================================
        case 4:             // DWORD
    //=========================================================

            for( i=0; i<DATA_BYTES/4; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%08X ", MyData.dword[i]);
                else
                    pos += sprintf(buf+pos, "???????? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;
    }
    // Terminate the line
    buf[pos] = 0;
}


static DWORD GetDataLines()
{
    // If data frame is visible, we will advance so many lines of data
    if( pWin->d.fVisible )
        return( pWin->d.nLines - 1 );

    // Data window is not visible, so advance 8 or (history height-1) data lines
    if( pWin->h.nLines > 8 )
        return( 8 );

    return( pWin->h.nLines - 1 );
}


void DataDraw(BOOL fForce, DWORD newOffset)
{
    TADDRDESC Addr;
    int maxLines;
    int nLine = 1;

    if( pWin->d.fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pWin->d.Top+1);
        PrintLine(" Data                                               %s", sSize[deb.DumpSize-1]);
    }
    else
        if( fForce==FALSE )
            return;

    deb.dataAddr.offset = newOffset;    // Store the new offset to dump
    Addr = deb.dataAddr;                // Copy the current data address
    maxLines = GetDataLines();

    while( nLine <= maxLines )
    {
        GetDataLine(&Addr);
        if(dprinth(nLine++, "%s\r", buf)==FALSE)
            break;
    }

    if( pWin->d.fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdDdump(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Data dump command
*
*   subClass is:
*       0   D
*       1   DB
*       2   DW
*       4   DD
*
******************************************************************************/
BOOL cmdDdump(char *args, int subClass)
{
    if( subClass )
        deb.DumpSize = subClass;

    if( *args!=0 )
    {
        // Argument present: D <address> [L <len>]
        evalSel = deb.dataAddr.sel;
        deb.dataAddr.offset = Evaluate(args, &args);
        deb.dataAddr.sel = evalSel;
    }
    else
    {
        // No arguments - advance current address
        deb.dataAddr.offset += DATA_BYTES * GetDataLines();
    }

    DataDraw(TRUE, deb.dataAddr.offset);

    return( TRUE );
}

