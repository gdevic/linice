/******************************************************************************
*                                                                             *
*   Module:     messages.c                                                    *
*                                                                             *
*   Date:       05/04/02                                                      *
*                                                                             *
*   Copyright (c) 2002-2004 Goran Devic                                       *
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

        Code for custom messages. It includes the automatically generated
        file "messages.h" that contains the encrypted custom messages which
        are periodically displayed on the linice history line.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/04/02   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "messages.h"                   // Include custom messages

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

#define AVERAGE_HIT         50          // Average hit every so many calls...

// This is the encoding key:
static BYTE key[13] = { 0xDF, 0x38, 0x82, 0xF5, 0x5A, 0xA9, 0x90, 0x16, 0xB2, 0x27, 0xCC, 0x63, 0x7E };
// The same key exist in the tools/messages/messages.cpp project file

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern DWORD GetRdtsc(BYTE *buffer8);

/******************************************************************************
*                                                                             *
*   void DisplayMessage(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Display a custom message once in a while.
*
******************************************************************************/
void DisplayMessage(void)
{
    UINT nMessage, nPostpone;
    static BYTE rdtsc[8];               // Buffer to keep rdtsc value
    DWORD seed;                         // Current rnd seed
    char message[128];                  // Each line does not exceed 80 characters, but we wrap
    int i;                              // Index into message buffer
    int nPos;                           // Message reader position
    int nLen;                           // String length encoded in the first 2 bytes of a string
    BOOL fSignature = TRUE;             // Append signature

    // Use a random generator to see if we ought to print a random message...
    seed = GetRdtsc(rdtsc);
    seed = seed >> 8;

    nPostpone = seed % AVERAGE_HIT;     // Change this constant!!
    nMessage = (seed >> 8) % MAX_MESSAGES;

    if( nPostpone==1 )
    {
        // Print an empty line as a header to a message - looks better
        dprinth(1, "");
WriteSignature:
        // Read the total string length
        nLen = (BYTE)messages[nMessage][0] + (BYTE)messages[nMessage][1] * 256;

        // Print the designated message number, it can take multiple lines..
        nPos = 0;
        i = 0;
        while(nPos<nLen)
        {
            // Store a character at a time into the output buffer
            message[i] = messages[nMessage][nPos+2] ^ key[nPos % 13];

            i++;
            nPos++;

            // If we exceeded the line width, backtrack until we find the space,
            // then print until that line, and readjust the counters.
            // This we do so the words wrap nicely.
            if(i==79)
            {
                // Find the last space
                while(message[i]!=' ') i--, nPos--;
                nPos++;                 // Skip that space for continuation

                // Flush the message
                message[i] = 0;
                dprinth(1, "%s", message);
                i = 0;

                // The continuation should ignore leading spaces as well
                while((messages[nMessage][nPos+2] ^ key[nPos % 13])==' ') nPos++;
            }
        }
        // Flush the message
        message[i] = 0;
        dprinth(1, "%s", message);

        // Append the signature and exit if we already wrote signature
        if(fSignature)
        {
            fSignature = FALSE;
            nMessage = MAX_MESSAGES;
            goto WriteSignature;
        }
    }
}

