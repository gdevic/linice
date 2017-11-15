/******************************************************************************
*                                                                             *
*   Module:     ice-ioctl.h                                                   *
*                                                                             *
*   Date:       03/03/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains module IOCTL numbers and shared data
        structures

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/03/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_IOCTL_H_
#define _ICE_IOCTL_H_

#ifdef __KERNEL__
#include <ioctl.h>                      // Include io control defines
#else
#include <sys/ioctl.h>                  // Include io control defines
#endif // __KERNEL__

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

typedef struct
{
    int nSize;                          // Size of this structure in bytes
    int fLowercase;
    int nSymbolSize;
    int nHistorySize;                   // Size of the history buffer

    char sInit[MAX_STRING];             // Init string
    char keyFn [4 * 12][MAX_STRING];    // Key assignment for F, SF, AF and CF keys

} TINITPACKET, *PTINITPACKET;


/////////////////////////////////////////////////////////////////
// SYMBOL TABLE STRUCTURE
/////////////////////////////////////////////////////////////////

#define MAGIC_SYMTAB        0x33445566  // Magic symbol ID number
#define MAX_MODULE_NAME     16          // Length of the module name string (incl. 0)
#define MAX_SYMBOL_NAME     32          // Length of a symbol name string (incl. 0)

typedef struct
{
    DWORD dwAddress;                    // Symbol address
    DWORD Type;                         // Symbol type code
    char name[MAX_SYMBOL_NAME];         // ASCIIZ Symbol name
} TSYM, *PTSYM;

typedef struct tagTSYMTAB
{
    DWORD magic;                        // Magic ID number
    DWORD size;                         // Size of the whole structure
    char name[MAX_MODULE_NAME];         // Name of the symbol table
    DWORD nElem;                        // How many symbols are defined here
    DWORD Flags;                        // Flags

    struct tagTSYMTAB *next;            // Next table in a list

    TSYM sym[1];                        // Symbol array
/*  [0..nElem]      Array of symbol structures */

} TSYMTAB, *PTSYMTAB;

// If symbols are sorted by address, we will use binary search,
// otherwise, we are using linear search
#define SYMF_SORTED     0x0001          // Symbols are sorted by address


/////////////////////////////////////////////////////////////////
// DEVICE IO CONTROL CODES
/////////////////////////////////////////////////////////////////

#define ICE_IOC_MAGIC       'I'         // Magic IOctl number (8 bits)

#define ICE_IOCTL_INIT       _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x81, sizeof(TINITPACKET))
#define ICE_IOCTL_ADD_SYM    _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x82, sizeof(TSYMTAB))
#define ICE_IOCTL_REMOVE_SYM _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x83, MAX_MODULE_NAME)


#endif //  _ICE_IOCTL_H_
