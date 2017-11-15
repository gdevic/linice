/******************************************************************************
*                                                                             *
*   Module:     Capture.c                                                     *
*                                                                             *
*   Date:       05/29/2003                                                    *
*                                                                             *
*   Copyright (c) 2003 Goran Devic                                            *
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

        This module contains code that implement capture interface.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/29/03   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <errno.h>                      // Include errno
#include <stdlib.h>                     // Include standard library
#include <string.h>                     // Include strings header file
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#ifndef WIN32
#include <unistd.h>                     // Include standard UNIX header file
#include <sys/ioctl.h>                  // Include ioctl header file
#define stricmp     strcasecmp          // Weird gnu c call..
#else // WIN32
#include <io.h>
#include <malloc.h>
#endif // WIN32

#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos

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
*   void OptCapture(char *pStr)                                               *
*                                                                             *
*******************************************************************************
*
*   Calls the capture interface and saves the result
*
******************************************************************************/
void OptCapture(char *pStr)
{
    int hIce;                           // Handle to the module file
    int size;                           // Size of the buffer
    char name[MAX_MODULE_NAME];         // Module name
    char *p;                            // Memory block pointer
    int status;                         // IOCTL return status
    int fd;                             // Output file descriptor

    // Open linice device for reading and writing
    hIce = open("/dev/"DEVICE_NAME, O_RDWR);
    if( hIce>=0 )
    {
        // Read the size and the module name
        sscanf(pStr, "%d,%s", &size, name);

        printf("\n%d, ""%s""\n", size, name);

        // Allocate memory block to store the result
        p = calloc(1, size);
        if(p)
        {
            // Copy the string parameter into the start of the block
            strcpy(p, name);

            // Call the driver to fill in the block
            status = ioctl(hIce, ICE_IOCTL_CAPTURE, p);
            if( status>=0 )
            {
                // Write the content of the buffer into a file
                strcat(name, ".bin");
                fd = open(name, O_WRONLY | O_CREAT | O_TRUNC);
                if(fd>=0)
                {
                    write(fd, p, size);

                    close(fd);
                }
                else
                    fprintf(stderr, "Unable to open out file %s!\n", name);
            }
            else
                fprintf(stderr, "IOCTL failed (%d)!\n", status);

            // Be a good citizen and free the memory block before exit
            free(p);
        }
        else
            fprintf(stderr, "Unable to allocate %d bytes!\n", size);

        // Close the module handle
        close(hIce);
    }
    else
        fprintf(stderr, "Unable to open Linice module!\n");
}

