/******************************************************************************
*                                                                             *
*   Module:     unassemble.c                                                  *
*                                                                             *
*   Date:       11/16/00                                                      *
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

BYTE test[10] = 
{
    0x3C, 0xAA
};

static char sLine[256];

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

void PrintCode()
{
    DWORD lines;
    DWORD nLen;
    BYTE codes[16];
    TDisassembler dis;
    WORD sel;
    DWORD offset;
    int i;

    lines = deb.wc.nLines;
    deb.codeMode = DC_ASM;              // Initially disassemble pure assembly
    sel = deb.codeSel;
    offset = deb.codeOffset;


    dis.dwFlags = DIS_DATA32 | DIS_ADDRESS32;
    dis.wSel = sel;
//    dis.bpTarget = (BYTE *) offset;
    dis.bpTarget = (BYTE *) test;

    dis.szDisasm = sLine;
    dis.pCode = codes;

//    while( lines-- > 0 )
    {

codes[15] = 0x88;
        Disassembler( &dis );

if( codes[15] != 0x88 )
    dprint("CODES[15] CORRUPTED!!!\n");

        dprint("len: %02X  flags=%X as=%d ", dis.bInstrLen, dis.dwFlags, dis.bAsciiLen );
        for( i=0; i<dis.bInstrLen;i++ )
        {
            dprint("%02X ", dis.pCode[i]);
        }
        dprint(" %s\n", dis.szDisasm);
        dputc('\n');
                   
        // Print selector:offset

        // Clean the printout string line
        memset(sLine, ' ', 256);
        nLen = 0;

        nLen += sprintf(sLine+nLen, "%04X:%08X  ", sel, (int) offset );
        dprint("-%d-", nLen);

        // If the code is on, print code bytes
#if 0
        if( deb.fSetCode )
        {
            for( i=0; i<MIN(dis.bInstrLen, 10); i++)
            {
                nLen += sprintf(sLine+nLen, "%02X", codes[i]);
                dprint("-%d-", nLen);
            }
        }
#endif
        // Get the disassembled line

        nLen += sprintf(sLine+nLen, "%s", dis.szDisasm );
        dprint("-%d-", nLen);

        // and finally print it out

        dprint("%s\n", sLine);

        dis.bpTarget += dis.bInstrLen;
        offset       += dis.bInstrLen;
    }
}    

