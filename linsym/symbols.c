/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       03/09/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains code to load and unload symbol tables.
        They are preprocessed here into constant images that are loaded
        into ice and used there without much processing.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/09/01   Initial version                                      Goran Devic *
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
    int hIce;

    //====================================================================
    // Send the name down to the module
    //====================================================================
    hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
    if( hIce>=0 )
    {
        int status;

        status = ioctl(hIce, ICE_IOCTL_REMOVE_SYM, sName);
        close(hIce);

        printf("IOCTL=%d\n", status);
    }
    else
    {
        printf("Error opening device!\n");
    }
}
