/******************************************************************************
*                                                                             *
*   Module:     HashTable.c                                                   *
*                                                                             *
*   Date:       10/23/01                                                      *
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

        This module contains the code to write out a symbol hash table

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/23/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "Common.h"                     // Include platform specific set

#include "primes.h"                     // Include table of prime numbers


/******************************************************************************
*                                                                             *
*   BOOL HashSymbolAdd(char *pName, DWORD dwValue)                            *
*                                                                             *
*******************************************************************************
*
*   Generic add to a symbol hash table - this way we add all symbols that
*   we care to find fast (globals, static, etc.)
*
*   Where:
*
******************************************************************************/
BOOL HashSymbolAdd(char *pName, DWORD dwValue)
{
    return(0);
}


/******************************************************************************
*                                                                             *
*   BOOL WriteHashTable(int fd, FILE *fGlobals, DWORD nGlobals)               *
*                                                                             *
*******************************************************************************
*
*   Stores symbols in a hash table and writes out
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fGlobals - file that contains symbols to be stored
*       nGlobals - number of symbols to store
*
******************************************************************************/
BOOL WriteHashTable(int fd, FILE *fGlobals, DWORD nGlobals)
{
    TSYMHASH Hash;                      // Hash table header
    TSYMHASH1 *pHash;                   // Pointer to a hash table array

    int nHash;                          // Number of slots in a hash table
    int i;                              // Generic counter
    char sName[MAX_SYMBOL_LEN+1];       // Read symbol name here
    DWORD dName, Value;                 // Symbol name offset and value

    // We need to have at least one symbol to place in a table
    if( nGlobals )
    {
        // ..but not too many that we can't handle
        if( nGlobals < primes[PRIMES_MAX-1]/3 )
        {
            // Allocate hash table large enough to accomodate this many symbols
            // We need about 3 times as many elements, prime number

            for(i=0; i<PRIMES_MAX; i++)
            {
                if( nGlobals*3 < primes[i] )
                    break;
            }

            nHash = primes[i];
            printf("nGlobals = %d  Hash table size = %d\n", nGlobals, nHash);

            pHash = malloc(sizeof(TSYMHASH1) * nHash);
            if( pHash )
            {
                // Clear up the array
                memset(pHash, 0, sizeof(TSYMHASH1) * nHash);

                // Rewind the input file containing the symbol data
                fseek(fGlobals, 0, SEEK_SET);

                for(i=0; i<(int)nGlobals; i++)
                {
                    // Read lines containing the symbol name, offset to name and the value
                    i = fscanf(fGlobals, "%s,%X,%X\n", sName, &dName, &Value);

                    printf("%08X =%08X  %s\n", dName, Value, sName);
                }

                // Fill in the hash table header structure
                Hash.hType  = HTYPE_SYMBOL_HASH;
                Hash.dwSize = sizeof(TSYMHASH) + (nHash-1)*sizeof(TSYMHASH1);
                Hash.nHash  = nHash;


            }
            else
            {
                printf("Error allocating memory for hash table (%d bytes)\n", sizeof(TSYMHASH1) * nHash);
                printf("         Global symbols will not be stored\n");

                return( FALSE );
            }
        }
        else
        {
            printf("Problem: Too many global symbols for the hash table..\n");
            printf("         Global symbols will not be stored\n");

            return( FALSE );
        }
    }

    return( TRUE );
}

