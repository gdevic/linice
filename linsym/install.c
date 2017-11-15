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
#ifdef SIM
#define printf printk
#endif // SIM
#endif // WIN32

#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern unsigned int opt;                // Various option flags

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// We need few addresses that we read from the kernel symbol table, since there
// are usually no easy ways to get them from the running kernel

// These are used to set up debugger keyboard hook:
DWORD handle_kbd_event = 0;
DWORD handle_scancode = 0;

// This symbol address is used to access kernel module list:
DWORD module_list = 0;

// Starting with the 2.4.18 kernel, this symbol is not available for kernel modules:
DWORD sys_call_table = 0;


extern int system2(char *command);

FILE *fpSystemMap = NULL;               // System map file handler

// We deal with the optional keyboard layout override (from US-mapping):
BOOL ReadKbdMapping(char TargetLayout[3][128], char *pRequestedLayout);


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


/******************************************************************************
*
*   Returns a default SuSE System.map file path/name
*
******************************************************************************/
char *MkSuSE(void)
{
    static char sSystemMap[256] = { 0 };    // We will compose it into this buffer
    FILE *fp;                               // File that we just created

    system2("echo /boot/System.map-`uname -r` > /tmp/.linice.System.map");

    fp = fopen("/tmp/.linice.System.map", "r");
    if( fp )
    {
        // Read the contents into our buffer
        fscanf(fp, "%s", sSystemMap);

        fclose(fp);
    }

    system2("rm -rf /tmp/.linice.System.map");

    return( sSystemMap );
}

/******************************************************************************
*                                                                             *
*   BOOL OpenSystemMap(char *pSystemMap)                                      *
*                                                                             *
*******************************************************************************
*
*   Tries to open a system map file given its name.
*
*   Returns:
*       TRUE - we successfully opened the file
*       FALSE - can not open the file
*
******************************************************************************/
BOOL OpenSystemMap(char *pSystemMap)
{
    // If the file is already opened, return (we only use first found file in a serial of calls)
    if( pSystemMap && fpSystemMap==NULL )
    {
        // Try to open this file
        VERBOSE1 printf("Trying %s..\n", pSystemMap);

        fpSystemMap = fopen(pSystemMap, "r");

        if( fpSystemMap )
        {
            VERBOSE1 printf("SYSTEM.MAP: %s\n", pSystemMap);

            // We were able to open it - keep fpSystemMap opened
            return( TRUE );
        }
    }

    // Either the file is already opened or we can't open it
    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL GetSymbolExports(void)                                               *
*                                                                             *
*******************************************************************************
*
*   Opens system symbol map and scans for exports that we need. SystemMap file
*   should have already been opened.
*
*   Returns:
*       TRUE - all necessary symbols loaded
*       FALSE - can not find symbol.map file
*
******************************************************************************/
static BOOL GetSymbolExports(void)
{
    int items;
    DWORD address;
    char code;
    char sSymbol[128];

    handle_kbd_event = handle_scancode = module_list = sys_call_table = 0;

    // If the system map file has not been opened, return failure
    if( fpSystemMap )
    {
        VERBOSE2 printf("Reading kernel locations from System.map file:\n");

        // Look for the addresses of some functions in the system map file until
        // we either reach the end of the System.map file or we have them all read

        while( !feof(fpSystemMap) && (!handle_kbd_event || !handle_scancode || !module_list || !sys_call_table) )
        {
            items = fscanf(fpSystemMap, "%X %c %s\n", &address, &code, sSymbol);
            if( items==3 && strcmp("handle_kbd_event", sSymbol)==0 )
            {
                VERBOSE2 printf("   handle_kbd_event = %08X\n", address);
                handle_kbd_event = address;
            }
            else
            if( items==3 && strcmp("handle_scancode", sSymbol)==0 )
            {
                VERBOSE2 printf("   handle_scancode = %08X\n", address);
                handle_scancode = address;
            }
            else
            if( items==3 && strcmp("module_list", sSymbol)==0 )
            {
                VERBOSE2 printf("   module_list = %08X\n", address);
                module_list = address;
            }
            else
            if( items==3 && strcmp("sys_call_table", sSymbol)==0 )
            {
                VERBOSE2 printf("   sys_call_table = %08X\n", address);
                sys_call_table = address;
            }
        }

        fclose(fpSystemMap);
        fpSystemMap = NULL;

        return( TRUE );                 // We successfully read symbols
    }

    return( FALSE );                    // System.map file was not opened
}


/******************************************************************************
*                                                                             *
*   void OpInstall()                                                          *
*                                                                             *
*******************************************************************************
*
*   Loads linice.o module.  Feeds it the init file so it can configure itself.
*   Looks for the System.map file and read all necessary symbols that linice
*   cannot obtain as a kernel module and needs for proper operation.
*
*   Where:
*       pSystemMap is the default path/name of the System.map file
*
*   Returns:
*       TRUE - module loaded and initialized
*       FALSE - a problem happened, we need to exit
*
******************************************************************************/
BOOL OptInstall(char *pSystemMap)
{
    int hIce;
    FILE *fp;
    int status;
    char sLine[256], *pStr;
    char sKey[16], sValue[256];
    TINITPACKET Init;
    int nLine, i;


    // Initialization: zero out the init packet
    memset(&Init, 0, sizeof(TINITPACKET));
    Init.nSize = sizeof(TINITPACKET);

    //====================================================================
    // Installation check - If Linice module already installed, return
    //====================================================================
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        fprintf(stderr, "Linice module already installed, or /dev/%s exists.\n", DEVICE_NAME);

        return( TRUE );
    }

    // Remove linice device file (useful when debugging)
    // Anyways, this dev file should not exist there at this point...
    system2("rm /dev/ice");

    // When looking for the System.map file, user command line argument
    // has precedence over linice.dat file specification and lastly, default

    // It makes sense to display error and exit if the user-specified map
    // file can't be opened since he probably mistyped it

    if( pSystemMap )
    {
        if( OpenSystemMap(pSystemMap)==FALSE )
        {
            fprintf(stderr, "Unable to open specified map file: %s\n\n", pSystemMap);

            return( FALSE );
        }
    }

    //====================================================================
    // Look for the linice config file and read its assignments
    //====================================================================
    fp = fopen("linice.dat", "r");
    if( fp==NULL )
    {
        // Look in the /etc directory
        fp = fopen("/etc/linice.dat", "r");
        if( fp==NULL )
        {
            // Failed to open the config file - load default settings
            fprintf(stderr, "Failed to open linice.dat file - loading default settings.\n");

            // Set up default values:

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

        // Cut off end of line characters
        if( strchr(sLine, '\n') ) *(char *)strchr(sLine, '\n') = 0;
        if( strchr(sLine, '\r') ) *(char *)strchr(sLine, '\r') = 0;

        // Change all TABs into spaces
        while( strchr(sLine, '\t') )
        {
            *(char *)strchr(sLine, '\t') = ' ';
        }

        // Get to the first meaningful character in a line
        pStr = sLine;
        while( *pStr==' ' ) pStr++;

        // Skip comment line and empty lines
        if((*pStr==';')||(*pStr=='#')||(*pStr==0))
            continue;

        // Get the assignment in the form KEY=VALUE or KEY="value"
        sscanf(pStr, "%[a-zA-Z0-9.]s", sKey);
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
             GetString(Init.sInit, pStr);
        else
        if(stricmp(sKey, "layout"  )==0)
        {
            // If we return TRUE, Init.Layout has been updated
            if( ReadKbdMapping(Init.Layout, pStr)==TRUE )
            {
                VERBOSE2 printf("Keyboard layout: %s\n", pStr);
            }
            else
            {
                fprintf(stderr, "Invalid keyboard layout %s!\n", pStr);

                // We probably dont want to continue if the keyboard layout is bad
                return( FALSE );
            }
        }else
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

        // Try to open the system map file...
        if(stricmp(sKey, "System.map")==0) OpenSystemMap(pStr); else
        {
            VERBOSE2 printf("Line %d skipped: %s\n", nLine, sLine);
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


    // Lastly, try several more default locations for System.map file
    OpenSystemMap("/boot/System.map");  // Default for RedHat
    OpenSystemMap(MkSuSE());            // Make default for SuSE linux
    OpenSystemMap("./System.map");      // Current directory
    OpenSystemMap("/System.map");       // File system root

    // Scan system map file for some symbols that we need to send to linice
    if( GetSymbolExports() )
    {
        // Load the linice.o device driver module:
        //  -x   do not export externs
        //  -f   force load even if kernel version does not match

        sprintf(sLine, "insmod -x -f linice_`uname -r`/linice.o ice_debug_level=1 kbd=%d scan=%d pmodule=%d sys=%d",
            handle_kbd_event,
            handle_scancode,
            module_list,
            sys_call_table);

        VERBOSE2 printf("%s\n", sLine);

        status = system2(sLine);

        // insmod must return 0 to load module correctly
        if( status==0 )
        {
            //====================================================================
            // Send the init packet down to the module
            //====================================================================
            hIce = open("/dev/"DEVICE_NAME, O_RDONLY);

            if( hIce>=0 )
            {
                status = ioctl(hIce, ICE_IOCTL_INIT, &Init);

                close(hIce);

                VERBOSE1 printf("Linice installed.\n");

                // Return success
                return( TRUE );
            }

            fprintf(stderr, "Error opening /dev/%s device!\n", DEVICE_NAME);

            // Unload linice module

            system2("rmmod linice");
        }

        fprintf(stderr, "Error loading linice module!\n");
    }
    else
    {
        // We did not find System.map - this file needs to be used since we
        // really can't hook without it..
        fprintf(stderr, "\n");
        fprintf(stderr, "Failed to open System.map file!\n");
        fprintf(stderr, "Please use option -i:<System.map> to specify its path and name,\n");
        fprintf(stderr, "or add a line SYSTEM.MAP=<path/name> to linice.dat configuration file.\n");
        fprintf(stderr, "\n");
    }

    // Return failure
    return( FALSE );
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

        // Unload the linice.o device driver module
        status = system2("rmmod linice");

        // rmmod must return 0 when uninstalling a module correctly
        if( status != 0 )
        {
            // If we cannot unload linice because it crashed, use the
            // last resort and try to force unload it

            hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
            if( hIce>=0 )
            {
                VERBOSE1 printf("Forcing unload...\n");

                status = ioctl(hIce, ICE_IOCTL_EXIT_FORCE, 0);
                close(hIce);

                // Theoretically, by now our bad module count is reset back to 0...

                // Unload the linice.o device driver module
                status = system2("rmmod linice");
            }
        }

        VERBOSE1 printf("Linice uninstalled.\n");
    }
    else
        fprintf(stderr, "Cannot communicate with the Linice module! Is Linice installed?\n");
}
