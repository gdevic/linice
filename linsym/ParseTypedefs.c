/******************************************************************************
*                                                                             *
*   Module:     ParseTypedefs.c                                               *
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


typedef struct
{
    int len;                            // String len
    char *pStr;                         // Type definition string
    DWORD id;                           // Type id number

} TBASICTYPEDEF;

static TBASICTYPEDEF basic[] = {
    {  4, "int:", 1 },
    {  5, "char:", 2 },
    {  9, "long int:", 3 },
    { 13, "unsigned int:", 4 },
    { 18, "long unsigned int:", 5 },
    { 14, "long long int:", 6 },
    { 23, "long long unsigned int:", 7 },
    { 10, "short int:", 8 },
    { 19, "short unsigned int:", 9 },
    { 12, "signed char:", 10 },
    { 14, "unsigned char:", 11 },
    {  6, "float:", 12 },
    {  7, "double:", 13 },
    { 12, "long double:", 14 },
    { 12, "complex int:", 15 },
    { 14, "complex float:", 16 },
    { 15, "complex double:", 17 },
    { 20, "complex long double:", 18 },
    {  5, "void:", 19 },
    {  0  }
};


DWORD BasicTypedef(char *pDef)
{
    TBASICTYPEDEF *pType;

    pType = &basic[0];
    while( pType->len )
    {
        if( !strncmp(pDef, pType->pStr, pType->len) )
            return( pType->id );

        pType++;
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   BOOL ParseTypedefs(int fd, int fs, BYTE *pBuf)                            *
*                                                                             *
*******************************************************************************
*
*   Loads and parses type definitions
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
******************************************************************************/
BOOL ParseTypedefs(int fd, int fs, BYTE *pBuf)
{
    TSYMTYPEDEF Header;                 // Source typedef section header
    TSYMTYPEDEF1 list;                  // Typedef record
    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position
    int nTypedefs = 0;                  // Number of typedefs

#define MAX_TYPEDEF     8192            // Max buffer len
    char *pDefBuf;                      // Buffer to concatenate long typedef line
    char *pDef = NULL;                  // Pointer to a buffer
    char *pDefinition;                  // Temp pointer to a typedef definition string
    int nNameLen;                       // Length of the typedef name string
    int nDefLen;                        // Length of the typedef definition string

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
    printf("||         PARSE TYPEDEFS                                                  ||\n");
    printf("=============================================================================\n");

    pElfHeader = (Elf32_Ehdr *) pBuf;

    // Allocate a buffer in which we will store a complete typedef line, for those
    // that are broken into multiple stabs
    pDefBuf = (char*) malloc(MAX_TYPEDEF);
    if( pDefBuf==NULL )
    {
        printf("ERROR: Unable to allocate %d bytes for a typedef buffer!\n", MAX_TYPEDEF);
        return( FALSE );
    }

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
    // but here we extract only typedef information

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
                // Type definition: this symbol is shared with local symbol, but if the
                // pStab->n_value == 0, it is a type definition
                case N_LSYM:
                    if(pStab->n_value==0)
                    {
                        int maj, min;

                        // If the definition is split into multiple lines, we will concat them
                        // back together and do the final processing on that complete string
                        if( pStr[strlen(pStr)-1]=='\\' )
                        {
                            if( pDef==NULL )
                            {
                                pDef = pDefBuf;
                                pDef[0] = 0;
                            }

                            strcpy(pDef, pStr);
                            pDef += strlen(pDef) - 1;
                            *pDef = 0;
                        }
                        else
                        {
                            if( pDef==NULL )
                                pDef = pStr;
                            else
                                pDef = pDefBuf;

                            // Do final processing of the typdef definition:
                            // If the type defined here is one of the basic types that we know
                            // how to process, there is no need to store the complete definition
                            // string - we store the name and the definition enum in the dDef field:

                            // Decode typedef ID numbers
                            if(sscanf(strchr(pDef, '('), "(%d,%d)", &maj, &min)!=2)
                            {
                                printf("Error scanning TYPEDEF ID %s\n", pDef);
                                return( FALSE );
                            }

                            // Find the address where the typedef name ends and its definition starts
                            nNameLen = strchr(pDef, '=') - pDef;
                            pDefinition = pDef + nNameLen + 1;

                            nDefLen = strlen(pDefinition);

                            list.dName = dfs;
                            // Copy the typedef name into the strings and zero terminate it
                            write(fs, pDef, nNameLen);
                            write(fs, "", 1);               // This will append 0
                            dfs += nNameLen + 1;

                            if( (list.dDef = BasicTypedef(pDef)) == 0 )
                            {
                                list.dDef = dfs;
                                // Copy the typedef definition into the strings
                                write(fs, pDefinition, nDefLen+1);
                                dfs += nDefLen + 1;
                            }

                            list.maj  = maj;
                            list.min  = min;

                            write(fd, &list, sizeof(TSYMTYPEDEF1));

                            printf("TYPEDEF(%d,%d) %s\n", maj, min, pDef);

                            pDef = NULL;        // Reset the pointer to a typedef string

                            nTypedefs++;
                        }
                    }
                break;

                // New source file: a typedef record is based off a main source file, so start one
                case N_SO:
                    printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        printf("=========================================================\n");

                        // End of source - close the active typedef record structure

                        // At this point we know the total size of the header. Fill in the
                        // missing information and rewrite the header
                        Header.dwSize    = sizeof(TSYMTYPEDEF) + sizeof(TSYMTYPEDEF1) * (nTypedefs-1);
                        Header.nTypedefs = nTypedefs;

                        // If we did not find any line numbers, all we do here is
                        // reposition the file pointer to the start of the header
                        // and do nothing
                        lseek(fd, fileOffset, SEEK_SET);

                        if( nTypedefs > 0 )
                        {
                            write(fd, &Header, sizeof(TSYMTYPEDEF)-sizeof(TSYMTYPEDEF1));
                            lseek(fd, 0, SEEK_END);
                        }
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

                            // Get the id number of this specific file
                            file_id = GetFileId(pSoDir, pSo);

                            // We got a new main source file... Start filling up the header
                            Header.hType     = HTYPE_TYPEDEF;
                            Header.dwSize    = 0;           // To be written later
                            Header.file_id   = file_id;
                            Header.nTypedefs = 0;           // To be written later

                            nTypedefs = 0;

                            // Get the current file position so we can come back later

                            // Get the file stats
                            fstat(fd, &fd_stat);

                            fileOffset = fd_stat.st_size;

                            // Write the header the first time
                            write(fd, &Header, sizeof(TSYMTYPEDEF)-sizeof(TSYMTYPEDEF1));
                        }
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

    // Free the typedef buffer
    free(pDefBuf);

    return( TRUE );
}

