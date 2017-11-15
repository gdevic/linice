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
        They are preprocessed here into constant images that are loaded
        into ice and used there.

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

#ifndef WINDOWS
#include <sys/ioctl.h>                  // Include ioctl header file
#include <unistd.h>                     // Include standard UNIX header file
#else // WINDOWS
#include <io.h>
#endif // WINDOWS


#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include io control codes

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
*   Loads a symbol table, processesit and use IOCTL call to load it into
*   the ice.
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
        fd = open(sName, O_RDONLY);
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
                        //====================================================================
                        // Send the synbol file down to the module
                        //====================================================================
                        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
                        if( hIce>=0 )
                        {
//                          printf("IOCTL: %X param: %X\n", ICE_IOCTL_ADD_SYM, pBuf);

                            status = ioctl(hIce, ICE_IOCTL_ADD_SYM, pBuf);
                            close(hIce);

                            printf("AddSymbolTable: IOCTL=%d\n", status);
                        }
                        else
                            printf("AddSymbolTable IOCTL Failed!\n");
                    }
                    else
                        printf("%s is an invalid Linice symbol file!\n", sName);
                }
                else
                    printf("Error reading symbol table %s\n", sName);

                free(pBuf);
            }
            else
                printf("Error allocating memory\n");
        }
        else
            printf("Unable to open symbol file %s\n", sName);
    }
    else
        printf("Unable to access symbol file %s\n", sName);
}

/******************************************************************************
*                                                                             *
*   void OptRemoveSymbolTable(char *sName)                                    *
*                                                                             *
*******************************************************************************
*
*   Unloads a symbol table of a specified name from the debugger.
*
*   Where:
*       sName is the short name of the symbol table to remove
*
******************************************************************************/
void OptRemoveSymbolTable(char *sName)
{
    int hIce, status;

    //====================================================================
    // Send the name down to the module
    //====================================================================
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        status = ioctl(hIce, ICE_IOCTL_REMOVE_SYM, sName);
        close(hIce);

        printf("RemoveSymbolTable: IOCTL=%d\n", status);
    }
    else
        printf("Error opening device!\n");
}

