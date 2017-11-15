/******************************************************************************
*                                                                             *
*   Module:     debug.h                                                       *
*                                                                             *
*   Date:       09/03/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
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

