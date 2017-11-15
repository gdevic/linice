/******************************************************************************
*                                                                             *
*   Module:     stabs.h                                                       *
*                                                                             *
*   Date:       06/10/01                                                      *
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

        Define wrappers for ELF stabs.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 06/10/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _STABS_H_
#define _STABS_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Define so we can generate enums
#define __define_stab(NAME, CODE, STRING) NAME=CODE,

enum __stab_debug_code
{
#include "stab.def"
LAST_UNUSED_STAB_CODE
};

#undef __define_stab

//////////////////////////////////////////////////////////////////////////////

typedef struct
{
    int code;
    char *name;
} TSTABNAME;

// Define so we can generate structures of stab codes
#define __define_stab(NAME, CODE, STRING)  { CODE, STRING },

TSTABNAME StabName[] = {
#include "stab.def"
{ 0, NULL }
};


#endif // _STABS_H_