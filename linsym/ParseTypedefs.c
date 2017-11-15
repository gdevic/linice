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

#include "loader.h"                     // Include global protos

#include <ctype.h>

extern PSTR dfs;                        // Global pointer to strings (to append)

extern WORD GetFileId(char *pSoDir, char *pSo);

#define MAX_TYPEDEF     32768           // Max buffer len for the concat typedef string

static char DefBuf[MAX_TYPEDEF];        // Actual concat typedef string buffer
static int nTypedefs;                   // Number of typedefs


typedef struct
{
    int len;                            // String len
    char *pStr;                         // Type definition string
    char id;                            // Type id number

} TBASICTYPEDEF;


// This is a stack of subdefinitions so we can resolve this in the opposite order
#define MAX_SUBDEF      256             // Maximum number of sub-definitions that we can handle
static char *Subdef[MAX_SUBDEF];
static int nSubdef;

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


/******************************************************************************
*                                                                             *
*   char BasicTypedef(char *pDef)                                             *
*                                                                             *
*******************************************************************************
*
*   Looks if the addressed string contains one of the built-in types.
*
*   Where:
*       pDef is the string to examine
*
*   Returns:
*       0 - Type is not one of the built-in types
*       nonzero - Basic built-in type ID
*
******************************************************************************/
char BasicTypedef(char *pDef)
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
*   BOOL ParseDef(int fd, int fs, char *pDefBuf)                              *
*                                                                             *
*******************************************************************************
*
*   Parses a single line of type definition. It splits the multiple sub-defs
*   into separate definctions so we can find them faster.
*
*   Note: The line will be heavily modified!
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pDefBuf - complete line of a type definition
*
*   Returns:
*       TRUE - Typedefs line parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseDef(int fd, int fs, char *pDefBuf)
{
    char *pDef;                         // Moving pointer to the definition
    char *pSub;                         // Pointer to the subdefinition ")="
    char *pSubend;                      // Pointer to the end of the subdefinition
    char *pSubnum;                      // Pointer to the subdefinition number "(x,y)="
    char c, cBasic;                     // Temporary characters
    TSYMTYPEDEF1 list;                  // Typedef record
    int nNameLen;                       // Type name length

    nSubdef = 0;                        // Zero subdefinitions so far
    pDef = pDefBuf;                     // Adjust the pointer that we will move along

    // Scan the typedef definition and stack up the pointers to ")=" substrings
    while( nSubdef<MAX_SUBDEF && (Subdef[nSubdef] = pDef = strstr(pDef, ")=")) )
        pDef += 2, nSubdef++;

    // Make sure that we did not overflow our buffer. Chances are slim because we allow
    // large nesting, but let's still handle it, oh well...
    if(nSubdef>=MAX_SUBDEF)
    {
        fprintf(stderr, "Error: Typedef definition too complex:\n");
        fprintf(stderr, "%s\n", pDefBuf);
        return(FALSE);
    }

    // Pop the subdefinitions from the stack and extract them out of the original string
    while(nSubdef--)
    {
        // Clear the list record just in case
        memset(&list, 0, sizeof(TSYMTYPEDEF1));

        // Find the start of the subdefinition - its number (x,y)=... by looking for the "("
        pSubnum = Subdef[nSubdef];
        while(*pSubnum!='(') pSubnum--;

        // Find the start of the actual subdefinition - we already have it ")="
        pSub = Subdef[nSubdef] + 2;

        // Find the end of the actual subdefinition: depending on the type of the def,
        // we may need to parse it
        pSubend = pSub;
        switch(*pSub)
        {
        case '(':   // This is a simple type number redirection: (x,y)=(i,j)
        case '*':   // This is a pointer redirection: (x,y)=*(i,j)
        case 'f':   // Function reference: (x,y)=f(i,j)
                    // Look for the closing parenthesis
            pSubend = strchr(pSub,')');
            break;

        case 'r':   // Self-defined integer value: (x,y)=r(i,j);00000000;0037777777;
                    // Look for the third semicolon
            pSubend = strchr(strchr(strchr(pSub,';')+1,';')+1,';');
            break;

        case 'a':   // Array: (x,y)=ar(i,j);m;n;(k,l)
                    // Look for the second closing bracket
            pSubend = strchr(strchr(pSub,')')+1,')');
            break;

        case 'x':   // Forward defined type: (x,y)=xs_TX:
                    // Look for the trailing colon
            pSubend = strchr(pSub,':');

        default:                        // All other types we take completely
            pSubend = strchr(pSub, '\0') - 1;
        }

        // Read in the typedef numbers and form the typedef record
        if( sscanf(pSubnum, "(%hd,%hd)", &list.maj, &list.min) != 2 )
        {
            fprintf(stderr, "Error reading typedef!\n");
            return( FALSE );
        }

        // If the subdef is the last one (or the very first define), append the typedef name to it
        // Since we are going reverse order, that last one will be the original definition with the name
        if( !nSubdef )
        {
            // If this is a basic built-in typedef, shortcut it here

            if( (cBasic = BasicTypedef(pDefBuf)) )
            {
                // Basic type definition: pName still points to the name string (like "int",..)
                // but the pDef points to the basic type identifier (small numbers 1...19) followed by 0.

                list.pName = dfs;       // Write the typedef name string
                nNameLen = strchr(pDefBuf,':')-pDefBuf;
                write(fs, pDefBuf, nNameLen);
                write(fs, "", 1);

                dfs += nNameLen + 1;

                list.pDef = dfs;
                write(fs, &cBasic, 1);
                write(fs, "", 1);

                dfs += 2;

                write(fd, &list, sizeof(TSYMTYPEDEF1));
                nTypedefs++;

                VERBOSE2 printf("(%d,%d) = {%d} %s\n", list.maj, list.min, cBasic, basic[cBasic-1].pStr );

                return( TRUE );         // Return here to avoid writing it over again
            }
            else
            {
                // It is a complext type or a variable implied type (anonymous)
                if( *pDefBuf!='(' )
                {
                    // Write the type name since this is not anonymous type

                    list.pName = dfs;       // Write the typedef name string
                    nNameLen = strchr(pDefBuf,':')-pDefBuf;
                    write(fs, pDefBuf, nNameLen);
                    write(fs, "", 1);

                    dfs += nNameLen + 1;

                    {   // This is only to assist printing a nice substring
                        c = *(pDefBuf + nNameLen);
                        *(pDefBuf + nNameLen) = 0;
                        VERBOSE2 printf("(%d,%d) %s = ", list.maj, list.min, pDefBuf );
                        *(pDefBuf + nNameLen) = c;
                    }
                }
            }
        }

        pSubend++;

        // Write the definition string
        list.pDef = dfs;
        write(fs, pSub, pSubend-pSub);
        write(fs, "", 1);
        dfs += pSubend-pSub + 1;

        // Write the typedef record
        write(fd, &list, sizeof(TSYMTYPEDEF1));
        nTypedefs++;

        {   // This is only to assist printing a nice substring
            c = *pSubend;
            *pSubend = 0;
            VERBOSE2 printf("(%d,%d) = %s\n", list.maj, list.min, pSub );
            *pSubend = c;
        }

        // Move the rest of the original typedef over the subdefinition
        memmove(pSub-1, pSubend, strlen(pSubend)+1);
    }

    return(TRUE);
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
*   Returns:
*       TRUE - Typedefs parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseTypedefs(int fd, int fs, BYTE *pBuf)
{
    TSYMTYPEDEF Header;                 // Source typedef section header
    WORD file_id = 0;                   // Current file ID number
    long fileOffset = 0;                // Temp file offset position

    char *pDefBuf = DefBuf;             // Buffer to concatenate long typedef line
    char *pDef = NULL;                  // Pointer to a buffer
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

    nTypedefs = 0;

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE TYPEDEFS                                                  ||\n");
    VERBOSE2 printf("=============================================================================\n");
    VERBOSE1 printf("Parsing type definition.\n");

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

                    VERBOSE2 printf("HdrSym size: %lX\n", pStab->n_value);
                break;

                // Function parameter symbols: The parameter symbol can also contains implicit type definition
                // We consider them the same way as global symbols (which can also contain further typedef)
                case N_PSYM:

                // Global symbols: Since the global symbols can also contain implicit type definition,
                // we need to process those as additional separate types. Note that the canonical (simple)
                // type globals are already stored in the parse global code - here we only need to process
                // the complex definitions
                case N_GSYM:
                    // Find if the global symbol definition is complex, and if not so, break out
                    if(!strchr(pStr, '='))
                        break;

                    // Found a complex global definition, move the pointer of the string beyond the symbol
                    // name so to avoid storing the symbol name as the type name (the definition will be
                    // stored as an anonymous type
                    pStr = strchr(pStr, '(');

                    // Note: This code continues into the N_LSYM case...
                    goto ProcessType;

                // Static symbols: The same rule applies with the static symbols, need to process them
                // since they may contain complex definition
                case N_STSYM:
                    // Find if the static symbol definition is complex, and if not so, break out
                    if(!strchr(pStr, '='))
                        break;

                    // Found a complex static definition, move the pointer of the string beyond the symbol
                    // name so to avoid storing the symbol name as the type name (the definition will be
                    // stored as an anonymous type
                    pStr = strchr(pStr, '(');

                    // Note: This code continues into the N_LSYM case...
                    goto ProcessType;

                // Type definition: this symbol is shared with local symbol, but if the
                // pStab->n_value == 0, it is a type definition
                case N_LSYM:
                    if(pStab->n_value==0)
                    {
ProcessType:
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

                            // Call a function that parses the complete definition line

                            ParseDef(fd, fs, pDefBuf);

                            pDef = NULL;        // Reset the pointer to a typedef string
                        }
                    }
                break;

                // New source file: a typedef record is based on a main source file, so start one
                case N_SO:
                    VERBOSE2 printf("SO  ");
                    if( *pStr==0 )
                    {
                        // Empty name - end of source file
                        VERBOSE2 printf("End of source. Text section offset: %08lX\n", pStab->n_value);
                        VERBOSE2 printf("=========================================================\n");

                        // End of source - close the active typedef record structure

                        // At this point we know the total size of the header. Fill in the
                        // missing information and rewrite the header
                        Header.h.dwSize  = sizeof(TSYMTYPEDEF) + sizeof(TSYMTYPEDEF1) * (nTypedefs-1);
                        Header.nTypedefs = nTypedefs;

                        // Write the header back up
                        lseek(fd, fileOffset, SEEK_SET);

                        write(fd, &Header, sizeof(TSYMTYPEDEF)-sizeof(TSYMTYPEDEF1));
                        lseek(fd, 0, SEEK_END);
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

                            // Get the id number of this specific file
                            file_id = GetFileId(pSoDir, pSo);

                            // We got a new main source file... Start filling up the header
                            Header.h.hType   = HTYPE_TYPEDEF;
                            Header.h.dwSize  = 0;           // To be written later
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

