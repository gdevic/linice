/******************************************************************************
*                                                                             *
*   Module:     History.c                                                     *
*                                                                             *
*   Date:       08/14/02                                                      *
*                                                                             *
*   Copyright (c) 2002 Goran Devic                                            *
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

        This module contains the code to log the linice history text

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/14/02   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <unistd.h>                     // Include standard UNIX header file
#include <sys/ioctl.h>                  // Include ioctl header file

#include "Common.h"                     // Include platform specific set

#include "ice-ioctl.h"                  // Include io control codes
#include "loader.h"                     // Include loader global protos


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char Buf[MAX_STRING+1];          // History buffer line to fill in


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void OptLogHistory(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Fetches all history lines and saves them into a file.
*   Depending on the option, we may append or simply create new file.
*
******************************************************************************/
void OptLogHistory(void)
{
    int hIce;
    int status;
    FILE *fp;                           // Output file structure

    // Open the output file in the required mode (create/truncate or append)

    if( opt & OPT_LOGFILE_APPEND )
    {
        // File needs to be opened to append. If the file does not exist,
        // we will create it
        fp = fopen(pLogfile, "a+");
    }
    else
    {
        // File will be created or truncated if already exist
        fp = fopen(pLogfile, "w");
    }

    if( fp )
    {
        // Send the exit ioctl to the driver so it can decrement possibly
        // multiple usage count to only 1 (useful when loader crashes)
        hIce = open("/dev/"DEVICE_NAME, O_RDONLY);
        if( hIce>=0 )
        {
            // Reset the history buffer reading pointer inside the linice
            // We ignore the return value from this call as it should be 0
            status = ioctl(hIce, ICE_IOCTL_HISBUF_RESET, 0);

            memset(Buf, 0, sizeof(Buf));    // Just in case - zero terminate string(s)

            // Loop and get all the lines available, until we are signalled end
            while( (status = ioctl(hIce, ICE_IOCTL_HISBUF, &Buf))==0 )
            {
                fprintf(fp, "%s\n", Buf);
            }

            close(hIce);
        }
        else
            fprintf(stderr, "Cannot communicate with the Linice module - is Linice loaded?!\n");

        fclose(fp);
    }
    else
        fprintf(stderr, "Error opening output history log file %s!\n", pLogfile);
}
