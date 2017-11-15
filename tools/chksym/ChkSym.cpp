// ChkSym.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef WIN32

#include <assert.h>
#define ASSERT assert
#include <io.h>
#include <malloc.h>

//#include <unistd.h>                     // Include standard UNIX header file
#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
//#include <sys/ioctl.h>                  // Include ioctl header file
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#include "linux/elf.h"
#include "stab_gnu.h"

#else // WIN32

#endif // WIN32

#include "ice-symbols.h"                // Include symbol file structures
#include "stabs.h"                      // Include STABS defines and structures


BOOL ChkIgnore(TSYMHEADER *pHead, char *pStr)
{
    printf("HTYPE_IGNORE\n");
    printf("  Skipping this section...\n");

    return( TRUE );
}


BOOL ChkFunctionLines(TSYMHEADER *pHead, char *pStr)
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


BOOL ChkFunctionScope(TSYMHEADER *pHead, char *pStr)
{
    WORD nTokens;
    TSYMFNSCOPE *pFuncScope;

    pFuncScope = (TSYMFNSCOPE *) pHead;

    printf("HTYPE_FUNCTION_SCOPE\n");
    printf("  dName = %s\n", pStr + pFuncScope->dName);
    printf("  file_id = %d\n", pFuncScope->file_id);
    printf("  dwStartAddress = %08X\n", pFuncScope->dwStartAddress);
    printf("  dwEndAddress   = %08X\n", pFuncScope->dwEndAddress);
    printf("  nTokens = %d d\n", pFuncScope->nTokens);

    for( nTokens=0; nTokens<pFuncScope->nTokens; nTokens++ )
    {
        printf("    Token: %3d : %08X %08X  ", nTokens, pFuncScope->list[nTokens].p1, pFuncScope->list[nTokens].p2);
        switch(  pFuncScope->list[nTokens].TokType )
        {
            case TOKTYPE_PARAM:     printf("TOKTYPE_PARAM %s\n", pStr + pFuncScope->list[nTokens].p2);       break;
            case TOKTYPE_RSYM:      printf("TOKTYPE_RSYM  %s\n", pStr + pFuncScope->list[nTokens].p2);       break;
            case TOKTYPE_LSYM:      printf("TOKTYPE_LSYM  %s\n", pStr + pFuncScope->list[nTokens].p2);       break;
            case TOKTYPE_LCSYM:     printf("TOKTYPE_LCSYM %s\n", pStr + pFuncScope->list[nTokens].p2);       break;
            case TOKTYPE_LBRAC:     printf("TOKTYPE_LBRAC {\n");       break;
            case TOKTYPE_RBRAC:     printf("TOKTYPE_RBRAC }\n");       break;
        }
    }

    return(TRUE);
}


BOOL ChkStatic(TSYMHEADER *pHead, char *pStr)
{
    TSYMSTATIC *pStatic;

    pStatic = (TSYMSTATIC *) pHead;

    printf("HTYPE_STATIC *** TODO ***\n");

    return(TRUE);
}


BOOL ChkGlobals(TSYMHEADER *pHead, char *pStr)
{
    DWORD nGlob;
    TSYMGLOBAL *pGlob;

    pGlob = (TSYMGLOBAL *) pHead;

    printf("HTYPE_GLOBALS\n");
    printf("  nGlobals = %d d\n", pGlob->nGlobals);

    for( nGlob=0; nGlob<pGlob->nGlobals; nGlob++)
    {
        printf("    %3d: %08X %08X F:%02X %s\n", nGlob,
            pGlob->global[nGlob].dwStartAddress,
            pGlob->global[nGlob].dwEndAddress,
            pGlob->global[nGlob].bFlags,
            pStr + pGlob->global[nGlob].dName);
    }

    return( TRUE );
}


BOOL ChkSource(TSYMHEADER *pHead, char *pStr)
{
    DWORD nLine;
    TSYMSOURCE *pSrc;

    pSrc = (TSYMSOURCE *) pHead;

    printf("HTYPE_SOURCE\n");
    printf("  file_id     = %d\n", pSrc->file_id);
    printf("  dSourcePath = %s\n", pStr + pSrc->dSourcePath);
    printf("  dSourceName = %s\n", pStr + pSrc->dSourceName);
    printf("  nLines      = %d\n", pSrc->nLines);

    for( nLine=0; nLine<pSrc->nLines; nLine++ )
    {
        printf("    %3d: %s\n", nLine + 1, pStr + pSrc->dLineArray[nLine]);
    }

    return( TRUE );
}

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

BOOL ChkTypedefs(TSYMHEADER *pHead, char *pStr)
{
    WORD nTypedefs;
    TSYMTYPEDEF *pType;

    pType = (TSYMTYPEDEF *) pHead;

    printf("HTYPE_TYPEDEF\n");
    printf("  file_id   = %d\n", pType->file_id);
    printf("  nTypedefs = %d\n", pType->nTypedefs);

    for( nTypedefs=0; nTypedefs<pType->nTypedefs; nTypedefs++ )
    {
        if( pType->list[nTypedefs].dDef <= TYPEDEF__LAST )
        {
            printf("    %03d  TYPEDEF(%2d,%2d) BASIC %-25s  %s\n",
                nTypedefs,
                pType->list[nTypedefs].maj,
                pType->list[nTypedefs].min,
                basic[pType->list[nTypedefs].dDef],
                pStr + pType->list[nTypedefs].dName );
        }
        else
        {
            printf("    %03d  TYPEDEF(%2d,%2d) %s = %s\n",
                nTypedefs,
                pType->list[nTypedefs].maj,
                pType->list[nTypedefs].min,
                pStr + pType->list[nTypedefs].dName,
                pStr + pType->list[nTypedefs].dDef );
        }
    }

    return( TRUE );
}

BOOL ChkReloc(TSYMHEADER *pHead, char *pStr)
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

BOOL CheckSymStructure(char *pBuf, DWORD nLen)
{
    TSYMTAB *pSym;                      // Symbol file header
    TSYMHEADER *pHead;                  // Generic section header
    char *pStr;
    int nSection = 0;

    // Assign pointers and check the main header
    pSym = (TSYMTAB *) pBuf;

    printf("Signature     = %s\n", pSym->sSig);
	printf("Table Name    = %s\n", pSym->sTableName);
	printf("Version       = %d.%d\n", pSym->Version>>8, pSym->Version&0xFF);
    printf("dwSize        = %d d\n", pSym->dwSize);
    printf("dStrings      = %04X\n", pSym->dStrings);

    if( pSym->dwSize==nLen )
    {
        pStr = pSym->dStrings + pBuf;
        pHead = pSym->header;

        while( pHead->hType != HTYPE__END )
        {
            printf("Section %d: dwSize: %d d\n", nSection, pHead->dwSize);
            switch( pHead->hType )
            {
                case HTYPE_GLOBALS:
                    ChkGlobals(pHead, pStr);
                    break;

                case HTYPE_SOURCE:
                    ChkSource(pHead, pStr);
                    break;

                case HTYPE_FUNCTION_LINES:
                    ChkFunctionLines(pHead, pStr);
                    break;

                case HTYPE_FUNCTION_SCOPE:
                    ChkFunctionScope(pHead, pStr);
                    break;

                case HTYPE_STATIC:
                    ChkStatic(pHead, pStr);
                    break;

                case HTYPE_TYPEDEF:
                    ChkTypedefs(pHead, pStr);
                    break;

                case HTYPE_IGNORE:
                    ChkIgnore(pHead, pStr);
                    break;

                case HTYPE_RELOC:
                    ChkReloc(pHead, pStr);
                    break;

                case HTYPE__END:
                    printf("HTYPE__END\n");
                    printf("  dwSize=%d d\n", pHead->dwSize);
                    break;

                default:
                    printf("ERROR: Invalid section header\n");
                    break;
            }

            // Advance to the next header
            nSection++;
            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
            if( (DWORD)pHead<(DWORD)pSym || (DWORD)pHead>(DWORD)pStr )
            {
                printf("ERROR: Header traverses into the weeds..\n");
                break;
            }
        }
    }
    else
        printf("ERROR: dwSize (%d) does not match file len (%d)\n", pSym->dwSize, nLen);

    return( FALSE );
}


int main(int argc, char* argv[])
{
    int fd;
    int nLen;
    int status;
    char *pBuf;

    printf("Symbol file to check: %s\n", argv[1]);

    fd = open(argv[1], O_RDONLY | O_BINARY);
    if( fd>0 )
    {
        nLen = filelength(fd);
        printf("File length: %d\n", nLen);

        pBuf = (char*) malloc(nLen);
        if( pBuf!=NULL )
        {
            // Load a complete symbol file into a buffer
            status = read(fd, pBuf, nLen);
            if( status==nLen )
            {
                CheckSymStructure(pBuf, nLen);
            }
            else
                printf("Error reading %d/%d bytes\n", status, nLen);
        }
        else
            printf("Error allocating memory\n");
    }
    else
        printf("Error opening symbol file %s\n", argv[1]);

	return 0;
}

