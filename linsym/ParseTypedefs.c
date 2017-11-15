/******************************************************************************
*                                                                             *
*   Module:     ParseTypedefs.c                                               *
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

// Define structure that holds the information about the types defined in another
// file - they have their own major type number, so we need to establish the
// adjustment correlation array

typedef struct
{
    char *pFile;                        // Pointer to the include file name
    WORD file_id;                       // file ID of the base file where that include appears
    WORD maj;                           // Major number associated with this include file

} TEXTYPEDEF;

#define MAX_EXTYPEDEF   65535

// We define this array as uninitialized, static, so it uses BSS section, does not affect the code size...
// The maximum size is 64K entries - the maximum number of include files.
static TEXTYPEDEF ExType[MAX_EXTYPEDEF];// Array containing the external typedef structure
static UINT nExType;                    // Number of files stashed into the ExType array

static TSYMADJUST Rel[MAX_EXTYPEDEF];   // Local adjustment array that we are building for each source


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
*   BOOL ParseDef(int fd, int fs, char *pDefBuf, WORD file_id)                *
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
*       file_id - source file ID of the current source
*
*   Returns:
*       TRUE - Typedefs line parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseDef(int fd, int fs, char *pDefBuf, WORD file_id)
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

                list.file_id = file_id;
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

                    list.file_id = file_id;
                    list.pName = dfs;       // Write the typedef name string
                    nNameLen = strchr(pDefBuf,':')-pDefBuf;

                    // There is a case where the type name is a space. We dont want that.
                    // Reference lookup: gconv.h:37 produces e__GCONV_OK:0,__GCONV_NOCONV...
                    if( *pDefBuf==' ' )
                        nNameLen = 0;

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
        list.file_id = file_id;
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
    int i, j;                           // Generic counters
    WORD nLocalInclude = 0;             // Local major number counter

    nTypedefs = 0;
    nExType   = 0;

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
                    // Local symbol may also contain a complex definition, so search for it
                    if(pStab->n_value!=0 && strchr(pStr, '='))
                    {
                        // Found a complex local definition, move the pointer of the string beyond the symbol
                        // name so to avoid storing the symbol name as the type name (the definition will be
                        // stored as an anonymous type
                        pStr = strchr(pStr, '(');

                        // Note: This code continues into the N_LSYM case...
                        goto ProcessType;
                    }

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

                            ParseDef(fd, fs, pDefBuf, file_id);

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
                        Header.nRel      = nLocalInclude + 1;
                        Header.pRel      = (TSYMADJUST *) dfs;

                        // Write out the reference array with the strings
                        write(fs, &Rel, sizeof(TSYMADJUST) * Header.nRel);
                        dfs += sizeof(TSYMADJUST) * Header.nRel;

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
                            Header.h.dwSize  = sizeof(TSYMTYPEDEF)-sizeof(TSYMTYPEDEF1);
                            Header.file_id   = file_id;
                            Header.nRel      = 0;           // To be written later
                            Header.pRel      = NULL;        // To be written later
                            Header.nTypedefs = 0;           // To be written later

                            nTypedefs = 0;
                            nLocalInclude = 0;              // We start counting from 0 (major numbers)

                            // Entry 0 in the type reference array always points to the root source
                            Rel[0].file_id   = file_id;
                            Rel[0].adjust    = 0;           // Dont adjust this type

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

                // New include file within the source file. The type definitions that are defined
                // there will have increased major number
                case N_BINCL:
                    nLocalInclude = nLocalInclude + 1;      // This is a new include within this source file

                    // This major type is defined within the current source code, so store that record
                    Rel[nLocalInclude].file_id = file_id;
                    Rel[nLocalInclude].adjust  = 0;         // Needs no adjustment

                    nExType = nExType + 1;                  // New external type record

                    // TODO: This is a bug, We should never be in this situation. Investigate...
                    if(nExType<MAX_EXTYPEDEF)
                    {
                        // Add the include to the list of external include files

                        ExType[nExType].pFile = malloc(strlen(pStr)+1);
                        strcpy(ExType[nExType].pFile, pStr);

                        ExType[nExType].file_id = file_id;
                        ExType[nExType].maj     = nLocalInclude;

                        VERBOSE2 printf("BINCL local=%d global=%d %s\n", nLocalInclude, nExType, pStr);
                    }
                    else
                    {
                        fprintf(stderr, "ERROR: Too many nested include files!\n");
                        nExType--;
                    }
                break;

                // New include file that is defined within the scope of another source file.
                case N_EXCL:
                    nLocalInclude = nLocalInclude + 1;      // This is a new include within this source file

                    // We need to find the include file that is being referenced and form the
                    // reference array item with the source file ID and adjustment value of the major type number

                    VERBOSE2 printf("EXCL local=%d %s => ", nLocalInclude, pStr);

                    for(j=1; j<=(int)nExType; j++ )
                    {
                        if( !strcmp(pStr, ExType[j].pFile) )
                        {
                            // We found the include file name - add the adjustment array record

                            Rel[nLocalInclude].file_id = ExType[j].file_id;
                            Rel[nLocalInclude].adjust  = ExType[j].maj - file_id;

                            VERBOSE2 printf("file_id=%d %d\n", Rel[nLocalInclude].file_id, Rel[nLocalInclude].adjust);
                            break;
                        }
                    }

                    // Right now we will flag this as critical error to make sure things behave the way we expect
                    if( j>(int)nExType )
                    {
                        fprintf(stderr, "ELF/STABS Error: EXCL refers to nonexisting BINCL\n");
                        return( FALSE );
                    }

                break;
            }

            pStab++;
        }

        // We are finished, so we can free all the external file reference records
        for(; nExType>0; nExType-- )
            free(ExType[nExType-1].pFile);

        return( TRUE );
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( FALSE );
}

