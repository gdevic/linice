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

#include <ioctl.h>                      // Include io control defines

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
    char keyFn [4][12][MAX_STRING];     // Key assignment for F, SF, AF and CF keys

} TINITPACKET, *PTINITPACKET;


/////////////////////////////////////////////////////////////////
// SYMBOL TABLE STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct tagTSYMTAB
{
    DWORD size;                         // Size of this structure
    struct tagTSYMTAB *next;            // Next table in a list
    char name[16];                      // Name of the symbol table
    DWORD nElem;                        // How many symbols are here

    DWORD pType;        // = &address[nElem]
    DWORD pName;        // = &address[nElem * 2]
    DWORD pNamePool;    // = &address[nElem * 3]

    DWORD address[1];                   // Array of symbol addresses
/*  [0..nElem]      Symbol address
pType:
    [0..nElem]      Symbol type
pName:
    [0..nElem]      Offest into symbols names pool
pNameLookup:
    ['a'..'z','*']  27 offsets into symbols names pool, indexed
pNamePool:
    [ASCIIZ,...]    Pool of symbol names
*/

} TSYMTAB, *PTSYMTAB;


/////////////////////////////////////////////////////////////////
// DEVICE IO CONTROL CODES
/////////////////////////////////////////////////////////////////

#define ICE_IOC_MAGIC       'I'         // Magic IOctl number (8 bits)

#define ICE_IOCTL_INIT       _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x81, sizeof(TINITPACKET))
#define ICE_IOCTL_ADD_SYM    _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x82, sizeof(TSYMTAB))
#define ICE_IOCTL_REMOVE_SYM _IOC(_IOC_WRITE, ICE_IOC_MAGIC, 0x83, sizeof(TSYMTAB))


#endif //  _ICE_IOCTL_H_
