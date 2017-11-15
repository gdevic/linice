/******************************************************************************
*
*   Module:     input.c
*
*   Date:       08/03/96
*
*   Copyright (c) 1996-2001 Goran Devic                                    
*                                                            
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

		This module contains control functions for input

	TODO:
		Instead of cli/sti, use some sort of semaphores

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/03/96   1.00  Original                                        Goran Devic
* 10/26/00         Modified for LinIce                             Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

/******************************************************************************
*   Global Variables
******************************************************************************/

// We define character type as a 16-bit unsigned int so we can pack
// shift state with it in the top bits

typedef unsigned short int CHAR;

#define NEXT_KQUEUE(i) (((i)+1 >= MAX_INPUT)? 0 : (i)+1)

static volatile CHAR kQueue[ MAX_INPUT ];   // Input code circular queue
static volatile int head = 0, tail = 0;     // Head and Tail of that queue


/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

/******************************************************************************
*   Functions
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

    while( head == tail ) {;}

    // Get a character from the queue - make it uninterruptible (atomic)

    DisableInterrupts();

    c = kQueue[ head ];

    head = NEXT_KQUEUE( head );

    EnableInterrupts();

    return( c );
}


/******************************************************************************
*                                                                             *
*   CHAR PutKey( CHAR Key )                                                   *
*                                                                             *
*******************************************************************************
*
*	This function serves input module (keyboard, serial) with the enqueue
*	input character.
*
*   Where:
*		Key is the key code to enque
*
*   Returns:
*
******************************************************************************/
CHAR PutKey( CHAR Key )
{
    CHAR c;

    // Store a code into the input queue - make it uninterruptible (atomic)

    DisableInterrupts();

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

    EnableInterrupts();

    return( c );
}

