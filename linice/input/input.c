/******************************************************************************
*                                                                             *
*   Module:     input.c                                                       *
*                                                                             *
*   Date:       09/04/97                                                      *
*                                                                             *
*   Copyright (c) 1997, 2001 Goran Devic                                      *
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

        This module contains control functions for input.
        All input comes through this module.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/04/97   Original                                             Goran Devic *
* 04/26/00   Modified for LinIce                                  Goran Devic *
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

static volatile CHAR kQueue[ MAX_INPUT_QUEUE ];   // Input code circular queue
static volatile int head = 0, tail = 0;           // Head and Tail of that queue


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void InterruptPoll();

/******************************************************************************
*                                                                             *
*   CHAR GetKey( IN BOOL fBlock )                                             *
*                                                                             *
*******************************************************************************
*
*   This function returns a key from the input buffer.
*   If a key is not available and the fBlock argument is True, it polls until
*   a key becomes available.  Otherwise, it returns code 0.
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
CHAR GetKey( BOOL fBlock )
{
    CHAR c;

    // If the blocking is False, return 0 if a key is not available

    if( fBlock==FALSE && head==tail )
        return( 0 );

    // Poll for the input character

    while( head == tail )
    {
        InterruptPoll();
    }

    // Get a character from the queue - make it uninterruptible (atomic)

    CLI();

    c = kQueue[ head ];

    head = NEXT_KQUEUE( head );

    STI();

    return( c );
}


/******************************************************************************
*                                                                             *
*   void PutKey( CHAR Key )                                                   *
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
void PutKey( CHAR Key )
{
    int bNext;

    // Store a code into the input queue - make it uninterruptible (atomic)

    CLI();

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

    STI();
}

