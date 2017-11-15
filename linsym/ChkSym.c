/******************************************************************************
*                                                                             *
*   Module:     ChkSym.c                                                      *
*                                                                             *
*   Date:       07/22/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
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

        This module contains the symbol checker code.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 07/22/02   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <errno.h>                      // Include errno
#include <stdlib.h>                     // Include standard library
#include <string.h>                     // Include strings header file
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#ifndef WIN32
#include <unistd.h>                     // Include standard UNIX header file
#include <sys/ioctl.h>                  // Include ioctl header file
#include <sys/types.h>                  // Include file operations
#include <sys/ioctl.h>                  // Include ioctl header file
#define O_BINARY    0
#define stricmp     strcasecmp          // Weird gnu c call..
#else // WIN32
#include <io.h>
//#include <malloc.h>
#endif // WIN32

#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos
#include "ice-symbols.h"                // Include symbol file structures
#include "stabs.h"                      // Include STABS defines and structures

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char *basic[] = {
    "??",
    "INT",
    "CHAR",
    "LONG INT",
    "UNSIGNED INT",
    "LONG UNSIGNED INT",
    "LONG LONG INT",
    "LONG LONG UNSIGNED INT",
    "SHORT INT",
    "SHORT UNSIGNED INT",
    "SIGNED CHAR",
    "UNSIGNED CHAR",
    "FLOAT",
    "DOUBLE",
    "LONG DOUBLE",
    "COMPLEX INT",
    "COMPLEX FLOAT",
    "COMPLEX DOUBLE",
    "COMPLEX LONG DOUBLE",
    "VOID",
};

static char *SymTableTypes[] = {
    "Undefined",
    "Kernel",
    "Module",
    "Application",
};

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static BOOL ChkIgnore(TSYMHEADER *pHead, DWORD pStr)
{
    printf("HTYPE_IGNORE\n");
    printf("  Skipping this section...\n");

    return( TRUE );
}


static BOOL ChkFunctionLines(TSYMHEADER *pHead, DWORD pStr)
{
    WORD nLine;
    TSYMFNLIN *pFuncLin;

    pFuncLin = (TSYMFNLIN *) pHead;

    printf("HTYPE_FUNCTION_LINES\n");
    printf("  dwStartAddress = %08X\n", pFuncLin->dwStartAddress);
    printf("  dwEndAddress   = %08X\n", pFuncLin->dwEndAddress);

    for( nLine=0; nLine<pFuncLin->nLines; nLine++ )
    {
        printf("    Line: %3d  : %3X  file_id: %d\n",
            pFuncLin->list[nLine].line, pFuncLin->list[nLine].offset, pFuncLin->list[nLine].file_id);
    }

    return(TRUE);
}


static BOOL ChkFunctionScope(TSYMHEADER *pHead, DWORD pStr)
{
    WORD nTokens;
    TSYMFNSCOPE *pFuncScope;

    pFuncScope = (TSYMFNSCOPE *) pHead;

    printf("HTYPE_FUNCTION_SCOPE\n");
    printf("  pName = %s\n", pStr + pFuncScope->pName);
    printf("  file_id = %d\n", pFuncScope->file_id);
    printf("  dwStartAddress = %08X\n", pFuncScope->dwStartAddress);
    printf("  dwEndAddress   = %08X\n", pFuncScope->dwEndAddress);
    printf("  nTokens = %d d\n", pFuncScope->nTokens);

    for( nTokens=0; nTokens<pFuncScope->nTokens; nTokens++ )
    {
        printf("    Token: %3d : %08X %08X  ", nTokens, pFuncScope->list[nTokens].param, (DWORD)pFuncScope->list[nTokens].pName);
        switch(  pFuncScope->list[nTokens].TokType )
        {
            case TOKTYPE_PARAM:     printf("TOKTYPE_PARAM %s\n", pStr + pFuncScope->list[nTokens].pName);       break;
            case TOKTYPE_RSYM:      printf("TOKTYPE_RSYM  %s\n", pStr + pFuncScope->list[nTokens].pName);       break;
            case TOKTYPE_LSYM:      printf("TOKTYPE_LSYM  %s\n", pStr + pFuncScope->list[nTokens].pName);       break;
            case TOKTYPE_LCSYM:     printf("TOKTYPE_LCSYM %s seg:%d\n", pStr + pFuncScope->list[nTokens].pName, pFuncScope->list[nTokens].bSegment);  break;
            case TOKTYPE_LBRAC:     printf("TOKTYPE_LBRAC {\n");       break;
            case TOKTYPE_RBRAC:     printf("TOKTYPE_RBRAC }\n");       break;
            default:
                printf("Unknown token type of %X!\n", pFuncScope->list[nTokens].TokType);
                return(FALSE);
        }
    }

    return(TRUE);
}


static BOOL ChkStatic(TSYMHEADER *pHead, DWORD pStr)
{
    DWORD nStat;
    TSYMSTATIC *pStatic;

    pStatic = (TSYMSTATIC *) pHead;

    printf("HTYPE_STATIC\n");

    printf("  file_id = %d\n", pStatic->file_id);
    printf("  nStatic = %d d\n", pStatic->nStatics);

    for( nStat=0; nStat<pStatic->nStatics; nStat++)
    {
        printf("    %3d: seg:%d  %08X %s = %s\n", nStat,
            pStatic->list[nStat].bSegment,
            pStatic->list[nStat].dwAddress,
            pStr + pStatic->list[nStat].pName,
            pStatic->list[nStat].pDef ? pStr + pStatic->list[nStat].pDef : "NULL");
    }

    return(TRUE);
}


static BOOL ChkGlobals(TSYMHEADER *pHead, DWORD pStr)
{
    DWORD nGlob;
    TSYMGLOBAL *pGlob;

    pGlob = (TSYMGLOBAL *) pHead;

    printf("HTYPE_GLOBALS\n");
    printf("  nGlobals = %d d\n", pGlob->nGlobals);

    for( nGlob=0; nGlob<pGlob->nGlobals; nGlob++)
    {
        printf("    %3d: %08X %08X seg:%02X file_id:%d %s = %s\n", nGlob,
            pGlob->list[nGlob].dwStartAddress,
            pGlob->list[nGlob].dwEndAddress,
            pGlob->list[nGlob].bSegment,
            pGlob->list[nGlob].file_id,
            pStr + pGlob->list[nGlob].pName,
            pGlob->list[nGlob].pDef ? pStr + pGlob->list[nGlob].pDef : "NULL");
    }

    return( TRUE );
}


static BOOL ChkSource(TSYMHEADER *pHead, DWORD pStr)
{
    DWORD nLine;
    TSYMSOURCE *pSrc;
    BYTE bSpaces;

    pSrc = (TSYMSOURCE *) pHead;

    printf("HTYPE_SOURCE\n");
    printf("  file_id     = %d\n", pSrc->file_id);
    printf("  pSourcePath = %s\n", pStr + pSrc->pSourcePath);
    printf("  pSourceName = %s\n", pStr + pSrc->pSourceName);
    printf("  nLines      = %d\n", pSrc->nLines);

    // Dump only first 3 lines of the source - no need to dump the complete file
    for( nLine=0; nLine<pSrc->nLines && nLine<3; nLine++ )
    {
        printf("    %3d: ", nLine + 1);
        bSpaces = *(BYTE *)(pStr + pSrc->pLineArray[nLine]);
        while(bSpaces!=0)
        {
            printf(" ");
            bSpaces--;
        }

        printf("%s\n", pStr + pSrc->pLineArray[nLine]+1);
    }

    return( TRUE );
}

static BOOL ChkTypedefs(TSYMHEADER *pHead, DWORD pStr)
{
    WORD nTypedefs, nRel;
    TSYMTYPEDEF *pType;
    TSYMADJUST *pAdjust;

    pType = (TSYMTYPEDEF *) pHead;

    printf("HTYPE_TYPEDEF\n");
    printf("  file_id   = %d\n", pType->file_id);
    printf("  nTypedefs = %d\n", pType->nTypedefs);
    printf("  nRel      = %d\n", pType->nRel);
    printf("  pRel      = %X\n", (UINT) pType->pRel);

    if( pType->nRel==0 || pType->pRel==0 )
    {
        printf("WARNING: nRel or pRel is zero!\n");
    }

    for(nRel=0; nRel<pType->nRel; nRel++)
    {
        pAdjust = (TSYMADJUST *)(pStr + (BYTE *)(&pType->pRel[nRel]));

        printf("    %2d file_id: %2d  adjust: %d\n", nRel, pAdjust->file_id, pAdjust->adjust);
    }

    printf("  --- typedefs ---\n");

    for( nTypedefs=0; nTypedefs<pType->nTypedefs; nTypedefs++ )
    {
        if( *(pStr + pType->list[nTypedefs].pDef) <= TYPEDEF__LAST )
        {
            printf("    %03d (%d,%d) id=%d %s = %s\n",
                nTypedefs,
                pType->list[nTypedefs].maj,
                pType->list[nTypedefs].min,
                pType->list[nTypedefs].file_id,
                basic[(int)*(pStr + pType->list[nTypedefs].pDef)],
                pStr + pType->list[nTypedefs].pName );
        }
        else
        {
            printf("    %03d  TYPEDEF(%d,%d) id=%d %s = %s\n",
                nTypedefs,
                pType->list[nTypedefs].maj,
                pType->list[nTypedefs].min,
                pType->list[nTypedefs].file_id,
                pStr + pType->list[nTypedefs].pName,
                pStr + pType->list[nTypedefs].pDef );
        }
    }

    return( TRUE );
}

static BOOL ChkReloc(TSYMHEADER *pHead, DWORD pStr)
{
    TSYMRELOC *pReloc;
    WORD nReloc;

    pReloc = (TSYMRELOC *) pHead;

    printf("HTYPE_RELOC\n");
    printf("  nReloc = %d\n", pReloc->nReloc);

    for( nReloc=0; nReloc<pReloc->nReloc; nReloc++)
    {
        printf("  %02d refFixup  = %08X \n", nReloc, pReloc->list[nReloc].refFixup);
        printf("     refOffset = %08X \n",           pReloc->list[nReloc].refOffset);
    }

    return( TRUE );
}

static BOOL CheckSymStructure(char *pBuf, DWORD nLen)
{
    TSYMTAB *pSym;                      // Symbol file header
    TSYMHEADER *pHead;                  // Generic section header
    DWORD pStr;                         // Strings section
    int nSection = 0;                   // Counting section number
    BOOL fTest = FALSE;                 // Return value from the test
    DWORD Size;                         // Total size parsed so far

    // Assign pointers and check the main header
    pSym = (TSYMTAB *) pBuf;

    printf("Signature     = %s\n", pSym->sSig);
	printf("Table Name    = %s\n", pSym->sTableName);
	printf("Version       = %d.%d\n", pSym->Version>>8, pSym->Version&0xFF);
    printf("Table Type    = %d %s\n", pSym->SymTableType, SymTableTypes[pSym->SymTableType]);
    printf("dwSize        = %d d\n", pSym->dwSize);
    printf("dStrings      = %04X\n", pSym->dStrings);

    if( pSym->SymTableType>SYMTABLETYPE_APP )
    {
        printf("ERROR: Invalid symbol table type\n");

        return( FALSE );
    }

    if( pSym->SymTableType==SYMTABLETYPE_UNDEF )
        printf("WARNING: Undefined symbol table type!\n");

    // Calculate the initial minimal size
    Size = sizeof(TSYMTAB) - sizeof(TSYMHEADER);

    if( pSym->dwSize==nLen )
    {
        pStr = pSym->dStrings + (DWORD) pBuf;
        pHead = pSym->header;

        while( Size < pSym->dStrings )
        {
            printf("Section=%d, offset=%d/%d, dwSize=%d d\n", nSection, Size, pSym->dStrings, pHead->dwSize);

            switch( pHead->hType )
            {
                case HTYPE_GLOBALS:
                    fTest = ChkGlobals(pHead, pStr);
                    break;

                case HTYPE_SOURCE:
                    fTest = ChkSource(pHead, pStr);
                    break;

                case HTYPE_FUNCTION_LINES:
                    fTest = ChkFunctionLines(pHead, pStr);
                    break;

                case HTYPE_FUNCTION_SCOPE:
                    fTest = ChkFunctionScope(pHead, pStr);
                    break;

                case HTYPE_STATIC:
                    fTest = ChkStatic(pHead, pStr);
                    break;

                case HTYPE_TYPEDEF:
                    fTest = ChkTypedefs(pHead, pStr);
                    break;

                case HTYPE_IGNORE:
                    fTest = ChkIgnore(pHead, pStr);
                    break;

                case HTYPE_RELOC:
                    fTest = ChkReloc(pHead, pStr);
                    break;

                case HTYPE__END:
                    printf("HTYPE__END\n");
                    printf("  dwSize=%d d\n", pHead->dwSize);
                    fTest = TRUE;       // This is a valid section
                    break;

                default:
                    printf("ERROR: Invalid section header\n");
                    break;
            }

            if(!fTest)
                return(FALSE);

            // Advance to the next header
            nSection++;
            Size += pHead->dwSize;

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
            if( (DWORD)pHead<(DWORD)pSym || (DWORD)pHead>(DWORD)pStr )
            {
                printf("ERROR: Header traverses into the weeds..\n");

                return( FALSE );
            }
        }

        printf("Check complete.\n");

        return( TRUE );
    }
    else
        printf("ERROR: dwSize (%d) does not match file len (%d)\n", pSym->dwSize, nLen);

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   int OptCheck(char *pName)                                                 *
*                                                                             *
*******************************************************************************
*
*   Loads symbol map file and checks it for consistency, dumps it.
*
*   Where:
*       pName is the name of the symbol file to check
*
*   Return:
*       TRUE - Symbol file appears correct
*       FALSE - Symbol file is invalid
*
******************************************************************************/
int OptCheck(char *pName)
{
    struct stat fd_stat;
    int fd;
    int nLen;
    int status;
    char *pBuf;

    printf("Checking: %s\n", pName);

    fd = open(pName, O_RDONLY | O_BINARY);
    if( fd>0 )
    {
        fstat(fd, &fd_stat);

        nLen = fd_stat.st_size;
        printf("File length: %d\n", nLen);

        pBuf = (char*) malloc(nLen);
        if( pBuf!=NULL )
        {
            // Load a complete symbol file into a buffer
            status = read(fd, pBuf, nLen);
            if( status==nLen )
            {
                return( CheckSymStructure(pBuf, nLen) );
            }
            else
                printf("Error reading %d/%d bytes\n", status, nLen);
        }
        else
            printf("Error allocating memory\n");
    }
    else
        printf("Error opening %s\n", pName);

	return 0;
}

