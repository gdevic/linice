/******************************************************************************
*                                                                             *
*   Module:     ElfToSym.c                                                    *
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

        This module contains the main ELF buffer parsing code

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/05/00   Initial version                                      Goran Devic *
* 03/20/04   Renamed from load_elf to ElfToSym                    Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <fcntl.h>                      // Include file control file

#include "Common.h"                     // Include platform specific set

#include "ice-version.h"                // Include version file

#include "loader.h"                     // Include global protos

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#ifdef WIN32
#define FILE_MODE       _S_IREAD | _S_IWRITE
#else
#define FILE_MODE       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
#endif

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

PSTR dfs;                               // Global pointer to strings (to append)

extern char *GlobalsName2Section(char *pName);
extern BOOL ParseDump(BYTE *pElf);
extern BOOL DumpElfHeader(Elf32_Ehdr *pElf);
extern BOOL StoreGlobalSyms(BYTE *pElf);
extern BOOL StoreSourceFiles(BYTE *pElf);
extern BOOL ParseSource(int fd, int fs);
extern BOOL ParseGlobals(int fd, int fs, BYTE *pElf);
extern BOOL ParseStatic(int fd, int fs, BYTE *pElf);
extern BOOL ParseFunctionLines(int fd, int fs, BYTE *pElf);
extern BOOL ParseFunctionScope(int fd, int fs, BYTE *pElf);
extern BOOL ParseTypedefs(int fd, int fs, BYTE *pElf);
extern BOOL ParseReloc(int fd, int fs, BYTE *pElf);


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
                        VERBOSE3 DumpElfHeader(&Elf);

                        // Get the total length of the file, allocate memory and load it in
                        pBuf = (BYTE*) malloc(fd_stat.st_size);
                        if( pBuf )
                        {
                            lseek(fd, 0, SEEK_SET);
                            status = read(fd, pBuf, fd_stat.st_size);
                            if( status==fd_stat.st_size )
                            {
                                // Dump the ELF structure just for our information
                                VERBOSE3 ParseDump(pBuf);

                                return( pBuf );
                            }
                            else
                                fprintf(stderr, "Error reading file %s\n", sName);

                            free(pBuf);
                        }
                        else
                            fprintf(stderr, "Error allocating memory\n");
                    }
                    else
                        fprintf(stderr, "Invalid ELF file format %s!\n", sName);
                }
                else
                    fprintf(stderr, "Only relocatable and executable ELF files are supported!\n");
            }
            else
                fprintf(stderr, "The file %s is not a valid ELF file\n", sName);
        }
        else
            fprintf(stderr, "Error reading file %s", sName);
    }
    else
        fprintf(stderr, "Unable to open file %s\n", sName);

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void PushString(int fd, char *pStr)                                       *
*                                                                             *
*******************************************************************************
*
*   Utility function that simply pushes a string into a string stream.
*
*   Where:
*       fd is the output string file
*       pStr is the string to append
*
******************************************************************************/
static void PushString(int fd, char *pStr)
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
*   BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName)               *
*                                                                             *
*******************************************************************************
*
*   Given the buffer containing an ELF file with symbolic information, generate
*   the debugger proprietary symbol file.
*
*   Where:
*       pElf - buffer in memory with the complete ELF file to be parsed
*       pSymName - path/name of the output symbol file
*       pTableName - internal name of this symbol table
*
*   Returns:
*       True for OK
*       False for critical error
*
******************************************************************************/
BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName)
{
    Elf32_Ehdr *pElfHeader;             // Pointer to the ELF header
    int fd;                             // Output symbol file descriptor
    int i;
    TSYMTAB SymTab;                     // Symbol table main header structure
    static TSYMHEADER HeaderEnd =       // Terminating header
    { HTYPE__END, sizeof(TSYMHEADER) };

    int fs;                             // Temp file descriptor for strings
    char *pBuf;                         // Temporary buffer

    // Clear the symbol table header structure
    memset(&SymTab, 0, sizeof(TSYMTAB));

    // If the pElf is NULL, exit
    if( pElf )
    {
        // Create and truncate the symbol file name
        VERBOSE1 printf("Creating symbol file: %s\n", pSymName);

        // Delete the file if it already exists. We do that so not to inherit permissions
        unlink(pSymName);

        fd = open(pSymName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, FILE_MODE);
        if( fd>0 )
        {
            // First parse and load all global symbols so we can refer to them later
            // as we need them. Global symbol table contains more information about
            // symbols such are global and static variables and functions.
            if( StoreGlobalSyms(pElf) )
            {
                // Parse ELF structure for all referenced source files.
                // Queue these source file names into an array so anyone can start
                // referring to them using a file_id
                if( StoreSourceFiles(pElf) )
                {
                    // Find the type of the file: kernel, module or an user app.
                    // Kernel and app have the type set to executable.
                    // App has the global symbol "main"
                    pElfHeader = (Elf32_Ehdr *) pElf;

                    if( pElfHeader->e_type==ET_EXEC )
                    {
                        // Look for the symbol "main" with the globals
                        if( GlobalsName2Section("main")==NULL )
                            SymTab.SymTableType = SYMTABLETYPE_KERNEL;
                        else
                            SymTab.SymTableType = SYMTABLETYPE_APP;
                    }
                    else                // ET_REL
                    {
                        // Relocatable EFL file is the kernel loadable module only
                        SymTab.SymTableType = SYMTABLETYPE_MODULE;

                        // For kernel modules, strip the trailing ".o", so the name will
                        // match internal kernel module name
                        if( strlen(pTableName)>2 && !strcmp(pTableName+strlen(pTableName)-2, ".o") )
                            *(char *)(pTableName+strlen(pTableName)-2) = 0;
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
                        // We write { 0, 0 } so we can use it to address source line + bSpaces
                        write(fs, "\0", 2);
                        dfs = (PSTR) 2;         // Start with the offset 2 of the strings

                        PushString(fd, "Symbol information for Linice kernel level debugger");
                        PushString(fd, "Copyright 2000-2005 by Goran Devic");

                        // Parse ELF section for the global symbols and complete globals information
                        // Also write out globals symbol table section
                        if( ParseGlobals(fd, fs, pElf) )
                        {
                            // Parse static symbol definition
                            if( ParseStatic(fd, fs, pElf) )
                            {
                                // Parse all referenced source files and store them into symbol file
                                if( ParseSource(fd, fs) )
                                {
                                    // Function lines will parse ELF file and extract tokens
                                    if( ParseFunctionLines(fd, fs, pElf) )
                                    {
                                        // Function variables and their scopes
                                        if( ParseFunctionScope(fd, fs, pElf) )
                                        {
                                            // Type definitions bound to each major source file
                                            if( ParseTypedefs(fd, fs, pElf) )
                                            {
                                                // Relocation information, written only for object files (kernel modules)
                                                if( ParseReloc(fd, fs, pElf) )
                                                {
                                                    // Add the terminating section HTYPE__END
                                                    write(fd, &HeaderEnd, sizeof(HeaderEnd));

                                                    // Copy all strings to the end of the headers (append)

                                                    // Rewind the strings file and read them all into a buffer
                                                    lseek(fs, 0, SEEK_SET);
                                                    pBuf = (char *) malloc((UINT) dfs);
                                                    if( pBuf!=NULL )
                                                    {
                                                        // Store the offset to the strings (current top of the fd file)
                                                        SymTab.dStrings = lseek(fd, 0, SEEK_CUR);

                                                        read(fs, pBuf, (UINT) dfs);
                                                        write(fd, pBuf, (UINT) dfs);

                                                        // Total size is headers + strings
                                                        SymTab.dwSize = SymTab.dStrings + (UINT) dfs;

                                                        // Write out the symbol header
                                                        lseek(fd, 0, SEEK_SET);
                                                        write(fd, &SymTab, sizeof(TSYMTAB)-sizeof(TSYMHEADER));

                                                        // Close the symbol file
                                                        close(fd);

                                                        free(pBuf);

                                                        return( TRUE );
                                                    }
                                                    else
                                                        fprintf(stderr, "Unable to allocate memory\n");
                                                }
                                                else
                                                    fprintf(stderr, "Error parsing relocation data\n");
                                            }
                                            else
                                                fprintf(stderr, "Error parsing typedefs\n");
                                        }
                                        else
                                            fprintf(stderr, "Error parsing function scope\n");
                                    }
                                    else
                                        fprintf(stderr, "Error parsing function lines\n");
                                }
                                else
                                    fprintf(stderr, "Error writing source files\n");
                            }
                            else
                                fprintf(stderr, "Error writing statics section\n");
                        }
                        else
                            fprintf(stderr, "Error writing globals section\n");
                    }
                    else
                        fprintf(stderr, "Error writing symbol file %s\n", pSymName);
                }
                else
                    fprintf(stderr, "Error parsing source file names\n");
            }
            else
                fprintf(stderr, "Error parsing global symbols\n");
        }
        else
            fprintf(stderr, "Unable to create symbol file %s\n", pSymName);
    }

    return( FALSE );
}

