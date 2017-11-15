/******************************************************************************
*                                                                             *
*   Module:     symutils.c                                                    *
*                                                                             *
*   Date:       10/19/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

// TODO: Broken implementation. Since we dont traverse all the subdirectories,
//       it is pointless to keep the exclusion directory list.

typedef struct
{
    char sDir[MAX_PATH];                // Path name
    BOOL fInclude;                      // Include or exclude this directory

} TDIR;

static FILE *fPath;                     // Queue file containing source paths
static int nPath;                       // Number of custom paths
static TDIR *dir;                       // Directory list

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
*   BOOL DirAdd(char *pName)                                                  *
*                                                                             *
*******************************************************************************
*
*   Adds a directory to the list of directories:
*   It also decodes the directory path: if the first character is
*   '-' exclude this directory
*   '+' or '/' include this directory
*
*   Where:
*       pName is the directory name
*
*   SIDE-EFFECT:
*       This string will be modified!
*
*   Returns:
*       TRUE - add ok
*       FALSE - misformatted directory name or other (critical) error
*
******************************************************************************/
static BOOL DirAdd(char *pName)
{
    BOOL fInclude = TRUE;               // By default, include directory
    int nLen;                           // Length of the directory

    if( pName && *pName )
    {
        // Skip the leading spaces
        while( *pName==' ' ) pName++;

        // Zero out the trailing spaces and other non-characters
        nLen = strlen(pName);
        while( nLen && !isprint(pName[nLen-1]) ) nLen--;

        if( nLen )
        {
            // Terminate the name string
            pName[nLen] = 0;

            // The first character can be +,- or directory root
            switch( *pName )
            {
                case '-':   fInclude = FALSE;
                            pName++;
                    break;
                case '+':   fInclude = TRUE;
                            pName++;
                    break;
            }

            // Change all backward-slashes into forward slashes
            while( nLen )
            {
                if( pName[nLen]=='\\' )
                    pName[nLen] = '/';
                nLen--;
            }

            // Add only one directory by default
            // Zap the trailing '/'
            nLen = strlen(pName);
            if( pName[nLen-1]=='/' )
                pName[nLen-1] = 0;

            // Write out a custom file path and the include status
            fprintf(fPath, "%s %d\n", pName, fInclude);
            VERBOSE1 printf(" (%c) %s\n", fInclude? '+':'-', pName);

            nPath++;
        }

        return( TRUE );
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL ProcessSourcesPath(char *pathSources)                                *
*                                                                             *
*******************************************************************************
*
*   Given the optional set of source paths, add them to the list.
*   Alternatively, a file name can be given that contains a set of source paths.
*
*   Where:
*       pathSources is the set of source paths
*
*   Returns:
*       TRUE for ok
*       FALSE if there was a problem adding a source path line
*
******************************************************************************/
BOOL ProcessSourcesPath(char *pathSources)
{
    static char sPath[MAX_PATH];
    struct stat status;
    char *pEnd, *pPath;
    FILE *fp;
    int i;

    // Path to the sources comes in two flavors: directly specified string, or
    // a file containing more detailed path specification
    //
    // /usr/src/code;/usr/src/code/part             <= separation by ';'
    // path-spec.txt                                <= path specification in a file

    // Parse each specification, they should be separated by ';'
    pPath = pathSources;

    do
    {
        // Copy the current path into our buffer

        // Find the end of this specification
        pEnd = strchr(pPath, ';');
        if( pEnd )
            strncpy(sPath, pPath, pEnd-pPath);
        else
            strcpy(sPath, pPath);

        // If a path is in fact a file, consider it a path specification file, and
        // process it line by line as it were a list of paths
        if( stat(sPath, &status)==0 )
        {
            if( S_IFREG & status.st_mode )
            {
                // The file is a regular file. Open it and parse it.
                fp = fopen(sPath, "r");
                if( fp )
                {
                    do
                    {
                        // Read a single line of a file
                        fgets(sPath, MAX_PATH, fp);

                        if( !DirAdd(sPath) )
                            return( FALSE );

                    }while(!feof(fp));

                    fclose(fp);

                    continue;
                }
                else
                {
                    fprintf(stderr, "Error opening path descriptor file %s\n", sPath);
                    return( FALSE );
                }
            }
            else
            {
                // The path specifies a directory
                ;
            }
        }
        else
        {
            fprintf(stderr, "Bad path specification: %s\n", sPath);
            return( FALSE );
        }

        // Add the current directory string specification
        if( !DirAdd(sPath) )
            return( FALSE );

        pPath = pEnd + 1;
    }
    while( pEnd );

    // We have stored all the custom paths, now we need to read them into the buffer
    dir = malloc(nPath * sizeof(TDIR));
    if( dir )
    {
        // Rewind the temp file so we read from the start
        fseek(fPath, 0L, SEEK_SET);

        for(i=0; i<nPath; i++)
            fscanf(fPath, "%s %d\n", dir[i].sDir, &dir[i].fInclude);

        return( TRUE );
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL FindPathExclusion(char *pPath)                                       *
*                                                                             *
*******************************************************************************
*
*   Searches a set of paths and returns the state of inclusion.
*
*   Where:
*       pPath is the complete path name of the file
*
*   Returns:
*       FALSE - Path is found and it is excluded
*       TRUE  - Path is either not found or it is included
*
******************************************************************************/
static BOOL FindPathExclusion(char *pPath)
{
    int i;
    int nPathLen, nDirLen;
    BOOL fInclude = TRUE;

    nPathLen = strlen(pPath);

    for(i=0; i<nPath; i++)
    {
        nDirLen  = strlen(dir[i].sDir);
        if( nDirLen<nPathLen && pPath[nDirLen]=='/' )
        {
            if( !strnicmp(pPath, dir[i].sDir, nDirLen) )
                fInclude = dir[i].fInclude;
        }
    }

    return( fInclude );
}

/******************************************************************************
*                                                                             *
*   BOOL OpenSourceFile(FILE **fp, char *pPath)                               *
*                                                                             *
*******************************************************************************
*
*   Tries to open a source file given its default path and path/name
*   Retries with the set of custom paths, including the exclusion paths.
*
*   Where:
*       fp will be set if a file is opened
*       pPath is the root source path format
*
*   Returns:
*       FALSE - File could not be found
*       TRUE - File is found, but:
*           fp = NULL - file should not be opened since it is on an excluson path
*           fp != NULL - file is opened
*
******************************************************************************/
BOOL OpenSourceFile(FILE **fp, char *pPath)
{
    static char sPath[MAX_PATH];
    int i;

    // TODO: There is an inherent bug in what we are doing. Read below...
    // Since we lose the notion of the root path versus the relative file path/name in the
    // SO and SODIR merge (ParseSource.c), we can't tell whether a file with the same name
    // is from the correct directory. Example: If we have two files src.c, in different
    // directories, this code will not be able to tell them apart when comparing with
    // different custom paths, but only the first occurence will be considered.

    *fp = NULL;

    // Concatenate the root path and the source path so we can look for the exclusion
    strcpy(sPath, pPath);

    // Look if the path is a superset for the exclusion, and return if the file is excluded
    if( FindPathExclusion(sPath)==FALSE )
        return( TRUE );                 // Default path is excluded

    // Try all the user inclusion paths and see if we can find the file
    for(i=0; i<nPath; i++)
    {
        // Only for the inclusion paths, dont do it for the exclusion paths
        if( dir[i].fInclude )
        {
            // Form the alternate root path name
            strcpy(sPath, dir[i].sDir);
            strcat(sPath, strrchr(pPath, '/'));

            // Try to open this file to see if the path is correct
            if( (*fp = fopen(sPath, "rt")) )
            {
                // We were able to open this file, but is the path excluded?
                if( FindPathExclusion(sPath)==TRUE )
                    return( TRUE );     // OK - path is allowed, and the file is opened

                fclose(*fp);            // Close the file since the path is not allowed
                *fp = NULL;

                return( TRUE );         // File found, but the path is excluded
            }
            // We could not find the file there. Continue with the next custom path...
        }
    }

    return( FALSE );                    // File not found in all paths
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
    time_t lapse;

    // Create a temp file to store user source paths
    fPath = tmpfile();                  // Temp file for paths
    nPath = 0;                          // Number of file paths read

    if( ProcessSourcesPath(pathSources) )
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
                        // Determine the internal module name based off the input file name
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
                        }
                        else
                        {
                            // ASCII map symbol file
                            TranslateMapFile(fd_out, fin, pathIn, prop.st_size);
                        }

                        lapse = time(NULL) - lapse;
                        VERBOSE1 printf("Complete in %ld sec\n", lapse? lapse : 1);
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

    fclose(fPath);
}
