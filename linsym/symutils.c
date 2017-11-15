/******************************************************************************
*                                                                             *
*   Module:     symutils.c                                                    *
*                                                                             *
*   Date:       04/19/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This file contains symbol utility functions.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/19/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <malloc.h>
#include <errno.h>

#include "ice-ioctl.h"                  // Include shared header file


/******************************************************************************
*                                                                             *
*   int MakeSymbolTable(PTSYMTAB pTable, BYTE *pBuf, char *pTableName)        *
*                                                                             *
*******************************************************************************
*
*   Where:
*
*   Returns:
*       size of the symbol table structure
*
******************************************************************************/
int MakeSymbolTable(PTSYMTAB pTable, FILE *fin, char *pTableName)
{
    int items, count;
    PTSYM pSym;
    DWORD dwAddress = 0;

    // Initialize the symtab header

    memset((void*) pTable, 0, sizeof(TSYMTAB));
    pTable->magic = MAGIC_SYMTAB;
    strncpy(pTable->name, pTableName, MAX_MODULE_NAME);

    // Assume sorted, we will reset if we find the addresses out of order
    pTable->Flags = SYMF_SORTED;

    pSym = &pTable->sym[0];
    count = 0;

    while( !feof(fin) )
    {
        items = fscanf(fin, "%x %c %s\n", &pSym->dwAddress, (char *)&pSym->Type, pSym->name);
        pSym->name[MAX_SYMBOL_NAME-1] = 0;

        // If the new address is lower than the old one, reset the sequential bit
        if(pSym->dwAddress<dwAddress)
            pTable->Flags &= ~SYMF_SORTED;
        else
            dwAddress = pSym->dwAddress;

        pSym++;
        count++;
    }

    pTable->nElem = count;
    pTable->size = count * sizeof(TSYM) + sizeof(TSYMTAB) - sizeof(TSYM);

    return( pTable->size );
}


void DumpSymbolTable(PTSYMTAB pTable)
{
    int i;

    printf("-Header--------------------------------------\n");
    printf("Magic number: %08X\n", pTable->magic);
    printf("Module name:  %s\n", pTable->name);
    printf("Size:         %x (%d)\n", pTable->size, pTable->size);
    printf("nElem:       (%d)\n", pTable->nElem);
    printf("Flags:        %X\n", pTable->Flags);
    printf("-Symbols-------------------------------------\n");
    for(i=0; i < (int)pTable->nElem; i++)
    {
        printf(" %03d %08X %c -> %s\n", i,
            pTable->sym[i].dwAddress,
            pTable->sym[i].Type & 0xFF,
            pTable->sym[i].name);
    }
}

/******************************************************************************
*                                                                             *
*   void TranslateSymbolTable(char *sName, char *sOut, char *sModuleName)     *
*                                                                             *
*******************************************************************************
*
*   Translates common symbol table file into a private SYMTAB structure which
*   is written to a file.
*
*   Where:
*       sName is the input symbol file name
*       sOut is the output SYMTAB file name to be created
*       sModuleName is the module name to be set for that symbols
*
******************************************************************************/
void TranslateSymbolTable(char *sName, char *sOut, char *sModuleName)
{
    FILE *fin;
    int fd;
    struct stat prop;
    int status;
    PTSYMTAB pTable;
    DWORD size;

    // Find the size of the input file
    status = stat(sName, &prop);
    if(status==0)
    {
        // Try to open a given symbol table file
        fin = fopen(sName, "r");
        if(fin!=NULL)
        {
            // Allocate memory for the symbol table structure
            // Pretty much guess how large the table is going to be
            size = prop.st_size * 2 + sizeof(TSYMTAB) + 1024;
            pTable = (PTSYMTAB) malloc(size);
            if(pTable!=NULL)
            {
                // Create the symbol structure out of symbols
                size = MakeSymbolTable(pTable, fin, sModuleName);

                DumpSymbolTable(pTable);

                // Write the binary symtab file
                fd = open(sOut, O_WRONLY | O_TRUNC | O_CREAT );
                if( fd>0 )
                {
                    status = write(fd, (void *) pTable, size);
                    if(status!=size)
                        printf("Error writing the symbol file %s\n", sOut);

                    close(fd);
                }
                else
                {
                    printf("Unable to create output file: %s\n", sOut);
                }

                free(pTable);
            }
            else
            {
                printf("Unable to allocate %d bytes\n", size);
            }

            fclose(fin);
        }
        else
        {
            printf("Could not open symbol table file: %s\n", sName);
        }
    }
    else
    {
        switch(status)
        {
        case EACCES:
            printf("Error accessing symbol file %s\n", sName);
            break;
        case EBADF:
            printf("Bad file %s\n", sName);
            break;
        case ENOENT:
            printf("Symbol file %s unreachable\n", sName);
            break;
        default:
            printf("Unable to stat file %s\n", sName);
        }
    }
}


/******************************************************************************
*                                                                             *
*   void OptTranslateSymbolTable(char *sName, char *sOut)                     *
*                                                                             *
*******************************************************************************
*
*   Translates common symbol table file into a private SYMTAB structure which
*   is written to a file.
*
*   Where:
*       sName is the input symbol file name
*       sOut is the output SYMTAB file name to be created
*
******************************************************************************/
void OptTranslateSymbolTable(char *sName, char *sOut)
{
    TranslateSymbolTable(sName, sOut, "unknown");
}

