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

#include <linux/fs.h>                   // Include file operations file

#include "ice-keycode.h"                // Include keyboard codes
#include "ctype.h"                      // Include character types definition
#include "string.h"                     // Include standard string declarations

#include "ice-types.h"                  // Include exended data types

///////////////////////////////////////////////////////////////////////////////
//
// Message indices
//
///////////////////////////////////////////////////////////////////////////////
#define NOERROR                     0   // "NOERROR"
#define ERR_SYNTAX                  1   // "Syntax error"
#define ERR_COMMAND                 2   // "Unknown command or macro"
#define ERR_NOT_IMPLEMENTED         3   // "Not yet implemented"
#define ERR_MEMORY                  4   // "Out of memory"

#define ERR_BPDUP                   5   // "Duplicate breakpoint"
#define ERR_BP_TOO_MANY             6   // "No more breakpoints available"
#define ERR_DRUSED                  7   // "Debug register is already being used"
#define ERR_DRUSEDUP                8   // "All debug registers used"

#define ERR_EXP_WHAT                9   // "Expression?? What expression?"

#define ERR_INT_OUTOFMEM            10  // "Internal error: out of memory"
#define ERR_DIV0                    11  // "Division by zero"

#define MSG_LAST                    12  //  - Last message index -

#define ERR_BPINT                   100 // "Invalid interrupt number"
#define ERR_BPIO                    101 // "Invalid port number"
#define ERR_BPMWALIGN               102 // "BPMW address must be on WORD boundary"
#define ERR_BPMDALIGN               103 // "BPMD address must be on DWORD boundary"
#define ERR_BPNUM                   104 // "Invalid breakpoint number %d"

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


extern BYTE *ice_init_heap( size_t size );
extern void * _kMalloc( BYTE *pHeap, DWORD size );
extern void _kFree( BYTE *pHeap, void *mPtr );

#endif //  _CLIB_H_

