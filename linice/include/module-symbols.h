/******************************************************************************
*                                                                             *
*   Module:     ice.h                                                         *
*                                                                             *
*   Date:       04/27/2000                                                    *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        This header file contains global Ice data structures

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 04/27/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _MODULE_SYMBOLS_H_
#define _MODULE_SYMBOLS_H_

/******************************************************************************
*                                                                             *
*   Linux kernel Extern functions                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

/////////////////////////////////////////////////////////////////
// PRIVATE DATA OF A SINGLE SYMBOL TABLE
/////////////////////////////////////////////////////////////////
// Define structure that is

typedef struct
{
    struct TSYMTAB *next;               // Next symbol structure in a list

    char *pStrings;                     // Pointer to strings
    int relocCode;                      // Current code relocated offset (modules only)
    int relocData;                      // Current data relocated offset (modules only)

} TSYMPRIV;



#endif //  _MODULE_SYMBOLS_H_

