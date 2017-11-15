/******************************************************************************
*                                                                             *
*   Module:     install.c                                                     *
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

        This module contains code to load Linice debugger module and feed
        it the init parameters as read from the config file

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

#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <stdlib.h>                     // Include standard library
#include <string.h>                     // Include string library

#ifndef WIN32
#include <unistd.h>                     // Include standard UNIX header file
#include <sys/ioctl.h>                  // Include ioctl header file
#define stricmp     strcasecmp          // Weird gnu c call..
#else // WIN32
#include <io.h>
#include <malloc.h>
#endif // WIN32

#include "ice-ioctl.h"                  // Include shared header file

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

// These two addresses are read from the kernel symbol table.
// They are used to set up debugger keyboard hook.
DWORD handle_kbd_event = 0;
DWORD handle_scancode = 0;

// This symbol address is used to access kernel module list
DWORD module_list = 0;

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

    // We dont need heading spaces - get rid of them
    while( *pStr==' ' ) pStr++;

    // Copy the string
    while((*pStr!='\n') && ((*pStr!='\r')) && (*pStr!='"') && (n++<MAX_STRING))
        *p++ = *pStr++;

    // Zero terminate the destination buffer
    *p = 0;
}


BOOL AbortLoad(char *sMsg)
{
    printf("%s\n", sMsg);
    printf("Press a key to continue, or ESC to abort loading debugger\n");

    return( getc(stdin)==27 );
}

/******************************************************************************
*                                                                             *
*   BOOL GetSymbolExports(char *pSystemMap)                                   *
*                                                                             *
*******************************************************************************
*
*   Opens system symbol map and scans for exports that we need.
*
*   Where:
*       pSystemMap is default path/name of the system map file
*
******************************************************************************/
static BOOL GetSymbolExports(char *pSystemMap)
{
    int items, found = 0;
    FILE *fp;
    DWORD address;
    char code;
    char sSymbol[128];

    // Try default system map or user-supplied
    fp = fopen(pSystemMap, "r");
    if( fp==NULL )
    {
        // Then try the one in the current directory
        fp = fopen("./System.map", "r");
        if( fp==NULL )
        {
            if( AbortLoad("Failed to open System.map file!") )
                exit(-1);
        }
    }

    // Look for the addresses of some functions in the system map file
    while( !feof(fp) )
    {
        items = fscanf(fp, "%X %c %s\n", &address, &code, sSymbol);
        if( items==3 && strcmp("handle_kbd_event", sSymbol)==0 )
        {
            printf("handle_kbd_event = %08X\n", address);
            handle_kbd_event = address;
            if( found++==3 )
                break;
        }
        else
        if( items==3 && strcmp("handle_scancode", sSymbol)==0 )
        {
            printf("handle_scancode = %08X\n", address);
            handle_scancode = address;
            if( found++==3 )
                break;
        }
        else
        if( items==3 && strcmp("module_list", sSymbol)==0 )
        {
            printf("module_list = %08X\n", address);
            module_list = address;
            if( found++==3 )
                break;
        }
    }

    fclose(fp);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void OpInstall()                                                          *
*                                                                             *
*******************************************************************************
*
*   Loads linice.o module.  Feeds it the init file so it can configure itself.
*   Looks for the System.map file and the keyboard routine "handle_kbd_event"
*   to send to ice.o as a parameter, so it can hook into the keyboard handler.
*   Looks for symbol "module_list" as well.
*
*   Where:
*       pSystemMap is the default path/name of the System.map file
*
******************************************************************************/
void OptInstall(char *pSystemMap)
{
    int hIce;
    FILE *fp;
    int status;
    char sLine[256], *pStr;
    char sKey[16], sValue[256];
    TINITPACKET Init;
    int nLine, i;

    //====================================================================
    // Installation check - If Linice module already installed, return
    //====================================================================
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        fprintf(stderr, "Linice module already installed!\n");
        return;
    }

    // Remove linice device file (useful when debugging linice)
    // Anyways, this file should not exist there at this point...
    system2("rm /dev/ice >& /dev/null");

    GetSymbolExports(pSystemMap);

    // Load the linice.o device driver module
    sprintf(sLine, "insmod linice.o ice_debug_level=1 kbd=%d scan=%d pmodule=%d",
        handle_kbd_event,
        handle_scancode,
        module_list);

    status = system2(sLine);

    // insmod must return 0 to load module correctly
    if( status==0 )
    {
        printf("Linice module loaded.\n");

        // Zero out the init packet
        memset(&Init, 0, sizeof(TINITPACKET));
        Init.nSize = sizeof(TINITPACKET);

        // Look for the config file and feed it to the module
        fp = fopen("linice.dat", "r");
        if( fp==NULL )
        {
            // Look in the /etc directory
            fp = fopen("/etc/linice.dat", "r");
            if( fp==NULL )
            {
                // Failed to open the config file - load default settings
                printf("Failed to open linice.dat file - loading default settings.\n");

                // Set some default values.. Some other we still would like to have
                // set up even if we found init file

                strcpy(Init.keyFn[ 0], "h;");               // F1
                strcpy(Init.keyFn[ 1], "^wr;");             // F2
                strcpy(Init.keyFn[ 2], "^src;");            // F3
                strcpy(Init.keyFn[ 3], "^rs;");             // F4
                strcpy(Init.keyFn[ 4], "^x;");              // F5
                strcpy(Init.keyFn[ 5], "^ec;");             // F6
                strcpy(Init.keyFn[ 6], "^here;");           // F7
                strcpy(Init.keyFn[ 7], "^t;");              // F8
                strcpy(Init.keyFn[ 8], "^bpx;");            // F9
                strcpy(Init.keyFn[ 9], "^p;");              // F10
                strcpy(Init.keyFn[10], "^G @SS:ESP;");      // F11
                strcpy(Init.keyFn[11], "^p ret;");          // F12
                strcpy(Init.keyFn[12], "^format;");         // SF1
                strcpy(Init.keyFn[24], "^wr;");             // AF1
                strcpy(Init.keyFn[25], "^wd;");             // AF2
                strcpy(Init.keyFn[26], "^wc;");             // AF3
                strcpy(Init.keyFn[27], "^ww;");             // AF4
                strcpy(Init.keyFn[28], "CLS;");             // AF5
                strcpy(Init.keyFn[34], "^dd dataaddr->0;"); // AF11
                strcpy(Init.keyFn[35], "^dd dataaddr->4;"); // AF12
                strcpy(Init.keyFn[36], "altscr off; lines 60; wc 32; wd 8;"); // CF1
                strcpy(Init.keyFn[37], "^wr;^wd;^wc;");                       // CF2

                Init.fLowercase   = 1;          // Use lowercase disassembly
            }
        }

        // Set default values - these we do in any case before parsing the linice.dat file

        Init.nHistorySize = 16;         // 16 Kb
        Init.nSymbolSize  = 128;        // 128 Kb
        Init.nMacros      = 32;         // 32 macro entries
        Init.nVars        = 32;         // 32 user variables

        nLine = 0;

        // Read config file line by line and build the config packet
        while( fp && !feof(fp) )
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
            if(stricmp(sKey, "drawsize")==0)
                sscanf(pStr, "%d", &Init.nDrawSize);
            else
            if(stricmp(sKey, "hst")==0)
                sscanf(pStr, "%d", &Init.nHistorySize);
            else
            if(stricmp(sKey, "macros")==0)
                sscanf(pStr, "%d", &Init.nMacros);
            else
            if(stricmp(sKey, "vars")==0)
                sscanf(pStr, "%d", &Init.nVars);
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

        // Close the linice.dat file
        if( fp )
            fclose(fp);

        // Modify function key assignments to substitute the trailing semicolon
        // with the newline character
        for( i=0; i<48; i++)
        {
            if( strlen(Init.keyFn[i]) > 0 )
            {
                pStr = Init.keyFn[i] + strlen(Init.keyFn[i]) - 1;
                if( *pStr==';' )
                    *pStr = '\n';
            }
        }

        Init.nSymbolSize *= 1024;       // Make these values kilobytes
        Init.nDrawSize *= 1024;
        Init.nHistorySize *= 1024;
#if 0
        printf("fLowercase=%d\n", Init.fLowercase);
        printf("init=\"%s\"\n", Init.sInit);
        printf("sym=%d Kb\n", Init.nSymbolSize);
        printf("drawsize=%d Kb\n", Init.nDrawSize);
        printf("hst=%d Kb\n", Init.nHistorySize);
        printf("macros=%d Kb\n", Init.nMacros);
        printf("vars=%d Kb\n", Init.nVars);
        for(status=0; status<48; status++)
            printf("F%d=\"%s\"\n", status+1, Init.keyFn[status]);
#endif

        //====================================================================
        // Send the init packet down to the module
        //====================================================================
        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            status = ioctl(hIce, ICE_IOCTL_INIT, &Init);
            close(hIce);

            printf("IOCTL=%d\n", status);
        }
        else
        {
            fprintf(stderr, "Error opening /dev/%s device!\n", DEVICE_NAME);
        }
    }
    else
    {
        printf("Error loading linice.o module (%d)!\n", status);
    }

    return;
}


/******************************************************************************
*                                                                             *
*   void OptUninstall()                                                       *
*                                                                             *
*******************************************************************************
*
*   Removes linice.o module
*
******************************************************************************/
void OptUninstall()
{
    int hIce;
    int status;

    // Send the exit ioctl to the driver so it can decrement possibly
    // multiple usage count to only 1 (useful when loader crashes)
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        status = ioctl(hIce, ICE_IOCTL_EXIT, 0);
        close(hIce);

        printf("IOCTL=%d\n", status);
    }
    else
    {
        printf("Error opening device!\n");
    }

    // Unload the linice.o device driver module
    status = system2("rmmod linice");

    // rmmod must return 0 when uninstalling a module correctly
    if( status==0 )
    {
        printf("Linice module removed\n");
    }
    else
    {
        printf("Error unloading linice module (%d)!\n", status);
    }
}


