/******************************************************************************
*                                                                             *
*   Module:     clib.h                                                        *
*                                                                             *
*   Date:       09/11/00                                                      *
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

        This header file includes most of C-functions available as macros
        to a Linux module.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _CLIB_H_
#define _CLIB_H_

#include "ice-keycode.h"                // Include keyboard codes
#include "ctype.h"                      // Include character types definition
#include "string.h"                     // Include standard string declarations
#include "stdarg.h"                     // Include variable argument macros

#include "ice-types.h"                  // Include exended data types

///////////////////////////////////////////////////////////////////////////////
//
// Message indices
//
///////////////////////////////////////////////////////////////////////////////

// Error code defines - to add an error code, add it to the end of this list,
//                      following the definition and string in the errors.c

enum
{
    NOERROR = 0,
    ERR_SYNTAX,
    ERR_COMMAND,
    ERR_NOT_IMPLEMENTED,
    ERR_MEMORY,

    ERR_BPDUP,
    ERR_BP_TOO_MANY,
    ERR_DRUSED,
    ERR_DRUSEDUP,
    ERR_DRINVALID,

    ERR_EXP_WHAT,

    ERR_INT_OUTOFMEM,
    ERR_DIV0,

    ERR_BPINT,
    ERR_BPIO,
    ERR_BPMWALIGN,
    ERR_BPMDALIGN,
    ERR_BPNUM,

    ERR_TOO_COMPLEX,
    ERR_TOO_BIG,
    ERR_NOTAPOINTER,
    ERR_ELEMENTNOTFOUND,
    ERR_ADDRESS,
    ERR_SELECTOR,
    ERR_INVALIDOP,
    ERR_NOTARRAY,

} ERRORS;


///////////////////////////////////////////////////////////////////////////////
//
// Memory access returns these failure codes:
//
//      MEMACCESS_PF  - Accessed page was not present
//      MEMACCESS_GPF - Accessing selector was not valid
//      MEMACCESS_LIM - Accessing offset was larger than the selector limit
//
// These codes have to be (1) greater than 0xFF, so we can distinguish them
// from a successful access and, (2) have low byte set to FF so the functions
// that use them without checking do report a somewhat meaningful byte of 0xFF
//
// These are also hardcoded in i386.asm
//
///////////////////////////////////////////////////////////////////////////////
#define MEMACCESS_PF        0xF0FFFFFF  // Memory access caused a page fault
#define MEMACCESS_GPF       0xF1FFFFFF  // Memory access caused a GP fault
#define MEMACCESS_LIM       0xF2FFFFFF  // Memory access invalid limit

///////////////////////////////////////////////////////////////////////////////
// From stdlib, defining function in printf.c
///////////////////////////////////////////////////////////////////////////////

extern int sprintf( char *str, const char *format, ... );
extern int vsprintf( char *dest, const char *format, va_list arg );


#endif //  _CLIB_H_

