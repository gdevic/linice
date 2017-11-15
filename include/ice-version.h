/******************************************************************************
*                                                                             *
*   Module:     ice-version.h                                                 *
*                                                                             *
*   Date:       10/28/03                                                      *
*                                                                             *
*   Copyright (c) 2003-2004 Goran Devic                                       *
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

        This header file contains version codes for all components.

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
#ifndef _ICE_VERSION_H_
#define _ICE_VERSION_H_

// These defines are processed during the build process, so they may not need
// to change names. At this point I dont anticipate that those 3 version
// numbers should ever be different for a single build, so they are defined
// using the same root defines.

#define MAJOR_VERSION       1   // (Keep this comment for the Perl script)
#define MINOR_VERSION       0   // (Keep this comment for the Perl script)

// Define current Linice version
//
#define LINICEVER           ((MAJOR_VERSION << 8) | (MINOR_VERSION))

// Define current symbol file version
//
#define SYMVER              ((MAJOR_VERSION << 8) | (MINOR_VERSION))

// Define current linsym program version
//
#define LINSYMVER           ((MAJOR_VERSION << 8) | (MINOR_VERSION))


#endif //  _ICE_VERSION_H_

