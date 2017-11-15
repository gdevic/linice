/******************************************************************************
*                                                                             *
*   Module:     eval.h                                                        *
*                                                                             *
*   Date:       11/06/2003                                                    *
*                                                                             *
*   Copyright (c) 2003 Goran Devic                                            *
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

        This header file contains expression evaluator structures and exports.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 11/06/03   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _EVAL_H_
#define _EVAL_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

// Define a basic expression result structure

typedef struct
{
    BYTE bType;                         // Type of the value in this record
    void *pData;                        // Address of the data value
    char *pType;                        // Optional pointer to complex type description
    DWORD Data;                         // Numerical data value storage space

} TExItem;                              // Define the expression result item

// Define value for the bType field of TItem structure

enum
{
    EXTYPE_EMPTY,                       // (not used)
    EXTYPE_LITERAL,                     // Literal value:  1, 0x8000000, 'ABCD'
    EXTYPE_REGISTER,                    // Register value: eax, cs
    EXTYPE_SYMBOL,                      // Symbol value:   GetHex
    EXTYPE_ADDRESS,                     // Address value:  40:17, fs:18, &x
};


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/


#endif //  _EVAL_H_

