/******************************************************************************
*                                                                             *
*   Module:     ParseStatic.c                                                 *
*                                                                             *
*   Date:       02/12/04                                                      *
*                                                                             *
*   Copyright (c) 2004-2005 Goran Devic                                       *
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
* 02/12/04   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "Common.h"                     // Include platform specific set

#include "loader.h"                     // Include global protos

extern PSTR dfs;

extern WORD GetFileId(char *pSoDir, char *pSo);
extern BYTE GlobalsName2SectionNumber(char *pName);

/******************************************************************************
*                                                                             *
*   BOOL StoreStaticVariableData(int fd, FILE *fStatics, int nStatics, int file_id)
*                                                                             *
*******************************************************************************
*
*   Reads the temp file containing the data for the static array and writes out
*   the static array.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fStatics - temp file handle containing static data
*       nStatics - number of static data
*       file_id  - file id of the static blob
*
*   Returns:
*       TRUE - stored zero or more static data
*       FALSE - error (can't allocate memory)
*
******************************************************************************/
static BOOL StoreStaticVariableData(int fd, FILE *fStatics, int nStatics, int file_id)
{
    TSYMSTATIC *pStatic;                // Pointer to the buffer
    TSYMSTATIC1 *pStatic1;              // Pointer to the element
    int nSegment;                       // Variable segment number

    // Rewind the temporary file
    fseek(fStatics, 0L, SEEK_SET);

    if( !nStatics )
        return( TRUE );

    // Allocate buffer to read all of the static data
    if( (pStatic = malloc(sizeof(TSYMSTATIC) + (nStatics-1) * sizeof(TSYMSTATIC1))) )
    {
        VERBOSE2 printf("Storing static %d data for file_id=%d\n", nStatics, file_id);

        // Stuff the header of the static symbol data sturcture
        pStatic->h.hType = HTYPE_STATIC;
        pStatic->h.dwSize = sizeof(TSYMSTATIC) + (nStatics-1) * sizeof(TSYMSTATIC1);
        pStatic->nStatics = nStatics;
        pStatic->file_id  = file_id;

        // Read in the data from the temp file and store it in the array
        pStatic1 = &pStatic->list[0];

        while( nStatics-- )
        {
            fscanf(fStatics, "%08X %d %d %d\n", &pStatic1->dwAddress, (int *) &pStatic1->pName, (int *) &pStatic1->pDef, &nSegment);
            pStatic1->bSegment = (BYTE) nSegment;

            pStatic1++;
        }

        // Write out the complete array into the output handle
        write(fd, (void *) pStatic, pStatic->h.dwSize);

        // Free the buffer and rewind the temp file so we can reuse it
        free(pStatic);
        fseek(fStatics, 0L, SEEK_SET);
    }
    else
        fprintf(stderr, "Unable to allocate memory\n");

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL ParseStatic(int fd, int fs, BYTE *pBuf)                              *
*                                                                             *
*******************************************************************************
*
*   Loads and parses static symbols from various ELF sections.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
******************************************************************************/
BOOL ParseStatic(int fd, int fs, BYTE *pBuf)
{
    WORD file_id = 0;                   // Current file ID number
    int nCurrentSection;                // Current string section offset
    int nSectionSize;                   // Current section string size
    int nSegment;                       // Variable segment number

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
    char *pSoDir = "";                  // Source code directory
    char *pSo = "";                     // Current source file
    int i, nLen;
    FILE *fStatics;                     // Temp file descriptor for static descriptors
    int nStatics;                       // Number of static symbols in one file


    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE STATIC                                                    ||\n");
    VERBOSE2 printf("=============================================================================\n");

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

    // Create a temp file to store static definitions
    fStatics = tmpfile();

    // Reset the static symbol counter
    nStatics = 0;

    //=========================
    // Parse STABS
    //=========================
    // We parse STABS exactly as we do a generic ELF section parsing,
    // but here we extract only static symbols

    if( SecStab && SecStabstr )
    {
        // Parse stab section
        pStab = (StabEntry *) ((char*)pElfHeader + SecStab->sh_offset);
        i = SecStab->sh_size / sizeof(StabEntry);
        nCurrentSection = 0;
        nSectionSize = 0;
        while( i-- )
        {
            pStr = (char *)pElfHeader + SecStabstr->sh_offset + pStab->n_strx + nCurrentSection;

            switch( pStab->n_type )
            {
                // 0x00 (N_UNDEF) is actually storing the current section string size
                case N_UNDF:
                    // We hit another string section, need to advance the string offset of the previous section
                    nCurrentSection += nSectionSize;
                    // Save the (new) currect string section size
                    nSectionSize = pStab->n_value;

                    VERBOSE2 printf("HdrSym size: %lX\n", pStab->n_value);
                break;

                case N_STSYM:
                    VERBOSE2 printf("STSYM: file_id=%d  %s\n", file_id, pStr);

                    // Found a static symbol, write out the address and offsets to name and definition

                    // Get the segment number while we have the name string intact
                    nSegment = GlobalsName2SectionNumber(pStr);

                    //--------------------------------------------------------------------------------
                    // Print the address and the offset to the name (first)
                    fprintf(fStatics, "%08X %d ", (DWORD) pStab->n_value, (int) dfs);

                    nLen = strchr(pStr, ':') - pStr;        // Get the length of the symbol name part

                    write(fs, pStr, nLen);                  // Write the name into strings
                    dfs += nLen;

                    // Write terminate string with 0
                    write(fs, "", 1);               // This will append 0
                    dfs += 1;

                    //--------------------------------------------------------------------------------
                    // Print the offset to the definition
                    fprintf(fStatics, "%d ", (int) dfs);    // Continue with the offset to the definition

                    pStr = pStr + nLen + 1;                 // Point to the definition string
                    nLen = strlen(pStr) + 1;                // Add 1 for the zero-termination

                    write(fs, pStr, nLen);
                    dfs += nLen;

                    //--------------------------------------------------------------------------------
                    // Print the segment number in which the variable resides
                    fprintf(fStatics, "%d\n", nSegment);    // End with the segment number

                    // Increment the number of static symbols
                    nStatics++;

                break;

                case N_SO:
                    VERBOSE2 printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        VERBOSE2 printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        VERBOSE2 printf("=========================================================\n");

                        // Dump the static data that we found
                        StoreStaticVariableData(fd, fStatics, nStatics, file_id);

                        // Reset the static symbol counter
                        nStatics = 0;
                    }
                    else
                    {
                        if( *(pStr + strlen(pStr) - 1)=='/' )
                        {
                            // Directory
                            VERBOSE2 printf("Source directory: %s\n", pStr);

                            // Store the pointer to a directory so we can use it later for
                            // SO and SOL stabs
                            pSoDir = pStr;
                        }
                        else
                        {
                            // File
                            VERBOSE2 printf("Source file: %s\n", pStr );

                            // Store the pointer to a file as a current source file
                            pSo = pStr;
                        }
                        file_id = GetFileId(pSoDir, pSo);
                    }
                break;

                case N_SOL:
                    VERBOSE2 printf("SOL  ");
                    VERBOSE2 printf("%s\n", pStr);

                    // Change of source - this is either a complete path/name or just
                    // a file name in which case we keep last path
                    pSo = pStr;

                    file_id = GetFileId(pSoDir, pSo);
                break;
            }

            pStab++;
        }

        return( TRUE );
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( FALSE );
}

