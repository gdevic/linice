/******************************************************************************
*                                                                             *
*   Module:     ParseReloc.c                                                  *
*                                                                             *
*   Date:       12/25/01 Chrismas                                             *
*                                                                             *
*   Copyright (c) 2001-2005 Goran Devic                                       *
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

        This module contains parsing code to make reloc section

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 12/25/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "Common.h"                     // Include platform specific set

#include "loader.h"                     // Include global protos

extern PSTR dfs;                        // Global pointer to strings (to append)

extern int nCommons;                    // Number of COMMON symbols
int        nCommonsFound = 0;           // Local count of number of COMMONs found and relocated

extern BOOL GlobalsName2Address(DWORD *p, char *pName);
extern char *GlobalsSection2Address(DWORD *p, int nGlobalIndex, char *pSectionName);
extern char *GlobalsGetSectionName(int nGlobalIndex);
extern BYTE QueryCommonsName(char *pName);


#define MAX_RELOC       256             // Maximum number of relocation entries

static TSYMRELOC1 Reloc1[MAX_RELOC];    // Relocation entries that we will fill in

/******************************************************************************
*                                                                             *
*   BOOL ParseReloc(int fd, int fs, BYTE *pBuf)                               *
*                                                                             *
*******************************************************************************
*
*   Parses the input object file and creates reloc symbol file section.
*   This is only done for object file types.
*
*   Globals are read from the (global) array pGlobals, and there are nGlobals
*   items.
*
*   Where:
*       fd - symbol table file descriptor (to write to)
*       fs - strings file (to write to)
*       pBuf - buffer containing the ELF file
*
*   Returns:
*       TRUE - Relocation data parsed and stored
*       FALSE - Critical error
*
******************************************************************************/
BOOL ParseReloc(int fd, int fs, BYTE *pBuf)
{
    TSYMRELOC Reloc;                    // Reloc section header

    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecRelText = NULL;      // Section .REL.TEXT

    Elf32_Rel *pRel;                    // Pointer to a relocation entry
    char *pStr;                         // Pointer to a stab string
    int i, nItems;
    DWORD dwAddress;                    // Temp address value
    BOOL fdata, frodata, fbss;          // Sections that we have
    int nGlobalIndex;                   // Index variable
    char *pName;                        // Symbol name
    int nSection;                       // Section number for the commons symbol

    VERBOSE2 printf("=============================================================================\n");
    VERBOSE2 printf("||         PARSE RELOCATION INFORMATION                                    ||\n");
    VERBOSE2 printf("=============================================================================\n");
    VERBOSE1 printf("Parsing relocation information.\n");

    pElfHeader = (Elf32_Ehdr *) pBuf;

    // Ok, we have the complete file inside the buffer...
    // Find the section header and the string table of section names
    Sec = (Elf32_Shdr *) &pBuf[pElfHeader->e_shoff];
    SecName = &Sec[pElfHeader->e_shstrndx];

    // Only relocatable files such is object files are parsed in this section
    if( pElfHeader->e_type!=ET_REL )
    {
        VERBOSE1 printf("Not a relocatable file. Section skipped.\n");
        return( TRUE );
    }

    for( i=1; i<pElfHeader->e_shnum; i++ )
    {
        SecCurr = &Sec[i];
        pStr = (char *)pBuf + SecName->sh_offset + SecCurr->sh_name;

        if( strcmp(".rel.text", pStr)==0 )
            SecRelText = SecCurr;
    }

    //=========================
    // Parse REL.text section
    //=========================

    // This might not be the best way to go, but it works:
    //
    //  Code relocation is easy:
    //
    //  We get the address of the module `init_module` function from the kernel module descriptor
    //  and the relocation amount is that address minus the relative offset of that function as
    //  found in the global symbol table.
    //
    //  Data relocation is much more tricky for modules:
    //
    //  In order to find the relocation value for global data, we need to find a single data item
    //  and store one address where it is addressed to it by the module code, call it `sample'
    //  since it is for us just an address to sample real variable address once when module is
    //  loaded. Then, the relocation amount is this target address minus the relative offset of that
    //  global variable that we store in the symbol table relocation record.
    //
    //  We look for the data relocation section within the ELF file and then for each symbol
    //  there we loop over the global symbols that we have. We stop as soon as we find a global
    //  variable that matches the reloc item.
    //  We need to do that in order to find (1) the offset in the text section where the debugger
    //  will pick up the already relocated variable address (Reloc.refOffset), and (2) the true offset
    //  of that variable (Reloc.symOffset), so the debugger can examine and figure out the relocation
    //  of the data segment for an already loaded (and relocated) kernel module.
    //

    // Make sure we start with the clean record
    memset(&Reloc, 0, sizeof(Reloc));
    memset(&Reloc1, 0, sizeof(Reloc1));

    if( SecRelText )
    {
        // Search for the global function symbol
        if( GlobalsName2Address(&dwAddress, "init_module") )
        {
            Reloc1[0].refOffset = dwAddress;        // Both entries 0 contain the same value
            Reloc1[0].refFixup  = dwAddress;        // of the init_module offset!

            VERBOSE2 printf("Anchor symbol: 'init_module' at %08X\n", Reloc1[0].refOffset);

            pRel = (Elf32_Rel *) ((char*)pElfHeader + SecRelText->sh_offset);

            // We did not process these yet
            fdata = frodata = fbss = FALSE;

            // Calculate the number of entries in this section
            if( SecRelText->sh_entsize && SecRelText->sh_size )
            {
                nItems = SecRelText->sh_size / SecRelText->sh_entsize;

                VERBOSE2 printf("Searching %d fixup references...\n", nItems);

                // Loop over all relocation entries until we find all the references from each of the sections

                while( nItems-- )
                {
                    switch( ELF32_R_TYPE(pRel->r_info) )
                    {
                        case R_386_NONE:
                            VERBOSE2 printf("R_386_NONE %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                            break;

                        case R_386_32:
                        {
                            // Found a 32 bit relocation data item that we can use to calculate final offset

                            // Index of a global symbol (1-based) that the reloc entry applies
                            nGlobalIndex = ELF32_R_SYM(pRel->r_info) - 1;

                            VERBOSE2 printf("R_386_32   .text=%08X info=%05X\n", pRel->r_offset, pRel->r_info);

                            if( !fdata && (pName=GlobalsSection2Address(&dwAddress, nGlobalIndex, ".data"))!=NULL )
                            {
                                VERBOSE2 printf("Global .data symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[1].refOffset = dwAddress;
                                Reloc1[1].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                fdata = TRUE;
                            }

                            if( !frodata && (pName=GlobalsSection2Address(&dwAddress, nGlobalIndex, ".rodata"))!=NULL )
                            {
                                VERBOSE2 printf("Static variables .rodata symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[2].refOffset = dwAddress;
                                Reloc1[2].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                frodata = TRUE;
                            }

                            if( !fbss && (pName=GlobalsSection2Address(&dwAddress, nGlobalIndex, ".bss"))!=NULL )
                            {
                                VERBOSE2 printf("Uninitialized global .bss symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[3].refOffset = dwAddress;
                                Reloc1[3].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                fbss = TRUE;
                            }

                            if( (pName=GlobalsSection2Address(&dwAddress, nGlobalIndex, "COMMON"))!=NULL )
                            {
                                // Test if this sybol already have the relocation record
                                if( (nSection = QueryCommonsName(pName)) )
                                {
                                    if( Reloc1[nSection].refFixup==0 )
                                    {
                                VERBOSE2 printf("Uninitialized COMMON_%d symbol %X:\n", nSection, nGlobalIndex);
                                VERBOSE2 printf(" Using relocation fixup at .text=%08X, symbol=%s\n", pRel->r_offset, pName);

                                // COMMON symbols are different since they dont give the address, only the alignment
                                // and size, which we dont use
                                Reloc1[nSection].refFixup  = pRel->r_offset - Reloc1[0].refOffset;
                                Reloc1[nSection].refOffset = 0;

                                nCommonsFound++;
                                    }
                                }
                            }
                        }
                        break;

                        case R_386_PC32:
                        {
                            // Found a 32 bit relocation data item that we can use to calculate final offset

                            // Index of a global symbol (1-based) that the reloc entry applies
                            nGlobalIndex = ELF32_R_SYM(pRel->r_info) - 1;

                            VERBOSE2 printf("R_386_PC32 .text=%08X info=%05X\n", pRel->r_offset, pRel->r_info);
#if 0
// We dont seem to need the record of this particular type, since in all the tests so far all the
// relocation could be completed using only the above R_386_32

                            // Find what segment this entry applies to

                            pName = GlobalsGetSectionName(nGlobalIndex);

                            if( !fdata && !strcmp(pName, ".data") )
                            {
                                VERBOSE2 printf("Global .data symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[1].refOffset = dwAddress;
                                Reloc1[1].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                fdata = TRUE;
                            }

                            if( !frodata && !strcmp(pName, ".rodata") )
                            {
                                VERBOSE2 printf("Static variables .rodata symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[2].refOffset = dwAddress;
                                Reloc1[2].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                frodata = TRUE;
                            }

                            if( !fbss && !strcmp(pName, ".bss") )
                            {
                                VERBOSE2 printf("Uninitialized global .bss symbol %X:\n", nGlobalIndex);
                                VERBOSE2 printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, dwAddress, pName);

                                Reloc1[3].refOffset = dwAddress;
                                Reloc1[3].refFixup  = pRel->r_offset - Reloc1[0].refOffset;

                                fbss = TRUE;
                            }
#endif
                        }
                        break;

                        case R_386_GOT32:
                            VERBOSE2 printf("R_386_GOT32 %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_PLT32:
                            VERBOSE2 printf("R_386_PLT32 %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_COPY:
                            VERBOSE2 printf("R_386_COPY %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_GLOB_DAT:
                            VERBOSE2 printf("R_386_GLOB_DAT %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_JMP_SLOT:
                            VERBOSE2 printf("R_386_JMP_SLOT %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_RELATIVE:
                            VERBOSE2 printf("R_386_RELATIVE %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_GOTOFF:
                            VERBOSE2 printf("R_386_GOTOFF %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        case R_386_GOTPC:
                            VERBOSE2 printf("R_386_GOTPC %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                        break;

                        default:
                            VERBOSE2 printf("INVALID RELOC TYPE %08X info=%05X\n", pRel->r_offset, pRel->r_info);
                    }

                    pRel++;

                    // Exit early if we got all the sections
#if 0
// We cannot exit early any more since we need all the relocation sections in order to find all the
// offsets for the COMMON symbols.

                    if( fdata && frodata && fbss )
                        break;
#endif
                }

                VERBOSE2 printf("Found %d of %d COMMON symbols\n", nCommonsFound, nCommons - 4);

                // Basically, we should have at least 4 records now --
                // The ones above 4 are the COMMON symbols relocations
                Reloc.nReloc = nCommons;

                // Write out relocation record

                Reloc.h.hType  = HTYPE_RELOC;
                Reloc.h.dwSize = sizeof(TSYMRELOC) + sizeof(TSYMRELOC1) * (Reloc.nReloc-1);

                // Write out the base structure and the relocation array
                write(fd, &Reloc, sizeof(TSYMRELOC) - sizeof(TSYMRELOC1));
                write(fd, &Reloc1, sizeof(TSYMRELOC1) * Reloc.nReloc);
            }
            return( TRUE );
        }
        else
            fprintf(stderr, "Error locating expected global function `init_module'\n");
    }
    else
        fprintf(stderr, "Failed to get the relocation record for the .text section!\n");

    return( FALSE );
}
