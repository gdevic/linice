// Elf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string.h>

#define _CPP

#include "ice-symbols.h"                // Include symbol file structures
#include "loader.h"                     // Include global protos

extern "C" {

    BYTE *LoadElf(char *sName);
    BOOL ElfToSym(BYTE *pElf, char *pSymName, char *pTableName);
    unsigned int opt;
}


int main(int argc, char* argv[])
{
    char *sElf, sSym[128], sTable[32], *pName;
    BYTE *pBuf;

    if(argc!=1)
	{
		// Get the object module/app name
		sElf = argv[1];

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
			if(sTable[strlen(sTable)-1]=='o' && sTable[strlen(sTable)-2]=='.')
			{
				sTable[strlen(sTable)-2] = 0;
			}

		pBuf = LoadElf(sElf);
		ElfToSym(pBuf, sSym, sTable);

        fprintf(stderr, "Finished.\n");

		fgetc(stdin);
	}
	else
	{
		printf("Usage: %s <object>\n", argv[0]);
	}

	return 0;
}

