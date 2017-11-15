/******************************************************************************
*                                                                             *
*   Module:     debug.h                                                       *
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

#ifdef DBG // ----------------------------------------------------------------

extern int ice_debug_level;

#define INFO(args)                                          \
{                                                           \
    if(ice_debug_level > 0)                                 \
    {                                                       \
        printk("Info: %s,%d: ", __FILE__, __LINE__);        \
        printk##args;                                       \
    }                                                       \
}


#define ERROR(args)                                         \
{                                                           \
    printk("Error: %s,%d: ", __FILE__, __LINE__);           \
    printk##args;                                           \
}


#else // DBG -----------------------------------------------------------------

#define INFO(args)      NULL
#define ERROR(args)     NULL

#endif // DBG ----------------------------------------------------------------


#endif //  _DEBUG_H_

