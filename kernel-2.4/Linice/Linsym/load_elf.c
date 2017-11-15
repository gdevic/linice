/******************************************************************************
*                                                                             *
*   Module:     load_elf.c                                                    *
*                                                                             *
*   Date:       11/16/00                                                      *
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

        Loading ELF file

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/16/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
//#include <assert.h>
//#include <sys/types.h>                  // Include file operations
#include <malloc.h>
#include <string.h>                     // Include strings header file
//#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#ifdef WINDOWS
#include <io.h>
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>
#include <stdlib.h>                     // Include standard library file
#include "msvc/elf.h"
#else // !WINDOWS
#include <unistd.h>                     // Include standard UNIX header file
#include <linux/elf.h>                  // Load ELF prototypes
#include <sys/stat.h>                   // Include file operations
#endif // !WINDOWS

#include "ice-types.h"                  // Include private data types
#include "ice-symbols.h"                // Include symbol file defines
#include "loader.h"                     // Include loader global protos
#include "primes.h"                     // Insert table of 1024 prime numbers
#include "stabs.h"                      // Load GNU STABS definitions


void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct
{
    unsigned long n_strx;
    unsigned char n_type;
    unsigned char n_other;
    unsigned short n_desc;
    unsigned long n_value;
} PACKED StabEntry;

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

extern int symcmp(const void *p1, const void *p2);
extern DWORD str2key(char *str);

/******************************************************************************
*                                                                             *
*   BOOL LoadSource(char *pPath, char *pName, TFILE *pFileCur)                *
*                                                                             *
*******************************************************************************
*
*   Loads a source file.
*
******************************************************************************/
BOOL LoadSource(char *pPath, char *pName, TFILE *pFileCur)
{
    struct stat prop;                   // Source code stat variable
    char sLocalName[256];               // Local source path/name
    char *pBuf, *pBufCur;               // Buffer to store source file, current line
    FILE *fin;                          // Source file descriptor
    int i, lines;                       // Line counter
    int status;

    // Search for the source: first in the current directory or the one
    // given as a command line parameter --source
    strcpy(sLocalName, pSource);        // Copy default path
    strcpy(sLocalName, "/");
    strcat(sLocalName, pName);          // Append file name

    status = stat(sLocalName, &prop);
    if(status)
    {
        // Source file is not found.
        // Search the default STABS path or a newly entered path name
        strcpy(sLocalName, pPath);      // Copy STABS path
        strcpy(sLocalName, "/");
        strcat(sLocalName, pName);      // Append file name

        status = stat(sLocalName, &prop);
        if(status)
        {
            // Source file is not there. We'll have to ask you
            // where to look for it, if the --prompt was not specified
            if( !(opt & OPT_PROMPT) )
            {
                do
                {
                    // Prompt for the missing source
                    printf("Enter the correct PATH to the source file %s: ", pName);
                    gets(sLocalName);

                    // Properly append source name
                    if( sLocalName[strlen(sLocalName)-1] != '/' )
                        strcat(sLocalName, "/");
                    strcat(sLocalName, pName);      // Append file name

                    status = stat(sLocalName, &prop);
                }
                while( status != 0 );
            }
            else
            {
                // Well, can't find the source file so return w/o it
                return(FALSE);
            }
        }
    }

    // Allocate buffer for the source file
    pBufCur = pBuf = malloc(prop.st_size);
    if( pBuf )
    {
        // Open the source file, load it line by line counting them
        fin = fopen(sLocalName, "r");
        if(fin!=NULL)
        {
            lines = 0;

            while( !feof(fin) )
            {
                fgets(pBufCur, 256, fin);
                pBufCur += strlen(pBufCur) + 1;
                lines++;
            }

            // Allocate array of char pointer to file lines
            pFileCur->pLine = calloc(sizeof(char*) * lines, 1);

            // Store pointers to source file lines
            pBufCur = pBuf;
            for(i=0; i<lines; i++)
            {
                pFileCur->pLine[i] = pBufCur;
                pBufCur += strlen(pBufCur) + 1;
            }

            return(TRUE);
        }
        else
        {
            printf("LoadSource: Error opening %s\n", sLocalName);
        }
    }
    else
    {
        printf("LoadSource: Error allocating memory!\n");
        exit(1);
    }

    return(FALSE);
}


/******************************************************************************
*                                                                             *

*                                                                             *
*******************************************************************************
*
*   Parse ELF file and create Linice symbol file
*
*   Where:
*       Elf - ELF header
*       pBuf - buffer where the ELF input file was loaded
*       nameOut - the name of the output module
*       filesize - size of the complete ELF input file
*
******************************************************************************/
void ParseSections(int fd, Elf32_Ehdr *Elf, BYTE *pBuf, char *nameOut, DWORD filesize)
{
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
    int i;                              // Generic counter

    TSYMTAB Symtab;                     // Symbol table header to build
    TGLOBAL Global;                     // Gobals header
    TSYM_PUB *pSymPub;                  // Pointer to a sym publics
    WORD *pHash;                        // Pointer to a publics hash table
    char *pStrings, *pStrCur;           // Pointer to strings, current string
    DWORD key, index, collisions;       // Used for hashing
    DWORD items;                        // Generic item counter
    DWORD written;                      // Bytes written so far

    TSYMBOL Symbol;                     // Global symbol structure
    TFUN *pFun, *pFunCur;               // Pointer to function descriptor, current
    TLINE *pLine, *pLineCur = NULL;     // Pointer to line numbers, current
    TLEX *pLex, *pLexCur = NULL;        // Pointer to lexical scope, current

    TSRC Src;                           // Global source file structure
    TFILE *pFile, *pFileCur;            // Pointer to file array, current
    char sPath[256] = { 0 };            // Source code path


    // Ok, we have the complete file inside the buffer...
    // Find the section header and the string table of section names
    Sec = (Elf32_Shdr *) &pBuf[Elf->e_shoff];
    SecName = &Sec[Elf->e_shstrndx];

    for( i=1; i<Elf->e_shnum; i++ )
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

    //===============================================================
    // Allocate all buffers
    //===============================================================
    // We will overallocate buffers, and then parse the ELF file and
    // store symbols as we find them. Lastly, we will cut the extras

    // Overallocate buffer for global symbols
    pSymPub = calloc(filesize, 1);

    // Overallocate buffer for strings
    pStrCur = pStrings = calloc(filesize, 1);

    // Overallocate buffer for the function descriptor array
    pFunCur = pFun = calloc(filesize, 1);

    // Overallocate buffer for files: 256 files?  :-)
    pFileCur = pFile = calloc(sizeof(TFILE) * 256, 1);

    // Make sure all allocations made it
    if( !pSymPub || !pStrings || !pFun || !pFile )
    {
        printf("Error allocating memory!\n");
        return;
    }

    Symbol.pFun = pFun;

    //===============================================================
    // Read ELF global symbol table and build the SYM_PUB array
    //===============================================================

    items = 0;
    if( SecSymtab && SecStrtab )
    {
        char *pSecStr;                  // Section string name
        DWORD Section;                  // Section code name

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

            pSecStr = "";
            Section = 0;

            switch(pSym->st_shndx)
            {
            case SHN_UNDEF: printf("EXTERNDEF\n");
                break;
            case SHN_ABS  : printf("ABSOLUTE\n");
                break;
            case SHN_COMMON: printf("COMMON\n");
                break;
            default:
                pSecStr = pBuf + SecName->sh_offset + Sec[pSym->st_shndx].sh_name;
                printf("%s\n", pSecStr );
            }

            // If the symbol has the name, proceed to the following check
            // (Sometimes the name is 0 and sometimes the first char is space)
            if( pSym->st_name && *(pStr+pSym->st_name)!=' ' )
            {
                // If the section string name is data, text, bss, we need to store it
                if( !strcmp(pSecStr, ".text") )
                    Section = (DWORD)'T';
                else
                if( !strcmp(pSecStr, ".data") )
                    Section = (DWORD)'D';
                else
                if( !strcmp(pSecStr, ".bss") )
                    Section = (DWORD)'?';

                // Store the symbol if qualified
                if( Section )
                {
                    strcpy(pStrCur, pStr+pSym->st_name);

                    pSymPub[items].dwAddress = pSym->st_value;
                    pSymPub[items].Section = Section;
                    pSymPub[items].pName = pStrCur;

                    pStrCur += strlen(pStrCur) + 1;
                    items++;
                }
            }

            pSym++;
        }
    }
    else
    {
        printf("No global symbols in the file (!)\n");
        exit(2);
    }

    //===============================================================
    // Sort the symbol table by address
    //===============================================================
    qsort((void *)pSymPub, items, sizeof(TSYM_PUB), symcmp);

    Global.nSyms       = items;
    Global.dwAddrStart = pSymPub[0].dwAddress;
    Global.dwAddrEnd   = pSymPub[Global.nSyms-1].dwAddress;

    // Have a hash table that is several times the number of elements, and find the
    // prime value that is at least that from our hash table
    // The max value in the current prime table is 38873, that divided by 4 is about
    // 9700 entries. Since even the kernel has just about 6000, we are fine.
    for( i=1; i<1023; i++)
    {
        if( primes[i] > items * 4 )
            break;
    }

    Global.nHash = primes[i];

    pHash = calloc(sizeof(WORD) * Global.nHash, 1);

    if( pHash==NULL )
    {
        printf("Error allocating memory\n");
        return;
    }

    //===============================================================
    // Hash symbol names into the hash table
    //===============================================================
    // Loop for each string in the symbol table array, hash it in
    collisions = 0;
    for( i=0; i<(int)items; i++)
    {
#if 1
        printf("%08X index: %s\n", pSymPub[i].dwAddress, pSymPub[i].pName);
        fflush(NULL);
#endif

        key = str2key(pSymPub[i].pName);
        index = key % Global.nHash;

        while( pHash[index] != 0 )
        {
            collisions++;
            if( ++index==Global.nHash )
                index = 0;
        }
        // Store the item index into the hash table
        pHash[index] = (WORD)(i + 1);
    }

    printf("Hash table collisions: %d out of %d\n", collisions, items);


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
                break;

                case N_FNAME:
                    printf("FNAME  ");
                    printf("%s\n", pStr);
                break;

                //-----------------------------------------------------------------------
                //                              FUNCTION
                //-----------------------------------------------------------------------
                case N_FUN:
                    if( *pStr )
                    {                       // FUNCTION START
                        printf("FUN---START-%08lX--%s\n", pStab->n_value, pStr);

                        // Copy the function starting address into the current function descriptor
                        pFunCur->dwAddress = pStab->n_value;

                        // Copy the function name into the strings and link it in
                        // We are not interested in the type of function return, so we'll cut it off
                        strcpy(pStrCur, pStr);      // Copy the function name+return_type
                        strcat(pStrCur, ":");       // Append marking ":"
                        *(strchr(pStr, ':')) = 0;   // Cut off at first ":"

                        pFunCur->pName = pStrCur;
                        pStrCur += strlen(pStrCur) + 1;

                        // Overallocate buffer for line numbers
                        pLineCur = pLine = calloc(filesize, 1);

                        // Overallocate buffer for function lexical scope array
                        pLexCur = pLex = calloc(filesize, 1);

                        if( !pLine || !pLex )
                        {
                            printf("Error allocating memory\n");
                            return;
                        }

                        // Link in those two arrays
                        pFun[Symbol.nFun].pLine = pLine;
                        pFun[Symbol.nFun].pLex  = pLex;
                    }
                    else
                    {                   // FUNCTION END
                        printf("FUN---END--------- +%lX\n\n", pStab->n_value);

                        // Copy the function ending address
                        pFunCur->dwEndAddress = pStab->n_value;

                        pLineCur = NULL;
                        pLexCur = NULL;

                        // Next function...
                        pFunCur++;
                        Symbol.nFun++;
                    }
                break;

                case N_STSYM:
                    printf("STSYM  ");
                    printf("line: %d DSS  %08lX  %s\n", pStab->n_desc, pStab->n_value, pStr);
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
                    printf("%s in %d\n", pStr, (int) pStab->n_value);
                break;

                case N_M2C:
                    printf("M2C  ");
                    printf("%s\n", pStr);
                break;

                //-----------------------------------------------------------------------
                //                     LINE NUMBERS WITHIN A FUNCTION
                //-----------------------------------------------------------------------
                case N_SLINE:
                    printf("SLINE  line: %2d -> +%lX\n", pStab->n_desc, pStab->n_value);

                    // Line address is the relative offset from the function start
                    pLineCur->dwAddress = pFunCur->dwAddress + pStab->n_value;
                    pLineCur->dwLine    = pStab->n_desc;

                    // Advance to the next line slot
                    pLineCur++;
                    pFun->nLine++;
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

                //-----------------------------------------------------------------------
                //                              SOURCE FILE
                //-----------------------------------------------------------------------
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
                            // Directory first
                            printf("Source directory: %s\n", pStr);
                            strcpy(sPath, pStr);
                        }
                        else
                        {
                            // File
                            printf("Source file: %s  Text section offset: %08lX\n", pStr, pStab->n_value );

                            // If the level of debug info requires, package the sources
                            if( nTranslate >= TRAN_PACKAGE )
                                if(LoadSource(sPath, pStr, pFileCur))
                                {
                                    // Store the file path/name in the string area
                                    strcpy(pStrCur, sPath);     // Copy the file path
                                    strcat(pStrCur, "/");
                                    strcat(pStrCur, pStr);      // Append the file name
                                    pFile->pFileName = pStrCur;
                                    pStrCur += strlen(pStrCur) + 1;

                                    Src.nFile++;
                                    pFileCur++;
                                }

                            // Reset the sPath variable after using it
                            sPath[0] = 0;
                        }
                    }
                break;

                case N_LSYM:
                    // If continuation of line...
                    if(fCont)
                    {
                        fCont = FALSE;
                        printf("                   %s\n", pStr);
                    }
                    else
                    {
                    printf("LSYM   ");
                    if(pStab->n_value==0)
                    {
                        // n_value=0 -> Type definition
                        if(pStab->n_desc==0)
                        {
                            // n_desc=0 -> built in type
                            printf("          TYPEDEF: %s\n", pStr);
                        }
                        else
                        {
                            // n_desc!=0 -> line number where it is declared
                            printf("line: %2d  TYPEDEF: %s\n", pStab->n_desc, pStr);
                        }
                    }
                    //-----------------------------------------------------------------------
                    //                  LOCAL VARIABLE TO A FUNCTION
                    //-----------------------------------------------------------------------
                    else
                    {
                        // n_value != 0 -> variable address relative to EBP

                        // n_desc = line number where the symbol is declared
                        printf("line: %2d LOCAL_VARIABLE [EBP+%02lX] %s\n", pStab->n_desc, pStab->n_value, pStr);

                        // Add a local variable to a lexical scope
                        pLexCur->token = LEX_LOCAL;

                        strcpy(pStrCur, pStr);              // Copy the variable name + type
                        pLexCur->pName = pStrCur;
                        pStrCur += strlen(pStrCur) + 1;

                        pLexCur->value = pStab->n_value;    // Copy the EBP value

                        pLexCur++;
                        pFun->nLex++;
                    }
                    }
                    // Check if this line will continue on the next one
                    if(pStr[strlen(pStr)-1]=='\\')
                        fCont=TRUE;
                break;

                case N_BINCL:
                    printf("BINCL  ");
                    printf("%s\n", pStr);
                break;

                case N_SOL:
                    printf("SOL  ");
                    printf("%s\n", pStr);
                break;

                //-----------------------------------------------------------------------
                //                      PARAMETER TO A FUNCTION
                //-----------------------------------------------------------------------
                case N_PSYM:
                    printf("PSYM   ");
                    printf("line: %d PARAM [EBP+%lX]  %s\n", pStab->n_desc, pStab->n_value, pStr);

                    // Add a parameter as it was a local variable to a function
                    pLexCur->token = LEX_PARAM;

                    strcpy(pStrCur, pStr);              // Copy the parameter name + type
                    pLexCur->pName = pStrCur;
                    pStrCur += strlen(pStrCur) + 1;

                    pLexCur->value = pStab->n_value;    // Copy the EBP value

                    pLexCur++;
                    pFun->nLex++;
                break;

                case N_EINCL:
                    printf("EINCL  ");
                    printf("%s\n", pStr);
                break;

                case N_ENTRY:
                    printf("N_ENTRY %08lX\n", pStab->n_value);
                break;

                case N_EXCL:
                    printf("EXCL  ");
                    printf("%s\n", pStr);
                break;

                case N_SCOPE:
                    printf("SCOPE  ");
                    printf("%s\n", pStr);
                break;

                //-----------------------------------------------------------------------
                //                      FUNCTION OPEN LEXICAL SCOPE
                //-----------------------------------------------------------------------
                case N_LBRAC:
                    printf("LBRAC              +%lX  {  (%d)\n", pStab->n_value, pStab->n_desc);

                    pLexCur->token = LEX_LBRAC;

                    // Address of the scope is relative to the function start
                    pLexCur->value = pFun->dwAddress + pStab->n_value;

                    pLexCur++;
                    pFun->nLex++;
                break;

                //-----------------------------------------------------------------------
                //                      FUNCTION CLOSE LEXICAL SCOPE
                //-----------------------------------------------------------------------
                case N_RBRAC:
                    printf("RBRAC              +%lX  }\n", pStab->n_value);

                    pLexCur->token = LEX_RBRAC;

                    // Address of the scope is relative to the function start
                    pLexCur->value = pFun->dwAddress + pStab->n_value;

                    pLexCur++;
                    pFun->nLex++;
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

    //===============================================================
    // We have everything needed to generate output symbol file
    //===============================================================
    strcpy(Symtab.Sig, SYMSIG);

    Symtab.size =   sizeof(TSYMTAB) +
                    (pStrCur - pStrings) +
                    sizeof(TGLOBAL) +
                    sizeof(TSYM_PUB) * Global.nSyms +
                    sizeof(WORD) * Global.nHash;

    strncpy(Symtab.name, nameOut, MAX_MODULE_NAME);
    Symtab.next = NULL;

    //--------------
    //  TSYMTAB    |    sizeof(TSYMTAB)
    //--------------
    //  Strings    |    pStrCur - pStrings
    //--------------
    //  TGLOBAL    |    sizeof(TGLOBAL)
    //--------------
    //  TSYM_PUB[] |    sizeof(TSYM_PUB) * TGLOBAL->nSyms
    //--------------
    //  WORD Hash[]|    sizeof(WORD) * TGLOBAL->nHash
    //--------------

    //--------------
    //  TSYMBOL    |    sizeof(TSYMBOL)
    //--------------
    //  TFUN []    |    sizeof(TFUN) * TSYMBOL->nFun
    //--------------
    //  TLINE [][] |    sizeof(TLINE) * TFUN[]->nLine * TSYMBOL->nFun
    //--------------
    //  TLEX [][]  |    sizeof(TLEX) * TFUN[]->nLine * TSYMBOL->nFun
    //--------------



    //===============================================================
    // Make all pointers relocatable offsets from the start of the file
    //===============================================================

    // Make symbol's name references relative
    for(i=0; i<(int)Global.nSyms; i++ )
    {
        pSymPub[i].pName = (char *) ((pSymPub[i].pName - pStrings) + sizeof(TSYMTAB));
    }

    Symtab.pStrings = (char *) sizeof(TSYMTAB);
    Symtab.pGlobals = (TGLOBAL *) (Symtab.pStrings + (pStrCur - pStrings));
    Symtab.pTypes   = NULL;
    Symtab.pSymbols = NULL;
    Symtab.pSrc     = NULL;

    Global.pSym     = (TSYM_PUB *)((DWORD) Symtab.pGlobals + sizeof(TGLOBAL));
    Global.pHash    = (WORD *)((DWORD) Global.pSym + sizeof(TSYM_PUB) * Global.nSyms);

    written = 0;
    written += write(fd, &Symtab, sizeof(TSYMTAB));
    written += write(fd, pStrings, pStrCur - pStrings);
    written += write(fd, &Global, sizeof(TGLOBAL));
    written += write(fd, pSymPub, sizeof(TSYM_PUB) * Global.nSyms);
    written += write(fd, pHash, sizeof(WORD) * Global.nHash);

    if( written != Symtab.size )
    {
        printf("Error writing symbol file\n");
    }
    else
    {
        printf("Module name: %s\n", nameOut);
        printf("  Translated %d global symbols\n", Global.nSyms);
    }
}


/******************************************************************************
*                                                                             *
*   void TranslateElf                                                         *


*                                                                             *
*******************************************************************************
*
*   Loads an ELF executable or object module and translates it.
*
*   Where:
*       fd - the output linice symbol file to write
*       fi - input ELF file to scan
*       pathSources - the path(s) where to find source files
*       nameOut - the name of the output module
*       nLevel - the level of packaging
*       filesize - size of the input file

#define TRAN_PUBLICS        1
#define TRAN_TYPEINFO       2
#define TRAN_SYMBOLS        3
#define TRAN_SOURCE         4
#define TRAN_PACKAGE        5

*
******************************************************************************/
void TranslateElf(int fd, int fi, char *pathSources, char *nameOut, int nLevel, DWORD filesize)
{
    Elf32_Ehdr *pElf;
    int status;
    BYTE *pBuf;

    // Get the size of the file, allocate memory and load it in
    pBuf = (BYTE*) malloc(filesize);
    if( pBuf )
    {
        lseek(fi, 0, SEEK_SET);
        status = read(fi, pBuf, filesize);
        if( status==(int)filesize )
        {
            pElf = (Elf32_Ehdr*) pBuf;

            // Do some basic ELF file sanity check
            if( pElf->e_ident[0]==ELFMAG0
             && pElf->e_ident[1]==ELFMAG1
             && pElf->e_ident[2]==ELFMAG2
             && pElf->e_ident[3]==ELFMAG3
             && pElf->e_machine==EM_386
             && pElf->e_ehsize==sizeof(Elf32_Ehdr)
             && pElf->e_shentsize==sizeof(Elf32_Shdr)
             && pElf->e_shoff!=0 )
             {
                // We only support relocatable and executable files
                if( pElf->e_type==ET_REL || pElf->e_type==ET_EXEC )
                {
                    // Dump the ELF header sections
#if 1
                    printf("File length: %d\n", filesize);
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
#endif


                    ParseSections(fd, pElf, pBuf, nameOut, filesize);



                }
                else
                    printf("Only relocatable and executable ELF files are supported\n");
            }
            else
                printf("Input file is not a valid ELF file\n");
        }
        else
            printf("Error reading input ELF file\n");

        free(pBuf);
    }
    else
        printf("Error allocating memory");
}
