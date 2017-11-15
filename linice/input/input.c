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
* 10/26/00   Modified for LinIce                                  Goran Devic *
* 03/10/01   Second revision                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ioctl.h"                      // Include our own IOCTL numbers
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
        ;
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
*   CHAR PutKey( CHAR Key )                                                   *
*                                                                             *
*******************************************************************************
*
*   This function serves input module (keyboard, serial) with the enqueue
*   input character.
*
*   Where:
*       Key is the key code to enque
*
*   Returns:
*
******************************************************************************/
CHAR PutKey( CHAR Key )
{
    int bNext;
    CHAR c;

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

    return( c );
}

