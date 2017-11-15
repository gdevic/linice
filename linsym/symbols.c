/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       09/09/00                                                      *
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

        This module contains code to load and unload symbol tables.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/09/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <malloc.h>                     // Include allocation header

#ifndef WIN32
#include <sys/ioctl.h>                  // Include ioctl header file
#include <unistd.h>                     // Include standard UNIX header file
#define O_BINARY    0
#else // WIN32
#include <io.h>
#ifdef SIM
#define printf printk
#endif // SIM
#endif // WIN32


#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include io control codes

#include "loader.h"                     // Include global protos

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
*                                                                             *
*   void OptAddSymbolTable(char *sName)                                       *
*                                                                             *
*******************************************************************************
*
*   Loads a symbol table - use IOCTL call to load it into the debugger.
*
*   Where:
*       sName is the file name of the symbols to load
*
******************************************************************************/
void OptAddSymbolTable(char *sName)
{
    struct stat prop;
    int fd, hIce, status;
    void *pBuf;

    // Get the file length of the symbol file
    status = stat(sName, &prop);
    if( status==0 )
    {
        // Open the symbol table file
        fd = open(sName, O_RDONLY | O_BINARY);
        if( fd>0 )
        {
            // Get the total length of the file, allocate memory and load it in
            pBuf = malloc(prop.st_size);
            if( pBuf )
            {
                status=read(fd, pBuf, prop.st_size);
                if( status == prop.st_size )
                {
                    // Make sure it is a valid symbol file
                    if( !strcmp(pBuf, SYMSIG) )
                    {
                        //====================================================
                        // Send the synbol file down to the debugger module
                        //====================================================
                        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
                        if( hIce>=0 )
                        {
//                          printf("IOCTL: %X param: %X\n", ICE_IOCTL_ADD_SYM, pBuf);

                            status = ioctl(hIce, ICE_IOCTL_ADD_SYM, pBuf);
                            close(hIce);

                            VERBOSE2 printf("AddSymbolTable: IOCTL=%d\n", status);

                            printf("Symbol table '%s' added.\n", sName);
                        }
                        else
                            fprintf(stderr, "Error opening /dev/%s device!\n", DEVICE_NAME);
                    }
                    else
                        fprintf(stderr, "%s is an invalid Linice symbol file!\n", sName);
                }
                else
                    fprintf(stderr, "Error reading symbol table %s\n", sName);

                free(pBuf);
            }
            else
                fprintf(stderr, "Error allocating memory\n");
        }
        else
            fprintf(stderr, "Unable to open symbol file %s\n", sName);
    }
    else
        fprintf(stderr, "Unable to access symbol file %s\n", sName);
}

/******************************************************************************
*                                                                             *
*   void OptRemoveSymbolTable(char *sTableName)                               *
*                                                                             *
*******************************************************************************
*
*   Unloads a symbol table of a specified name from the debugger.
*
*   Where:
*       sName is the internal name of the symbol table to remove
*
******************************************************************************/
void OptRemoveSymbolTable(char *sTableName)
{
    int hIce, status;

    //====================================================================
    // Send the name down to the module
    //====================================================================
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        status = ioctl(hIce, ICE_IOCTL_REMOVE_SYM, sTableName);
        close(hIce);

        VERBOSE2 printf("RemoveSymbolTable [%s]: IOCTL=%d\n", sTableName, status);

        printf("Symbol table '%s' removed.\n", sTableName);
    }
    else
        fprintf(stderr, "Error opening /dev/%s device!\n", DEVICE_NAME);
}

