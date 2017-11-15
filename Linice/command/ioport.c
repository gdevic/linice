/******************************************************************************
*                                                                             *
*   Module:     ioport.c                                                      *
*                                                                             *
*   Date:       10/30/00                                                      *
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

        IO port commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/30/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

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

static char *sErr[] = {
    "",                             // No error
    "Syntax error",
    "Parameters required",
};

#define ERR_OK                              0x00
#define ERR_SYNTAX_ERROR                    0x01
#define ERR_PARAMETERS_REQUIRED             0x02

#define ERROR_PRINT(err)  if( err ) dprinth(1, sErr[err])


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void Outpb(DWORD port, DWORD value);
extern void Outpw(DWORD port, DWORD value);
extern void Outpd(DWORD port, DWORD value);

extern DWORD Inpb(DWORD port);
extern DWORD Inpw(DWORD port);
extern DWORD Inpd(DWORD port);

/******************************************************************************
*                                                                             *
*   BOOL cmdOut(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Output BYTE, WORD or DWORD to a port
*
******************************************************************************/
BOOL cmdOut(char *args, int subClass)
{
    DWORD port, value, err = ERR_OK;

    if( Expression(&port, args, &args) )
    {
        if( Expression(&value, args, &args) )
        {
            if( (port <= 0xFFFF) && *args==0 )
            {
                switch( subClass )
                {
                    case 1:
                        if( value <= 0xFF )
                            Outpb(port, value);
                        else
                            err = ERR_SYNTAX_ERROR;
                        break;

                    case 2:
                        if( value <= 0xFFFF )
                            Outpw(port, value);
                        else
                            err = ERR_SYNTAX_ERROR;
                        break;

                    case 4:
                            Outpd(port, value);
                        break;
                }
            }
            else
                err = ERR_SYNTAX_ERROR;
        }
        else
            err = ERR_PARAMETERS_REQUIRED;
    }
    else
        err = ERR_PARAMETERS_REQUIRED;

    ERROR_PRINT(err);

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdIn(char *args, int subClass)                                      *
*                                                                             *
*******************************************************************************
*
*   Input BYTE, WORD or DWORD from a port
*
******************************************************************************/
BOOL cmdIn(char *args, int subClass)
{
    DWORD port, err = ERR_OK;

    if( Expression(&port, args, &args) )
    {
        if( (port <= 0xFFFF) && *args==0 )
        {
            switch( subClass )
            {
                case 1:
                    dprinth(1, "%02X", Inpb(port));
                    break;

                case 2:
                    dprinth(1, "%04X", Inpw(port));
                    break;

                case 4:
                    dprinth(1, "%08X", Inpd(port));
                    break;
            }
        }
        else
            err = ERR_SYNTAX_ERROR;
    }
    else
        err = ERR_PARAMETERS_REQUIRED;

    ERROR_PRINT(err);

    return( TRUE );
}

