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

#ifdef WIN32
#include <io.h>
#else // !WIN32
#include <unistd.h>
#endif // WIN32

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines
//#include "primes.h"                     // Insert table of 1024 prime numbers


extern BYTE *LoadElf(char *sName);
extern BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName);

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
void TranslateMapFile(int fd, FILE* fIn, char *nameOut, int size)
{
}


/******************************************************************************
*                                                                             *
*   void OptTranslate                                                         *
*       (char *pathOut, char *pathIn, char *pathSources, int nLevel)          *
*                                                                             *
*******************************************************************************
*
*   Translate an ELF file into a symbol file and write it out.
*
*   Several types of files that can be translated are:
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
    BYTE *pBuf;
    char *pTableName;

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
                    if(strrchr(pathIn, '/')!=NULL)
                        pTableName = strrchr(pathIn, '/') + 1;
                    else
                        pTableName = pathIn;

                    fd_in = fileno(fin);
                    if( elf[1]=='E' && elf[2]=='L' && elf[3]=='F' )
                    {
                        // ELF file
                        pBuf = LoadElf(pathIn);
                        if( pBuf )
                        {
                            ElfToSym(pBuf, pathOut, pTableName);
                        }
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

