/******************************************************************************
*                                                                             *
*   Module:     Translate.c                                                   *
*                                                                             *
*   Date:       07/22/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2005 Goran Devic                                       *
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
