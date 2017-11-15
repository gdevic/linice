/******************************************************************************
*                                                                             *
*   Module:     eval.h                                                        *
*                                                                             *
*   Date:       11/06/2003                                                    *
*                                                                             *
*   Copyright (c) 2003-2005 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   *
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

    BYTE *pData;                        // Address to get the data value:
                        // Literal:  Pointer to its own Data store
                        // Register: Pointer to the actual register in deb.r
                        // Symbol:   Pointer to symbol's memory footprint
                        // Address:  Pointer to its own Data store (offset part)

    TSYMTYPEDEF1 Type;                  // Symbol type descriptor
    WORD wSel;                          // Selector for the address type
    UINT Data;                          // Numerical data value storage space

} TExItem;                              // Define the expression result item

// Define value for the bType field of TExItem structure

#define EXTYPE_EMPTY        0               // (not used)
#define EXTYPE_LITERAL      1               // Literal value:  1, 0x8000000, 'ABCD'
#define EXTYPE_REGISTER     2               // Register value: eax, cs
#define EXTYPE_SYMBOL       3               // Symbol value:   GetHex
#define EXTYPE_ADDRESS      4               // Address value:  40:17, fs:18, &x


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/


#endif //  _EVAL_H_

