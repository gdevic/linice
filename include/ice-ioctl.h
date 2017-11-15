/******************************************************************************
*                                                                             *
*   Module:     ice-ioctl.h                                                   *
*                                                                             *
*   Date:       09/03/00                                                      *
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

        This header file contains module IOCTL numbers and shared data
        structures.

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
#ifndef _ICE_IOCTL_H_
#define _ICE_IOCTL_H_

#ifndef __KERNEL__
#ifndef WINDOWS
#include <sys/ioctl.h>                  // Include io control defines
#endif // WINDOWS
#endif // __KERNEL__

#ifdef WINDOWS
#define _IOC 0
#define _IOC_WRITE 0
#define ioctl(a,b,c)   printf("IOCTL\n");
#endif // WINDOWS

#include "ice-types.h"                  // Include exended data types
#include "ice-limits.h"                 // Include global program limits

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

/////////////////////////////////////////////////////////////////
// INIT STRUCTURE
/////////////////////////////////////////////////////////////////

// Define the first init packet that linsym sends initially

typedef struct
{
    int nSize;                          // Size of this structure in bytes
    int fLowercase;                     // Initial value of the lowercase variable
    int nSymbolSize;                    // Size of the simbol buffer
    int nDrawSize;                      // Size of the X-display framebuffer
    int nHistorySize;                   // Size of the history buffer
    int nMacros;                        // Number of macros
    int nVars;                          // Number of user variables

    char sInit[MAX_STRING];             // Init string
    char keyFn [4 * 12][MAX_STRING];    // Key assignment for F, SF, AF and CF keys
    char Layout[3][128];                // Keyboard layout override for key & Shift & Alt

} PACKED TINITPACKET, *PTINITPACKET;

// Define X init packet that is sent by the xice helper

typedef struct
{
    DWORD pFrameBuf;                    // Address of the framebuffer
    DWORD dwDrawSize;                   // Size of the X-display framebuffer to allocate
    DWORD xres;                         // X resolution in pixels
    DWORD yres;                         // Y resolution in pixels
    DWORD bpp;                          // BYTES per pixel
    DWORD stride;                       // Display stride
    DWORD redShift;                     // Red component shift in bits
    DWORD greenShift;                   // Green component shift in bits
    DWORD blueShift;                    // Blue component shift in bits
    DWORD redMask;                      // Red mask
    DWORD greenMask;                    // Green mask
    DWORD blueMask;                     // Blue mask
    DWORD redColAdj;                    // Red color adjust value
    DWORD greenColAdj;                  // Green color adjust value
    DWORD blueColAdj;                   // Blue color adjust value

} PACKED TXINITPACKET;


/////////////////////////////////////////////////////////////////
// DEVICE IO CONTROL CODES
/////////////////////////////////////////////////////////////////
//
//  ICE_IOCTL_INIT
//      Sent after the module has been loaded. Send init packet
//      that contains information from the config file and heyboard
//      hook addresses. Linice can not operate without this packet
//      being sent. If loader detects a valid active DGA device,
//      it can also pass XInit structure.
//
//  ICE_IOCTL_EXIT
//      Sent before unloading the module. If the loader crashes,
//      the module usage count will not be 0, and the module will
//      not unload. This call is sent to reset the usage count to 1
//
//  ICE_IOCTL_EXIT_FORCE
//      Used mainly during the development to force unload with
//      module usage count reset back to 0
//
//  ICE_IOCTL_ADD_SYM
//      Sent any time to add a symbol table.
//
//  ICE_IOCTL_REMOVE_SYM
//      Sent any time to remove a symbol table.
//
//  ICE_IOCTL_XDGA
//      Sent by the xice to init or reset output to X linear framebuffer
//
//  ICE_IOCTL_HISBUF_RESET
//      Sent by the linsym before fetching history lines to reset the reader
//
//  ICE_IOCTL_HISBUF
//      Sent by the linsym multiple times to retrieve line by line of the
//      history buffer. When finished, call returns error instead of 0.
//

#define ICE_IOC_MAGIC       'I'         // Magic IOctl number (8 bits)

#define ICE_IOCTL_INIT          _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x81, sizeof(TINITPACKET))
#define ICE_IOCTL_EXIT          _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x82, 0)
#define ICE_IOCTL_EXIT_FORCE    _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x83, 0)
#define ICE_IOCTL_ADD_SYM       _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x84, sizeof(TSYMTAB))
#define ICE_IOCTL_REMOVE_SYM    _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x85, MAX_MODULE_NAME)
#define ICE_IOCTL_XDGA          _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x86, sizeof(TXINITPACKET))
#define ICE_IOCTL_HISBUF_RESET  _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x87, 0)
#define ICE_IOCTL_HISBUF        _IOC(_IOC_READ,  ICE_IOC_MAGIC, 0x88, MAX_STRING)


#endif //  _ICE_IOCTL_H_

