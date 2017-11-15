/******************************************************************************
*                                                                             *
*   Module:     ice-limits.h                                                  *
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

        This header file contains global program limits.
        If you need to change some parameters, change it here.
        However, there is no guarrantee that things will work :-}

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/03/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_LIMITS_H_
#define _ICE_LIMITS_H_

//////////////////////////////////////////////////////////////////////
// Define character device that the driver assumes in /dev/*
//
#define DEVICE_NAME         "ice"       // Define LinIce device name (/dev)

//////////////////////////////////////////////////////////////////////
// Length of the module name string (incl. 0) and internal symbol table name
#define MAX_MODULE_NAME     32

//////////////////////////////////////////////////////////////////////
// Length of a symbol name in characters, variable names
#define MAX_SYMBOL_LEN      64

// Default symbol file signature - hardcoded length in the symbol header struct
#define SYMSIG              "SYM"

// Define current symbol file version - 1.0
#define SYMVER              0x0100

//////////////////////////////////////////////////////////////////////
// Define the maxlimum length (including terminating zero) of:
//  * initialization string
//  * keyboard function define
//  * command line buffer
//  * source line width
//  * print buffer
//
#define MAX_STRING          256

//////////////////////////////////////////////////////////////////////
// Define number of command lines stored as a history that can be
// retrieved during the line edit
//
#define MAX_HISTORY         32

//////////////////////////////////////////////////////////////////////
// Level of allowed recursion within a macro command and within the
// expression evaluation
//
#define MAX_MACRO_RECURSE   8
#define MAX_EVAL_RECURSE    8

//////////////////////////////////////////////////////////////////////
// Define maximum number of breakpoints that we keep
//
#define MAX_BREAKPOINTS     256

//////////////////////////////////////////////////////////////////////
// Define number of bytes per line for data dump command (not tested)
// Must be divisible by 4
#define DATA_BYTES          16

//////////////////////////////////////////////////////////////////////
// Define maximum X and Y size of the output window in all modes
// and devices
#define MAX_X               160
#define MAX_Y               90

//////////////////////////////////////////////////////////////////////
// Define if the serial out connection will use polling method or
// interrupts. As of this writing, interrupts still have some problems
// (ocasional lost character), so the polling is used.
#define SERIAL_POLLING      1

//////////////////////////////////////////////////////////////////////
// Define size of the internal debugger memory heap
#define MAX_HEAP            4096

#endif //  _ICE_LIMITS_H_

