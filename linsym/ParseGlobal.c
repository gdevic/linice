/******************************************************************************
*                                                                             *
*   Module:     ParseGlobals.c                                                *
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

#include <ctype.h>                      // Test the character

extern PSTR dfs;


// Define internal array structre that holds global symbols for a lookup
#define MAX_SECTION_LEN     32          // Size of the section name string

typedef struct
{
    DWORD dwAddress;                    // Start address of a symbol
    DWORD dwEndAddress;                 // End address of a symbol
    DWORD dwAttribute;                  // Attributes of a symbol
    char SectionName[MAX_SECTION_LEN];  // Section name string
    char Name[MAX_SYMBOL_LEN+1];        // Symbol canonical name

    char *pDef;                         // Pointer to symbol definition
    WORD file_id;                       // Defined in this file ID (only globals)
} TGLOBAL;

static TGLOBAL *pGlobals = NULL;        // Array of global symbols
static int nGlobals = 0;                // Number of global symbols

// The array to keep the names of the symbols (variables) from the COMMON sections
// nCommons is global since we use it as a top entry when doing the symbol
// relocation since each COMMON variable has its own relocation record on top of
// the standard predefined (hardcoded) segments.

#define MAX_STANDARD_SEG    9           // Number of standard segments
#define MAX_COMMONS         256         // This is the hard-coded since we use BYTE to index it

static char sCommon[MAX_COMMONS][MAX_STRING];
int nCommons = MAX_STANDARD_SEG + 1;    // First available slot - skip standard sections


extern WORD GetFileId(char *pSoDir, char *pSo);
extern DWORD GetGlobalSymbolAddress(char *pName);

int GetGlobalsSection(char *pSection, char *pName);


/******************************************************************************
*                                                                             *
*   BOOL StoreGlobalSyms(BYTE *pBuf)                                          *
*                                                                             *
*******************************************************************************
*
*   Loads and stores global symbol table into an lookup array.
*
*   Where:
*       pBuf - buffer containing the ELF file
*
*   Return:
*       TRUE - Globals array set up: pGlobals, nGlobals
*       FALSE - Critical error setting up
*
******************************************************************************/
BOOL StoreGlobalSyms(BYTE *pBuf)
{
    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecStab = NULL;         // Section .STAB
    Elf32_Shdr *SecStabstr = NULL;      // Section .STABSTR
    Elf32_Shdr *SecSymtab = NULL;       // Section .SYMTAB
    Elf32_Shdr *SecStrtab = NULL;       // Section .STRTAB

    FILE *fGlobals;                     // Temp file descriptor for globals
    Elf32_Sym *pSym;                    // Pointer to a symtab entry
    char *pStr;                         // Pointer to a stab string
    int i, nameLen;                     // Temporary values
    char sSymbol[MAX_SYMBOL_LEN+1];     // Buffer to copy symbol name

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         STORE ELF GLOBAL SYMBOL TABLE                                   ||\n");
    VERBOSE2 printf("=============================================================================\n");
    VERBOSE1 printf("Parsing global symbols.\n");

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

    // Create a temp file in which we will queue all global symbols
    // We use text representation of the data that we encounter during
    // the parsing of the ELF global symbol table
    fGlobals = tmpfile();
    nGlobals = 0;

    //==============================
    // Parse ELF global symbol table
    //==============================
    if( SecSymtab && SecStrtab )
    {
        char *pSecName;         // Pointer to a section name string

        pSym = (Elf32_Sym *) (pBuf + SecSymtab->sh_offset);
        pStr = (char *)pBuf + SecStrtab->sh_offset;
        i = SecSymtab->sh_size / sizeof(Elf32_Sym) - 1;

        // Skip the null entry
        pSym++;

        while( i-- )
        {
            switch(pSym->st_shndx)
            {
                case SHN_UNDEF  : pSecName = "EXTERNDEF"; break;
                case SHN_ABS    : pSecName = "ABSOLUTE";  break;
                case SHN_COMMON : pSecName = "COMMON";    break;
                default:          pSecName = pBuf + SecName->sh_offset + Sec[pSym->st_shndx].sh_name;
            }

            if( (nVerbose>=2) )
            {
                printf("st_name  = %04X  %s\n", pSym->st_name, pStr+pSym->st_name);
                printf("st_value = %08X\n", pSym->st_value );
                printf("st_size  = %04X\n", pSym->st_size );
                printf("st_info  = %02X\n", pSym->st_info );
                printf("st_other = %02X\n", pSym->st_other );
                printf("st_shndx = %04X  %s\n", pSym->st_shndx, pSecName);
                printf("\n");
            }

            // Save a global symbol with all its attributes so we can select from it later when we need globals.
            // We should not have empty symbol string name, so special case a null-strings.
            nameLen = strlen(pStr+pSym->st_name);

            // Some local symbols are stored with the .# postfix. Remove it as it causes problems matching
            // them later: Example: buf.0
            if( nameLen>2 && *(pStr+pSym->st_name+nameLen-2)=='.' && isdigit(*(pStr+pSym->st_name+nameLen-1)) )
                nameLen -= 2;

            // Do the same for the symbols with 2-digit extension number
            if( nameLen>3 && *(pStr+pSym->st_name+nameLen-3)=='.' && isdigit(*(pStr+pSym->st_name+nameLen-1)) && isdigit(*(pStr+pSym->st_name+nameLen-2)))
                nameLen -= 3;

            // Since we store global variables, but the value field contains only the variable required alignment
            // instead of its relative address within its segment, we zap that value to 0 so we can use the same
            // process to get to that variable, once when we relocate its segment.
            if( pSym->st_shndx==SHN_COMMON )
                pSym->st_value = 0x00000000;

            // The other special case is the symbol name containing a space. In the gcc apps there is a
            // symbol "<command line>" which can corrupt our string-based parsing
            if(nameLen && !strchr(pStr+pSym->st_name, ' '))
            {
                // Copy the string into the temporary buffer so we can handle oversized strings
                strncpy(sSymbol, pStr+pSym->st_name, MIN(MAX_SYMBOL_LEN, nameLen));
                sSymbol[MIN(MAX_SYMBOL_LEN, nameLen)] = '\0';

                fprintf(fGlobals, "%08X %08X %04X %10s %s\n",
                    pSym->st_value,
                    pSym->st_value + pSym->st_size,
                    pSym->st_info,
                    strlen(pSecName)? pSecName : "?",
                    sSymbol);

                nGlobals++;
            }
            else
            {
                fprintf(fGlobals, "%08X %08X %04X %10s %s\n",
                    pSym->st_value,
                    pSym->st_value + pSym->st_size,
                    pSym->st_info,
                    strlen(pSecName)? pSecName : "?",
                    "?");

                nGlobals++;
            }

            // Advance the symbol pointer to the next entry
            pSym++;
        }

        // We are done reading all symbols that we are interested with; form the lookup array
        ASSERT(nGlobals);

        // Allocate memory to store the global symbols
        pGlobals = (TGLOBAL *) malloc(sizeof(TGLOBAL)*nGlobals);
        if( pGlobals!=NULL )
        {
            // Clear the array since we depend on some fields to be 0 (file_id and pDef most notably)
            memset(pGlobals, 0, sizeof(TGLOBAL)*nGlobals);

            // Rewind the input file to the start
            fseek(fGlobals, 0, SEEK_SET);

            for(i=0; i<nGlobals; i++)
            {
                fscanf(fGlobals, "%08X %08X %04X %s %s\n",
                    &pGlobals[i].dwAddress,
                    &pGlobals[i].dwEndAddress,
                    &pGlobals[i].dwAttribute,
                    pGlobals[i].SectionName,
                    pGlobals[i].Name);
            }
        }
        else
        {
            fprintf(stderr, "Unable to allocate memory\n");
            return(FALSE);
        }
    }
    else
        fprintf(stderr, "No global symbols in the file (!)\n");

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL WriteGlobalsSection(int fd, int fs)                                  *
*                                                                             *
*******************************************************************************
*
*   Writes out globals symbol table section.
*
*   Globals are read from the (global) array pGlobals, and there are at most
*   nGlobals items. Only items we are interested in are actually stored into
*   the symbol table.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*
*   Implicit:
*       pGlobals - array of all globals
*       nGlobals - number of globals items
*
******************************************************************************/
static BOOL WriteGlobalsSection(int fd, int fs)
{
    TSYMGLOBAL *pHeader;                // Globals header
    DWORD dwSize;                       // Final size of the above structure
    int nGlobalsStored;                 // Number of global symbols stored
    int i, nLen;                        // Counter
    int nSection;                       // Segment of that particular symbol
    char *p;                            // Generic character pointer

    dwSize = sizeof(TSYMGLOBAL) + sizeof(TSYMGLOBAL1)*(nGlobals-1);

    // Allocate memory to store the global symbols
    pHeader = (TSYMGLOBAL *) malloc(dwSize);
    if( pHeader!=NULL )
    {
        nGlobalsStored = 0;

        for( i=0; i<nGlobals; i++ )
        {
            // Assign the section name; possibly add the section for COMMON

            nSection = GetGlobalsSection(pGlobals[i].SectionName, pGlobals[i].Name);

            if( nSection>=0 )
            {
                VERBOSE2 printf("%2d %08X %08X %04X %10s %s\n",
                    nSection,
                    pGlobals[i].dwAddress,
                    pGlobals[i].dwEndAddress,
                    pGlobals[i].dwAttribute,
                    pGlobals[i].SectionName,
                    pGlobals[i].Name);

                pHeader->list[nGlobalsStored].dwStartAddress = pGlobals[i].dwAddress;
                pHeader->list[nGlobalsStored].dwEndAddress   = pGlobals[i].dwEndAddress;
                pHeader->list[nGlobalsStored].pName          = dfs;
                pHeader->list[nGlobalsStored].file_id        = pGlobals[i].file_id;
                pHeader->list[nGlobalsStored].bSegment       = nSection;

                // Copy the symbol name into the strings
                write(fs, pGlobals[i].Name, strlen(pGlobals[i].Name)+1);
                dfs += strlen(pGlobals[i].Name)+1;

                // Copy the symbol definition string into the strings (if defined)
                if(pGlobals[i].pDef)
                {
                    // Since we need to keep only canonical type definitions, if a definition
                    // is complex, cut it into a simple, first-level definition.
                    // The parse typedef code will pick up the complex part.

                    if((p = strchr(pGlobals[i].pDef, '=')))
                    {
                        // Complex definition will be broken up; stores only the basic portion:
                        nLen = p - pGlobals[i].pDef;

                        pHeader->list[nGlobalsStored].pDef = dfs;
                        write(fs, pGlobals[i].pDef, nLen);
                        dfs += nLen;

                        // Write terminate string with 0
                        write(fs, "", 1);               // This will append 0
                        dfs += 1;
                    }
                    else
                    {
                        // Simple definition can be stored as-is

                        pHeader->list[nGlobalsStored].pDef = dfs;
                        write(fs, pGlobals[i].pDef, strlen(pGlobals[i].pDef)+1);

                        dfs += strlen(pGlobals[i].pDef)+1;
                    }
                }
                else
                    pHeader->list[nGlobalsStored].pDef = 0;

                // Increment the actual number of global symbols stored
                nGlobalsStored++;
            }
            else
            {
                // Print out skipped symbol

                VERBOSE2 printf(" - %08X %08X %04X %10s %s\n",
                    pGlobals[i].dwAddress,
                    pGlobals[i].dwEndAddress,
                    pGlobals[i].dwAttribute,
                    pGlobals[i].SectionName,
                    pGlobals[i].Name);
            }
        }

        // Recalculate actual size based on the number of global items actually stored
        dwSize = sizeof(TSYMGLOBAL) + sizeof(TSYMGLOBAL1)*(nGlobalsStored-1);

        // Fill up the globals header
        pHeader->h.hType  = HTYPE_GLOBALS;
        pHeader->h.dwSize = dwSize;
        pHeader->nGlobals = nGlobalsStored;

        // Final write out of the globals section in one single block
        write(fd, pHeader, dwSize);

        free(pHeader);

        VERBOSE2 printf("Parsed %d global items; stored %d\n", nGlobals, nGlobalsStored);

        return( TRUE );
    }
    else
        fprintf(stderr, "Unable to allocate memory\n");

    return(FALSE);
}


/******************************************************************************
*                                                                             *
*   BOOL ParseGlobals(int fd, int fs, BYTE *pBuf)                             *
*                                                                             *
*******************************************************************************
*
*   Loads and parses global symbols from the various ELF sections.
*   Since we already have all global symbols in pGlobals array, we are only
*   interested into getting a file_id of a global symbol, so we can update
*   our pGlobals.
*
*   For the variables, we also wish to update pointer to its typedef.
*
*   In the continuation of this function we actually write out globals section
*   into our symbol table.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
******************************************************************************/
BOOL ParseGlobals(int fd, int fs, BYTE *pBuf)
{
    WORD file_id = 0;                   // Current file ID number
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

    StabEntry *pStab;                   // Pointer to a stab entry
    char *pStr, *p;                     // Pointer to a stab string
    char *pSoDir = "";                  // Source code directory
    char *pSo = "";                     // Current source file
    int i, n, nLen;

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE GLOBALS                                                   ||\n");
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

    //=========================
    // Parse STABS
    //=========================
    // We parse STABS exactly as we do a generic ELF section parsing,
    // but here we extract only global symbols file_id data

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

                case N_GSYM:
                    VERBOSE2 printf("GSYM: file_id=%d  %s\n", file_id, pStr);

                    p = strchr(pStr, ':');
                    if(p)
                    {
                        nLen = p - pStr;
                        if(nLen)
                        {
                            // Search the array of globals for a symbol name
                            for(n=0; n<nGlobals; n++)
                            {
                                if(pGlobals[n].Name[nLen]=='\0' && !memcmp(pGlobals[n].Name, pStr, nLen))
                                {
                                    // Update the file_id of that global symbol
                                    pGlobals[n].file_id = file_id;

                                    // Update pointer to the typedef of that globals symbol
                                    pGlobals[n].pDef = &pStr[nLen + 1];

                                    break;
                                }
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Global symbol without definition: %s\n", pStr);
                            return(FALSE);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Global symbol without typedef: %s\n", pStr);
                        return(FALSE);
                    }

                    // At this point we really want to check if a global symbol was
                    // referenced from a STAB section, but not included into a global
                    // ELF table...
                    if( n==nGlobals )
                    {
                        fprintf(stderr, "ELF specifies global %s but not in the globals section\n", pStr);
                        return(FALSE);
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

        // Write out globals into our symbol table globals section

        return( WriteGlobalsSection(fd, fs) );
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL GlobalsName2Address(DWORD *p, char *pName)                           *
*                                                                             *
*******************************************************************************
*
*   Looks for a global symbol with the specified name.
*   "Absolute" entries are ignored since they are not symbols.
*
*   Where:
*       p is the address of the variable to receive address
*       pName is the symbol name
*
*   Returns:
*       TRUE - symbol was found and the address was stored
*       FALSE - symbol was not found. Address is not updated.
*
******************************************************************************/
BOOL GlobalsName2Address(DWORD *p, char *pName)
{
    TGLOBAL *pGlob;
    char sSymbol[MAX_SYMBOL_LEN+1];
    int i;

    // Both arguments and the globals array have to exist
    if( p && pName && pGlobals )
    {
        // Symbol name may have extra characters at the end, so trim it
        strncpy(sSymbol, pName, MAX_SYMBOL_LEN);
        sSymbol[MAX_SYMBOL_LEN] = '\0';               // Force zero-terminate the string

        if( strchr(sSymbol, ':') )
            *(char *)strchr(sSymbol, ':') = 0;

        pGlob = pGlobals;

        for(i=0; i<nGlobals; i++)
        {
            if( !strcmp(pGlob->Name, sSymbol) && strcmp(pGlob->SectionName, "ABSOLUTE") )
            {
                // Found it! Store the address into the caller's variable and return
                *p = pGlob->dwAddress;
                return( TRUE );
            }

            pGlob++;
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   char *GlobalsName2Section(char *pName)                                    *
*                                                                             *
*******************************************************************************
*
*   Returns the section name of a global symbol.
*   "Absolute" entries are ignored since they are not symbols.
*
*   Where:
*       pName is the symbol name
*
*   Returns:
*       Pointer to the section name
*       NULL if the section name could not be found
*
******************************************************************************/
char *GlobalsName2Section(char *pName)
{
    TGLOBAL *pGlob;
    char sSymbol[MAX_SYMBOL_LEN+1];
    int i;

    // Argument and the globals array have to exist
    if( pName && pGlobals )
    {
        // Symbol name may have extra characters at the end, so trim it
        strncpy(sSymbol, pName, MAX_SYMBOL_LEN);
        sSymbol[MAX_SYMBOL_LEN] = '\0';               // Force zero-terminate the string

        if( strchr(sSymbol, ':') )
            *(char *)strchr(sSymbol, ':') = 0;

        pGlob = pGlobals;

        for(i=0; i<nGlobals; i++)
        {
            if( !strcmp(pGlob->Name, sSymbol) && strcmp(pGlob->SectionName, "ABSOLUTE") )
            {
                // Found it! Return the section name
                return( pGlob->SectionName );
            }

            pGlob++;
        }
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   BYTE QueryCommonsName(char *pName)                                        *
*                                                                             *
*******************************************************************************
*
*   Returns the index into the sCommons array of a variable with the given
*   name. Since that function only considers COMMON types, it returns 0 if the
*   variable could not be found.
*
******************************************************************************/
BYTE QueryCommonsName(char *pName)
{
    int i;

    for(i=MAX_STANDARD_SEG; i<nCommons; i++)
    {
        if( !strcmp(sCommon[i], pName) )
            return( (BYTE) i );
    }

    return( 0 );
}

/******************************************************************************
*                                                                             *
*   int GetGlobalsSection(char *pSection, char *pName)                        *
*                                                                             *
*******************************************************************************
*
*   Returns the section number of a symbol. It also creates a new entry in the
*   sCommon array for the variables of the COMMON type.
*
*   Where:
*       pSection is the section name. Example: .data
*       pName is the symbol name
*
*   Returns:
*       0-MAX_STANDARD_SEG             - Standard sections
*       MAX_STANDARD_SEG - MAX_COMMONS - Symbol in the COMMON section
*       -1                             - Section not found
*
******************************************************************************/
int GetGlobalsSection(char *pSection, char *pName)
{
    int i;

    // Assign the right section number for predefined sections
    //
    // For now, .text is always 0. The linice uses this to handle symbols in the
    // expression evaluator (code will not be dereferenced). This could be done
    // better and parse sections and their flags, but this works for most of the
    // cases.
    //
    if( !strcmp(pSection, ".text") )  return( 0 );
    if( !strcmp(pSection, ".data") )  return( 1 );
    if( !strcmp(pSection, ".rodata")) return( 2 );
    if( !strcmp(pSection, ".bss") )   return( 3 );

    // Extended section names. These are introduced to cover kernel 2.4 symbols
    if( !strcmp(pSection, ".text.init") )              return( 4 );
    if( !strcmp(pSection, ".data.init") )              return( 5 );
    if( !strcmp(pSection, ".setup.init") )             return( 6 );
    if( !strcmp(pSection, ".initcall.init") )          return( 7 );
    if( !strcmp(pSection, ".data.cacheline_aligned") ) return( 8 );

    // TODO: Let's sneak .modinfo section along with the COMMONS and see how it goes...

    if( !strcmp(pSection, "COMMON") || !strcmp(pSection, ".modinfo") )
    {
        // This is a common symbol. Try to find it in the existing list of commons, and if not there,
        // add it to the list of commons
        i = QueryCommonsName(pName);
        if( i )                         // If we found it, return its index
            return( i );

        // Did not find the common variable yet in the array. Add it.
        // Index i already points to the first unused entry.
        if( nCommons < MAX_COMMONS )
        {
            strcpy(sCommon[nCommons], pName);

            nCommons++;                 // Adds one more entry

            return( nCommons-1 );
        }
        else
        {
            fprintf(stderr, "Too many symbols in the COMMON section. The limit is %d.\n", MAX_COMMONS-MAX_STANDARD_SEG);
        }
    }

    // Return invalid section number
    return( -1 );
}

/******************************************************************************
*                                                                             *
*   BYTE GlobalsName2SectionNumber(char *pName)                               *
*                                                                             *
*******************************************************************************
*
*   Returns the section number of a symbol. It also creates a new entry in the
*   sCommon array for the variables of the COMMON type.
*
*   Use this function when only a variable name is known.
*
*   Where:
*       pName is the symbol name
*
*   Returns:
*       0-MAX_STANDARD_SEG             - Standard sections
*       MAX_STANDARD_SEG - MAX_COMMONS - Symbol in the COMMON section
*
******************************************************************************/
BYTE GlobalsName2SectionNumber(char *pName)
{
    char *pSection;                     // Section name
    int nSection;                       // Section number

    // Get the section name and its number, possibly creating a new COMMON section
    pSection = GlobalsName2Section(pName);

    // The section should be found. If now, we want to know about that case!
    if(pSection)
    {
        nSection = GetGlobalsSection(pSection, pName);

        // Whoever is calling this function, it should never return an invalid section number.
        // If it does, we want to find out about it!
        if( nSection<0 )
        {
            fprintf(stderr, "Invalid section in GlobalsName2SectionNumber() for %s\n", pName);
            nSection = 1;       // What to do? Return a fake .data section
        }
    }
    else
    {
        fprintf(stderr, "Internal bug! - %s\n", pName);
        nSection = 1;           // What to do? Return a fake .data section
    }

    return( (BYTE)(nSection & 0xFF) );
}

/******************************************************************************
*                                                                             *
*   char *GlobalsSection2Address(DWORD *p, int nGlobalIndex, char *pSection)  *
*                                                                             *
*******************************************************************************
*
*   Looks for a global symbol with the specified combination of attribute and
*   section name, returning a matching symbol name and the address.
*
*   Where:
*       p is the address of the variable to receive the symbol address
*       nGlobalIndex is the index of the global record to query
*       pName is the global section name to search
*
*   Returns:
*       Address of the symbol name - symbol was found and the address was stored
*       NULL - symbol was not found. Address is not updated.
*
******************************************************************************/
char *GlobalsSection2Address(DWORD *p, int nGlobalIndex, char *pSectionName)
{
    TGLOBAL *pGlob;

    pGlob = &pGlobals[nGlobalIndex];

    if( !strcmp(pGlob->SectionName, pSectionName) )
    {
        // Found it! Store the address into the caller's variable and return
        // with the address of the symbol name
        *p = pGlob->dwAddress;
        return( pGlob->Name );
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   char *GlobalsGetSectionName(int nGlobalIndex)                             *
*                                                                             *
*******************************************************************************
*
*   Returns the section name of the global entry of a given index.
*
******************************************************************************/
char *GlobalsGetSectionName(int nGlobalIndex)
{
    TGLOBAL *pGlob;

    pGlob = &pGlobals[nGlobalIndex];

    return( pGlob->SectionName );
}
