/******************************************************************************
*                                                                             *
*   Module:     ParseSource.c                                                 *
*                                                                             *
*   Date:       09/05/00                                                      *
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

#include "loader.h"                     // Include global protos

extern PSTR dfs;                        // Global pointer to strings (to append)

//****************************************************************************
//
// Assumption: There can be at max 65535 source files and each file can have
//             at most 65535 lines.
//             Each function can be at most 64K long (line offsets)
//
// Assumption: C function tmpfile() will never fail
//
//****************************************************************************

static int nSO = 0;                     // Total number of source files
static int nSources = 0;                // Pruned number of source files
static char *pSources = NULL;           // Memory buffer to store file names

extern BOOL OpenUserSourceFile(FILE **fp, char *pPath);

/******************************************************************************
*                                                                             *
*   BOOL SetupSourcesArray(FILE *fSources, long maxLen)                       *
*                                                                             *
*******************************************************************************
*
*   Sets up the array containing the source file names. Prunes duplicates.
*
*   Where:
*       fSources - queue of source file path names to store (have duplicates)
*       maxLen - total length of the non-pruned strings (the len of the fSources file)
*
*   Return:
*       TRUE - Sources array set up: pSources, nSources
*       FALSE - Critical error setting up
*
******************************************************************************/
static BOOL SetupSourcesArray(FILE *fSources, long maxLen)
{
    char *pSourcesTop;                  // Pointer to the last source stored
    char *ptr;                          // Pointer to a current source file name
    char pTmp[FILENAME_MAX];            // Temporary buffer

    // We read all sources that were queued up in the fSources temp file,
    // and then prune the duplicate files.

    // At this point we assume there is at least one source file
    ASSERT(maxLen);

    // Allocate memory to keep the file names that we will read from the fSources
    pSources = pSourcesTop = (char*) malloc(maxLen);
    if( pSources!=NULL )
    {
        // Reposition the queue file to the start
        fseek(fSources, 0, SEEK_SET);

        VERBOSE1 printf("Unique source files:\n");

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

                nSources++;             // Increment the counter

                VERBOSE1 printf("  file_id: %d  %s\n", nSources, pTmp);
            }
        }

        return( TRUE );
    }
    else
        fprintf(stderr, "Unable to allocate memory\n");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL StoreSourceFiles(BYTE *pBuf)                                         *
*                                                                             *
*******************************************************************************
*
*   Loads and stores source file names (and paths) for later references.
*
*   Where:
*       pBuf - buffer containing the ELF file
*
*   Return:
*       TRUE - Sources array set up: pSources, nSources
*       FALSE - Critical error setting up
*
******************************************************************************/
BOOL StoreSourceFiles(BYTE *pBuf)
{
    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecStab = NULL;         // Section .STAB
    Elf32_Shdr *SecStabstr = NULL;      // Section .STABSTR
    Elf32_Shdr *SecSymtab = NULL;       // Section .SYMTAB
    Elf32_Shdr *SecStrtab = NULL;       // Section .STRTAB

    StabEntry *pStab;                   // Pointer to a stab entry
    char *pStr;                         // Pointer to a stab string
    BOOL fCont;                         // Line continuation?
    char *pSoDir = NULL;                // Source code directory
    int i;
    int nCurrentSection;                // Current string section offset
    int nSectionSize;                   // Current section string size
    FILE *fSources;                     // Temp file descriptor for source file names

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE SOURCE REFERENCES                                         ||\n");
    VERBOSE2 printf("=============================================================================\n");

    ASSERT(sizeof(StabEntry)==12);

    pElfHeader = (Elf32_Ehdr *) pBuf;

    // Ok, we have the complete file inside the buffer...
    // Find the section header and the string table of section names
    Sec = (Elf32_Shdr *) &pBuf[pElfHeader->e_shoff];
    SecName = &Sec[pElfHeader->e_shstrndx];

    for( i=1; i<pElfHeader->e_shnum; i++ )
    {
        SecCurr = &Sec[i];
        pStr = (char *)pBuf + SecName->sh_offset + SecCurr->sh_name;

        if( strcmp(".stab", pStr)==0 )
            SecStab = SecCurr;
        else
        if( strcmp(".stabstr", pStr)==0 )
            SecStabstr = SecCurr;
        else
        if( strcmp(".symtab", pStr)==0 )
            SecSymtab = SecCurr;
        else
        if( strcmp(".strtab", pStr)==0 )
            SecStrtab = SecCurr;
    }

    // Create a temp file to store source file names
    fSources = tmpfile();


    //=========================
    // Parse STABS for source file references
    //=========================
    if( SecStab && SecStabstr )
    {
        // Parse stab section
        pStab = (StabEntry *) (pBuf + SecStab->sh_offset);
        i = SecStab->sh_size / sizeof(StabEntry);
        nCurrentSection = 0;
        nSectionSize = 0;
        fCont=FALSE;
        while( i-- )
        {
            pStr = (char *)pBuf + SecStabstr->sh_offset + pStab->n_strx + nCurrentSection;
#if 0
            if(opt & OPT_VERBOSE)
            {
                printf("unsigned long n_strx  = %08X\n", pStab->n_strx );
                printf("unsigned char n_type  = %02X\n", pStab->n_type );
                printf("unsigned char n_other = %02X\n", pStab->n_other );
                printf("unsigned short n_desc = %04X\n", pStab->n_desc );
                printf("unsigned long n_value = %08X\n", pStab->n_value );
            }
#endif

            switch( pStab->n_type )
            {
                // 0x00 (N_UNDEF) is actually storing the current section string size
                case N_UNDF:
                    // We hit another string section, need to advance the string offset of the previous section
                    nCurrentSection += nSectionSize;
                    // Save the (new) currect string section size
                    nSectionSize = pStab->n_value;

                    VERBOSE2 printf("HdrSym size: %lX\n", pStab->n_value);
                    VERBOSE2 printf("=========================================================\n");
                break;

                case N_SO:
                    VERBOSE2 printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        VERBOSE2 printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        VERBOSE2 printf("=========================================================\n");

                        nSO++;
                    }
                    else
                    {
                        if( *(pStr + strlen(pStr) - 1)=='/' )
                        {
                            // Directory
                            VERBOSE2 printf("Source directory: %s\n", pStr);

                            // Store the pointer to a directory so we can use it later for SO and SOL stabs
                            pSoDir = pStr;
                        }
                        else
                        {
                            // File
                            VERBOSE2 printf("Source file: %s\n", pStr );

                            // If the source file comes with a full path, use only that string
                            // (dont prepend the formal path string)
                            if( *pStr!='/' )
                                fprintf(fSources, "%s%s\n", pSoDir, pStr);
                            else
                                fprintf(fSources, "%s\n", pStr);
                        }
                    }
                break;

                case N_SOL:
                    VERBOSE2 printf("SOL  ");
                    VERBOSE2 printf("%s\n", pStr);

                    // Change of source - if the first character is not '/', we need to
                    // prefix the last defined file path to complete the directory
                    if( *pStr!='/' && *pStr!='\\' )
                    {
                        if( pSoDir )
                            fprintf(fSources, "%s", pSoDir);
                    }
                    fprintf(fSources, "%s\n", pStr);
                break;
            }

            pStab++;
        }

        // Set up the sources array
        if( nSO )
            return( SetupSourcesArray(fSources, ftell(fSources)) );

        VERBOSE2 printf("No sources to load!\n");
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL WriteSourceFile(int fd, int fs, char *ptr, WORD file_id)             *
*                                                                             *
*******************************************************************************
*
*   Loads and parses a single source file. Each line is limited to MAX_STRING
*   characters in width.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       ptr - file path name string
*       file_id - file_id to assign to this file
*
*   Returns:
*       TRUE - file is parsed and stored, could also be file is completely skipped
*       FALSE - Critical memory allocation error
*
******************************************************************************/
static BOOL WriteSourceFile(int fd, int fs, char *ptr, WORD file_id)
{
    int nLines, i;                      // Running count of number of lines
    BYTE bSpaces;                       // Number of heading spaces in a line
    TSYMSOURCE *pHeader;                // Source header structure
    DWORD dwSize;                       // Final size of the above structure
    FILE *fp = NULL;                    // Source file descriptor
    char pTmp[FILENAME_MAX];            // Temporary buffer
	char *pName;						// Temp file name pointer

    // Max allowable line length in a source code: This does not mean we will store
    // that complete line - we store only up to MAX_STRING characters of each line!
#define MAX_LINE_LEN    1024

    char sLine[MAX_LINE_LEN];           // Single source line

    // Open the source file. If it can't be opened for some reason, display
    // message and input the new file path/name from the console
    strncpy(pTmp, ptr, FILENAME_MAX-1);
    pTmp[FILENAME_MAX-1] = 0;

    while( fp==NULL )
    {
        // Strip trailing 0A, 0D characters
        if( strchr(pTmp, 0x0A) )
            *strchr(pTmp, 0x0A) = 0;

        if( strchr(pTmp, 0x0D) )
            *strchr(pTmp, 0x0D) = 0;

        if( OpenUserSourceFile(&fp, pTmp)==FALSE )
        {
            printf("Unable to open source file %s -- skipping...\n", pTmp);

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
        nLines++;
    }

    if( nLines==0 )
        return( TRUE );

    // Allocate the buffer for a header structure + line array
    dwSize = sizeof(TSYMSOURCE) + sizeof(DWORD)*(nLines-1);
    pHeader = (TSYMSOURCE *) malloc(dwSize);
    if( pHeader==NULL )
        return( FALSE );

    pHeader->h.hType     = HTYPE_SOURCE;
    pHeader->h.dwSize    = dwSize;
    pHeader->file_id     = file_id;
    pHeader->nLines      = nLines;
    pHeader->pSourcePath = dfs;
    pHeader->pSourceName = dfs;

	// Find the name proper (without the path)
	pName = strrchr(pTmp, '/');
	if(pName==NULL)
		pName = strrchr(pTmp, '\\');
	if(pName!=NULL)
        pHeader->pSourceName += pName - pTmp + 1;

    // Write the string - source path and name
	write(fs, pTmp, strlen(pTmp)+1);
    dfs += strlen(pTmp)+1;

    // Reset the file pointer into our source file
    fseek(fp, 0, SEEK_SET);

    for( i=0; i<nLines; i++ )
    {
        // Read the whole line - hopefully it will fit into our buffer
        // TODO: Can we do this differently? So we dont depend on the max source line size?
        fgets(sLine, MAX_LINE_LEN, fp);

        // Cut a line into the maximum allowable source line width
        sLine[MAX_STRING-1] = 0;

        // Do a small file size optimization: loop from the back to the front
        // of a line and cut all trailing spaces, tabs, 0A and 0D characters (newlines)
        ptr = strchr(sLine, '\0') - 1;
        while( ptr>=sLine && (*ptr==' ' || *ptr==0x09 || *ptr==0x0A || *ptr==0x0D) )
        {
            *ptr-- = 0;
        }

        // Second optimization: Trim all the spaces from the front of the line, the very
        // First BYTE in the source line is the number of spaces
        ptr = sLine;
        bSpaces = 0;
        while( *ptr++==' ' )
        {
            bSpaces++;
        }

        ptr--;
        // ptr now points to the real start of the source line, ignoring heading spaces

        // Another size optimization: If the line is empty then, we dont even store
        // { bSpaces, '\0' }  but point to the start of the strings area where we
        // already have { 0, 0 } pseudo-string

        if( strlen(ptr) )
        {
            // Size of the line is not zero - store it

            pHeader->pLineArray[i] = dfs;
            // Write the number of spaces as the first byte of line string
            write(fs, &bSpaces, 1);

            // Write a line (or whatever is left of it) to the strings file
            write(fs, ptr, strlen(ptr)+1);
            dfs += strlen(ptr)+1+1;
        }
        else
        {
            // Size of the line is zero - point to the start of the strings area

            pHeader->pLineArray[i] = 0;
        }
    }

    // Lastly, write out the header structure to fd
    write(fd, pHeader, dwSize);

    free(pHeader);

    // Close the source file descriptor
    fclose(fp);

    return( TRUE );
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
*   Returns:
*       TRUE - Sources written ok
*       FALSE - Critical error writing sources
*
******************************************************************************/
BOOL ParseSource(int fd, int fs)
{
    char *ptr;                          // Pointer to a current source file name
    int i;                              // Generic counter
    WORD file_id;                       // File_id counter to assign

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE SOURCES                                                   ||\n");
    VERBOSE2 printf("=============================================================================\n");

    file_id = 1;                        // Start with file ID of 1

    // We loop for each source file, load and parse it, and write it out
    ptr = pSources;
    for( i=0; i<nSources; i++ )
    {
        if( WriteSourceFile(fd, fs, ptr, file_id)==FALSE)
            return(FALSE);

        file_id++;                      // Increment file number
        ptr += strlen(ptr) + 1;         // And select next file path/name
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   WORD GetFileId(char *pSoDir, char *pSo)                                   *
*                                                                             *
*******************************************************************************
*
*   Given the path and the file name, search the array of all pruned sources and
*   find the index of the referenced one.
*
*   Where:
*       pSoDir - path to the file
*       pSo - file name or partial path/name
*
*   Returns:
*       0 - Source is not found
*       n > 0   - Index of the source file (file_id)
*
******************************************************************************/
WORD GetFileId(char *pSoDir, char *pSo)
{
    char PathName[FILENAME_MAX];        // Temp buffer to store final string
    char *pPathName;                    // Pointer to a path name string
    char *ptr;                          // Pointer to a current source file name
    int i;                              // Generic counter

    // If the first character is '/', use it since that is absolute path/name,
    // otherwise, concat the path name with the file name
    if( *pSo != '/' )
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

