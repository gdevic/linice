/******************************************************************************
*                                                                             *
*   Module:     stdarg.h                                                      *
*                                                                             *
*   Date:       04/20/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2005 Goran Devic                                       *
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

        This header file contains defines supporting variable argument lists.
        ANSI C / POSIX stdarg header file.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/20/96   Original                                             Goran Devic *
* 04/09/03   Modified for Linice                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _STDARG_H_
#define _STDARG_H_

/*********************************************************************
*   Include Files
**********************************************************************/

/*********************************************************************
*   Local Variables and Defines
**********************************************************************/

typedef char *va_list[1];

// These are really fun !

#define va_start(ap,pn) ((ap)[0]=(char *)&pn+((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)

#define va_arg(ap,type)     ((ap)[0]+=((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),(*(type *)((ap)[0]-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))

#define va_end(ap)      ((ap)[0]=0,(void)0)


/*********************************************************************
*   Global Functions
**********************************************************************/


#endif // _STDARG_H_
