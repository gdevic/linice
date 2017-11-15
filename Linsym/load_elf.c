/******************************************************************************
*                                                                             *
*   Module:     load_elf.c                                                    *
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

        This module contains the main ELF buffer parsing code

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


//****************************************************************************
//
// Assumption: There can be at max 65535 source files and each file can have
//             at most 65535 lines.
//             Each function can be at most 64K long (line offsets)
//
// Assumption: C function tmpfile() will never fail
//
//****************************************************************************

FILE *fSources;                         // Temp file descriptor for source file names

FILE *fGlobals;                         // Temp file descriptor for globals
int nGlobals;                           // Number of global items that we stored in a file


const char *SecType[] =
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

DWORD nGSYM;                            // Total number of global symbols
TSource SO[MAX_SO];                     // Source file structure
DWORD nSO;                              // Number of basic source files

int dfs;                                // Top offset into strings file

extern BOOL SetupSourcesArray(FILE *fSources);
extern BOOL ParseSource(int fd, int fs);
extern BOOL ParseGlobal(int fd, int fs, FILE *fGlobals, int nGlobals);
extern BOOL ParseFunctionLines(int fd, int fs, BYTE *pElf);
extern BOOL ParseFunctionScope(int fd, int fs, BYTE *pBuf);
extern BOOL ParseTypedefs(int fd, int fs, BYTE *pBuf);


int ParseSectionsPass1(BYTE *pBuf)
{
    Elf32_Ehdr *pElfHeader;             // ELF header

    Elf32_Shdr *Sec;                    // Section header array
    Elf32_Shdr *SecName;                // Section header string table
    Elf32_Shdr *SecCurr;                // Current section
    Elf32_Shdr *SecStab = NULL;         // Section .STAB
    Elf32_Shdr *SecStabstr = NULL;      // Section .STABSTR
    Elf32_Shdr *SecSymtab = NULL;       // Section .SYMTAB
    Elf32_Shdr *SecStrtab = NULL;       // Section .STRTAB

    Elf32_Sym *pSym;                    // Pointer to a symtab entry
    StabEntry *pStab;                   // Pointer to a stab entry
    char *pStr;                         // Pointer to a stab string
    BOOL fCont;                         // Line continuation?
    char *pSoDir = NULL;                // Source code directory
    int i;

    printf("=============================================================================\n");
    printf("||         PARSE SECTIONS PASS 1                                           ||\n");
    printf("=============================================================================\n");

    ASSERT(sizeof(StabEntry)==12);

    pElfHeader = (Elf32_Ehdr *) pBuf;

    // PASS1: Reset the counter variables
    nSO = 0;
    nGSYM = 0;
    memset(&SO, 0, sizeof(SO));

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
#endif
#if 0
        printf("%s\n", pBuf + SecName->sh_offset + SecCurr->sh_name );
#endif
    }

    //==============================
    // Parse ELF global symbol table
    //==============================
    if( SecSymtab && SecStrtab )
    {
        pSym = (Elf32_Sym *) (pBuf + SecSymtab->sh_offset);
        pStr = (char *)pBuf + SecStrtab->sh_offset;
        i = SecSymtab->sh_size / sizeof(Elf32_Sym) - 1;

        // Skip the null entry
        pSym++;

        printf("Symbol table:\n");
        while( i-- )
        {
            printf("\n");
            printf("st_name  = %04X  %s\n", pSym->st_name, pStr+pSym->st_name);
            printf("st_value = %08X\n", pSym->st_value );
            printf("st_size  = %04X\n", pSym->st_size );
            printf("st_info  = %02X\n", pSym->st_info );
            printf("st_other = %02X\n", pSym->st_other );
            printf("st_shndx = %04X ", pSym->st_shndx);
            switch(pSym->st_shndx)
            {
            case SHN_UNDEF: printf("EXTERNDEF\n");
                break;
            case SHN_ABS  : printf("ABSOLUTE\n");
                break;
            case SHN_COMMON: printf("COMMON\n");
                break;
            default:
                printf("%s\n", pBuf + SecName->sh_offset + Sec[pSym->st_shndx].sh_name );
            }

            // If a symbol represent a global function or variable, save it in
            // a fGlobal file:
            //  11h - Global symbol (variable)
            //  12h - Global symbol (function)
            if( pSym->st_info==0x11 || pSym->st_info==0x12 )
            {
                fprintf(fGlobals, "%s %08X %08X\n", pStr+pSym->st_name, pSym->st_value, pSym->st_value + pSym->st_size);
                nGlobals++;
                printf("Global symbol queued\n");
            }

            pSym++;
        }
    }
    else
        printf("No symbols in the file (!)\n");

    //=========================
    // Parse STABS
    //=========================
    if( SecStab && SecStabstr )
    {
        // Parse stab section
        pStab = (StabEntry *) (pBuf + SecStab->sh_offset);
        i = SecStab->sh_size / sizeof(StabEntry);
        fCont=FALSE;
        while( i-- )
        {
            pStr = (char *)pBuf + SecStabstr->sh_offset + pStab->n_strx;
#if 0
            printf("unsigned long n_strx  = %08X\n", pStab->n_strx );
            printf("unsigned char n_type  = %02X\n", pStab->n_type );
            printf("unsigned char n_other = %02X\n", pStab->n_other );
            printf("unsigned short n_desc = %04X\n", pStab->n_desc );
            printf("unsigned long n_value = %08X\n", pStab->n_value );
#endif

            switch( pStab->n_type )
            {
                case N_UNDF:
                    printf("UNDF  ");
                    printf("%s\n", pStr);
                    printf("=========================================================\n");
                break;

                case N_GSYM:
                    printf("GSYM   ");
                    printf("line: %d  %s\n", pStab->n_desc, pStr);

                    SO[nSO].nGSYM++;
                    nGSYM++;
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

                        SO[nSO].nFUN++;
                        nGSYM++;
                    }
                break;

                case N_STSYM:
                    printf("STSYM  ");
                    printf("line: %d DSS  %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);

                    SO[nSO].nSTSYM++;
                break;

                case N_LCSYM:
                    printf("LCSYM  ");
                    printf("line: %d BSS %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);
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

                        nSO++;
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
                            SO[nSO].pPath = pStr;
                        }
                        else
                        {
                            // File
                            printf("Source file: %s\n", pStr );

                            fprintf(fSources, "%s%s\n", pSoDir, pStr);
                            SO[nSO].pName = pStr;
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
                            if(sscanf(strchr(pStr, '('), "(%d,%d)", &maj, &min)!=2)
                            {
                                printf("Error scanning TYPEDEF ID %s\n", pStr);
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

                            // If the line will continue, dont print \n
                            SO[nSO].nTYPEDEF++;
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
                    SO[nSO].nBINCL++;

                    printf("BINCL [%d] ", SO[nSO].nBINCL);
                    printf("%s\n", pStr);
                break;

                case N_EXCL:
                    SO[nSO].nBINCL++;

                    printf("EXCL  [%d] ", SO[nSO].nBINCL);
                    printf("%s\n", pStr);
                break;

                case N_SOL:
                    printf("SOL  ");
                    printf("%s\n", pStr);

                    // Change of source - if the first character is not '/', we need to
                    // prefix the last defined file path to complete the directory
                    if( *pStr!='/' && *pStr!='\\' )
                    {
                        fprintf(fSources, "%s", pSoDir);
                    }
                    fprintf(fSources, "%s\n", pStr);
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
        printf("No STAB section in the file\n");

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   BYTE *LoadElf(char *sName)                                                *
*                                                                             *
*******************************************************************************
*
*   Loads an ELF executable or object module.
*
*   Returns:
*       pointer to a buffer containing the ELF file.
*       NULL for error
*
******************************************************************************/
BYTE *LoadElf(char *sName)
{
    Elf32_Ehdr Elf;                     // ELF header
    struct stat fd_stat;                // ELF file stats
    int fd, status;
    BYTE *pBuf;

    // Open the ELF binary file
    fd = open(sName, O_RDONLY | O_BINARY);
    if( fd>0 )
    {
        // Get the file stats
        fstat(fd, &fd_stat);

        // Read the ELF header
        status = read(fd, &Elf, sizeof(Elf32_Ehdr));
        if( status==sizeof(Elf32_Ehdr) )
        {
            // Make sure it's an ELF file
            if( Elf.e_ident[0]==ELFMAG0
             && Elf.e_ident[1]==ELFMAG1
             && Elf.e_ident[2]==ELFMAG2
             && Elf.e_ident[3]==ELFMAG3 )
            {
                // We only support relocatable and executable files
                if( Elf.e_type==ET_REL || Elf.e_type==ET_EXEC )
                {
                    // Make some basic sanity checks
                    if( Elf.e_machine==EM_386
                     && Elf.e_ehsize==sizeof(Elf32_Ehdr)
                     && Elf.e_shentsize==sizeof(Elf32_Shdr)
                     && Elf.e_shoff!=0 )
                    {
                        // Dump the ELF header sections
                        printf("File: %s  length: %ld\n", sName, fd_stat.st_size);
                        printf("Elf Header:\n");
                        printf("Elf32_Half e_type       = %04X      %s\n", Elf.e_type, Elf.e_type==ET_REL? "Relocatable":"Executable");
                        printf("Elf32_Half e_machine    = %04X      Intel\n", Elf.e_machine);
                        printf("Elf32_Word e_version    = %08X\n", Elf.e_version);
                        printf("Elf32_Addr e_entry      = %08X  %s\n", Elf.e_entry, Elf.e_entry==0? "No entry point":"");
                        printf("Elf32_Off  e_phoff      = %08X  %s\n", Elf.e_phoff, Elf.e_phoff==0? "No program header table":"Program header table offset");
                        printf("Elf32_Off  e_shoff      = %08X  %s\n", Elf.e_shoff, Elf.e_shoff==0? "No section header table":"Section header table offset");
                        printf("Elf32_Word e_flags      = %08X\n", Elf.e_flags);
                        printf("Elf32_Half e_ehsize     = %04X      Elf header size\n", Elf.e_ehsize);
                        printf("Elf32_Half e_phentsize  = %04X      Size of one pht entry\n", Elf.e_phentsize);
                        printf("Elf32_Half e_phnum      = %04X      Number of entries in pht\n", Elf.e_phnum);
                        printf("Elf32_Half e_shentsize  = %04X      Size of one sht entry\n", Elf.e_shentsize);
                        printf("Elf32_Half e_shnum      = %04X      Number of entries in sht\n", Elf.e_shnum);
                        printf("Elf32_Half e_shstrndx   = %04X      Sht entry of the section name string table\n", Elf.e_shstrndx);

                        // Get the total length of the file, allocate memory and load it in
                        pBuf = (BYTE*) malloc(fd_stat.st_size);
                        if( pBuf )
                        {
                            lseek(fd, 0, SEEK_SET);
                            status = read(fd, pBuf, fd_stat.st_size);
                            if( status==fd_stat.st_size )
                            {
                                return( pBuf );
                            }
                            else
                                printf("Error reading file %s\n", sName);

                            free(pBuf);
                        }
                        else
                            printf("Error allocating memory\n");
                    }
                    else
                        printf("Invalid ELF file format %s!\n", sName);
                }
                else
                    printf("Only relocatable and executable ELF files are supported!\n");
            }
            else
                printf("The file %s is not a valid ELF file\n", sName);
        }
        else
            printf("Error reading file %s", sName);
    }
    else
        printf("Unable to open file %s\n", sName);

    return( NULL );
}


void PushString(int fd, char *pStr)
{
    TSYMHEADER Header;                  // Generic header

    Header.hType = HTYPE_IGNORE;        // Ignore this header
    Header.dwSize = sizeof(TSYMHEADER) + strlen(pStr) + 1;

    // Push the header and the string
    write(fd, &Header, sizeof(TSYMHEADER));
    write(fd, pStr, strlen(pStr)+1);
}

/******************************************************************************
*                                                                             *
*   BOOL ElfToSym(BYTE *pElf, char *pSymName)                                 *
*                                                                             *
*******************************************************************************
*
*   Given the buffer containing an ELF file, translates it into a symbol
*   file which it saves under the pSymName file name.
*
*   Where:
*       pElf - buffer in memory with the complete ELF file to be parsed
*       pSymName - path/name of the output symbol file
*       pTableName - internal name of this symbol table
*
*   Returns:
*       True for OK
*       False for error
*
******************************************************************************/
BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName)
{
    int fd;                             // Output symbol file descriptor
    int i;
    TSYMTAB SymTab;                     // Symbol table main header structure
    static TSYMHEADER HeaderEnd =       // Terminating header
    { HTYPE__END, sizeof(TSYMHEADER) };

    int fs;                             // Temp file descriptor for strings
    char pTmp[FILENAME_MAX];            // Temporary buffer
    char *pBuf;                         // Temporary buffer

    // If the pElf is NULL, exit
    if( pElf )
    {
        // Create and truncate the symbol file name
        printf("Creating symbol file: %s\n", pSymName);

        fd = open(pSymName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
        if( fd>0 )
        {
            // Create a temp file to store source file names
            fSources = tmpfile();

            // Create a temp file in which we will queue all global symbols
            // We use text representation of the data that we encounter during
            // the parsing of the ELF global symbol table
            fGlobals = tmpfile();
            nGlobals = 0;

            // PASS 1 - Parse sections and count all the pieces that we will use
            ParseSectionsPass1(pElf);

            // Print the statistics
            printf("Number of major source files: %d\n", nSO);

            for(i=0; i<(int)nSO; i++)
            {
                printf("Source file %d: %s%s\n", i, SO[i].pPath, SO[i].pName);
                printf("  Number of types defined:  %d\n", SO[i].nTYPEDEF);
                printf("  Number of include files:  %d\n", SO[i].nBINCL);
                printf("  Number of functions:      %d\n", SO[i].nFUN);
                printf("  Number of global symbols: %d\n", SO[i].nGSYM);
                printf("  Number of static symbols: %d\n", SO[i].nSTSYM);
            }

            printf("Total number of global symbols (funct+var): %d\n", nGSYM);

            // Print the list of the source files to load
            printf("Source files to load:\n");
            fseek(fSources, 0, SEEK_SET);
            while( !feof(fSources) )
            {
                fgets(pTmp, FILENAME_MAX, fSources);
                printf("  %s", pTmp);
            }

            // Set the symbol file header signature
            strcpy(SymTab.sSig, SYMSIG);

            // Set the internal symbol file name
            // Zero terminate it in the case it's too long
            strcpy(SymTab.sTableName, pTableName);
            SymTab.sTableName[MAX_MODULE_NAME-1] = 0;

            // Set the symbol file version number
            SymTab.Version = SYMVER;

            SymTab.dwSize = 0;          // To be written later
            SymTab.dStrings = 0;        // To be written later

            // Write a base header of the symbol table
            i = write(fd, &SymTab, sizeof(TSYMTAB)-sizeof(TSYMHEADER));
            if( i==sizeof(TSYMTAB)-sizeof(TSYMHEADER) )
            {
                // Create a temp file in which we will append all strings
                fs = fileno(tmpfile());

                // Make the first string by default a zero length string so we can
                // safely use offset 0 to represent a non-string and invalid value
                write(fs, "", 1);
                dfs = 1;                // Start with the offset 1 of the strings

                PushString(fd, "Symbol information for LinIce kernel level debugger");
                PushString(fd, "Copyright 2000-2001 Goran Devic");

                // The very first thing we need to do is queue these source
                // file names into an array so anyone can start referring to
                // them using a file_id
                SetupSourcesArray(fSources);

                // Using the temp fGlobals file where we queued all global items
                // (and nGlobals which contains the number of items),
                // load and push the global symbol information
                ParseGlobal(fd, fs, fGlobals, nGlobals);

                // Using the temp fSources file as the source path/name queue,
                // load and push extracted source files into fd/fs
                ParseSource(fd, fs);

                // Function lines will parse ELF file and extract tokens
                ParseFunctionLines(fd, fs, pElf);

                // Function variables and their scopes
                ParseFunctionScope(fd, fs, pElf);

                // Type definitions bound to each major source file
                //
                ParseTypedefs(fd, fs, pElf);


                // Add the terminating section HTYPE__END
                write(fd, &HeaderEnd, sizeof(HeaderEnd));

                // Copy all strings at the end of the headers
                // Rewind the strings file
                lseek(fs, 0, SEEK_SET);
                pBuf = (char *) malloc(dfs);
                if( pBuf!=NULL )
                {
                    // Store the offset to the strings (current top of the fd file)
//                    SymTab.dStrings = filelength(fd);
                    SymTab.dStrings = lseek(fd, 0, SEEK_CUR);

                    read(fs, pBuf, dfs);
                    write(fd, pBuf, dfs);

                    // Total size is headers + strings
                    SymTab.dwSize = SymTab.dStrings + dfs;

                    // Write out the symbol header
                    lseek(fd, 0, SEEK_SET);
                    write(fd, &SymTab, sizeof(TSYMTAB)-sizeof(TSYMHEADER));

                    // Close the symbol file
                    close(fd);

                    free(pBuf);

                    return( TRUE );
                }
                else
                    printf("Unable to allocate memory\n");
            }
            else
                printf("Error writing symbol file %s\n", pSymName);
        }
        else
            printf("Unable to create symbol file %s\n", pSymName);
    }

    return( FALSE );
}

