/******************************************************************************
*                                                                             *
*   Module:     install.c                                                     *
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

// Need to follow the task switch
DWORD switch_to = 0;

// New symbols for 2.6 kernel
DWORD start_symtab = 0;
DWORD stop_symtab = 0;
DWORD start_symtab_gpl = 0;
DWORD stop_symtab_gpl = 0;


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
*   BOOL IsRootUser(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Checks if the current user running this command is a "root" user.
*   Since this command is only called from the places where we have to be
*   a root user to continue, we print out an error message if not root.
*
*   Returns:
*       TRUE - the user is root
*       FALSE - the user is NOT root
*
******************************************************************************/
BOOL IsRootUser(void)
{
    char *username;                     // Pointer to the login user name

    // Beware the getlogin may fail and return NULL (fixed by Guillaume)
    if( (username = getlogin())==NULL )
        username = "?";

    // We require for either the login name or the USER environment variable to be 'root'

    VERBOSE1 printf("getlogin()=%s\n", username);
    VERBOSE1 printf("USER      =%s\n", getenv("USER"));

    if( !strcmp("root", username) || !strcmp(getenv("USER"), "root") )
        return( TRUE );

    // Print the error message since we will return FALSE and quit the command

    fprintf(stderr, "You have to be a root user in order to run this command!\n");
    fprintf(stderr, "(If you did use the 'su' command, try using the 'su -' format instead.)\n");

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

    handle_kbd_event = handle_scancode = module_list = sys_call_table =
    switch_to = start_symtab = stop_symtab = start_symtab_gpl = stop_symtab_gpl = 0;

    // If the system map file has not been opened, return failure
    if( fpSystemMap )
    {
        VERBOSE2 printf("Reading kernel locations from System.map file:\n");

        // TODO: Should we depend on System.map file? Some of these symbols
        //       can be read from /proc/ksyms
        //
        // handle_scancode
        // sys_call_table


        // Look for the addresses of some functions in the system map file until
        // we either reach the end of the System.map file or we have them all read

        while( !feof(fpSystemMap) && (!handle_kbd_event || !handle_scancode || !module_list ||
        !sys_call_table || !switch_to || !start_symtab || !stop_symtab ||
               !start_symtab_gpl || !stop_symtab_gpl ) )
        {
            items = fscanf(fpSystemMap, "%X %c %s\n", &address, &code, sSymbol);
            if( items==3 && strcmp("handle_kbd_event", sSymbol)==0 )
            {
                VERBOSE2 printf("   handle_kbd_event = %08X\n", address);
                handle_kbd_event = address;
            }
            else
            if( items==3 && strcmp("kbd_event", sSymbol)==0 )           // Test: 2.6 kernels
            {
                VERBOSE2 printf("   kbd_event = %08X\n", address);
                handle_kbd_event = address;
            }
            else
            if( items==3 && strcmp("handle_scancode", sSymbol)==0 )
            {
                VERBOSE2 printf("   handle_scancode = %08X\n", address);
                handle_scancode = address;
            }
            else
            if( items==3 && strcmp("kbd_keycode", sSymbol)==0 )         // Test: 2.6 kernels
            {
                VERBOSE2 printf("   kbd_keycode = %08X\n", address);
                handle_scancode = address;
            }
            else
            if( items==3 && strcmp("module_list", sSymbol)==0 )
            {
                VERBOSE2 printf("   module_list = %08X\n", address);
                module_list = address;
            }
            else
            if( items==3 && strcmp("modules", sSymbol)==0 )             // Test: 2.6 kernels
            {
                VERBOSE2 printf("   modules = %08X\n", address);
                module_list = address;
            }
            else
            if( items==3 && strcmp("sys_call_table", sSymbol)==0 )
            {
                VERBOSE2 printf("   sys_call_table = %08X\n", address);
                sys_call_table = address;
            }
            else
            if( items==3 && strcmp("__switch_to", sSymbol)==0 )
            {
                VERBOSE2 printf("   switch_to = %08X\n", address);
                switch_to = address;
            }
            else
            if( items==3 && strcmp("__start___ksymtab", sSymbol)==0 )       // 2.6 kernels
            {
                VERBOSE2 printf("   __start___ksymtab = %08X\n", address);
                start_symtab = address;
            }
            else
            if( items==3 && strcmp("__stop___ksymtab", sSymbol)==0 )        // 2.6 kernels
            {
                VERBOSE2 printf("   __stop___ksymtab = %08X\n", address);
                stop_symtab = address;
            }
            else
            if( items==3 && strcmp("__start___ksymtab_gpl", sSymbol)==0 )   // 2.6 kernels
            {
                VERBOSE2 printf("   __start___ksymtab_gpl = %08X\n", address);
                start_symtab_gpl = address;
            }
            else
            if( items==3 && strcmp("__stop___ksymtab_gpl", sSymbol)==0 )    // 2.6 kernels
            {
                VERBOSE2 printf("   __stop___ksymtab_gpl = %08X\n", address);
                stop_symtab_gpl = address;
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
*   BOOL OptInstall(char *pSystemMap)                                         *
*                                                                             *
*******************************************************************************
*
*   Loads linice.o module.  Feeds it the init file so it can configure itself.
*   Looks for the System.map file and read all necessary symbols that linice
*   cannot obtain as a kernel module and needs for proper operation.
*
*   Where:
*       pSystemMap is the user-add path/name of the System.map file
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

    // Only the root user may install the module
    if( !IsRootUser() )
        return( FALSE );

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

    // When looking for the System.map file, user command line argument
    // has precedence over linice.dat file specification and lastly, default

    // It makes sense to display error and exit if the user-specified map
    // file can't be opened since he probably mistyped it

    if( pSystemMap )
    {
        if( OpenSystemMap(pSystemMap)==FALSE )
        {
            fprintf(stderr, "Unable to open specified map file: %s\n", pSystemMap);

            return( FALSE );
        }
    }

    //====================================================================
    // Look for the linice config file and read its assignments
    //====================================================================
    fp = fopen("linice.dat", "r");
    if( fp==NULL )
    {
        VERBOSE1 printf("Looking for linice.dat in /etc");

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
        // TODO: This is not documented
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
        // In order to do "insmod" with the module name, we need to know where to
        // find it (user might called linsym from any directory). Environment variable
        // LINICE should be set to the 'bin' of the linice built module IF the linsym
        // is not being executed from the default place (bin).

        // Load the linice.o device driver module:
        //  -x   do not export externs
        //  -f   force load even if kernel version does not match

        sprintf(sLine, "insmod -f linice_`uname -r`/linice.o ice_debug_level=1 kbd=%d scan=%d pmodule=%d sys=%d switchto=%d start_sym=%d stop_sym=%d start_sym_gpl=%d stop_sym_gpl=%d",
            handle_kbd_event,
            handle_scancode,
            module_list,
            sys_call_table,
            switch_to,
            start_symtab,
            stop_symtab,
            start_symtab_gpl,
            stop_symtab_gpl);

        VERBOSE2 printf("%s\n", sLine);

        status = system2(sLine);        // Execute the module load...

        // If failed on a first try, change the directory to the environment LINICE and retry
        if( status && getenv("LINICE") )
        {
            VERBOSE2 printf("Using environment variable LINICE = %s\n", getenv("LINICE"));
            chdir(getenv("LINICE"));    // Change directory to it

            status = system2(sLine);        // Try the module load again...
        }

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

                printf("Linice installed.\n");

                // Return success
                return( TRUE );
            }

            fprintf(stderr, "Error opening /dev/%s device!\n", DEVICE_NAME);

            // Unload linice module

            system2("rmmod linice");
        }
        else
        {   // We could not load linice module!
            fprintf(stderr, "Error loading linice module!\n\n");
        }
        VERBOSE2 printf("system(insmod()) returns %d\n", status);

        fprintf(stderr, "Possible causes are:\n");
        fprintf(stderr, "  * /dev/ice exists (delete it)\n");
        fprintf(stderr, "  * Not calling Linsym from the default Linice directory (its 'bin') -\n");
        fprintf(stderr, "    Set the environment variable LINICE with the path to that directory:\n");
        fprintf(stderr, "    Example:  export LINICE=/usr/src/linice/bin\n");
        fprintf(stderr, "  * Linice did not compile correctly so the module does not exist\n");
        fprintf(stderr, "  * Your Symbol.map file is not correct (use option -m <map>)\n");
        fprintf(stderr, "  * Unsupported kernel version 2.6.9 and above\n");
        fprintf(stderr, "\n");

    }
    else
    {
        // We did not find System.map - this file needs to be used since we
        // really can't hook without it..
        fprintf(stderr, "Failed to open System.map file!\n");
        fprintf(stderr, "Please use option --map <System.map> to specify its path and name.\n");
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

    // Only the root user may remove the module
    if( IsRootUser() )
    {
        // Send the exit ioctl to the driver so it can decrement possibly
        // multiple usage count to only 1 (useful when loader crashes)
        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            // Send the EXIT message to the Linice, so it can release its hooks
            status = ioctl(hIce, ICE_IOCTL_EXIT, 0);
            close(hIce);

            // If there are still any extension modules loaded, we cannot unload
            // since the modules are linked to us and wont let us unload cleanly.

            // Value of 1 for status is the special signal from Linice driver
            if( status != 1 )
            {
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

                printf("Linice uninstalled.\n");
            }
            else
                fprintf(stderr, "One or more Linice extension modules are still loaded. Unload them first!\n");
        }
        else
            fprintf(stderr, "Cannot communicate with the Linice module! Is Linice installed?\n");
    }
}
