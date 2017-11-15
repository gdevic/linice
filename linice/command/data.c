/******************************************************************************
*                                                                             *
*   Module:     data.c                                                        *
*                                                                             *
*   Date:       11/15/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 11/15/00   Original                                             Goran Devic *
* 03/11/01   Second edition                                       Goran Devic *
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

// If you change this, make sure it is divisible by 4
#define DATA_BYTES         16           // So many bytes per line

static union
{
    BYTE byte[DATA_BYTES];
    WORD word[DATA_BYTES/2];
    DWORD dword[DATA_BYTES/4];

} MyData;

static BOOL fValid[DATA_BYTES];

static char buf[MAX_STRING];

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
        if( (fValid[i]==TRUE) && isprint(MyData.byte[i]) )
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

    pos += sprintf(buf+pos, "\n");
}


void PrintDataLines(int maxLines)
{
    int nLine = 1;
    TADDRDESC Addr;

    Addr = deb.dataAddr;                // Copy the current data address

    while( nLine < maxLines )
    {
        GetDataLine(&Addr);
        if(dprinth(nLine++, buf)==FALSE)
            break;
    }
}

static char *sSize[4] = { "byte", "word", "", "dword" };

void DataDraw(void)
{
    PrintLine(" Data                                               %s", sSize[deb.DumpSize-1]);

    PrintDataLines(pWin->d.nLines);
}


BOOL cmdData(char *args)
{
    PrintDataLines(8);

    return( TRUE );
}

