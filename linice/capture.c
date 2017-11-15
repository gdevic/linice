/******************************************************************************
*                                                                             *
*   Module:     capture.c                                                     *
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

        This module contains code to capture linice/kernel state for the use
        with the simulator and as a service to linsym.

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

#include "module-header.h"              // Include types commonly defined for a module

#include "ice-ioctl.h"                  // Include our own IOCTL numbers
#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()
#include "errno.h"                      // Include kernel error numbers
#include "ioctl.h"                      // Include IO control macros

/******************************************************************************
*                                                                             *
*   External functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL FindModule(const char *name, TMODULE *pMod);

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
*   int Capture(void *p)                                                      *
*                                                                             *
*******************************************************************************
*
*   Gets a buffer that only contains the module image to be captured, and
*   copies it into that same buffer.
*
******************************************************************************/
int Capture(void *p)
{
    TMODULE Mod;                        // Module structure
    char name[MAX_STRING];              // Module name (from user)

    // Get the module name
    ice_copy_from_user(name, p, MAX_STRING);

    dprinth(1, "Capture: %s", name);

    if( FindModule(name, &Mod) )
    {
        // Module was found. Copy the data: TMODULE info and the module itself

        ice_copy_to_user(p, &Mod, sizeof(TMODULE));
        ice_copy_to_user((char *)p+sizeof(TMODULE), Mod.pmodule, Mod.size);
    }
    else
    {
        // We did not find a module with that name. Return failure:
        return( -EFAULT );
    }

    return( 0 );
}
