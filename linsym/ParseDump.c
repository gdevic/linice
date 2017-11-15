/******************************************************************************
*                                                                             *
*   Module:     ParseDump.c                                                   *
*                                                                             *
*   Date:       10/26/03                                                      *
*                                                                             *
*   Copyright (c) 2003-2005 Goran Devic                                       *
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

        This module contains the ELF buffer dumping code

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


/******************************************************************************
*                                                                             *
*   BOOL DumpElfHeader(Elf32_Ehdr *pElf)                                      *
*                                                                             *
*******************************************************************************
*
*   Pretty-prints the ELF header proper.
*
*   Where:
*       pElf - pointer to the ELF header
*
******************************************************************************/
BOOL DumpElfHeader(Elf32_Ehdr *pElf)
{
    printf("Elf Header:\n");
    printf("Elf32_Half e_type       = %04X      %s\n", pElf->e_type, pElf->e_type==ET_REL? "Relocatable":"Executable");
    printf("Elf32_Half e_machine    = %04X      Intel\n", pElf->e_machine);
    printf("Elf32_Word e_version    = %08X\n", pElf->e_version);
    printf("Elf32_Addr e_entry      = %08X  %s\n", pElf->e_entry, pElf->e_entry==0? "No entry point":"");
    printf("Elf32_Off  e_phoff      = %08X  %s\n", pElf->e_phoff, pElf->e_phoff==0? "No program header table":"Program header table offset");
    printf("Elf32_Off  e_shoff      = %08X  %s\n", pElf->e_shoff, pElf->e_shoff==0? "No section header table":"Section header table offset");
    printf("Elf32_Word e_flags      = %08X\n", pElf->e_flags);
    printf("Elf32_Half e_ehsize     = %04X      Elf header size\n", pElf->e_ehsize);
    printf("Elf32_Half e_phentsize  = %04X      Size of one pht entry\n", pElf->e_phentsize);
    printf("Elf32_Half e_phnum      = %04X      Number of entries in pht\n", pElf->e_phnum);
    printf("Elf32_Half e_shentsize  = %04X      Size of one sht entry\n", pElf->e_shentsize);
    printf("Elf32_Half e_shnum      = %04X      Number of entries in sht\n", pElf->e_shnum);
    printf("Elf32_Half e_shstrndx   = %04X      Sht entry of the section name string table\n", pElf->e_shstrndx);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL ParseDump(BYTE *pBuf)                                                *
*                                                                             *
*******************************************************************************
*
*   This is an utility function that dumps all the sections and records
*   from the ELF headers.
*
*   Where:
*       pBuf - buffer containing the ELF file
*
******************************************************************************/
BOOL ParseDump(BYTE *pBuf)
{
    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecStab = NULL;         // Section .STAB
    Elf32_Shdr *SecStabstr = NULL;      // Section .STABSTR
    Elf32_Shdr *SecSymtab = NULL;       // Section .SYMTAB
    Elf32_Shdr *SecStrtab = NULL;       // Section .STRTAB

    StabEntry *pStab;                   // Pointer to a stab entry
    char *pStr;                         // Pointer to a stab string
    BOOL fCont;                         // Line continuation?
    char *pSoDir = NULL;                // Source code directory
    int i;
    int nCurrentSection;                // Current string section offset
    int nSectionSize;                   // Current section string size

    printf("=============================================================================\n");
    printf("||         PARSE DUMP                                                      ||\n");
    printf("=============================================================================\n");

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

        // Dump the section header setting
#if 1
        {
            static const char *SecType[] =
            {
                "SHT_NULL",
                "SHT_PROGBITS",
                "SHT_SYMTAB",
                "SHT_STRTAB",
                "SHT_RELA",
                "SHT_HASH",
                "SHT_DYNAMIC",
                "SHT_NOTE",
                "SHT_NOBITS",
                "SHT_REL",
                "SHT_SHLIB",
                "SHT_DYNSYM"
            };

            printf("Section Header %02X\n", i);
            printf("Elf32_Word sh_name      = %04X      %s\n", SecCurr->sh_name, pBuf + SecName->sh_offset + SecCurr->sh_name );
            printf("Elf32_Word sh_type      = %04X      %s\n", SecCurr->sh_type, SecCurr->sh_type < 12? SecType[SecCurr->sh_type] : "sys" );
            printf("Elf32_Word sh_flags     = %04X      %s%s%s\n", SecCurr->sh_flags,
                SecCurr->sh_flags & SHF_WRITE? "SHF_WRITE ":"",
                SecCurr->sh_flags & SHF_ALLOC? "SHF_ALLOC ":"",
                SecCurr->sh_flags & SHF_EXECINSTR? "SHF_EXECINSTR ":"");
            printf("Elf32_Addr sh_addr      = %08X\n", SecCurr->sh_addr );
            printf("Elf32_Off  sh_offset    = %08X\n", SecCurr->sh_offset );
            printf("Elf32_Word sh_size      = %04X\n", SecCurr->sh_size );
            printf("Elf32_Word sh_link      = %04X\n", SecCurr->sh_link );
            printf("Elf32_Word sh_info      = %04X\n", SecCurr->sh_info );
            printf("Elf32_Word sh_addralign = %04X\n", SecCurr->sh_addralign );
            printf("Elf32_Word sh_entsize   = %04X\n", SecCurr->sh_entsize  );
        }
#endif
#if 1
        printf("%s\n", pBuf + SecName->sh_offset + SecCurr->sh_name );
#endif
    }

    //=========================
    // Parse STABS
    //=========================
    if( SecStab && SecStabstr )
    {
        // Parse stab section
        pStab = (StabEntry *) (pBuf + SecStab->sh_offset);
        i = SecStab->sh_size / sizeof(StabEntry);
        nCurrentSection = 0;
        nSectionSize = 0;
        fCont=FALSE;
        while( i-- )
        {
            pStr = (char *)pBuf + SecStabstr->sh_offset + pStab->n_strx + nCurrentSection;
#if 0
            printf("unsigned long n_strx  = %08X\n", pStab->n_strx );
            printf("unsigned char n_type  = %02X\n", pStab->n_type );
            printf("unsigned char n_other = %02X\n", pStab->n_other );
            printf("unsigned short n_desc = %04X\n", pStab->n_desc );
            printf("unsigned long n_value = %08X\n", pStab->n_value );
#endif

            switch( pStab->n_type )
            {
                // 0x00 (N_UNDEF) is actually storing the current section string size
                case N_UNDF:
                    // We hit another string section, need to advance the string offset of the previous section
                    nCurrentSection += nSectionSize;
                    // Save the (new) currect string section size
                    nSectionSize = pStab->n_value;

                    printf("HdrSym size: %lX\n", pStab->n_value);
                    printf("=========================================================\n");
                break;

                case N_GSYM:
                    printf("GSYM   ");
                    printf("line: %d  %s\n", pStab->n_desc, pStr);
                break;

                case N_FNAME:
                    printf("FNAME  ");
                    printf("%s\n", pStr);
                break;

                case N_FUN:
                    printf("FUN---");
                    if( *pStr==0 )
                    {
                        // Function end
                        printf("END--------- +%lX\n\n", pStab->n_value);
                    }
                    else
                    {
                        // Function start & name
                        printf("START-%08lX--%s\n", pStab->n_value, pStr);
                    }
                break;

                case N_STSYM:
// TODO: This is wrong - a static symbol can be in .data or .rodata section or even in .bss
                    printf("STSYM  ");
                    printf("line: %d segment-???  %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);
                break;

                case N_LCSYM:
// TODO: This is wrong - a static symbol can be in .data or .rodata section or even in .bss
                    printf("LCSYM  ");
                    printf("line: %d segment-??? %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);
                break;

                case N_MAIN:
                    printf("MAIN  ");
                    printf("%s\n", pStr);
                break;

                case N_ROSYM:
                    printf("ROSYM  ");
                    printf("%s\n", pStr);
                break;

                case N_PC:
                    printf("PC  ");
                    printf("%s\n", pStr);
                break;

                case N_NSYMS:
                    printf("NSYMS  ");
                    printf("%s\n", pStr);
                break;

                case N_NOMAP:
                    printf("NOMAP  ");
                    printf("%s\n", pStr);
                break;

                case N_OBJ:
                    printf("OBJ  ");
                    printf("%s\n", pStr);
                break;

                case N_OPT:
                    printf("OPT  %s\n", pStr);
                break;

                case N_RSYM:
                    printf("RSYM  REGISTER VARIABLE ");
                    printf("%s in %ld\n", pStr, pStab->n_value);
                break;

                case N_M2C:
                    printf("M2C  ");
                    printf("%s\n", pStr);
                break;

                case N_SLINE:
                    printf("SLINE  line: %2d -> +%lX\n", pStab->n_desc, pStab->n_value);
                break;

                case N_DSLINE:
                    printf("DSLINE  ");
                    printf("%s\n", pStr);
                break;

                case N_BSLINE:
                    printf("BSLINE  ");
                    printf("%s\n", pStr);
                break;

                case N_DEFD:
                    printf("DEFD  ");
                    printf("%s\n", pStr);
                break;

                case N_FLINE:
                    printf("FLINE  ");
                    printf("%s\n", pStr);
                break;

                case N_EHDECL:
                    printf("EHDECL  ");
                    printf("%s\n", pStr);
                break;

                case N_CATCH:
                    printf("CATCH  ");
                    printf("%s\n", pStr);
                break;

                case N_SSYM:
                    printf("SSYM  ");
                    printf("%s\n", pStr);
                break;

                case N_ENDM:
                    printf("ENDM  ");
                    printf("%s\n", pStr);
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
                        }
                    }
                break;

                case N_LSYM:
                    // If continuation of line...
                    if(fCont)
                    {
                        fCont = FALSE;
                        printf("%s", pStr);
                    }
                    else
                    {
                        printf("LSYM   ");
                        if(pStab->n_value==0)
                        {
                            int maj, min;

                            // Decode typedef ID numbers

                            // BUG: I dont know whether this is a bug in a compiler generating typedef stabs,
                            //      or a feature, but some of typedefs have incorrect format like
                            //      "persist:,480,32;" without parenthesis...
                            // Temp fix: Skip over these typedefs (they cause imbalanced scan)
                            if( strchr(pStr, '(')==NULL )
                                break;

                            if(sscanf(strchr(pStr, '('), "(%d,%d)", &maj, &min)!=2)
                            {
                                printf("Error scanning '(' TYPEDEF ID %s\n", pStr);
                                return -1;
                            }

                            // n_value=0 -> Type definition
                            if(pStab->n_desc==0)
                            {
                                // n_desc=0 -> built in type
                                printf("           TYPEDEF(%d,%d): %s", maj, min, pStr);
                            }
                            else
                            {
                                // n_desc!=0 -> line number where it is declared
                                printf("line: %4d TYPEDEF(%d,%d): %s", pStab->n_desc, maj, min, pStr);
                            }
                        }
                        else
                        {
                            // n_value != 0 -> variable address relative to EBP

                            // n_desc = line number where the symbol is declared
                            printf("line: %2d LOCAL_VARIABLE [EBP+%02lX] %s", pStab->n_desc, pStab->n_value, pStr);
                        }
                    }
                    if(pStr[strlen(pStr)-1]!='\\')
                        printf("\n");

                    // Check if this line will continue on the next one
                    if(pStr[strlen(pStr)-1]=='\\')
                        fCont=TRUE;
                break;

                case N_BINCL:
                    printf("BINCL %s\n", pStr);
                break;

                case N_EXCL:
                    printf("EXCL  %s\n", pStr);
                break;

                case N_SOL:
                    printf("SOL  ");
                    printf("%s\n", pStr);
                break;

                case N_PSYM:
                    printf("PSYM   ");
                    printf("line: %d PARAM [EBP+%lX]  %s\n", pStab->n_desc, pStab->n_value, pStr);
                break;

                case N_EINCL:
                    printf("EINCL  ");
                    printf("%s\n", pStr);
                break;

                case N_ENTRY:
                    printf("N_ENTRY %08lX\n", pStab->n_value);
                break;

                case N_LBRAC:
                    printf("LBRAC              +%lX  {  (%d)\n", pStab->n_value, pStab->n_desc);
                break;

                case N_SCOPE:
                    printf("SCOPE  ");
                    printf("%s\n", pStr);
                break;

                case N_RBRAC:
                    printf("RBRAC              +%lX  }\n", pStab->n_value);
                break;

                case N_BCOMM:
                    printf("BCOMM  ");
                    printf("%s\n", pStr);
                break;

                case N_ECOMM:
                    printf("ECOMM  ");
                    printf("%s\n", pStr);
                break;

                case N_ECOML:
                    printf("ECOML  ");
                    printf("%s\n", pStr);
                break;

                case N_WITH:
                    printf("WITH  ");
                    printf("%s\n", pStr);
                break;

                case N_NBTEXT:
                    printf("NBTEXT  ");
                    printf("%s\n", pStr);
                break;

                case N_NBDATA:
                    printf("NBDATA  ");
                    printf("%s\n", pStr);
                break;

                case N_NBBSS:
                    printf("NBBSS  ");
                    printf("%s\n", pStr);
                break;

                case N_NBSTS:
                    printf("NBSTS  ");
                    printf("%s\n", pStr);
                break;

                case N_NBLCS:
                    printf("NBLCS  ");
                    printf("%s\n", pStr);
                break;

                case N_LENG:
                    printf("LENG  ");
                    printf("%s\n", pStr);
                break;

                default:
                    printf("UNDEFINED STAB\n");
            }

            pStab++;
        }
    }
    else
        fprintf(stderr, "No STAB section in the file\n");

    return( TRUE );
}


