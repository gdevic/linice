/******************************************************************************
*                                                                             *
*   Module:     ParseGlobals.c                                                *
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

extern int dfs;


// Define internal array structre that holds global symbols for a lookup
typedef struct
{
    DWORD dwAddress;                    // Start address of a symbol
    DWORD dwEndAddress;                 // End address of a symbol
    DWORD dwAttribute;                  // Attributes of a symbol
    char SectionName[16];               // Section name string
    char Name[MAX_SYMBOL_LEN+1];        // Symbol canonical name

    char *pDef;                         // Pointer to symbol definition
    WORD file_id;                       // Defined in this file ID (only globals)
} TGLOBAL;

static TGLOBAL *pGlobals = NULL;        // Array of global symbols
static int nGlobals = 0;                // Number of global symbols



extern WORD GetFileId(char *pSoDir, char *pSo);
extern DWORD GetGlobalSymbolAddress(char *pName);


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
#if 0
            if((opt & OPT_VERBOSE) && (nVerbose==2))
            {
                printf("st_name  = %04X  %s\n", pSym->st_name, pStr+pSym->st_name);
                printf("st_value = %08X\n", pSym->st_value );
                printf("st_size  = %04X\n", pSym->st_size );
                printf("st_info  = %02X\n", pSym->st_info );
                printf("st_other = %02X\n", pSym->st_other );
                printf("st_shndx = %04X  %s\n", pSym->st_shndx, pSecName);
                printf("\n");
            }
#endif
            // Save a global symbol with all its attributes so we can select from it later when we need globals.
            // We should not have empty symbol string name, so special case a null-strings (skip them)
            nameLen = strlen(pStr+pSym->st_name);

            // Reject empty symbol names &&
            // The other special case is the symbol name containing a space. In the gcc apps there is a
            // symbol "<command line>" which can corrupt our string-based parsing
            if(nameLen && !strchr(pStr+pSym->st_name, ' '))
            {
                // Copy the string into the temporary buffer so we can handle oversized strings
                strncpy(sSymbol, pStr+pSym->st_name, MAX_SYMBOL_LEN);
                sSymbol[MAX_SYMBOL_LEN] = '\0';

                fprintf(fGlobals, "%08X %08X %04X %10s %s\n",
                    pSym->st_value,
                    pSym->st_value + pSym->st_size,
                    pSym->st_info,
                    strlen(pSecName)? pSecName : "?",
                    sSymbol);

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
    int eStore;                         // Store this particular symbol
    char *p;                            // Generic character pointer

    dwSize = sizeof(TSYMGLOBAL) + sizeof(TSYMGLOBAL1)*(nGlobals-1);

    // Allocate memory to store the global symbols
    pHeader = (TSYMGLOBAL *) malloc(dwSize);
    if( pHeader!=NULL )
    {
        nGlobalsStored = 0;

        for( i=0; i<nGlobals; i++ )
        {
            eStore = -1;                // -1 means 'dont store this symbol'

            // Store globals of this kind (MAX_SYMRELOC):
            //
            // 0) Program code segment:             0x12  ".text"
            // 1) Program data segment:             0x11  ".data"    (global variables)
//          // 2) Program data 2 segment:           0x01  ".data"    (static variables)           DOES NOT WORK THIS WAY
            // 3) Program data 3 segment:           0x11  ".COMMON"  (uninitialized globals)
            // 4) Program data 4 segment:           0x11  ".bss"     (uninitialized global data)

            if( pGlobals[i].dwAttribute==0x12 && !strcmp(pGlobals[i].SectionName, ".text") )     eStore = 0;
            if( pGlobals[i].dwAttribute==0x11 && !strcmp(pGlobals[i].SectionName, ".data") )     eStore = 1;
//          if( pGlobals[i].dwAttribute==0x01 && !strcmp(pGlobals[i].SectionName, ".data") )     eStore = 2;
            if( pGlobals[i].dwAttribute==0x11 && !strcmp(pGlobals[i].SectionName, "COMMON") )    eStore = 3;
            if( pGlobals[i].dwAttribute==0x11 && !strcmp(pGlobals[i].SectionName, ".bss") )      eStore = 4;

            VERBOSE2 printf("%c %08X %08X %04X %10s %s\n",
                eStore<0? ' ': eStore + '0',
                pGlobals[i].dwAddress,
                pGlobals[i].dwEndAddress,
                pGlobals[i].dwAttribute,
                pGlobals[i].SectionName,
                pGlobals[i].Name);

            if( eStore>=0 )
            {
                pHeader->global[nGlobalsStored].dwStartAddress = pGlobals[i].dwAddress;
                pHeader->global[nGlobalsStored].dwEndAddress   = pGlobals[i].dwEndAddress;
                pHeader->global[nGlobalsStored].dName          = dfs;
                pHeader->global[nGlobalsStored].file_id        = pGlobals[i].file_id;
                pHeader->global[nGlobalsStored].bFlags         = eStore;

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

                        pHeader->global[nGlobalsStored].dDef = dfs;
                        write(fs, pGlobals[i].pDef, nLen);
                        dfs += nLen;

                        // Write terminate string with 0
                        write(fs, "", 1);               // This will append 0
                        dfs += 1;
                    }
                    else
                    {
                        // Simple definition can be stored as-is

                        pHeader->global[nGlobalsStored].dDef = dfs;
                        write(fs, pGlobals[i].pDef, strlen(pGlobals[i].pDef)+1);

                        dfs += strlen(pGlobals[i].pDef)+1;
                    }
                }
                else
                    pHeader->global[nGlobalsStored].dDef = 0;

                // Increment the actual number of global symbols stored
                nGlobalsStored++;
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
            if( !strcmp(pGlob->Name, sSymbol) )
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
*   char *GlobalsSection2Address(DWORD *p, DWORD dwAttribute, char *pSection) *
*                                                                             *
*******************************************************************************
*
*   Looks for a global symbol with the specified combination of attribute and
*   section name, returning a matching symbol name and the address.
*
*   Where:
*       p is the address of the variable to receive address
*       dwAttribute is the filter symbol attribute that has to match
*       pName is the global section name to search
*
*   Returns:
*       Address of the symbol name - symbol was found and the address was stored
*       NULL - symbol was not found. Address is not updated.
*
******************************************************************************/
char *GlobalsSection2Address(DWORD *p, DWORD dwAttribute, char *pSectionName)
{
    TGLOBAL *pGlob;
    int i;

    // All arguments and the globals array have to exist
    if( p && pSectionName && pGlobals )
    {
        pGlob = pGlobals;

        for(i=0; i<nGlobals; i++)
        {
            if( dwAttribute==pGlob->dwAttribute && !strcmp(pGlob->SectionName, pSectionName) )
            {
                // Found it! Store the address into the caller's variable and return
                // with the address of the symbol name
                *p = pGlob->dwAddress;
                return( pGlob->Name );
            }

            pGlob++;
        }
    }

    return( NULL );
}



