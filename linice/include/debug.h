/******************************************************************************
*                                                                             *
*   Module:     debug.h                                                       *
*                                                                             *
*   Date:       09/03/00                                                      *
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

        This header file defines debug functions

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
#ifndef _DEBUG_H_
#define _DEBUG_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// We define this differently for MSVC and GCC. The former is used with the
// SIM define.

#ifdef DBG // ----------------------------------------------------------------
#ifndef SIM

extern int ice_debug_level;
extern void kPrint(char *p, ...);

#define INFO(args...)                                       \
{                                                           \
    if(ice_debug_level > 0)                                 \
    {                                                       \
        kPrint("Info: %s,%d: ", __FILE__, __LINE__);        \
        kPrint( args );                                     \
    }                                                       \
}


#define ERROR(args...)                                      \
{                                                           \
    kPrint("Error: %s,%d: ", __FILE__, __LINE__);           \
    kPrint(args);                                           \
}

#else // SIM

#define INFO(args)                                          \
    kPrint("Info: %s,%d: ", __FILE__, __LINE__);            \
    kPrint( ##args );                                       \

#define ERROR(args)                                         \
    kPrint("Error: %s,%d: ", __FILE__, __LINE__);           \
    kPrint( ##args );                                       \

#endif // SIM

#else // DBG -----------------------------------------------------------------

#ifndef SIM

#define INFO(args...)
#define ERROR(args...)

#else // SIM

#define INFO(args...)      do{;}while(0)
#define ERROR(args...)     do{;}while(0)

#endif // SIM

#endif // DBG ----------------------------------------------------------------


#endif //  _DEBUG_H_

