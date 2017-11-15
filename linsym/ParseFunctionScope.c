/******************************************************************************
*                                                                             *
*   Module:     ParseFunctionScope.c                                          *
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
******************************************************************************/
BOOL ParseFunctionScope(int fd, int fs, BYTE *pBuf)
{
    TSYMFNSCOPE Header;                 // Function scope section header
    TSYMFNSCOPE1 list;                  // Function scope record

    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position
    WORD nTokens = 0;                   // Number of tokens in a function
    BOOL fInFunction = FALSE;           // Are we inside a function scope?

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
    printf("||         PARSE FUNCTION SCOPE                                            ||\n");
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
    // but here we extract only basic function parameters and relevant tokens

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
                        Header.dwSize       = sizeof(TSYMFNSCOPE) + sizeof(TSYMFNSCOPE1) * (nTokens-1);
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
                        Header.hType          = HTYPE_FUNCTION_SCOPE;
                        Header.dwSize         = 0;      // To be written later

                        // Copy the function name into the strings
                        Header.dName          = dfs;
                        write(fs, pStr, strlen(pStr)+1);
                        dfs += strlen(pStr)+1;

                        Header.file_id        = file_id;
                        Header.dwStartAddress = pStab->n_value;

                        Header.dwEndAddress   = 0;      // To be written later
                        Header.nTokens        = 0;      // To be written later

                        // If the start address is not defined (0?) and this is an object file
                        // (kernel module), we can search the global symbols for the address
                        if( Header.dwStartAddress==0 )
                            Header.dwStartAddress = GetGlobalSymbolAddress(pStr);

                        // Print function start & name
                        printf("START-%08lX--%s\n", Header.dwStartAddress, pStr);

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

                // Parameter symbol to a function
                case N_PSYM:
                    printf("PSYM   ");
                    printf("line: %d PARAM [EBP+%lX]  %s\n", pStab->n_desc, pStab->n_value, pStr);

                    // Write out one token record
                    list.TokType = TOKTYPE_PARAM;
                    list.p1      = pStab->n_value;
                    list.p2      = dfs;
                    write(fs, pStr, strlen(pStr)+1);
                    dfs += strlen(pStr)+1;

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Register variable
                case N_RSYM:
                    printf("RSYM  REGISTER VARIABLE ");
                    printf("%s in %ld\n", pStr, pStab->n_value);

                    // Write out one token record
                    list.TokType = TOKTYPE_RSYM;
                    list.p1      = pStab->n_value;
                    list.p2      = dfs;
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

                    printf("LSYM   ");
                    printf("line: %2d LOCAL_VARIABLE [EBP+%02lX] %s\n", pStab->n_desc, pStab->n_value, pStr);
                    // n_value != 0 -> variable address relative to EBP
                    // n_desc = line number where the symbol is declared

                    // Write out one token record
                    list.TokType = TOKTYPE_LSYM;
                    list.p1      = pStab->n_value;
                    list.p2      = dfs;
                    write(fs, pStr, strlen(pStr)+1);
                    dfs += strlen(pStr)+1;

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Local static symbol in the BSS segment
                case N_LCSYM:
                    // If we are not within a function scope, it is a local variable
                    // TODO: Local scope variables
                    if(fInFunction)
                    {
                        printf("LCSYM  ");
                        printf("line: %d BSS %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);

                        // Write out one token record
                        list.TokType = TOKTYPE_LCSYM;
                        list.p1      = pStab->n_value;
                        list.p2      = dfs;
                        write(fs, pStr, strlen(pStr)+1);
                        dfs += strlen(pStr)+1;

                        write(fd, &list, sizeof(TSYMFNSCOPE1));

                        nTokens++;
                    }
                break;

                // Left-bracket: open a new scope
                case N_LBRAC:
                    printf("LBRAC              +%lX  {  (%d)\n", pStab->n_value, pStab->n_desc);

                    // Write out one token record
                    list.TokType = TOKTYPE_LBRAC;
                    list.p1      = pStab->n_value;
                    list.p2      = 0;   // Not used

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // Right-bracket: close a scope
                case N_RBRAC:
                    printf("RBRAC              +%lX  }\n", pStab->n_value);

                    // Write out one token record
                    list.TokType = TOKTYPE_RBRAC;
                    list.p1      = pStab->n_value;
                    list.p2      = 0;   // Not used

                    write(fd, &list, sizeof(TSYMFNSCOPE1));

                    nTokens++;
                break;

                // We can ignore N_SOL (change of source) since the function scope does
                // not care for it
            }

            pStab++;
        }
    }
    else
        printf("No STAB section in the file\n");

    return( 0 );
}

