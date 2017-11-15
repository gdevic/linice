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

extern int system2(char *command);

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
    status = system2("insmod linice.o ice_debug_level=1");

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
        Init.nSize = sizeof(TINITPACKET);

        // Set default values

        Init.nHistorySize = 1024;
        Init.nSymbolSize  = 4096;

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
            sscanf(pStr, "%[a-zA-Z0-9]s", sKey);
            pStr = strchr(pStr,'=') + 1;

            // Decipher the key and load the appropriate value (number, boolean or string)
            if(stricmp(sKey, "lowercase")==0)
            {
                sscanf(pStr, "%s", sValue);
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

            if(stricmp(sKey, "f1"  )==0) GetString(Init.keyFn[ 0], pStr); else
            if(stricmp(sKey, "f2"  )==0) GetString(Init.keyFn[ 1], pStr); else
            if(stricmp(sKey, "f3"  )==0) GetString(Init.keyFn[ 2], pStr); else
            if(stricmp(sKey, "f4"  )==0) GetString(Init.keyFn[ 3], pStr); else
            if(stricmp(sKey, "f5"  )==0) GetString(Init.keyFn[ 4], pStr); else
            if(stricmp(sKey, "f6"  )==0) GetString(Init.keyFn[ 5], pStr); else
            if(stricmp(sKey, "f7"  )==0) GetString(Init.keyFn[ 6], pStr); else
            if(stricmp(sKey, "f8"  )==0) GetString(Init.keyFn[ 7], pStr); else
            if(stricmp(sKey, "f9"  )==0) GetString(Init.keyFn[ 8], pStr); else
            if(stricmp(sKey, "f10" )==0) GetString(Init.keyFn[ 9], pStr); else
            if(stricmp(sKey, "f11" )==0) GetString(Init.keyFn[10], pStr); else
            if(stricmp(sKey, "f12" )==0) GetString(Init.keyFn[11], pStr); else
            
            if(stricmp(sKey, "sf1" )==0) GetString(Init.keyFn[12], pStr); else
            if(stricmp(sKey, "sf2" )==0) GetString(Init.keyFn[13], pStr); else
            if(stricmp(sKey, "sf3" )==0) GetString(Init.keyFn[14], pStr); else
            if(stricmp(sKey, "sf4" )==0) GetString(Init.keyFn[15], pStr); else
            if(stricmp(sKey, "sf5" )==0) GetString(Init.keyFn[16], pStr); else
            if(stricmp(sKey, "sf6" )==0) GetString(Init.keyFn[17], pStr); else
            if(stricmp(sKey, "sf7" )==0) GetString(Init.keyFn[18], pStr); else
            if(stricmp(sKey, "sf8" )==0) GetString(Init.keyFn[19], pStr); else
            if(stricmp(sKey, "sf9" )==0) GetString(Init.keyFn[20], pStr); else
            if(stricmp(sKey, "sf10")==0) GetString(Init.keyFn[21], pStr); else
            if(stricmp(sKey, "sf11")==0) GetString(Init.keyFn[22], pStr); else
            if(stricmp(sKey, "sf12")==0) GetString(Init.keyFn[23], pStr); else

            if(stricmp(sKey, "af1" )==0) GetString(Init.keyFn[24], pStr); else
            if(stricmp(sKey, "af2" )==0) GetString(Init.keyFn[25], pStr); else
            if(stricmp(sKey, "af3" )==0) GetString(Init.keyFn[26], pStr); else
            if(stricmp(sKey, "af4" )==0) GetString(Init.keyFn[27], pStr); else
            if(stricmp(sKey, "af5" )==0) GetString(Init.keyFn[28], pStr); else
            if(stricmp(sKey, "af6" )==0) GetString(Init.keyFn[29], pStr); else
            if(stricmp(sKey, "af7" )==0) GetString(Init.keyFn[30], pStr); else
            if(stricmp(sKey, "af8" )==0) GetString(Init.keyFn[31], pStr); else
            if(stricmp(sKey, "af9" )==0) GetString(Init.keyFn[32], pStr); else
            if(stricmp(sKey, "af10")==0) GetString(Init.keyFn[33], pStr); else
            if(stricmp(sKey, "af11")==0) GetString(Init.keyFn[34], pStr); else
            if(stricmp(sKey, "af12")==0) GetString(Init.keyFn[35], pStr); else
            
            if(stricmp(sKey, "cf1" )==0) GetString(Init.keyFn[36], pStr); else
            if(stricmp(sKey, "cf2" )==0) GetString(Init.keyFn[37], pStr); else
            if(stricmp(sKey, "cf3" )==0) GetString(Init.keyFn[38], pStr); else
            if(stricmp(sKey, "cf4" )==0) GetString(Init.keyFn[39], pStr); else
            if(stricmp(sKey, "cf5" )==0) GetString(Init.keyFn[40], pStr); else
            if(stricmp(sKey, "cf6" )==0) GetString(Init.keyFn[41], pStr); else
            if(stricmp(sKey, "cf7" )==0) GetString(Init.keyFn[42], pStr); else
            if(stricmp(sKey, "cf8" )==0) GetString(Init.keyFn[43], pStr); else
            if(stricmp(sKey, "cf9" )==0) GetString(Init.keyFn[44], pStr); else
            if(stricmp(sKey, "cf10")==0) GetString(Init.keyFn[45], pStr); else
            if(stricmp(sKey, "cf11")==0) GetString(Init.keyFn[46], pStr); else
            if(stricmp(sKey, "cf12")==0) GetString(Init.keyFn[47], pStr); else
            {
                printf("Line %d skipped: %s", nLine, sLine);
            }
        }

#if 0
        printf("fLowercase=%d\n", Init.fLowercase);
        printf("init=\"%s\"\n", Init.sInit);
        printf("sym=%d\n", Init.nSymbolSize);
        printf("hst=%d\n", Init.nHistorySize);
        for(status=0; status<48; status++)
            printf("F%d=\"%s\"\n", status+1, Init.keyFn[status]);
#endif
        //====================================================================
        // Send the init packet down to the module
        //====================================================================
        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            int status;

            status = ioctl(hIce, ICE_IOCTL_INIT, &Init);
            close(hIce);

            printf("IOCTL=%d\n", status);
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

