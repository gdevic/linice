/******************************************************************************
*                                                                             *
*   Module:     errors.c                                                      *
*                                                                             *
*   Date:       03/09/02                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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
    int nError;                         // Error code
    char *pError;                       // Error string

} TERROR;

// To add an error message, add the define in the clib.h, followed by the structure update here.
// The order is not important since the search is by the error number.

static TERROR Errors[] =
{
    { NOERROR,             ""                                       }, // Not used; NOERROR must be 0
    { ERR_SYNTAX,          "Syntax error"                           },
    { ERR_COMMAND,         "Unknown command or macro"               },
    { ERR_NOT_IMPLEMENTED, "Not yet implemented"                    },
    { ERR_MEMORY,          "Out of memory"                          },

    { ERR_BPDUP,           "Duplicate breakpoint"                   },
    { ERR_BP_TOO_MANY,     "No more breakpoints available"          },
    { ERR_DRUSED,          "Debug register is already being used"   },
    { ERR_DRUSEDUP,        "All debug registers used"               },
    { ERR_DRINVALID,       "Invalid debug register number"          },

    { ERR_EXP_WHAT,        "Expression?? What expression?"          },

    { ERR_INT_OUTOFMEM,    "Out of memory"                          },
    { ERR_DIV0,            "Division by zero"                       },

    { ERR_BPINT,           "" },
    { ERR_BPIO,            "Invalid IO port number"                 },
    { ERR_BPMWALIGN,       "BPMW address must be on WORD boundary"  },
    { ERR_BPMDALIGN,       "BPMD address must be on DWORD boundary" },
    { ERR_BPNUM,           "Invalid breakpoint number"              },

    { ERR_TOO_COMPLEX,     "Expression too complex"                 },
    { ERR_TOO_BIG,         "Number (hex) too large"                 },
    { ERR_NOTAPOINTER,     "Expression value is not a pointer"      },
    { ERR_ELEMENTNOTFOUND, "Structure/union element not found"      },
    { ERR_ADDRESS,         "Expecting value, not address"           },
    { ERR_SELECTOR,        "Invalid selector value"                 },
    { ERR_INVALIDOP,       "Invalid operation"                      },
    { ERR_NOTARRAY,        "Subscript of a non array"               },

    { -1, "" }
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
*   void DebPrintErrorString()                                                *
*                                                                             *
*******************************************************************************
*
*   Prints the error string that is in the variable deb.error
*
******************************************************************************/
void DebPrintErrorString()
{
    int index;                          // Index to loop the error structure

    // Find the string that has the current error number (0 is no error, so we skip it)
    index = 1;

    while( Errors[index].nError != deb.error && Errors[index].nError > 0 )
        index++;

    if( Errors[index].nError >= 0 )
    {
        dprinth(1, "%s", Errors[index].pError);
    }
    else
    {
        dprinth(1, "Unknown error");    // Internal error - bad error number
    }
}
