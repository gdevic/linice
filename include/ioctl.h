/******************************************************************************
*                                                                             *
*   Module:     ioctl.h                                                       *
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
#ifndef _IOCTL_H_
#define _IOCTL_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_STRING      256

typedef struct
{
    int nSize;                          // Size of this structure in bytes
    int fLowercase;
    int nSymbolSize;
    int nHistorySize;

    char sInit[MAX_STRING];             // Init string
    char keyFn[12][MAX_STRING];         // Key assignment for F1..F12 keys
    char keySFn[12][MAX_STRING];        // Key assignment for SF1..SF12 keys
    char keyAFn[12][MAX_STRING];        // Key assignment for AF1..AF12 keys
    char keyCFn[12][MAX_STRING];        // Key assignment for CF1..CF12 keys

} TINITPACKET, *PTINITPACKET;


#define DEVICE_NAME         "ice"       // Define LinIce device name (/dev)

#define ICE_IOCTL_MAGIC 'i'

#define ICE_IOCTL_LOAD          _IO(ICE_IOCTL_MAGIC,0)
#define ICE_IOCTL_UNLOAD        _IO(ICE_IOCTL_MAGIC,1)
#define ICE_IOCTL_BREAK         _IO(ICE_IOCTL_MAGIC,2)
#define ICE_IOCTL_LOAD_SYM      _IO(ICE_IOCTL_MAGIC,3)
#define ICE_IOCTL_UNLOAD_SYM    _IO(ICE_IOCTL_MAGIC,4)



#endif //  _IOCTL_H_
