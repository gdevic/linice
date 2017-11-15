/******************************************************************************
*                                                                             *
*   Module:     errors.c                                                      *
*                                                                             *
*   Date:       03/09/02                                                      *
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

        This module contains code and data strings for error declaration.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/06/02   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "iceface.h"                    // Include iceface module stub protos
#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

typedef struct
{
    UINT errorCode;                     // Error code
    char *pError;                       // Error string

} TERROR;

// To add an error message, add the define in the clib.h, followed by the structure update here.
// The order is not important since the search is by the error number.

static TERROR Errors[] =
{
    { NOERROR,             ""                                           }, // Not used; NOERROR must be 0
    { ERR_SYNTAX,          "Syntax error"                               },
    { ERR_COMMAND,         "Unknown command or macro"                   },
    { ERR_NOT_IMPLEMENTED, "Not yet implemented"                        },
    { ERR_MEMORY,          "Out of memory"                              },

    { ERR_BPDUP,           "Duplicate breakpoint"                       },
    { ERR_BP_TOO_MANY,     "No more breakpoints available"              },
    { ERR_DRUSED,          "Debug register is already being used"       },
    { ERR_DRUSEDUP,        "All debug registers used"                   },
    { ERR_DRINVALID,       "Invalid debug register number"              },

    { ERR_EXP_WHAT,        "Expression?? What expression?"              },

    { ERR_INT_OUTOFMEM,    "Out of memory"                              },
    { ERR_DIV0,            "Division by zero"                           },

    { ERR_BPINT,           "" },
    { ERR_BPIO,            "Invalid IO port number"                     },
    { ERR_BPMWALIGN,       "BPMW address must be on WORD boundary"      },
    { ERR_BPMDALIGN,       "BPMD address must be on DWORD boundary"     },
    { ERR_BPNUM,           "Invalid breakpoint number"                  },
    { ERR_BPLINE,          "Selected line contains no effective code"   },

    { ERR_TOO_COMPLEX,     "Expression too complex"                     },
    { ERR_TOO_BIG_DEC,     "Number (dec) too large"                     },
    { ERR_TOO_BIG_HEX,     "Number (hex) too large"                     },
    { ERR_TOO_BIG_BIN,     "Number (binary) too large"                  },
    { ERR_TOO_BIG_OCT,     "Number (octal) too large"                   },
    { ERR_NOTAPOINTER,     "Expression value is not a pointer"          },
    { ERR_ELEMENTNOTFOUND, "Structure/union element not found"          },
    { ERR_ADDRESS,         "Expecting value, not address"               },
    { ERR_INVALIDOP,       "Invalid operation"                          },
    { ERR_NOTARRAY,        "Subscript of a non array"                   },

    { ERR_SELECTOR,        "Invalid selector 0x%04X"                    },
    { ERR_DATAWIN,         "Invalid data window number"                 },
    { ERR_NOEA,            "No effective address"                       },
    { ERR_INVADDRESS,      "Invalid Address"                            },

    { 0, NULL }             // Terminating entry has NULL pointer to string
};


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

/******************************************************************************
*                                                                             *
*   void PostError(UINT errorCode, UINT errorParam)                           *
*                                                                             *
*******************************************************************************
*
*   Reports an error with the optional error parameter.
*
******************************************************************************/
void PostError(UINT errorCode, UINT errorParam)
{
    // Add a new error only if there was no error logged so far, this we do
    // not to stomp on the first error that might caused all the successive errors
    if( deb.errorCode==NOERROR )
    {
        deb.errorCode = errorCode;
        deb.errorParam = errorParam;
    }
}

/******************************************************************************
*                                                                             *
*   void DebPrintErrorString()                                                *
*                                                                             *
*******************************************************************************
*
*   Prints the error string that is in the variable deb.errorCode
*
******************************************************************************/
void DebPrintErrorString()
{
    int index;                          // Index to loop the error structure

    // Find the string that has the current error number (0 is no error, so we skip it)
    index = 1;

    while( Errors[index].pError && Errors[index].errorCode != deb.errorCode )
        index++;

    if( Errors[index].pError )
    {
        dprinth(1, Errors[index].pError, deb.errorParam);
    }
    else
    {
        dprinth(1, "Unknown error");    // Internal error - bad error number
    }
}

/******************************************************************************
*                                                                             *
*   void kPrint(char *format, ...)                                            *
*                                                                             *
*******************************************************************************
*
*   Kernel print function. We print into a local buffer and then call the
*   real kernel printk via the interface file.
*
******************************************************************************/
void kPrint(char *format, ...)
{
    va_list arg;
    char buf[80];

    va_start( arg, format );
    ivsprintf(buf, format, arg);
    va_end(arg);

    ice_printk(buf);
}
