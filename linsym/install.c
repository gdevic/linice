/******************************************************************************
*                                                                             *
*   Module:     install.c                                                     *
*                                                                             *
*   Date:       03/05/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains code to load a module and feed it the
        init parameters as read from the config file

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/05/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <unistd.h>                     // Include standard UNIX header file
#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
#include <sys/ioctl.h>                  // Include ioctl header file
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#include "ioctl.h"                      // Include shared header file

#define stricmp     strcasecmp          // Weird gnu c call..

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


/******************************************************************************
*
*   Identifies a string and copies it into the destination address of up to
*   MAX_STRING len.
*
******************************************************************************/
void GetString(char *p, char *pStr)
{
    int n = 0;

    if(*pStr=='"') pStr++;
    while((*pStr!='\n') && ((*pStr!='\r')) && (*pStr!='"') && (n++<MAX_STRING))
        *p++ = *pStr++;
    *p = 0;
}


/******************************************************************************
*                                                                             *
*   void OpInstall()                                                          *
*                                                                             *
*******************************************************************************
*
*   Loads linice.o module.  Feeds it the init file so it can configure itself.
*
******************************************************************************/
void OptInstall()
{
    int hIce;
    FILE *fp;
    int status;
    char sLine[256], *pStr;
    char sKey[16], sValue[256];
    TINITPACKET Init;
    int nLine;

    // Load the linice.o device driver module
    status = system2("insmod linice.o");

    // insmod must return 0 to load module correctly
    if( status==0 )
    {
        printf("Linice module loaded.\n");

        // Look for the config file and feed it to the module
        fp = fopen("linice.dat", "r");
        if( fp==NULL )
        {
            // Look in the /etc directory
            fp = fopen("/etc/linice.dat", "r");
            if( fp==NULL )
            {
                // Failed to open the config file
                printf("Failed to open linice.dat file!\n");
                return;
            }
        }

        // Zero out the init packet
        memset(&Init, 0, sizeof(TINITPACKET));
        nLine = 0;

        // Read config file line by line and build the config packet
        while( !feof(fp) )
        {
            nLine++;
            fgets(sLine, sizeof(sLine), fp);
            // Get to the first meaningful character in a line
            for(pStr=sLine; (*pStr!='\n') && (*pStr!='\r') && (*pStr==' '); pStr++);
            // Skip comment line and empty lines
            if((*pStr==';')||(*pStr=='\n')||(*pStr=='\r'))
                continue;
            // Get the assignment in the form KEY=VALUE or KEY="value"
            sscanf(pStr, "%[a-zA-Z0-9]s", &sKey);
            pStr = strchr(pStr,'=') + 1;

            // Decipher the key and load the appropriate value (number, boolean or string)
            if(stricmp(sKey, "lowercase")==0)
            {
                sscanf(pStr, "%s", &sValue);
                if(stricmp(sValue,"on")==0)
                    Init.fLowercase = 1;
            } else
            if(stricmp(sKey, "sym")==0)
                sscanf(pStr, "%d", &Init.nSymbolSize);
            else
            if(stricmp(sKey, "hst")==0)
                sscanf(pStr, "%d", &Init.nHistorySize);
            else
            if(stricmp(sKey, "init")==0)
                 GetString(Init.sInit, pStr); else

            if(stricmp(sKey, "f1" )==0) GetString(Init.keyFn[ 0], pStr); else
            if(stricmp(sKey, "f2" )==0) GetString(Init.keyFn[ 1], pStr); else
            if(stricmp(sKey, "f3" )==0) GetString(Init.keyFn[ 2], pStr); else
            if(stricmp(sKey, "f4" )==0) GetString(Init.keyFn[ 3], pStr); else
            if(stricmp(sKey, "f5" )==0) GetString(Init.keyFn[ 4], pStr); else
            if(stricmp(sKey, "f6" )==0) GetString(Init.keyFn[ 5], pStr); else
            if(stricmp(sKey, "f7" )==0) GetString(Init.keyFn[ 6], pStr); else
            if(stricmp(sKey, "f8" )==0) GetString(Init.keyFn[ 7], pStr); else
            if(stricmp(sKey, "f9" )==0) GetString(Init.keyFn[ 8], pStr); else
            if(stricmp(sKey, "f10")==0) GetString(Init.keyFn[ 9], pStr); else
            if(stricmp(sKey, "f11")==0) GetString(Init.keyFn[10], pStr); else
            if(stricmp(sKey, "f12")==0) GetString(Init.keyFn[11], pStr); else
            
            if(stricmp(sKey, "sf1" )==0) GetString(Init.keySFn[ 0], pStr); else
            if(stricmp(sKey, "sf2" )==0) GetString(Init.keySFn[ 1], pStr); else
            if(stricmp(sKey, "sf3" )==0) GetString(Init.keySFn[ 2], pStr); else
            if(stricmp(sKey, "sf4" )==0) GetString(Init.keySFn[ 3], pStr); else
            if(stricmp(sKey, "sf5" )==0) GetString(Init.keySFn[ 4], pStr); else
            if(stricmp(sKey, "sf6" )==0) GetString(Init.keySFn[ 5], pStr); else
            if(stricmp(sKey, "sf7" )==0) GetString(Init.keySFn[ 6], pStr); else
            if(stricmp(sKey, "sf8" )==0) GetString(Init.keySFn[ 7], pStr); else
            if(stricmp(sKey, "sf9" )==0) GetString(Init.keySFn[ 8], pStr); else
            if(stricmp(sKey, "sf10")==0) GetString(Init.keySFn[ 9], pStr); else
            if(stricmp(sKey, "sf11")==0) GetString(Init.keySFn[10], pStr); else
            if(stricmp(sKey, "sf12")==0) GetString(Init.keySFn[11], pStr); else

            if(stricmp(sKey, "af1" )==0) GetString(Init.keyAFn[ 0], pStr); else
            if(stricmp(sKey, "af2" )==0) GetString(Init.keyAFn[ 1], pStr); else
            if(stricmp(sKey, "af3" )==0) GetString(Init.keyAFn[ 2], pStr); else
            if(stricmp(sKey, "af4" )==0) GetString(Init.keyAFn[ 3], pStr); else
            if(stricmp(sKey, "af5" )==0) GetString(Init.keyAFn[ 4], pStr); else
            if(stricmp(sKey, "af6" )==0) GetString(Init.keyAFn[ 5], pStr); else
            if(stricmp(sKey, "af7" )==0) GetString(Init.keyAFn[ 6], pStr); else
            if(stricmp(sKey, "af8" )==0) GetString(Init.keyAFn[ 7], pStr); else
            if(stricmp(sKey, "af9" )==0) GetString(Init.keyAFn[ 8], pStr); else
            if(stricmp(sKey, "af10")==0) GetString(Init.keyAFn[ 9], pStr); else
            if(stricmp(sKey, "af11")==0) GetString(Init.keyAFn[10], pStr); else
            if(stricmp(sKey, "af12")==0) GetString(Init.keyAFn[11], pStr); else
            
            if(stricmp(sKey, "cf1" )==0) GetString(Init.keyCFn[ 0], pStr); else
            if(stricmp(sKey, "cf2" )==0) GetString(Init.keyCFn[ 1], pStr); else
            if(stricmp(sKey, "cf3" )==0) GetString(Init.keyCFn[ 2], pStr); else
            if(stricmp(sKey, "cf4" )==0) GetString(Init.keyCFn[ 3], pStr); else
            if(stricmp(sKey, "cf5" )==0) GetString(Init.keyCFn[ 4], pStr); else
            if(stricmp(sKey, "cf6" )==0) GetString(Init.keyCFn[ 5], pStr); else
            if(stricmp(sKey, "cf7" )==0) GetString(Init.keyCFn[ 6], pStr); else
            if(stricmp(sKey, "cf8" )==0) GetString(Init.keyCFn[ 7], pStr); else
            if(stricmp(sKey, "cf9" )==0) GetString(Init.keyCFn[ 8], pStr); else
            if(stricmp(sKey, "cf10")==0) GetString(Init.keyCFn[ 9], pStr); else
            if(stricmp(sKey, "cf11")==0) GetString(Init.keyCFn[10], pStr); else
            if(stricmp(sKey, "cf12")==0) GetString(Init.keyCFn[11], pStr); else
            {
                printf("Line %d skipped: %s", nLine, sLine);
            }
        }

        printf("fLowercase=%d\n", Init.fLowercase);
        printf("init=\"%s\"\n", Init.sInit);
        printf("sym=%d\n", Init.nSymbolSize);
        printf("hst=%d\n", Init.nHistorySize);
        for(status=0; status<12; status++)
            printf("F%d=\"%s\"\n", status+1, Init.keyFn[status]);
        for(status=0; status<12; status++)
            printf("SF%d=\"%s\"\n", status+1, Init.keySFn[status]);
        for(status=0; status<12; status++)
            printf("AF%d=\"%s\"\n", status+1, Init.keyAFn[status]);
        for(status=0; status<12; status++)
            printf("CF%d=\"%s\"\n", status+1, Init.keyCFn[status]);

        //====================================================================
        // Send the init packet down to the module
        //====================================================================
        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            int status;

            status = ioctl(hIce, ICE_IOCTL_LOAD, &hIce);
            close(hIce);

            printf("IOCTL=%d  hice=%d\n", status, hIce);
        }
        else
        {
            printf("Error opening device!\n");
        }
    }
    else
    {
        printf("Error loading linice.o module (%d)!\n", status);
    }

    return;
}

