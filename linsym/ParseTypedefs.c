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

#include <ctype.h>

extern int dfs;

extern WORD GetFileId(char *pSoDir, char *pSo);

#define MAX_TYPEDEF     32768           // Max buffer len for the concat typedef string

static char DefBuf[MAX_TYPEDEF];        // Actual concat typedef string buffer

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

char *left(char *pString, int len);
char *memrchr(char *pMem, int c);

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

    char *pDefBuf = DefBuf;             // Buffer to concatenate long typedef line
    char *pDef = NULL;                  // Pointer to a buffer
    char *pDefinition;                  // Temp pointer to a typedef definition string
    char *pSub;                         // Pointer to a substring
    int nNameLen;                       // Length of the typedef name string
    int nDefLen;                        // Length of the typedef definition string
    char cType;                         // Scanned typedef type
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

    printf("=============================================================================\n");
    printf("||         PARSE TYPEDEFS                                                  ||\n");
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
    // but here we extract only typedef information

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

                    printf("HdrSym size: %lX\n", pStab->n_value);
                break;

                // Type definition: this symbol is shared with local symbol, but if the
                // pStab->n_value == 0, it is a type definition
                case N_LSYM:
                    if(pStab->n_value==0)
                    {
                        // If the definition is split into multiple lines, we will concat them
                        // back together and do the final processing on that complete string

                        // TODO: We really need to check if we overflowed the buffer
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
                            // Even if we did not have a multiple line definition, and therefore did
                            // not have it concatenated in the DefBuf, we copy the line there since we
                            // may be modifying it during the processing and we really dont want to
                            // be modifying a "master" definition line in our ELF buffer

                            if( pDef==NULL )
                            {
                                strcpy(pDefBuf, pStr);
                            }

                            pDef = pDefBuf;

                            // BUG???: I dont know whether this is a bug in a compiler generating typedef stabs,
                            //      or a feature, but some of typedefs have incorrect format like
                            //      "persist:,480,32;" without parenthesis...
                            // Temp fix: Skip over these typedefs (they cause imbalanced scan)

//                            if( strchr(pDef, '(')==NULL )
//                                break;

                            // Do final processing of the typdef definition:
                            // If the type defined here is one of the basic types that we know
                            // how to process, there is no need to store a complete definition
                            // string - we store the name and the definition enum in the dDef field:

                            // We will somewhat "undo" what gcc did in terms of the type definitions,
                            // since we dont want to search through all the strings for a (x,y) type,
                            // we split them into separate lines when multiple types are constructed

                            // Decode typedef record into a binary structure
                            if(sscanf(strchr(pDef, ':'), ":%c(%hd,%hd)", &cType, &list.maj, &list.min)!=3)
                            {
                                printf("Error scanning TYPEDEF ID %s\n", pDef);
                                return( FALSE );
                            }

                            // Find the address where the literal typedef name ends
                            nNameLen = strchr(pDef, ':') - pDef;

                            pDefinition = strchr(pDef, '=') + 1;

                            nDefLen = strlen(pDefinition);

                            list.dName = dfs;

                            // Copy the typedef type (one character) first into the name string
                            write(fs, &cType, 1);
                            dfs += 1;

                            // Copy the typedef name into the strings and zero terminate it
                            write(fs, pDef, nNameLen);
                            write(fs, "", 1);               // This will append 0
                            dfs += nNameLen + 1;

                            if( (list.dDef = BasicTypedef(pDef)) == 0 )
                            {
                                list.dDef = dfs;

                                // We do canonization in two steps:

                                // Step 1.  Outer nested type definition
                                //
                                // Nested definition that we process first are in the form
                                // <primary>:t(x,y)=(a,b)=...
                                // where (a,b) should be defined within a separate line
#if 1
                                if(*pDefinition=='(')
                                {
                                    TSYMTYPEDEF1 list2;         // Typedef record

                                    // Step 1b.  Inner doubly-nested definition
                                    //
                                    // A new definition that we extracted may itself be nested
                                    // If it is so, it is usually only that one extra step deep, so
                                    // we can process it like this, in-line

                                    if(sscanf(pDefinition, "(%hd,%hd)%c", &list2.maj, &list2.min, &cType)==3 && cType=='=')
                                    {
                                        // Create a new definition record containing the outer wrapper (short reference)
                                        
                                        // First, complete the original typedef record to point to the next one
                                        // Copy only the (a,b) part of definition
                                        nDefLen = strchr(pDefinition, '=') - pDefinition;

                                        write(fs, pDefinition, nDefLen);
                                        dfs += nDefLen;

                                        write(fs, "", 1);               // This will append 0
                                        dfs += 1;

                                        write(fd, &list, sizeof(TSYMTYPEDEF1));

                                        nTypedefs++;

                                        // Create a new definition line containing the rest of a def string

                                        list2.dName = list.dName;       // Reuse the same (parent) name
                                        list = list2;
                                        
                                        pDefinition += nDefLen + 1;     // Next definition string
                                        nDefLen = strlen(pDefinition);
                                    }
                                }
#if 1
                                // Step 2.   In-string nested type definitions
                                //
                                // Parse definition strings and extract all embedded sub-definitions
                                // into new individual typedefs

                                while((pSub = strchr(pDefinition, '=')) != NULL)
                                {
                                    TSYMTYPEDEF1 list3;         // Typedef record
                                    char *pSubEnd;              // Pointer to the end of a substring
                                    int nSubLen;                // Subtype string length

                                    TSYMTYPEDEF1 list4;         // Typedef record
                                    char *pSubSub;              // Pointer to the end of a substring
                                    int nSubSubLen;             // Subtype string length

                                    // We have internal sub-definitions that we need to extract
                                    if(sscanf(memrchr(pSub, '('), "(%hd,%hd)=", &list3.maj, &list3.min)==2)
                                    {
                                        pSub += 1;

                                        list3.dName = dfs-1;    // Should be 0
                                        list3.dDef = dfs;
                                        
                                        // Find the end of a sub-type by searching for comma or eol (zero)
                                        pSubEnd = pSub;
                                        while(*pSubEnd && ((*pSubEnd!=',') || 
                                            ((*(pSubEnd-1)>='0') && (*(pSubEnd-1)<='9'))) ) pSubEnd++;

                                        // Write out only the new sub-definition
                                        nSubLen = pSubEnd - pSub;

                                        // New sub-definition may itself be nested (one level)

                                        // This is the way we find out if we have doubly-nested definition
                                        // We look for a '=' character inside the new substring
                                        pSubSub = strchr(pSub, '=');
                                        if(pSubSub && ((pSubSub - pSub) < nSubLen))
                                        {
                                            // We need to split our sub-definition into two separate defines

                                            if(sscanf(memrchr(pSubSub, '('), "(%hd,%hd)=", &list4.maj, &list4.min)==2)
                                            {
                                                pSubSub += 1;

                                                list4.dName = dfs-1;        // Should be 0
                                                list4.dDef = dfs;

                                                nSubSubLen = nSubLen - (pSubSub-pSub);

                                                write(fs, pSubSub, nSubSubLen);
                                                dfs += nSubSubLen;

                                                write(fs, "", 1);       // This will append 0
                                                dfs += 1;

                                                write(fd, &list4, sizeof(TSYMTYPEDEF1));

                                                nTypedefs++;

                                                printf("sub DEF(%d,%d) = %s\n", list4.maj, list4.min, left(pSubSub, nSubSubLen));

                                                // Prepare parameters to write default sub-definition

                                                nSubLen -= nSubSubLen + 1;
                                            }
                                            else
                                            {
                                                ; // Unexpected tokens
                                            }
                                        }

                                        write(fs, pSub, nSubLen);
                                        dfs += nSubLen;

                                        write(fs, "", 1);       // This will append 0
                                        dfs += 1;

                                        write(fd, &list3, sizeof(TSYMTYPEDEF1));

                                        nTypedefs++;

                                        printf("SUB DEF(%d,%d) = %s\n", list3.maj, list3.min, left(pSub, nSubLen));

                                        // Now do the dangerous part - fold pDefinition string over the subdefinition
                                        // This is going to be an overlapping copy...

                                        // Move over (pSub-1) to start covering '='
                                        memmove(pSub-1, pSubEnd, strlen(pSubEnd) + 1);

                                        nDefLen -= nSubLen;
                                    }
                                    else
                                    {
                                        ; // Unexpected tokens
                                    }
                                }
#endif
#endif
                                // Copy the typedef definition into the strings

                                list.dDef = dfs;

                                write(fs, pDefinition, nDefLen);
                                dfs += nDefLen;

                                write(fs, "", 1);               // This will append 0
                                dfs += 1;

                                write(fd, &list, sizeof(TSYMTYPEDEF1));

                                printf("TYPEDEF(%d,%d) %s = %s\n", list.maj, list.min, left(pDef, nNameLen), pDefinition );
                            }
                            else
                            {
                                // Write out a simple, basic type

                                write(fd, &list, sizeof(TSYMTYPEDEF1));

                                printf("TYPEDEF(%d,%d) %s = BASIC(%d): %s\n", list.maj, list.min, left(pDef, nNameLen), list.dDef, pDefinition );
                            }

                            pDef = NULL;        // Reset the pointer to a typedef string

                            nTypedefs++;
                        }
                    }
                break;

                // New source file: a typedef record is based on a main source file, so start one
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

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   char *left(char *pString, int len)                                        *
*                                                                             *
*******************************************************************************
*
*   String helper function: 
*   Returns 'len' leftmost characters of a given string
*
******************************************************************************/
char *left(char *pString, int len)
{
    static char bad_design[MAX_TYPEDEF];

    strncpy(bad_design, pString, len);
    bad_design[len] = 0;

    return(bad_design);
}

/******************************************************************************
*                                                                             *
*   char *memrchr(char *pMem, int c)                                          *
*                                                                             *
*******************************************************************************
*
*   String helper function: 
*   Searches for a character 'c' backwards from the memory pointer 'pMem'
*
******************************************************************************/
char *memrchr(char *pMem, int c)
{
    while(*pMem != c) pMem--;

    return(pMem);
}

