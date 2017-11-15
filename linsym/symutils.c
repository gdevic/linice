/******************************************************************************
*                                                                             *
*   Module:     symutils.c                                                    *
*                                                                             *
*   Date:       10/19/00                                                      *
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

        This file contains symbol translation functions.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/19/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>

#ifdef WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines
#include "primes.h"                     // Insert table of 1024 prime numbers


extern void TranslateElf(int fd, int fi, char *pathSources, char *nameOut, int nLevel, DWORD filesize);

/******************************************************************************
*                                                                             *
*   DWORD str2key(char *str)                                                  *
*                                                                             *
*******************************************************************************
*
*   Creates a hash key for the given string.
*
******************************************************************************/
DWORD str2key(char *str)
{
    int len = strlen(str);
    DWORD key = 0;

    while( len-- )
    {
        key += *str++ << len;
    }

    return( key );
}


/******************************************************************************
*                                                                             *
*   void TranslateMapFile(int fd, FILE* fIn, char *nameOut, int size)         *
*                                                                             *
*******************************************************************************
*
*   Translates ASCII symbol files produced by the 'nm' command into a linice
*   symbol file (only publics are stored)
*
*   Where:
*       fd - the file descriptor of output symbol file
*       fIn - the file stream of the input symbol file
*       nameOut - the name of the output module
*       size - size of the input symbol file in bytes
*
******************************************************************************/
// Compare function for a quick sort
int symcmp(const void *p1, const void *p2)
{
    return( ((TSYM_PUB *)p1)->dwAddress > ((TSYM_PUB *)p2)->dwAddress );
}

void TranslateMapFile(int fd, FILE* fIn, char *nameOut, int size)
{
    char buf[80];                       // Temporary buffer
    DWORD i, items, status, index;
    DWORD key, collisions;              // Used for hashing
    DWORD written;                      // Bytes written so far

    TSYMTAB Symtab;
    TGLOBAL Global;
    TSYM_PUB *pSymPub;
    WORD *pHash;
    char *pStrings, *pStrCur;

    //===============================================================
    // Allocate all buffers
    //===============================================================
    // But before we can do that, we need to find out exactly how many symbols
    // a map file has, so loop once and count them
    items = 0;
    while( !feof(fIn) )
    {
        if( fgets(buf, 80, fIn)==NULL)
            break;
        items++;
    }
    fseek(fIn, 0, SEEK_SET);

    // Have a hash table that is several times the number of elements, and find the
    // prime value that is at least that from our hash table
    // The max value in the current prime table is 38873, that divided by 4 is about
    // 9700 entries. Since even the kernel has just about 6000, we are fine.
    for( i=1; i<1023; i++)
    {
        if( primes[i] > items * 4 )
            break;
    }

    Global.nSyms = items;
    Global.nHash = primes[i];

    pSymPub = calloc(sizeof(TSYM_PUB) * items, 1);
    pHash   = calloc(sizeof(WORD) * Global.nHash, 1);

    // Overallocate buffer for strings
    pStrCur = pStrings = calloc(size, 1);

    // Make sure all allocations made it
    if( !pSymPub || !pHash || !pStrings )
    {
        printf("Error allocating memory\n");
        return;
    }

    //===============================================================
    // Read symbol file and build the SYM_PUB array
    //===============================================================

    i = 0;
    while( !feof(fIn) )
    {
        status = fscanf(fIn, "%x %c %s\n",
            &pSymPub[i].dwAddress, (char *)&pSymPub[i].Section, pStrCur);

        pSymPub[i].pName = pStrCur;
        pStrCur += strlen(pStrCur) + 1;
        i++;
    }

    //===============================================================
    // Sort the symbol table by address
    //===============================================================
    qsort((void *)pSymPub, items, sizeof(TSYM_PUB), symcmp);

    Global.dwAddrStart = pSymPub[0].dwAddress;
    Global.dwAddrEnd   = pSymPub[Global.nSyms-1].dwAddress;

    //===============================================================
    // Hash symbol names into the hash table
    //===============================================================
    // Loop for each string in the symbol table array, hash it in
    collisions = 0;
    for( i=0; i<items; i++)
    {
#if 1
        printf("%08X index: %s\n", pSymPub[i].dwAddress, pSymPub[i].pName);
        fflush(NULL);
#endif

        key = str2key(pSymPub[i].pName);
        index = key % Global.nHash;

        while( pHash[index] != 0 )
        {
            collisions++;
            if( ++index==Global.nHash )
                index = 0;
        }
        // Store the item index into the hash table
        pHash[index] = (WORD)(i + 1);
    }

    printf("Hash table collisions: %d out of %d\n", collisions, items);

    //===============================================================
    // We have everything needed to generate output symbol file
    //===============================================================
    strcpy(Symtab.Sig, SYMSIG);

    Symtab.size =   sizeof(TSYMTAB) +
                    (pStrCur - pStrings) +
                    sizeof(TGLOBAL) +
                    sizeof(TSYM_PUB) * Global.nSyms +
                    sizeof(WORD) * Global.nHash;

    strncpy(Symtab.name, nameOut, MAX_MODULE_NAME);
    Symtab.next = NULL;

    //--------------
    //  TSYMTAB    |    sizeof(TSYMTAB)
    //--------------
    //  Strings    |    pStrCur - pStrings
    //--------------
    //  TGLOBAL    |    sizeof(TGLOBAL)
    //--------------
    //  TSYM_PUB[] |    sizeof(TSYM_PUB) * TGLOBAL->nSyms
    //--------------
    //  WORD Hash[]|    sizeof(WORD) * TGLOBAL->nHash
    //--------------

    //===============================================================
    // Make all pointers relocatable offsets from the start of the file
    //===============================================================

    // Make symbol's name references relative
    for(i=0; i<Global.nSyms; i++ )
    {
        pSymPub[i].pName = (char *) ((pSymPub[i].pName - pStrings) + sizeof(TSYMTAB));
    }

    Symtab.pStrings = (char *) sizeof(TSYMTAB);
    Symtab.pGlobals = (TGLOBAL *) (Symtab.pStrings + (pStrCur - pStrings));
    Symtab.pTypes   = NULL;
    Symtab.pSymbols = NULL;
    Symtab.pSrc     = NULL;

    Global.pSym     = (TSYM_PUB *)((DWORD) Symtab.pGlobals + sizeof(TGLOBAL));
    Global.pHash    = (WORD *)((DWORD) Global.pSym + sizeof(TSYM_PUB) * Global.nSyms);

    written = 0;
    written += write(fd, &Symtab, sizeof(TSYMTAB));
    written += write(fd, pStrings, pStrCur - pStrings);
    written += write(fd, &Global, sizeof(TGLOBAL));
    written += write(fd, pSymPub, sizeof(TSYM_PUB) * Global.nSyms);
    written += write(fd, pHash, sizeof(WORD) * Global.nHash);

    if( written != Symtab.size )
    {
        printf("Error writing symbol file\n");
    }
    else
    {
        printf("Module name: %s\n", nameOut);
        printf("  Translated %d global symbols\n", Global.nSyms);
    }
}


/******************************************************************************
*                                                                             *
*   void OptTranslate                                                         *
*       (char *pathOut, char *pathIn, char *pathSources, int nLevel)          *
*                                                                             *
*******************************************************************************
*
*   Translate file into a symbol table and write it to a file.
*
*   The types of files that can be translated are:
*       ASCII symbol file produced by the 'nm' command
*       ELF user mode executable file
*       ELF Kernel module object file
*
*   Where:
*       pathOut - Creating this symbol file
*       pathIn  - Source file to translate
*       pathSources - Path to search sources for source packaging
*       nLevel - level of translation
*
******************************************************************************/
void OptTranslate(char *pathOut, char *pathIn, char *pathSources, int nLevel)
{
    FILE *fin;
    struct stat prop;
    char elf[4];
    int fd_out, fd_in, status;

    // Find the size of the input file
    status = stat(pathIn, &prop);
    if(status==0)
    {
        // Open/Create the output file (binary, linsym-symbol table)
        fd_out = open(pathOut, O_WRONLY | O_TRUNC | O_CREAT );
        if( fd_out>0 )
        {
            // Try to open the input file so we can find out is it ASCII or ELF
            fin = fopen(pathIn, "rb");
            if(fin!=NULL)
            {
                // Read first 4 bytes of the file (ELF signature)
                status = fread(elf, 4, 1, fin);
                if( status==1 )
                {
                    // Determine the internal module name based off the input file name
                    // Use only trailing name and cut off anything after last '.'
                    if(strrchr(pathIn, '/')!=NULL)
                        pathIn = strrchr(pathIn, '/') + 1;
                    if(strchr(pathIn, '.')!=NULL)
                        *(strchr(pathIn, '.')) = 0;

                    fd_in = fileno(fin);
                    if( elf[1]=='E' && elf[2]=='L' && elf[3]=='F' )
                    {
                        // ELF file
                        TranslateElf(fd_out, fd_in, pathSources, pathIn, nLevel, prop.st_size);
                    }
                    else
                    {
                        // ASCII map symbol file
                        TranslateMapFile(fd_out, fin, pathIn, prop.st_size);
                    }
                }
                else
                    printf("Type of input file %s is not supported\n", pathIn);

                fclose(fin);
            }
            else
                printf("Error opening input file %s\n", pathIn);

            close(fd_out);
        }
        else
            printf("Access violation writing symbol file %s\n", pathOut);
    }
    else
    {
        switch(status)
        {
        case EACCES:
            printf("Error accessing input file %s\n", pathIn);
            break;
        case EBADF:
            printf("Bad input file %s\n", pathIn);
            break;
        case ENOENT:
            printf("Input file %s unreachable\n", pathIn);
            break;
        default:
            printf("Unable to stat input file %s\n", pathIn);
        }
    }
}

