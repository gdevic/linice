/******************************************************************************
*                                                                             *
*   Module:     Translate.c                                                   *
*                                                                             *
*   Date:       07/22/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2004 Goran Devic                                       *
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

        This module contains the Windows executable version of the linsym.
        Only the translation of symbols is supported.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 07/22/02   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>

#include "ice-version.h"                // Include version file
#include "ice-symbols.h"                // Include symbol file structures
#include "loader.h"                     // Include global protos

BOOL OptInstall(char *pSystemMap)
{
    fprintf(stderr, "This option is available only in Linux version of LINSYM.\n\n");
    return(TRUE);
}

void OptUninstall()
{
    fprintf(stderr, "This option is available only in Linux version of LINSYM.\n\n");
}

void OptLogHistory(void)
{
    fprintf(stderr, "This option is available only in Linux version of LINSYM.\n\n");
}

void OptAddSymbolTable(char *sName)
{
    fprintf(stderr, "This option is available only in Linux version of LINSYM.\n\n");
}

void OptRemoveSymbolTable(char *sName)
{
    fprintf(stderr, "This option is available only in Linux version of LINSYM.\n\n");
}
