/******************************************************************************
*                                                                             *
*   Module:     ParseFunctionScope.c                                          *
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
extern BYTE GlobalsName2SectionNumber(char *pName);

/******************************************************************************
*                                                                             *
*   BOOL ParseFunctionScope(int fd, int fs, BYTE *pBuf)                       *
*                                                                             *
*******************************************************************************
*
*   Loads and parses function scope fields and variables
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
*   Returns:
*       TRUE - Function scope parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseFunctionScope(int fd, int fs, BYTE *pBuf)
{
    TSYMFNSCOPE Header;                 // Function scope section header
    TSYMFNSCOPE1 list;                  // Function scope record

    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position
    WORD nTokens = 0;                   // Number of tokens in a function
    BOOL fInFunction = FALSE;           // Are we inside a function scope?
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
    int i;

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE FUNCTION SCOPE                                            ||\n");
    VERBOSE2 printf("=============================================================================\n");
    VERBOSE1 printf("Parsing function scope.\n");

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
    // but here we extract only basic function parameters and relevant tokens

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

                        // At this point we know the total size of the header
                        // as well as the function ending address. Fill in the
                        // missing information and rewrite the header
                        Header.h.dwSize     = sizeof(TSYMFNSCOPE) + sizeof(TSYMFNSCOPE1) * (nTokens-1);
                        Header.dwEndAddress = Header.dwStartAddress + pStab->n_value - 1;
                        Header.nTokens      = nTokens;

                        // Reposition the file pointer to the start of the header
                        lseek(fd, fileOffset, SEEK_SET);

                        write(fd, &Header, sizeof(TSYMFNSCOPE)-sizeof(TSYMFNSCOPE1));
                        lseek(fd, 0, SEEK_END);

                        fInFunction = FALSE;
                    }
                    else
                    {
                        // We will write a header but later, on an function end,
                        // rewind and rewite it with the complete information
                        // This we do so we can simply keep adding file tokens as
                        // TSYMFNSCOPE1 array...
                        Header.h.hType  = HTYPE_FUNCTION_SCOPE;
                        Header.h.dwSize = sizeof(TSYMFNSCOPE)-sizeof(TSYMFNSCOPE1);

                        // Copy the function name into the strings
                        Header.pName          = dfs;
                        write(fs, pStr, strlen(pStr)+1);
                        dfs += strlen(pStr)+1;

                        Header.file_id        = file_id;
                        Header.dwStartAddress = pStab->n_value;

                        Header.dwEndAddress   = 0;      // To be written later
                        Header.nTokens        = 0;      // To be written later

                        // If the start address is not defined (0?) and this is an object file
                        // (kernel module), we can search the global symbols for the address
                        if( Header.dwStartAddress==0 && GlobalsName2Address(&Header.dwStartAddress, pStr) )
                            ;

                        // Print function start & name
                        VERBOSE2 printf("START-%08X--%s\n", Header.dwStartAddress, pStr);

                        nTokens = 0;

                        // Get the current file position so we can come back later

                        // Get the file stats
                        fstat(fd, &fd_stat);

                        fileOffset = fd_stat.st_size;

                        // Write the header first time
                        write(fd, &Header, sizeof(TSYMFNSCOPE)-sizeof(TSYMFNSCOPE1));

                        fInFunction = TRUE;
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

                // Parameter symbol to a function
                case N_PSYM:
                    VERBOSE2 printf("PSYM   ");
                    VERBOSE2 printf("line: %d PARAM [EBP+%lX]  %s\n", pStab->n_desc, pStab->n_value, pStr);

                    // Write out one token record
                    list.TokType = TOKTYPE_PARAM;
                    list.param   = pStab->n_value;
                    list.pName   = dfs;
                    write(fs, pStr, strlen(pStr)+1);
                    dfs += strlen(pStr)+1;

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Register variable
                case N_RSYM:
                    VERBOSE2 printf("RSYM  REGISTER VARIABLE ");
                    VERBOSE2 printf("%s in %ld\n", pStr, pStab->n_value);

                    // Write out one token record
                    list.TokType = TOKTYPE_RSYM;
                    list.param   = pStab->n_value;
                    list.pName   = dfs;
                    write(fs, pStr, strlen(pStr)+1);
                    dfs += strlen(pStr)+1;

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Local symbol: this symbol is shared with typedefs, but if the
                // pStab->n_value != 0, it is a local symbol
                case N_LSYM:
                    if( pStab->n_value==0 )
                        break;

                    VERBOSE2 printf("LSYM   ");
                    VERBOSE2 printf("line: %2d LOCAL_VARIABLE [EBP+%02lX] %s\n", pStab->n_desc, pStab->n_value, pStr);
                    // n_value != 0 -> variable address relative to EBP
                    // n_desc = line number where the symbol is declared

                    // Write out one token record
                    list.TokType = TOKTYPE_LSYM;
                    list.param   = pStab->n_value;
                    list.pName   = dfs;
                    write(fs, pStr, strlen(pStr)+1);
                    dfs += strlen(pStr)+1;

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Local static symbol in the BSS segment
                case N_LCSYM:
                    // If we are not within a function scope, it is a local variable
                    if(fInFunction)
                    {
                        list.bSegment= GlobalsName2SectionNumber(pStr);

                        VERBOSE2 printf("LCSYM  ");
                        VERBOSE2 printf("line: %d seg:%d %08lX  %s\n", pStab->n_desc, list.bSegment, pStab->n_value, pStr);

                        // Write out one token record
                        list.TokType = TOKTYPE_LCSYM;
                        list.param   = pStab->n_value;
                        list.pName   = dfs;
                        write(fs, pStr, strlen(pStr)+1);
                        dfs += strlen(pStr)+1;

                        write(fd, &list, sizeof(TSYMFNSCOPE1));

                        nTokens++;
                    }
                break;

                // Left-bracket: open a new scope
                case N_LBRAC:
                    VERBOSE2 printf("LBRAC              +%lX  {  (%d)\n", pStab->n_value, pStab->n_desc);

                    // Write out one token record
                    list.TokType = TOKTYPE_LBRAC;
                    list.param   = pStab->n_value;
                    list.pName   = 0;   // Not used

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Right-bracket: close a scope
                case N_RBRAC:
                    VERBOSE2 printf("RBRAC              +%lX  }\n", pStab->n_value);

                    // Write out one token record
                    list.TokType = TOKTYPE_RBRAC;
                    list.param   = pStab->n_value;
                    list.pName   = 0;   // Not used

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // We can ignore N_SOL (change of source) since the function scope does
                // not care for it
            }

            pStab++;
        }

        return( TRUE );
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( FALSE );
}
