/******************************************************************************
*                                                                             *
*   Module:     ParseFunctionLines.c                                          *
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

extern WORD GetFileId(char *pSoDir, char *pSo);
extern BOOL GlobalsName2Address(DWORD *p, char *pName);

/******************************************************************************
*                                                                             *
*   BOOL ParseFunctionLines(int fd, int fs, BYTE *pBuf)                       *
*                                                                             *
*******************************************************************************
*
*   Loads and parses function lines tokens.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
*   Returns:
*       TRUE - Function lines parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseFunctionLines(int fd, int fs, BYTE *pBuf)
{
    TSYMFNLIN  Header;                  // Function line section header
    TSYMFNLIN1 list;                    // Line record
    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position
    int nLines = 0;                     // Number of lines described in a function
    int nCurrentSection;                // Current string section offset
    int nSectionSize;                   // Current section string size

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
    BOOL fFun = FALSE;                  // Are we inside a function block?
    int i;

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE FUNCTION LINES                                            ||\n");
    VERBOSE2 printf("=============================================================================\n");
    VERBOSE1 printf("Parsing function lines.\n");

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
    // We parse STABS exactly as we do a generic ELF section parsing,
    // but here we extract only basic function parameters and line numbers

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

                case N_FUN:
                    VERBOSE2 printf("FUN---");
                    if( *pStr==0 )
                    {
                        // Function end
                        VERBOSE2 printf("END--------- +%lX\n\n", pStab->n_value);

                        // We are out of a function block
                        fFun = FALSE;

                        // At this point we know the total size of the header
                        // as well as the function ending address. Fill in the
                        // missing information and rewrite the header
                        Header.h.dwSize     = sizeof(TSYMFNLIN) + sizeof(TSYMFNLIN1) * (nLines-1);
                        Header.dwEndAddress = Header.dwStartAddress + pStab->n_value - 1;
                        Header.nLines       = nLines;

                        lseek(fd, fileOffset, SEEK_SET);

                        write(fd, &Header, sizeof(TSYMFNLIN)-sizeof(TSYMFNLIN1));
                        lseek(fd, 0, SEEK_END);
                    }
                    else
                    {
                        // We are inside a function block
                        fFun = TRUE;

                        // We will write a header but later, on an function end,
                        // rewind and rewite it with the complete information
                        // This we do so we can simply keep adding file lines as
                        // TSYMFNLIN1 array...
                        Header.h.hType        = HTYPE_FUNCTION_LINES;
                        Header.h.dwSize       = sizeof(TSYMFNLIN)-sizeof(TSYMFNLIN1);
                        Header.dwStartAddress = pStab->n_value;
                        Header.dwEndAddress   = 0;      // To be written later
                        Header.nLines         = 0;      // To be written later

                        // If the start address is not defined (0?) and this is an object file
                        // (kernel module), we can search the global symbols for the address
                        if( Header.dwStartAddress==0 && GlobalsName2Address(&Header.dwStartAddress, pStr) )
                            ;

                        // Print function start & name
                        VERBOSE2 printf("START-%08X--%s\n", Header.dwStartAddress, pStr);

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
                    VERBOSE2 printf("SLINE  line: %2d -> +%lX  file_id: %d\n", pStab->n_desc, pStab->n_value, file_id);

                    // Write out one line record only if we are inside a function block
                    if( fFun )
                    {
                        list.file_id = file_id;
                        list.line    = pStab->n_desc;
                        list.offset  = (WORD) pStab->n_value;

                        write(fd, &list, sizeof(TSYMFNLIN1));

                        nLines++;
                    }
                break;

                case N_SO:
                    VERBOSE2 printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        VERBOSE2 printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        VERBOSE2 printf("=========================================================\n");
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
        printf("No STAB section in the file\n");

    return( FALSE );
}
