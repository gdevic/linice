/******************************************************************************
*                                                                             *
*   Module:     ice-version.h                                                 *
*                                                                             *
*   Date:       10/28/03                                                      *
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

#define MAJOR_VERSION       2   // (Keep this comment for the Perl script)
#define MINOR_VERSION       6   // (Keep this comment for the Perl script)

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

