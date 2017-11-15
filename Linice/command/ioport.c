/******************************************************************************
*                                                                             *
*   Module:     ioport.c                                                      *
*                                                                             *
*   Date:       10/30/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

