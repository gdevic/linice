/******************************************************************************
*                                                                             *
*   Module:     Translate.cpp                                                 *
*                                                                             *
*   Date:       07/22/02                                                      *
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

        This module contains the Windows executable version of the linsym.
        Only the translation of symbols is supported.

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
#include "stdafx.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>

#include "ice-version.h"                // Include version file
#include "ice-symbols.h"                // Include symbol file structures
#include "loader.h"                     // Include global protos

extern BYTE *LoadElf(char *sName);
extern BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName);
extern int ChkSym(char *pName);

unsigned int opt = OPT_VERBOSE;         // Default options
int nVerbose = 1;                       // Default verbose level


void PrintHelp()
{
	printf("Usage:  Translate [-v {0,1,2,3}] | [[-t file] | [-c file.sym]]\n");
    printf("  -t <file>            - Translate file into a symbol file\n");
    printf("  -c <file.sym>        - Check the symbol file for consistency\n");
    printf("  -v {0,1,2,3}         - Verbose level (0=silent)\n");
    printf("\n");
}

/******************************************************************************
*                                                                             *
*   int main(int argc, char* argv[])                                          *
*                                                                             *
*******************************************************************************
*
*   Main entry
*
******************************************************************************/
int main(int argc, char* argv[])
{
    char *sElf, sSym[128], sTable[32], *pName;
    BYTE *pBuf;
    int i;
    time_t lapse;

    // Print the basic banner
    printf("\nLinice Debugger Win32 Symbol Translator/Checker Version %d.%02d\n", LINSYMVER >> 8, LINSYMVER & 0xFF);
    printf("(C) Goran Devic, 2000-2003\n\n");

    // Parse the arguments and pick up the options
    nVerbose = 1;                       // Default verbose level of 1

    // If there were no arguments, just print help
    // Otherwise, we need to have even number of arguments
    if(argc==1 || (~argc&1) )
        PrintHelp();
    else
    for(i=1; i<argc; i+=2)
    {
        if(!stricmp(argv[i], "-t"))
        {
            sElf = argv[i+1];

            // Translate a file into a symbol file

		    // Form the output symbol file name
		    strcpy(sSym, sElf);
		    strcat(sSym, ".sym");

		    // Form the (internal) symbol table name (skip over the possible path)
		    pName = strrchr(sElf, '/');
		    if(pName==NULL)
			    pName = strrchr(sElf, '\\');
		    if(pName!=NULL)
			    pName++;
		    else
			    pName = sElf;

		    strcpy(sTable, pName);

		    // Strip the optional trailing '.o'
		    if(strlen(sTable)>2)
            {
			    if(sTable[strlen(sTable)-1]=='o' && sTable[strlen(sTable)-2]=='.')
			    {
				    sTable[strlen(sTable)-2] = 0;
			    }
            }

            time(&lapse);

		    pBuf = LoadElf(sElf);
		    ElfToSym(pBuf, sSym, sTable);

            lapse = time(NULL) - lapse;

            if(nVerbose>=1)
                printf("Complete in %d sec\n", lapse);

            break;
        }
        else
        if(!stricmp(argv[i], "-c"))
        {
            sElf = argv[i+1];

            ChkSym(sElf);

            break;
        }
        else
        if(!stricmp(argv[i], "-v"))
        {
            switch(*argv[i+1])
            {
            case '0': opt &= ~OPT_VERBOSE; break;
            case '1': nVerbose = 1; break;
            case '2': nVerbose = 2; break;
            case '3': nVerbose = 3; break;
            }
        }
        else
        {
            PrintHelp();
            break;
        }
    }

	return 0;
}

