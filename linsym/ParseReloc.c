/******************************************************************************
*                                                                             *
*   Module:     ParseReloc.c                                                  *
*                                                                             *
*   Date:       12/25/01 Chrismas                                             *
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

extern int dfs;

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
******************************************************************************/
BOOL ParseReloc(int fd, int fs, BYTE *pBuf)
{
    TSYMRELOC Reloc;                    // Reloc section header

    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecRel = NULL;          // Section .REL

    Elf32_Rel *pRel;                    // Pointer to a relocation entry
    char *pStr;                         // Pointer to a stab string
    int i, nItems;
    int nGlobalIndex;
    BOOL f1, f2, f3;                    // Sections

    printf("=============================================================================\n");
    printf("||         PARSE RELOCATION INFORMATION                                    ||\n");
    printf("=============================================================================\n");

    pElfHeader = (Elf32_Ehdr *) pBuf;

    // Ok, we have the complete file inside the buffer...
    // Find the section header and the string table of section names
    Sec = (Elf32_Shdr *) &pBuf[pElfHeader->e_shoff];
    SecName = &Sec[pElfHeader->e_shstrndx];

    // Only relocatable files such is object files are parsed in this section
    if( pElfHeader->e_type!=ET_REL )
    {
        printf("Not a relocatable file. Section skipped.\n");
        return( FALSE );
    }

    for( i=1; i<pElfHeader->e_shnum; i++ )
    {
        SecCurr = &Sec[i];
        pStr = (char *)pBuf + SecName->sh_offset + SecCurr->sh_name;

        if( strcmp(".rel.text", pStr)==0 )
            SecRel = SecCurr;
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
    //  found in the global symbol table (stored in the refInitModuleOffset field).
    //
    //  Data relocation is much more tricky for modules:
    //
    //  In order to find the relocation value for global data, we need to find a single data item
    //  and store one address of where it is addressed to by the module code, call it `sample'
    //  since it is for us just an address to sample real variable address once when module is
    //  relocated. Then, the relocation amount is this address minus the relative offset of that
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
    //  This we do for different types of segments:
    //      1) global data (".data" section)
    //      2) uninitialized global data (st_shndx = FFF2 "COMMON" section)

    if( SecRel )
    {
        // Make sure we start with the clean record
        memset(&Reloc, 0, sizeof(Reloc));

        // Search for the global function symbol
        for(nGlobalIndex=0; nGlobalIndex<nGlobals; nGlobalIndex++)
        {
            if( !strcmp(pGlobals[nGlobalIndex].Name, "init_module") )
            {
                Reloc.list[0].refOffset = pGlobals[nGlobalIndex].dwAddress;
                Reloc.list[0].refFixup  = pGlobals[nGlobalIndex].dwAddress;     // Entry 0 both contain the same value!

                printf("Global 'init_module':\n");
                printf(" Reference offset %08X\n", Reloc.list[0].refOffset);

                break;
            }
        }

        if( nGlobalIndex<nGlobals )
        {
            pRel = (Elf32_Rel *) ((char*)pElfHeader + SecRel->sh_offset);

            // We did not process these yet
            f1 = f2 = f3 = TRUE;
        
            // Calculate the number of entries in this section
            if( SecRel->sh_entsize && SecRel->sh_size )
            {
                nItems = SecRel->sh_size / SecRel->sh_entsize;

                // Loop over relocation entries

                while( nItems-- )
                {
                    if( ELF32_R_TYPE(pRel->r_info)==R_386_32 )
                    {
                        // Found a 32 bit relocation data item that we can use to calculate final offset

                        // Index of a global symbol (1-based) that the reloc entry applies
                        nGlobalIndex = ELF32_R_SYM(pRel->r_info) - 1;

                        // Store globals of this kind (MAX_SYMRELOC)
                        //
                        // 0) Program code segment:             0x12  ".text"
                        // 1) Program data segment:             0x11  ".data"    (global variables)
                        // 2) Program data 2 segment:           0x01  ".data"    (static variables)
                        // 3) Program data 3 segment:           0x11  "COMMON"   (uninitialized globals)
            
                        if( f1 && pGlobals[nGlobalIndex].wAttribute==0x11 && !strcmp(pGlobals[nGlobalIndex].SectionName, ".data") )
                        {
                            printf("Global .data symbol:\n");
                            printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, pGlobals[nGlobalIndex].dwAddress, pGlobals[nGlobalIndex].Name);

                            Reloc.list[1].refOffset = pGlobals[nGlobalIndex].dwAddress;
                            Reloc.list[1].refFixup  = pRel->r_offset - Reloc.list[0].refOffset;

                            f1 = FALSE;
                        }

#if 0  // These were to hold static symbols (locals to a source file), but it does not work this way
                        if( f2 && pGlobals[nGlobalIndex].wAttribute==0x01 && !strcmp(pGlobals[nGlobalIndex].SectionName, ".data") )
                        {
                            printf("Static variables .data symbol:\n");
                            printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, pGlobals[nGlobalIndex].dwAddress, pGlobals[nGlobalIndex].Name);

                            Reloc.list[2].refOffset = pGlobals[nGlobalIndex].dwAddress;
                            Reloc.list[2].refFixup  = pRel->r_offset - Reloc.list[0].refOffset;

                            f2 = FALSE;
                        }
#endif
f2 = FALSE;

                        if( f3 && pGlobals[nGlobalIndex].wAttribute==0x11 && !strcmp(pGlobals[nGlobalIndex].SectionName, "COMMON") )
                        {
                            printf("Uninitialized global COMMON symbol:\n");
                            printf(" Using relocation data record at .text=%08X, fixup=%08X, symbol=%s\n", pRel->r_offset, pGlobals[nGlobalIndex].dwAddress, pGlobals[nGlobalIndex].Name);

                            Reloc.list[3].refOffset = pGlobals[nGlobalIndex].dwAddress;
                            Reloc.list[3].refFixup  = pRel->r_offset - Reloc.list[0].refOffset;

                            f3 = FALSE;
                        }

                        if( f1==FALSE && f2==FALSE && f3==FALSE )
                            break;
                    }

                    pRel++;
                }

                // Write out relocation record
    
                Reloc.hType  = HTYPE_RELOC;
                Reloc.dwSize = sizeof(TSYMRELOC);

                // Fixed at 4 records for now
                Reloc.nReloc = MAX_SYMRELOC;

                write(fd, &Reloc, Reloc.dwSize);
            }
        }
        else
            printf("Error locating expected global function `init_module'\n");
    }
    else
        printf("Failed to get the relocation record!\n");

    return( TRUE );
}

