/******************************************************************************
*                                                                             *
*   Module:     printf.c                                                      *
*                                                                             *
*   Date:       02/26/96                                                      *
*                                                                             *
*   Copyright (c) 1996, 2003 Goran Devic                                      *
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

        This module contains the code for the printf() family of functions.

        Note: This file originates with the Yaos project and in this form
        it is a stripped-down version of it lacking a complete set of
        functions.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/28/96   Original                                             Goran Devic *
* 04/09/03   Modified for Linice                                  Goran Devic *
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

#define LEFT_JUSTIFY     0x0001
#define PRINT_SIGN       0x0002
#define PRINT_SPACE      0x0004
#define ZERO_PREFIX      0x0008
#define HEX_UPPERCASE    0x0010
#define NEGATIVE         0x0020
#define SHORT            0x0040

static char buf[10];
static const char hex[]="0123456789abcdef";
static const unsigned int dec[10] = {
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1
};


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

#define STR_APPEND(c)       { if(arg_buffer_size-->0) *str++ = (c); }

/******************************************************************************
*                                                                             *
*   int vsprintf( char *dest, const char *format, va_list arg )               *
*                                                                             *
*******************************************************************************
*
*   This is a printf function that prints to a string.  Character 0 is
#   appended after the last character in the destination (but is not counted).
*
*   Where:
*       dest is the string to print into
#       format is a printf style format
#       arg is the variable argument list
*
*   Returns:
*       number of characters printed
*
******************************************************************************/
int vsprintf( char *dest, const char *format, va_list arg )
{
    char * fmt = (char *)format;
    char c;
    char *str = dest;
    int i, j, k, l;
    int flags;
    int width;
    int nibbles;

    int arg_int;
    unsigned int arg_uint;
    char arg_chr;
    char *arg_str;
    int *arg_pint;
    int arg_buffer_size = 1023;         // Maximum size of the buffer to print


    while( (c = *fmt++) )
    {
        switch( c )
        {

            // New line - send LF and CR characters
            //
            case '\n' :

                STR_APPEND(0x0A);
                STR_APPEND(0x0D);

            break;


            // Format control characters
            //
            case '%' :
            {
                flags = 0;
                width = 0;
                nibbles = 8;

                c = *fmt++;

                // Sequence of flags
                //
                if( c == '-' )              // Left-justify
                {
                    flags |= LEFT_JUSTIFY;
                    c = *fmt++;
                }

                if( c == '+' )              // Force printing a sign
                {
                    flags |= PRINT_SIGN;
                    c = *fmt++;
                }

                if( c == ' ' )              // Use space instead of + sign
                {
                    if( !(flags & PRINT_SIGN) )
                    {
                        flags |= PRINT_SPACE;
                    }

                    c = *fmt++;
                }

                if( c == '0' )              // Prefix result with 0s
                {
                    flags |= ZERO_PREFIX;
                    c = *fmt++;
                }

                // Width field
                //
                while( (c>='0') && (c<='9') )
                {
                    width = width*10 + (c-'0');
                    c = *fmt++;
                }

                // Short integer
                if( c == 'h' )
                {
                    flags |= SHORT;
                    nibbles = 4;
                    c = *fmt++;
                }

                // Perform the conversion
                //
                switch( c )
                {

                    // Signed integer (32 bit)
                    //
                    case 'd':
                    case 'i':
                        if(flags & SHORT)
                            arg_int = va_arg( arg, short int );
                        else
                            arg_int = va_arg( arg, int );

                        if( arg_int < 0 )
                        {
                            flags |= NEGATIVE;
                            arg_uint = -arg_int;
                        }
                        else
                            arg_uint = arg_int;

                        goto UnsignedInt;

                    // Unsigned number (32 bit)
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  ZERO_PREFIX
                    //                  PRINT_SIGN
                    //                  PRINT_SPACE
                    //                  width
                    //                  NEGATIVE (from cases 'd','i')
                    //
                    case 'u':
                         if(flags & SHORT)
                            arg_uint = va_arg( arg, unsigned short int );
                         else
                            arg_uint = va_arg( arg, unsigned int );

                         // Print the decimal number into a temp buffer
                         // and count the number of digits excluding the
                         // leading zeroes
                         //
UnsignedInt:

                         k = l = 0;

                         for( i=0; i<10; i++ )
                         {
                            j = (arg_uint / dec[i]) + '0';

                            buf[ l++ ] = j;

                            // Count the significant digits
                            //
                            if( (j!='0') || (k>0) )
                                k++,
                                arg_uint %= dec[i];
                         }

                         if( k==0 )
                            k = 1;

                         // If the left justify was not set, we look into
                         // zero prefix flag and width
                         //
                         if( !(flags&LEFT_JUSTIFY) )
                         {
                             while( width-k > 0 )
                             {
                                if( flags&ZERO_PREFIX )
                                    STR_APPEND('0')
                                else
                                    STR_APPEND(' ');

                                width--;
                             }
                         }

                         // Take care of the sign
                         //
                         if( flags&NEGATIVE )
                            STR_APPEND('-')
                         else
                             if( flags&PRINT_SIGN )
                                STR_APPEND('+')
                             else
                                 if( flags&PRINT_SPACE )
                                    STR_APPEND(' ');

                         // If the left justify is set, print the number
                         // and pad it with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=10-k; i<10; i++ )
                                STR_APPEND(buf[i]);

                            while( width-k > 0 )
                            {
                                STR_APPEND(' ');
                                width--;
                            }
                            break;
                         }

                         // Finally, put out the digits
                         //
                         for( i=10-k; i<10; i++ )
                            STR_APPEND(buf[i]);

                    break;


                    // Hex number (unsigned)
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  ZERO_PREFIX
                    //                  HEX_UPPERCASE
                    //                  width
                    //
                    case 'X':
                         flags |= HEX_UPPERCASE;

                    case 'x':
                         if(flags & SHORT)
                            arg_uint = va_arg( arg, unsigned short int );
                         else
                            arg_uint = va_arg( arg, unsigned int );


                         // Print the hex number into a temp buffer
                         // and count the number of digits excluding the
                         // leading zeroes
                         //

                         k = l = 0;

                         for( i=28; i>=0; i-=4 )
                         {
                            j = hex[ (arg_uint >> i) & 0x0f ];

                            // Uppercase hex if needed
                            //
                            if( flags&HEX_UPPERCASE )
                                j = toupper(j);

                            buf[ l++ ] = j;

                            // Count the significant digits
                            //
                            if( (j!='0') || (k>0) )
                                k++;
                         }

                         if( k==0 )
                            k = 1;

                         // If the left justify is set, print the hex number
                         // and pad with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=nibbles-k; i<nibbles; i++ )
                                STR_APPEND(buf[i]);

                            while( width-k > 0 )
                            {
                                STR_APPEND(' ');
                                width--;
                            }

                            break;
                         }


                         // If the left justify was not set, we look into
                         // zero prefix flag and width
                         //
                         while( width-k > 0 )
                         {
                            if( flags&ZERO_PREFIX )
                                STR_APPEND('0')
                            else
                                STR_APPEND(' ');

                            width--;
                         }

                         // Finally, put out the digits
                         //
                         for( i=8-k; i<8; i++ )
                            STR_APPEND(buf[i]);

                    break;


                    // Pointer to a zero-terminated character string
                    //
                    // Affecting flags: LEFT_JUSTIFY
                    //                  width
                    //
                    case 's':

                         arg_str = va_arg( arg, char * );

                         // Find the length of the string

                         i = 0;
                         while( *(arg_str + i) ) i++;

                         // If right justified and width is greater than the
                         // string length, pad with spaces

                         if( !(flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                STR_APPEND(' ');
                             }

                         // Print the entire string

                         while( (c = *arg_str++) )
                         {
                            STR_APPEND(c);
                         }

                         // If the string was left justified and the width
                         // was greater than the string len, pad with spaces

                         if( (flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                STR_APPEND(' ');
                             }

                    break;


                    // Simple character
                    //
                    case 'c':

                         arg_chr = va_arg( arg, int );

                         STR_APPEND(arg_chr);

                    break;


                    // Pointer to integer that is written with the
                    // number of characters written so far.
                    // This does not affect output.
                    //
                    case 'n':

                         arg_pint = va_arg( arg, int * );

                         *arg_pint = str - dest;

                    break;

                    // This formatting character is adder to assist the print to buffer control
                    // When we know the maximum size of a buffer in which we print, and dont want
                    // to overflow the buffer, we use the "width" character:
                    //
                    // %w - that takes one parameter (int) as a maximum buffer size
                    //
                    case 'w':

                        arg_buffer_size = va_arg( arg, int );

                    break;
                };

            };

            break;


            // Print every character that was not in the above
            // category
            //
            default:

                STR_APPEND(c);
        }
    }

    // Append zero at the destination string

    *str = '\0';

    return( str - dest );
}


/******************************************************************************
*                                                                             *
*   int sprintf( char *str, const char *format, ... )                         *
*                                                                             *
*******************************************************************************
*
*   This is a printf function that prints to a string.
*
*   Where:
#       format is a printf style format
#       ... is the argument list
*
*   Returns:
*       number of characters printed
*
******************************************************************************/
int sprintf( char *str, const char *format, ... )
{
    va_list arg;
    int i;

    va_start( arg, format );
    i = vsprintf( str, format, arg );
    va_end( arg );
    return i;
}

