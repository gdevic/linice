/******************************************************************************
*                                                                             *
*   Module:     patch.c                                                       *
*                                                                             *
*   Date:       06/26/04                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This module contains the code to search the running kernel for
        the function signatures that are to be hooked.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 06/26/04   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Define the patch stream opcodes:
// [7:5] - opcode
// [4:0] - generic count of bytes for a max of 31 bytes

#define PATCH_LIST      0x20            // List of opcodes (n & 0x1F) max 31
#define PATCH_SKIP      0x40            // Skip # of bytes (n & 0x1F) max 31
#define PATCH_END       0xE0            // End of the stream

// Define the patch structure
typedef struct
{
    char *pName;                        // Name of the patch
    BYTE *b;                            // Stream of code tokens

} TPATCH;


//----------------------------------------------------------------------------
// kbd patch
//----------------------------------------------------------------------------

static BYTE kbd00[] = {
    PATCH_LIST | 8,
    0x56,                               // push  esi
    0x53,                               // push  ebx
    0x83, 0xEC, 0x0C,                   // sub   esp, 0C
    0xE4, 0x64,                         // in    al, 64
    0xBE,                               // mov   esi, ...
    PATCH_END
};

static BYTE kbd01[] = {
    PATCH_LIST | 2,
    0x56,                               // push  esi
    0x53,                               // push  ebx
    PATCH_END
};


static TPATCH Patch[] =
{
    { "kbd", kbd00 },
    { NULL, }
};

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern int ice_get_register_chrdev();


DWORD SearchPatch(BYTE *p)
{
    BYTE *pStream = p;                  // Save the start of the stream
    BYTE *pKernel, *pByte;              // Address to search and a temp
    int len = 0x400000;                 // Search so many bytes
    BYTE token, count;                  // Search token and count
    DWORD dwCheck = 0;                  // Checkpoint value

    // Estimate the starting address of the Linux kernel

    pKernel = (BYTE *)(ice_get_register_chrdev() & ~0xFFFFF);
INFO("Start %08X\n", pKernel);

    // Process search token stream until we run out of the kernel or reach
    // the end of the stream (return within)
    while( len>0 )
    {
        token = (*p) & 0xE0;            // Use only the token portion
        count = (*p) & 0x1F;            // Store the count portion

        switch( token )
        {
            case PATCH_LIST:            // Match the list of bytes
            {
                // Find the first byte
                if( (pByte = memchr(pKernel, *(p+1), len)) )
                {
                    len -= pByte - pKernel;
                    pKernel = pByte;

                    if( memcmp(pKernel, p+1, count)==0 )
                    {
                        dwCheck = pKernel;  // Check the start of the array

                        // Found the matching memory array
                        // Advance the stream pointer
                        p += count + 1;
INFO(">%08X %08X\n", dwCheck, pKernel);
INFO("%X\n", *p);
                        break;
                    }
                }
                else
                {
                    // Did not find a byte within the entire array
                    return( 0 );
                }

                // Did not match the memory array
                // Reset the search stream pointer
                p = pStream;
                dwCheck = 0;            // Reset the checkpoint
                pKernel++;              // Skip the BYTE that matched
            }
            break;

            case PATCH_SKIP:            // Skip a number of bytes
                pKernel += count;       // Advance the pointer
                len     -= count;       // Decrement the total len
                p++;                    // Advance the stream pointer
            break;

            case PATCH_END:             // Successfully reached the end
                return( dwCheck );

            default:
                return( 0 );            // Error in the stream
        }
    }

    // If we did not reach the end of the search stream, we did not find it

    return( 0 );
}

/******************************************************************************
*                                                                             *
*   DWORD FindPatch(char *pName)                                              *
*                                                                             *
*******************************************************************************
*
*   Search the kernel image for the code patch of a given name.
*
*   Where:
*       pName is the name of the code patch to search for
*
*   Returns:
*       0 if the code patch could not be found
*       Address of the code patch
*
******************************************************************************/
DWORD FindPatch(char *pName)
{
    DWORD dwAddress = 0;                // Assume 0
    TPATCH *pPatch = Patch;             // Start at the patch stream 0
    BOOL fFound = FALSE;                // Set 'not found'
    BOOL fValid = TRUE;                 // So far so good...

    while( pPatch->pName )
    {
        // Only search patch of a given name
        if( !strcmp(pName, pPatch->pName) )
        {
            if( (dwAddress = SearchPatch(pPatch->b)) )
            {
                INFO("Patch: %s -> %08X\n", pName, dwAddress);

                if( fFound )
                {
                    INFO("Patch: Multiple locations found!\n");
                    fValid = FALSE;         // Invalidate if multiple locations are found
                }
                else
                {
                    fFound = TRUE;
                }
            }
        }

        pPatch++;
    }

    // Only return the address if we found a single location that match
    if( fValid )
        return( dwAddress );

    return( 0 );
}
