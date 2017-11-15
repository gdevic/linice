/******************************************************************************
*                                                                             *
*   Module:     ParseSource.c                                                 *
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
*   BOOL ParseSource1(int fd, int fs, char *ptr, WORD file_id)                *
*                                                                             *
*******************************************************************************
*
*   Loads and parses a single source file
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       ptr - file path name string
*       file_id - file_id to assign to this file
*
******************************************************************************/
BOOL ParseSource1(int fd, int fs, char *ptr, WORD file_id)
{
    int nLines, i;                      // Running count of number of lines
    TSYMSOURCE *pHeader;                // Source header structure
    DWORD dwSize;                       // Final size of the above structure
    FILE *fp = NULL;                    // Source file descriptor
    char pTmp[FILENAME_MAX];            // Temporary buffer
    BOOL fRet = FALSE;                  // Assume we failed

#define MAX_LINE_LEN    1024            // Max allowable line in a source code
#define MAX_LINE        255             // Max line that we will store
    char sLine[MAX_LINE_LEN];           // Single source line

    // Open the source file. If it can't be opened for some reason, display
    // message and input the new file path/name from the console
    strcpy(pTmp, ptr);
    while( fp==NULL )
    {
        // Strip trailing 0A, 0D characters
        if( strchr(pTmp, 0x0A) )
            *strchr(pTmp, 0x0A) = 0;

        if( strchr(pTmp, 0x0D) )
            *strchr(pTmp, 0x0D) = 0;

        fp = fopen(pTmp, "rt");
        if( fp==NULL )
        {
            printf("File %s not found. Enter the file path/name or just press ENTER to skip it:\n", pTmp);
            fgets(pTmp, FILENAME_MAX, stdin);
            if( pTmp[0]=='\n' )
                return( TRUE );
        }
    }

    // Count the number of lines in a source file
    nLines = 0;
    while( !feof(fp) )
    {
        char *p;

        p = fgets(sLine, MAX_LINE_LEN, fp);
        if(p==NULL) break;
        printf("%3d: %2d %s", nLines + 1, strlen(sLine), sLine);
        nLines++;
    }

    if( nLines==0 )
        return( FALSE );

    // Allocate the buffer for a header structure + line array
    dwSize = sizeof(TSYMSOURCE) + sizeof(DWORD)*(nLines-1);
    pHeader = (TSYMSOURCE *) malloc(dwSize);

    pHeader->hType       = HTYPE_SOURCE;
    pHeader->dwSize      = dwSize;
    pHeader->file_id     = file_id;
    pHeader->nLines      = nLines;
    pHeader->dSourcePath = dfs;
    pHeader->dSourceName = dfs;

    // Write the string - source path and name
    write(fs, pTmp, strlen(pTmp)+1);
    dfs += strlen(pTmp)+1;

    // Reset the file pointer into our source file
    fseek(fp, 0, SEEK_SET);

    for( i=0; i<nLines; i++ )
    {
        fgets(sLine, MAX_LINE_LEN, fp);

        // Do a small file size optimization: loop from the back to the front
        // of a line and cut all trailing spaces, 0A and 0D characters (newlines)
        ptr = sLine + strlen(sLine) - 1;
        while( ptr>=sLine && (*ptr==' ' || *ptr==0x0A || *ptr==0x0D) )
        {
            *ptr-- = 0;
        }

        // Write a line to the strings file
        pHeader->dLineArray[i] = dfs;
        write(fs, sLine, strlen(sLine)+1);
        dfs += strlen(sLine)+1;
    }

    // Lastly, write out the header structure to fd
    write(fd, pHeader, dwSize);

    free(pHeader);

    // Close the source file descriptor
    fclose(fp);

    return( fRet );
}


int nSources = 0;                       // Counter variables
char *pSources = NULL;                  // Memory buffer to store file names

/******************************************************************************
*                                                                             *
*   BOOL SetupSourcesArray(FILE *fSources)                                    *
*                                                                             *
*******************************************************************************
*
*   Sets up the array containing the source file names. Prunes duplicates.
*
*   Where:
*       fSources - queue of source file path names to store (have duplicates)
*
******************************************************************************/
BOOL SetupSourcesArray(FILE *fSources)
{
    char *pSourcesTop;                  // Pointer to the last source stored
    char *ptr;                          // Pointer to a current source file name
    int nLen;                           // File len
    char pTmp[FILENAME_MAX];            // Temporary buffer
    struct stat fd_stat;                // file stats

    // We read all sources that were queued up in the fSources temp file,
    // and then prune the duplicate files.

    // Get the file stats
    fstat(fileno(fSources), &fd_stat);

    nLen = fd_stat.st_size;
    if( nLen==0 )
    {
        printf("No sources to load!\n");
        return( TRUE );
    }

    // Allocate memory to keep the file names that we will read from the fSources
    pSources = pSourcesTop = (char*) malloc(nLen);
    if( pSources!=NULL )
    {
        // Reposition the queue file to the start
        fseek(fSources, 0, SEEK_SET);

        printf("Unique sources:\n");

        while( !feof(fSources) )
        {
            // Read in a single file name
            fgets(pTmp, FILENAME_MAX, fSources);

            // Get rid of the 0x0A and 0x0D trailing characters
            if( strchr(pTmp, 0x0A) )
                *strchr(pTmp, 0x0A) = 0;

            if( strchr(pTmp, 0x0D) )
                *strchr(pTmp, 0x0D) = 0;

            // Compare it to all the file names already loaded
            ptr = pSources;
            while( ptr<pSourcesTop )
            {
                if( !strcmp(ptr, pTmp) )
                    break;

                ptr += strlen(ptr) + 1;
            }

            if( ptr>=pSourcesTop )
            {
                // A new file to put on the list
                strcpy(pSourcesTop, pTmp);
                pSourcesTop += strlen(pTmp);
                *pSourcesTop++ = 0;     // Null-terminate the string

                nSources++;             // Increment counter

                printf("file_id: %d  %s\n", nSources, pTmp);
            }
        }

        return( TRUE );
    }
    else
        printf("Unable to allocate memory\n");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL ParseSource(fd, fs)                                                  *
*                                                                             *
*******************************************************************************
*
*   Loads and parses source files and stores them
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*
******************************************************************************/
BOOL ParseSource(int fd, int fs)
{
    char *ptr;                          // Pointer to a current source file name
    int i;                              // Generic counter
    WORD file_id;                       // File_id counter to assign

    printf("=============================================================================\n");
    printf("||         PARSE SOURCES                                                   ||\n");
    printf("=============================================================================\n");

    file_id = 1;                        // Start with file ID of 1

    // We loop for each source file, load and parse it, and write it out
    ptr = pSources;
    for( i=0; i<nSources; i++ )
    {
        ParseSource1(fd, fs, ptr, file_id);
        file_id++;                      // Increment file number
        ptr += strlen(ptr) + 1;         // And select next file path/name
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   WORD GetFileId(char *pSoDir, char *pSo)                                   *
*                                                                             *
*******************************************************************************
*
*   Given the path and file name, search the array of sources and find which
*   equals.
*
*   Where:
*       pSoDir - path to the file
*       pSo - file name or a full path/name
*
******************************************************************************/
WORD GetFileId(char *pSoDir, char *pSo)
{
    char PathName[FILENAME_MAX];        // Temp buffer to store final string
    char *pPathName;                    // Pointer to a path name string
    char *ptr;                          // Pointer to a current source file name
    int i;                              // Generic counter

    // If file name does contain '/' or '\' characters, use it as is since it
    // already is a path/name. If not, prefix the given path.
    if( !strchr(pSo, '/') && !strchr(pSo, '\\') )
    {
        // We need to prefix given path and use that
        strcpy(PathName, pSoDir);
        strcat(PathName, pSo);
        pPathName = PathName;
    }
    else
        pPathName = pSo;

    // Search the loaded source names and try to find the match
    ptr = pSources;
    for( i=0; i<nSources; i++ )
    {
        if( !strcmp(ptr, pPathName) )
        {
            return( i+1 );
        }

        ptr += strlen(ptr) + 1;
    }

    return( 0 );
}

