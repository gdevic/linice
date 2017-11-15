/******************************************************************************
*                                                                             *
*   Module:     Common.h                                                      *
*                                                                             *
*   Date:       09/03/00                                                      *
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

        This header file contains the common set of include files and defines
        that are shared between MSVC and GCC build

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

#ifdef WIN32

#include <assert.h>
#define ASSERT assert
#include <io.h>
#include <malloc.h>

//#include <unistd.h>                     // Include standard UNIX header file
#include <string.h>                     // Include strings header file
#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
//#include <sys/ioctl.h>                  // Include ioctl header file
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <malloc.h>                     // Include memory allocation header

#include "linux/elf.h"
#include "stab_gnu.h"

#else // WIN32

#define _POSIX_SOURCE
#define O_BINARY		0				// Win32 needs that one


/* A pointer to a position in a file.  */
/* FIXME:  This should be using off_t from <sys/types.h>.
   For now, try to avoid breaking stuff by not including <sys/types.h> here.
   This will break on systems with 64-bit file offsets (e.g. 4.4BSD).
   Probably the best long-term answer is to avoid using file_ptr AND off_t
   in this header file, and to handle this in the BFD implementation
   rather than in its interface.  */
/* typedef off_t	file_ptr; */
//typedef long int file_ptr;

#define _SYS_TYPES_H

#include <assert.h>
#define ASSERT assert

#include <linux/types.h>

extern int open(const char *path, int oflag, ...);

#include <asm/fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file
#include <unistd.h>                     // Include standard UNIX header file
#include <string.h>                     // Include strings header file
//#include <sys/types.h>                  // Include file operations
#include <sys/stat.h>                   // Include file operations
//#include <sys/ioctl.h>                  // Include ioctl header file
#include <malloc.h>                     // Include memory allocation header

#include "linux/elf.h"
#include "stab_gnu.h"

#endif // WIN32

#include "ice-symbols.h"                // Include symbol file structures
#include "stabs.h"                      // Include STABS defines and structures

// Define internal descriptor for a global symbol
typedef struct
{
    DWORD dwAddress;                    // Start address of a symbol
    DWORD dwEndAddress;                 // End address of a symbol
    WORD wAttribute;                    // Attributes of a symbol
    char Name[MAX_SYMBOL_LEN];          // Symbol canonical name
} TGLOBAL;

extern TGLOBAL *pGlobals;               // Array of global symbols
extern int nGlobals;                    // Number of global symbols
