/******************************************************************************
*                                                                             *
*   Module:     ParseGlobals.c                                                *
*                                                                             *
*   Date:       09/05/00                                                      *
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

        This module contains the ELF buffer parsing code

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/05/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "Common.h"                     // Include platform specific set


extern int dfs;

/******************************************************************************
*                                                                             *
*   BOOL ParseGlobal(int fd, int fs, FILE *fGlobals, int nGlobals)            *
*                                                                             *
*******************************************************************************
*
*   Loads and parses source files and stores them
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       fGlobals - queue of global symbol informations in text format
*       nGlobals - number of global symbol items
*
******************************************************************************/
BOOL ParseGlobal(int fd, int fs, FILE *fGlobals, int nGlobals)
{
    int i;                              // Counter
    TSYMGLOBAL *pHeader;                // Globals header
    DWORD dwSize;                       // Final size of the above structure
#define MAX_SYMBOL_LEN  128
    char sSymbol[MAX_SYMBOL_LEN];       // Buffer to load a symbol name

    printf("=============================================================================\n");
    printf("||         PARSE GLOBALS                                                   ||\n");
    printf("=============================================================================\n");

    if( nGlobals==0 )
        return( TRUE );

    dwSize = sizeof(TSYMGLOBAL) + sizeof(TSYMGLOBAL1)*(nGlobals-1);

    // Allocate memory to store the global symbols
    pHeader = (TSYMGLOBAL *) malloc(dwSize);
    if( pHeader!=NULL )
    {
        // Fill up the globals header
        pHeader->hType    = HTYPE_GLOBALS;
        pHeader->dwSize   = dwSize;
        pHeader->nGlobals = nGlobals;

        // Rewind and read in all the global items from a queue file
        fseek(fGlobals, 0, SEEK_SET);

        for( i=0; i<nGlobals; i++ )
        {
            fscanf(fGlobals, "%s %08X %08X\n", sSymbol,
                &pHeader->global[i].dwStartAddress,
                &pHeader->global[i].dwEndAddress);

            pHeader->global[i].dName = dfs;

            // Copy the symbol name into the strings
            write(fs, sSymbol, strlen(sSymbol)+1);
            dfs += strlen(sSymbol)+1;
        }

        // Final write out of the globals section
        write(fd, pHeader, dwSize);

        free(pHeader);
    }
    else
        printf("Unable to allocate memory\n");

    return( FALSE );
}

