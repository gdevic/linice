/******************************************************************************
*                                                                             *
*   Module:     stdarg.h                                                      *
*                                                                             *
*   Date:       04/20/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2004 Goran Devic                                       *
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
