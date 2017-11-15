/******************************************************************************
*                                                                             *
*   Module:     ice-limits.h                                                  *
*                                                                             *
*   Date:       03/11/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains global program limits.
        If you need to change some parameters, change it here.

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
#ifndef _ICE_LIMITS_H_
#define _ICE_LIMITS_H_

//////////////////////////////////////////////////////////////////////
// Define character device that the driver assumes in /dev/*
//
#define DEVICE_NAME         "ice"       // Define LinIce device name (/dev)

//////////////////////////////////////////////////////////////////////
// Define the maxlimum length (including terminating zero) of:
//  * initialization string
//  * keyboard function define
//  * command line buffer
//
#define MAX_STRING      256


#endif //  _ICE_LIMITS_H_
