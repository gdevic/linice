/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       09/09/00                                                      *
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

