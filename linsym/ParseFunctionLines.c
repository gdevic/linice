/******************************************************************************
*                                                                             *
*   Module:     ParseFunctionLines.c                                          *
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

extern WORD GetFileId(char *pSoDir, char *pSo);
extern DWORD GetGlobalSymbolAddress(char *pName);

/******************************************************************************
*                                                                             *
*   BOOL ParseFunctionLines(int fd, int fs, BYTE *pBuf)                       *
*                                                                             *
*******************************************************************************
*
*   Loads and parses a single source file
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
******************************************************************************/
BOOL ParseFunctionLines(int fd, int fs, BYTE *pBuf)
{
    TSYMFNLIN  Header;                  // Function line section header
    TSYMFNLIN1 list;                    // Line record
    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position
    int nLines = 0;                     // Number of lines described in a function

    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecStab = NULL;         // Section .STAB
    Elf32_Shdr *SecStabstr = NULL;      // Section .STABSTR
    Elf32_Shdr *SecSymtab = NULL;       // Section .SYMTAB
    Elf32_Shdr *SecStrtab = NULL;       // Section .STRTAB

    struct stat fd_stat;                // ELF file stats
    StabEntry *pStab;                   // Pointer to a stab entry
    char *pStr;                         // Pointer to a stab string
    char *pSoDir = "";                  // Source code directory
    char *pSo = "";                     // Current source file
    int i;

    printf("=============================================================================\n");
    printf("||         PARSE FUNCTION LINES                                            ||\n");
    printf("=============================================================================\n");

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

    //=========================
    // Parse STABS
    //=========================
    // We parse STABS exactly as we do in ParseSectionsPass1() function,
    // but here we extract only basic function parameters and line numbers

    if( SecStab && SecStabstr )
    {
        // Parse stab section
        pStab = (StabEntry *) ((char*)pElfHeader + SecStab->sh_offset);
        i = SecStab->sh_size / sizeof(StabEntry);
        while( i-- )
        {
            pStr = (char *)pElfHeader + SecStabstr->sh_offset + pStab->n_strx;

            switch( pStab->n_type )
            {
                case N_FUN:
                    printf("FUN---");
                    if( *pStr==0 )
                    {
                        // Function end
                        printf("END--------- +%lX\n\n", pStab->n_value);

                        // At this point we know the total size of the header
                        // as well as the function ending address. Fill in the
                        // missing information and rewrite the header
                        Header.dwSize       = sizeof(TSYMFNLIN) + sizeof(TSYMFNLIN1) * (nLines-1);
                        Header.dwEndAddress = Header.dwStartAddress + pStab->n_value - 1;
                        Header.nLines       = nLines;

                        // If we did not find any line numbers, all we do here is
                        // reposition the file pointer to the start of the header
                        // and do nothing
                        lseek(fd, fileOffset, SEEK_SET);

                        if( nLines > 0 )
                        {
                            write(fd, &Header, sizeof(TSYMFNLIN)-sizeof(TSYMFNLIN1));
                            lseek(fd, 0, SEEK_END);
                        }
                    }
                    else
                    {
                        // We will write a header but later, on an function end,
                        // rewind and rewite it with the complete information
                        // This we do so we can simply keep adding file lines as
                        // TSYMFNLIN1 array...
                        Header.hType          = HTYPE_FUNCTION_LINES;
                        Header.dwSize         = 0;      // To be written later
                        Header.dwStartAddress = pStab->n_value;
                        Header.dwEndAddress   = 0;      // To be written later
                        Header.nLines         = 0;      // To be written later

                        // If the start address is not defined (0?) and this is an object file
                        // (kernel module), we can search the global symbols for the address
                        if( Header.dwStartAddress==0 )
                            Header.dwStartAddress = GetGlobalSymbolAddress(pStr);

                        // Print function start & name
                        printf("START-%08X--%s\n", Header.dwStartAddress, pStr);

                        nLines = 0;

                        // Get the current file position so we can come back later

                        // Get the file stats
                        fstat(fd, &fd_stat);

                        fileOffset = fd_stat.st_size;

                        // Write the header first time
                        write(fd, &Header, sizeof(TSYMFNLIN)-sizeof(TSYMFNLIN1));
                    }
                break;

                case N_SLINE:
                    printf("SLINE  line: %2d -> +%lX  file_id: %d\n", pStab->n_desc, pStab->n_value, file_id);

                    // Write out one line record
                    list.file_id = file_id;
                    list.line    = pStab->n_desc;
                    list.offset  = (WORD) pStab->n_value;

                    write(fd, &list, sizeof(TSYMFNLIN1));

                    nLines++;
                break;

                case N_SO:
                    printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        printf("=========================================================\n");
                    }
                    else
                    {
                        if( *(pStr + strlen(pStr) - 1)=='/' )
                        {
                            // Directory
                            printf("Source directory: %s\n", pStr);

                            // Store the pointer to a directory so we can use it later for
                            // SO and SOL stabs
                            pSoDir = pStr;
                        }
                        else
                        {
                            // File
                            printf("Source file: %s\n", pStr );

                            // Store the pointer to a file as a current source file
                            pSo = pStr;
                        }
                        file_id = GetFileId(pSoDir, pSo);
                    }
                break;

                case N_SOL:
                    printf("SOL  ");
                    printf("%s\n", pStr);

                    // Change of source - this is either a complete path/name or just
                    // a file name in which case we keep last path
                    pSo = pStr;

                    file_id = GetFileId(pSoDir, pSo);
                break;
            }

            pStab++;
        }
    }
    else
        printf("No STAB section in the file\n");

    return( 0 );
}

