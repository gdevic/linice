/******************************************************************************
*                                                                             *
*   Module:     ice-limits.h                                                  *
*                                                                             *
*   Date:       09/11/00                                                      *
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
#define DEVICE_NAME         "ice"       // Define Linice device name (/dev)

//////////////////////////////////////////////////////////////////////
// Length of the module name string (incl. 0) and internal symbol table name
//
#define MAX_MODULE_NAME     32

//////////////////////////////////////////////////////////////////////
// Length of a symbol name in characters, variable names
//
#define MAX_SYMBOL_LEN      64

// Default symbol file signature - hardcoded length in the symbol header struct
//
#define SYMSIG              "SYM"

//////////////////////////////////////////////////////////////////////
// Define the maxlimum length (including terminating zero) of:
//  * initialization string
//  * keyboard function define
//  * command line buffer
//  * source line width
//  * print buffer
//
#define MAX_STRING          128

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
// Define number of bytes per line for data dump command.
// This is hard-coded at 16 since data edit functions depend on that.
//
#define DATA_BYTES          16

//////////////////////////////////////////////////////////////////////
// Define maximum X and Y size of the output window in all modes and devices
//
#define MAX_OUTPUT_SIZEX    160         // ie. WIDTH
#define MAX_OUTPUT_SIZEY    90          // ie. LINES

//////////////////////////////////////////////////////////////////////
// Moving the X-Window Linice frame by this number of pixels
//
#define XWIN_MOVE           16

// Define the maximum XWindows backstore buffer allowed (in bytes)
//
#define MAX_XWIN_BUFFER     1024 * 1024 * 32

//////////////////////////////////////////////////////////////////////
// Number of graphics fonts available:
//  8x8
//  8x14
//  8x16
//
#define MAX_FONTS           3

//////////////////////////////////////////////////////////////////////
// Define if the serial out connection will use polling method or
// interrupts. As of this writing, interrupts still have some problems
// (ocasional lost character), so the polling is used.
//
#define SERIAL_POLLING      1

//////////////////////////////////////////////////////////////////////
// Define size of the internal debugger memory heap
//
#define MAX_HEAP            (128*1024)

//////////////////////////////////////////////////////////////////////
// Timer value for the cursor carret blink rate in graphics modes (out of 100Hz)
//
#define TIMER_CARRET        20

//////////////////////////////////////////////////////////////////////
// Number of IO APIC interrupt redirection registers that we keep
//
#define MAX_IOAPICREG       16

//////////////////////////////////////////////////////////////////////
// Maximum array size to expand; any more elements will be ignored
//
#define MAX_ARRAY_EXPAND    99

//////////////////////////////////////////////////////////////////////
// Number of characters in the keyboard input queue
//
#define MAX_INPUT_QUEUE     32


#endif //  _ICE_LIMITS_H_

