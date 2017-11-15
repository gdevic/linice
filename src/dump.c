/******************************************************************************
*                                                                             *
*   Module:     dump.c                                                        *
*                                                                             *
*   Date:       11/15/00                                                      *
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

static union
{
    BYTE byte[16];
    WORD word[8];
    DWORD dword[4];

} MyData;

static BOOL fValid[16];


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static void PrintAscii()
{
    int i;

    dputc(' ');

    for( i=0; i<16; i++)
    {
        if( (fValid[i]==TRUE) && isprint(MyData.byte[i]) )
            dputc( MyData.byte[i] );
        else
            dputc('.');
    }

    // Finish the line

    dputc('\n');
}    


void PrintData(void)
{
    DWORD lines;
    int i;
    DWORD value;
    WORD sel;
    DWORD offset;

    lines = deb.wd.nLines;
    sel = deb.dumpSel;
    offset = deb.dumpOffset;

    // Print the data window header

    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_LINE]);
    dprint("-Data---------------------------------------------------------------------------\n");
    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_NORMAL]);
    
    while( lines-- )
    {
        // Fill up our data buffer

        for( i=0; i<16; i++)
        {
            if( (value = GetByte(offset + i)) <= 0xFF )
            {
                MyData.byte[i] = value;
                fValid[i] = TRUE;
            }
            else
                fValid[i] = FALSE;
        }


        dprint("%04X:%08X ", sel, offset);

        switch( deb.dumpMode )
        {
        //=========================================================
            case DD_BYTE:
        //=========================================================

                // Print the data...

                for( i=0; i<16; i++)
                {
                    if( fValid[i]==TRUE )
                        dprint("%02X ", MyData.byte[i]);
                    else
                        dprint("?? ");
                }

                // ... and the ASCII representation
                PrintAscii();

            break;

        //=========================================================
            case DD_WORD:
        //=========================================================

                // Print the data...

                for( i=0; i<8; i++)
                {
                    if( fValid[i]==TRUE )
                        dprint("%04X ", MyData.word[i]);
                    else
                        dprint("???? ");
                }

                // ... and the ASCII representation
                PrintAscii();

            break;

        //=========================================================
            case DD_DWORD:
        //=========================================================

                // Print the data...

                for( i=0; i<4; i++)
                {
                    if( fValid[i]==TRUE )
                        dprint("%08X ", MyData.dword[i]);
                    else
                        dprint("???????? ");
                }

                // ... and the ASCII representation
                PrintAscii();

            break;

            default:
                
                break;

        }

        offset += 16;
    }
}    

