/******************************************************************************
*                                                                             *
*   Module:     input.c                                                       *
*                                                                             *
*   Date:       09/04/97                                                      *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

        This module contains control functions for input.
        All input comes through this module.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/04/97   Original                                             Goran Devic *
* 04/26/00   Modified for Linice                                  Goran Devic *
* 09/10/00   Second revision                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
// #include "ioctl.h"                      // Include our own IOCTL numbers
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()
#include "intel.h"                      // Include processor specific stuff


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

#define NEXT_KQUEUE(i) (((i)+1 >= MAX_INPUT_QUEUE)? 0 : (i)+1)

static volatile WCHAR kQueue[ MAX_INPUT_QUEUE ];  // Input code circular queue
static volatile int head = 0, tail = 0;           // Head and Tail of that queue


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void InterruptPoll();

/******************************************************************************
*                                                                             *
*   WCHAR GetKey( IN BOOL fBlock )                                            *
*                                                                             *
*******************************************************************************
*
*   This function returns a key from the input buffer.
*   If a key is not available and the fBlock argument is True, it polls until
*   a key becomes available.  Otherwise, it returns code 0.
*
*   If the polling is used, this function will manage showing and hiding
*   of the cursor carret using callback.
*
*   Where:
*       fBlock is a blocking request.  If set to True, the function polls
*       the input queue until a key is available.
*
*   Returns:
*       0 If no key was available
*       Pseudo-ASCII code of a next key in a queue
*
******************************************************************************/
WCHAR GetKey( BOOL fBlock )
{
    BOOL fCarret;                       // Cursor carret on/off state
    WCHAR c;

    // There are two distinct ways to handle input depending on polling or not

    if( fBlock )
    {
        // We need to poll for the input character. Turn the cursor (carret) on

        if( pOut && pOut->carret )
            (pOut->carret)(TRUE);

        deb.timer[1] = TIMER_CARRET;      // Set the carret blink rate
        fCarret = TRUE;                     // Cursor is currently on

        while( head == tail )
        {
            InterruptPoll();

            // If the blink timer expired, change the state of the carret
            if( deb.timer[1]==0 )
            {
                deb.timer[1] = TIMER_CARRET;  // Reset the timer value

                if( pOut && pOut->carret )
                    (pOut->carret)(fCarret = !fCarret);
            }
        }

        // Turn the cursor (carret) off

        if( pOut && pOut->carret )
            (pOut->carret)(FALSE);
    }
    else
    {
        // Polling is not used.. Return if there is no key in the queue

        if( head==tail )
            return( 0 );
    }

    // Pick up a key from the input queue and return success
    // Get a character from the queue - make it uninterruptible (atomic)

    LocalCLI();

    c = kQueue[ head ];

    head = NEXT_KQUEUE( head );

    LocalSTI();

    return( c );
}


/******************************************************************************
*                                                                             *
*   void PutKey( WCHAR Key )                                                  *
*                                                                             *
*******************************************************************************
*
*   This function serves input module (keyboard, serial) with the enqueue
*   input character functionality.
*
*   Where:
*       Key is the key code to enque
*
******************************************************************************/
void PutKey( WCHAR Key )
{
    int bNext;

    // Store a code into the input queue - make it uninterruptible (atomic)

    LocalCLI();

    bNext = NEXT_KQUEUE( tail );

    if( bNext != head )
    {
        kQueue[ tail ] = Key;
        tail = bNext;
    }
    else
    {
        // Hmmm. Not good.
    }

    LocalSTI();
}

