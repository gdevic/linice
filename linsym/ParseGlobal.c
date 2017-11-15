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


TGLOBAL *pGlobals = NULL;               // Array of global symbol descriptors
int nGlobals = 0;                       // Number of global variables

extern int dfs;

/******************************************************************************
*                                                                             *
*   BOOL ParseGlobal(int fd, int fs)                                          *
*                                                                             *
*******************************************************************************
*
*   Loads and parses global symbols and creates global section.
*   Globals are read from the (global) array pGlobals, and there are nGlobals
*   items.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*
******************************************************************************/
BOOL ParseGlobal(int fd, int fs)
{
    TSYMGLOBAL *pHeader;                // Globals header
    DWORD dwSize;                       // Final size of the above structure
    int nGlobalsStored;                 // Number of global symbols stored
    int i;                              // Counter

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
        nGlobalsStored = 0;

        for( i=0; i<nGlobals; i++ )
        {
            printf("%c %08X %08X %04X %s\n", 
               (pGlobals[i].wAttribute==0x11 || pGlobals[i].wAttribute==0x12)? '*':' ', 
                pGlobals[i].dwAddress, 
                pGlobals[i].dwEndAddress, 
                pGlobals[i].wAttribute, 
                pGlobals[i].Name);

            // If a global symbol is one of:
            //  11h - Global symbol (variable)
            //  12h - Global symbol (function)
            // store it in the linice symbol table
            if( pGlobals[i].wAttribute==0x11 || pGlobals[i].wAttribute==0x12 )
            {
                pHeader->global[nGlobalsStored].dwStartAddress = pGlobals[i].dwAddress;
                pHeader->global[nGlobalsStored].dwEndAddress   = pGlobals[i].dwEndAddress;
                pHeader->global[nGlobalsStored].dName          = dfs;
                pHeader->global[nGlobalsStored].bFlags         = 
                    (pGlobals[i].wAttribute==0x11) ? 0x01 : 0x00;

                // Copy the symbol name into the strings
                write(fs, pGlobals[i].Name, strlen(pGlobals[i].Name)+1);
                dfs += strlen(pGlobals[i].Name)+1;

                // Increment the actual number of global symbols stored
                nGlobalsStored++;
            }
        }

        // Recalculate new size based on the number of global items actually stored
        dwSize = sizeof(TSYMGLOBAL) + sizeof(TSYMGLOBAL1)*(nGlobalsStored-1);

        // Fill up the globals header
        pHeader->hType    = HTYPE_GLOBALS;
        pHeader->dwSize   = dwSize;
        pHeader->nGlobals = nGlobalsStored;

        // Final write out of the globals section
        write(fd, pHeader, dwSize);

        free(pHeader);

        printf("Parsed %d global items; stored %d\n", nGlobals, nGlobalsStored);
    }
    else
        printf("Unable to allocate memory\n");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   DWORD GetGlobalSymbolAddress(char *pName)                                 *
*                                                                             *
*******************************************************************************
*
*   Returns global symbol address for a given symbol name.
*
*   Where:
*       pName is the symbol name
*
******************************************************************************/
DWORD GetGlobalSymbolAddress(char *pName)
{
    TGLOBAL *pGlob;
    char sSymbol[MAX_SYMBOL_LEN];
    int i;

    if( pName!=NULL && pGlobals!=NULL )
    {
        // Symbol name may have extra characters at the end, so trim it
        strncpy(sSymbol, pName, MAX_SYMBOL_LEN-1);
        if( strchr(sSymbol, ':') )
            *(char *)strchr(sSymbol, ':') = 0;

        pGlob = pGlobals;

        for(i=0; i<nGlobals; i++)
        {
            if( !strcmp(pGlob->Name, sSymbol) )
                return( pGlob->dwAddress );

            pGlob++;
        }
    }

    return( 0 );
}


BOOL StoreGlobalSymbols(FILE *fGlobals, int nGlobals)
{
    TGLOBAL *pGlob;
    int i;

    pGlobals = malloc(sizeof(TGLOBAL) * nGlobals);
    if( pGlobals )
    {
        pGlob = pGlobals;

        // Rewind the input file to the start
        fseek(fGlobals, 0, SEEK_SET);
        
        // Read each global item and store it in the array
        for(i=0; i<nGlobals; i++ )
        {
            fscanf(fGlobals, "%08X %08X %04X %s\n", 
                &pGlob->dwAddress,
                &pGlob->dwEndAddress,
                &pGlob->wAttribute,
                &pGlob->Name);

            pGlob++;
        }

        return( TRUE );
    }

    return( FALSE );
}

