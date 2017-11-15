/******************************************************************************
*                                                                             *
*   Module:     Convert.c                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/4/97                                                       *
*                                                                             *
*   Copyright (c) 1997, 2000 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for conversion functions.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/4/97    Original                                             Goran Devic *
* 10/26/00   Modified for LinIce                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

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


static char str[32];
static char digits[36] = "0123456789abcdefghijklmnopqrstuvwxyz";


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

// OK stuck assert here for now...

void ice_assert(char * file, int line)
{
    printk("<1>ASSERTION FAILED: %s LINE %d\n", file, line);
}    


/******************************************************************************
*                                                                             *
*   unsigned long _strtoul( char *ptr, char **endptr, int base, int _signed ) *
*                                                                             *
*******************************************************************************
*
*   Internal function - string to unsigned long
*
*   Where:
*       ptr is the address of the string
#       endptr is the address of a variable to strore the first illegal char
#       base is the number base
#       _signed is 1 if a number suppose to be signed
*
*   Returns:
*       long int as number
*
******************************************************************************/
unsigned long _strtoul( char *ptr, char **endptr, int base, int _signed )
{
    unsigned long num = 0L;
    int minus = 0;
    int overflow = 0;
    int digit;

    // Skip the leading spaces

    while( isspace(*ptr) ) ptr++;

    // Fetch the optional leading sign character

    if( *ptr=='+' ) ptr++;
    else
    if( *ptr=='-' ) minus = 1, ptr++;

    // If base is zero, deduce the base from a number using standard
    // rules of "C" and skip over these characters

    if( base==0 )
    {
        if( *ptr=='0' )
        {
            if( (*++ptr=='x') || (*ptr=='X') )
                base = 16, ptr++;
            else
                base = 8;
        }
        else
            base = 10;
    }
    else    // If the base is hex, we can remove leading characters
    if( (base==16) && (*ptr=='0') && (tolower(*++ptr)=='x') )
        ptr++;

    // Base must be assigned at this point

    if( (base>=2) && (base<=36) )
    {
        while( *ptr )
        {
            // Check that the digit is inside the base set

            digit = strchr(digits, tolower(*ptr)) - digits;

            if( (digit>=0) && (digit<base) )
            {
                // Check for overflow

                if( num > (ULONG_MAX - digit)/base ) overflow = 1;

                num *= base;
                num += digit;
                ptr++;
            }
            else
                break;
        }

        // If we are converting a signed number, we need to check for
        // overflow now

        if( !overflow && _signed )
        {
            if( (minus && (num > -(unsigned long)LONG_MIN))
            || (!minus && (num > LONG_MAX)) )
               overflow = 1;
        }

        if( overflow )
        {
            errno = ERANGE;
            if( _signed )
            {
                if( minus )
                    num = LONG_MIN;
                else
                    num = LONG_MAX;
            }
            else
                num = LONG_MAX;
        }
        else
            if( minus ) num = -num;
    }
    else
        num = 0, errno = EDOM;

    // Store the end pointer if allowed

    if( endptr != NULL )  *endptr = ptr;

    return( num );
}


/******************************************************************************
*                                                                             *
*   unsigned long strtoul( const char *ptr, char **endptr, int base )         *
*                                                                             *
*******************************************************************************
*
*   Convert a string to unsigned long
*
*   Where:
*       ptr is the address of the string
#       endptr is the address of a variable to strore the first illegal char
#       base is the number base
*
*   Returns:
*       unsigned long int as number
*
******************************************************************************/
unsigned long strtoul( const char *ptr, char **endptr, int base )
{
    return (unsigned long) _strtoul( (char *)ptr, endptr, base, 0 );
}


/******************************************************************************
*                                                                             *
*   long strtol( const char *ptr, char **endptr, int base )                   *
*                                                                             *
*******************************************************************************
*
*   Convert a string to signed long
*
*   Where:
*       ptr is the address of the string
#       endptr is the address of a variable to strore the first illegal char
#       base is the number base
*
*   Returns:
*       signed long int as number
*
******************************************************************************/
long strtol( const char *ptr, char **endptr, int base )
{
    return (signed long) _strtoul( (char *)ptr, endptr, base, 1 );
}


/******************************************************************************
*                                                                             *
*   char *ltoa( long int num, char *buffer, int radix )                       *
*                                                                             *
*******************************************************************************
*
*   Convert long integer to ascii string.
*
*   Where:
*       num is the long number to convert
#       buffer is the address of a buffer to store the result
#       radix is the number radix
*
*   Returns:
*       address of a buffer with the converted number
*
******************************************************************************/
char *ltoa( long int num, char *buffer, int radix )
{
    unsigned long number;
    int i = 0, j = 0;
    int r;

    // Check for the special case of 0 and some sanity checking

    if( (num==0) || (radix<=1) || (radix>36) )
    {
        buffer[i++] = '0';
    }
    else
    {
        // Check for negative number with the radix of 10

        if( (num<0) && (radix==10) )
        {
           buffer[i++] = '-';
           number = -num;
        }
        else
            number = num;

        while( number >= radix )
        {
            r = number % radix;
            number /= radix;
            str[j++] = digits[r];
        }

        str[j] = digits[number];

        // Copy the buffer `str' backwards to the destination buffer

        for(;j>=0;j--) buffer[i++] = str[j];
    }

    buffer[i] = '\0';

    return( buffer );
}


/******************************************************************************
*                                                                             *
*   char *itoa( long int num, char *buffer, int radix )                       *
*                                                                             *
*******************************************************************************
*
*   Convert integer to ascii string.
*
*   Where:
*       num is the number to convert
#       buffer is the address of a buffer to store the result
#       radix is the number radix
*
*   Returns:
*       address of a buffer with the converted number
*
******************************************************************************/
char *itoa( int num, char *buffer, int radix )
{
    unsigned number;
    int i = 0, j = 0;
    int r;

    // Check for the special case of 0 and some sanity checking

    if( (num==0) || (radix<=1) || (radix>36) )
    {
        buffer[i++] = '0';
    }
    else
    {
        // Check for negative number with the radix of 10

        if( (num<0) && (radix==10) )
        {
           buffer[i++] = '-';
           number = -num;
        }
        else
            number = num;

        while( number >= radix )
        {
            r = number % radix;
            number /= radix;
            str[j++] = digits[r];
        }

        str[j] = digits[number];

        // Copy the buffer `str' backwards to the destination buffer

        for(;j>=0;j--) buffer[i++] = str[j];
    }

    buffer[i] = '\0';

    return( buffer );
}


/******************************************************************************
*                                                                             *
*   long int atol( const char *ptr )                                          *
*                                                                             *
*******************************************************************************
*
*   Convert ascii string to long number.
*
*   Where:
*       ptr is the address of a buffer containing a number
*
*   Returns:
*       long representation of a number
*
******************************************************************************/
long int atol( const char *ptr )
{
    long int num = 0;
    int minus = 0;

    // Skip the leading spaces

    while( isspace(*ptr) ) ptr++;

    // Check for optional '+' and '-' sign; plus is ignored (default)

    if( *ptr=='+' )
        ptr++;
    else
    if( *ptr=='-' )
        minus = 1, ptr++;

    // Scan the string of numbers

    while( isdigit(*ptr) )
    {
        num *= 10;
        num += *ptr - '0';
        ptr++;
    }

    if( minus ) return( -num );

    return( num );
}


/******************************************************************************
*                                                                             *
*   int atoi( const char *ptr )                                               *
*                                                                             *
*******************************************************************************
*
*   Convert ascii string to integer number.
*
*   Where:
*       ptr is the address of a buffer containing a number
*
*   Returns:
*       integer representation of a number
*
******************************************************************************/
int atoi( const char *ptr )
{
    return( (int)atol(ptr) );
}

