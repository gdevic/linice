/******************************************************************************
*                                                                             *
*   Module:     symutils.c                                                    *
*                                                                             *
*   Date:       10/19/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   *
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
#include <time.h>

#include <ctype.h>

#ifdef WIN32
#include <io.h>
#else // !WIN32
#include <unistd.h>
#endif // WIN32

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines

#include "loader.h"                     // Include global protos

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

#define MAX_PATH        256

static char sPath[MAX_PATH];            // Buffer to hold the source path

static char *pPathPrefix = NULL;        // Substitution path prefix
static char *pPathNew = NULL;           // Substitution path target (new path)


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern BYTE *LoadElf(char *sName);
extern BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName);
extern int strnicmp( const char *s1, const char *s2, size_t n );


/******************************************************************************
*                                                                             *
*   BOOL ProcessSubstitutionPath(char *pPathSubst)                            *
*                                                                             *
*******************************************************************************
*
*   Process the user substitution path specification.
*
*   Where:
*       pPathSubst is in the form "<path-prefix>:<new-path>"
*
*   Returns:
*       TRUE for ok
*       FALSE for bad path specification
*
******************************************************************************/
BOOL ProcessSubstitutionPath(char *pPathSubst)
{
    char *p;                            // Temporary pointer

    // It is legal not to have the custom subst path
    if( pPathSubst )
    {
        // The path substitution string must be in the form "<path-prefix>:<new-path>"
        p = strchr(pPathSubst, ':');
        if( !p )
            return( FALSE );

        // Assign the prefix of the path and the subst portion
        pPathPrefix = pPathSubst;

        *p = '\0';                      // Null-terminate the first string

        pPathNew = p+1;                 // Mark the start of the subst string
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL OpenUserSourceFile(FILE **fp, char *pPath)                           *
*                                                                             *
*******************************************************************************
*
*   Tries to open a source file given its default path and path/name
*
*   Where:
*       fp will be set if a file is opened
*       pPath is the root source path format
*
*   Returns:
*       FALSE - File could not be found
*       TRUE - File is found and opened, fp is set
*
******************************************************************************/
BOOL OpenUserSourceFile(FILE **fp, char *pPath)
{
    // If we have a user substitution path, apply it first
    if( pPathPrefix )
    {
        // Try to match the path prefix with the given path
        if( !strnicmp(pPath, pPathPrefix, strlen(pPathPrefix)) )
        {
            // The path prefix matched positively. Apply the substitution.
            strcpy(sPath, pPathNew);
            strcat(sPath, pPath + strlen(pPathPrefix));
        }
        else
        {
            // We could not match the path, so use what we are given
            strcpy(sPath, pPath);
        }

        VERBOSE1 printf("SUBST Source path = %s\n", sPath);
    }
    else
        strcpy(sPath, pPath);

    // Try to open this file to see if the path is correct
    if( (*fp = fopen(sPath, "rt")) )
    {
        return( TRUE );         // File found
    }

    return( FALSE );            // File not found
}


/******************************************************************************
*                                                                             *
*   void OptTranslate                                                         *
*       (char *pathOut, char *pathIn, char *pPathSubst)                       *
*                                                                             *
*******************************************************************************
*
*   Translate an ELF file into a symbol file and write out the new symbol file.
*
*   Several types of files that can be translated are:
*       ASCII symbol file produced by the 'nm' command
*       ELF user mode executable file
*       ELF Kernel module object file
*
*   Where:
*       pathOut - Creating this symbol file
*       pathIn  - Source file to translate
*       pPathSubst - Optional user path substitution string
*
******************************************************************************/
void OptTranslate(char *pathOut, char *pathIn, char *pPathSubst)
{
    FILE *fin;
    struct stat prop;
    char elf[4];
    int fd_out, fd_in, status;
    BYTE *pBuf;
    char *pTableName;
    time_t lapse;

    if( ProcessSubstitutionPath(pPathSubst) )
    {
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
                        // Determine the internal module name based on the input file name
                        if(strrchr(pathIn, '/')!=NULL)
                            pTableName = strrchr(pathIn, '/') + 1;
                        else
                            pTableName = pathIn;

                        fd_in = fileno(fin);
                        if( elf[1]=='E' && elf[2]=='L' && elf[3]=='F' )
                        {
                            time(&lapse);

                            // ELF file
                            pBuf = LoadElf(pathIn);
                            if( pBuf )
                            {
                                ElfToSym(pBuf, pathOut, pTableName);
                            }

                            lapse = time(NULL) - lapse;
                            VERBOSE1 printf("Complete in %ld sec\n", lapse? lapse : 1);
                        }
                        else
                            fprintf(stderr, "Invalid ELF file\n");
                    }
                    else
                        fprintf(stderr, "Type of input file %s is not supported\n", pathIn);

                    fclose(fin);
                }
                else
                    fprintf(stderr, "Error opening input file %s\n", pathIn);

                close(fd_out);
            }
            else
                fprintf(stderr, "Access violation writing symbol file %s\n", pathOut);
        }
        else
        {
            switch(status)
            {
            case EACCES:
                fprintf(stderr, "Error accessing input file %s\n", pathIn);
                break;
            case EBADF:
                fprintf(stderr, "Bad input file %s\n", pathIn);
                break;
            case ENOENT:
                fprintf(stderr, "Input file %s unreachable\n", pathIn);
                break;
            default:
                fprintf(stderr, "Unable to stat input file %s\n", pathIn);
            }
        }
    }
    else
        fprintf(stderr, "Invalid path substitution format %s\n", pPathSubst);
}
