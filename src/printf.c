/******************************************************************************
*                                                                             *
*   Module:     Printf.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/4/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the printf family of functions.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/28/96   Original                                             Goran Devic *
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

#define WRITE_BUF_LEN    1024

#define LEFT_JUSTIFY     0x0001
#define PRINT_SIGN       0x0002
#define PRINT_SPACE      0x0004
#define ZERO_PREFIX      0x0008
#define HEX_UPPERCASE    0x0010
#define NEGATIVE         0x0020


static char printbuf[WRITE_BUF_LEN];
static char buf[10];
static const char hex[16]="0123456789abcdef";
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
    char * fmt = format;
    char c;
    char *str = dest;
    int i, j, k, l;
    int flags;
    int width;

    int arg_int;
    unsigned int arg_uint;
    char arg_chr;
    char *arg_str;
    int *arg_pint;


    while( c = *fmt++ )
    {
        switch( c )
        {

            // New line - send LF and CR characters
            //
            case '\n' :

                *str++ = 0x0A;
                *str++ = 0x0D;

            break;


            // Format control characters
            //
            case '%' :
            {
                flags = 0;
                width = 0;

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


                // Perform the conversion
                //
                switch( c )
                {

                    // Signed integer (32 bit)
                    //
                    case 'd':
                    case 'i':
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
                                    *str++ = '0';
                                else
                                    *str++ = ' ';

                                width--;
                             }
                         }

                         // Take care of the sign
                         //
                         if( flags&NEGATIVE )
                            *str++ = '-';
                         else
                             if( flags&PRINT_SIGN )
                                *str++ = '+';
                             else
                                 if( flags&PRINT_SPACE )
                                    *str++ = ' ';

                         // If the left justify is set, print the number
                         // and pad it with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=10-k; i<10; i++ )
                                *str++ = buf[i];

                            while( width-k > 0 )
                            {
                                *str++ = ' ';
                                width--;
                            }
                            break;
                         }

                         // Finally, put out the digits
                         //
                         for( i=10-k; i<10; i++ )
                            *str++ = buf[i];

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
                            for( i=8-k; i<8; i++ )
                                *str++ = buf[i];

                            while( width-k > 0 )
                            {
                                *str++ = ' ';
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
                                *str++ = '0';
                            else
                                *str++ = ' ';

                            width--;
                         }

                         // Finally, put out the digits
                         //
                         for( i=8-k; i<8; i++ )
                            *str++ = buf[i];

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
                                *str++ = ' ';
                             }

                         // Print the entire string

                         while( c = *arg_str++ )
                         {
                            *str++ = c;
                         }

                         // If the string was left justified and the width
                         // was greater than the string len, pad with spaces

                         if( (flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                *str++ = ' ';
                             }

                    break;


                    // Simple character
                    //
                    case 'c':

                         arg_chr = va_arg( arg, int );

                         *str++ = arg_chr;

                    break;


                    // Pointer to integer that is written with the
                    // number of characters written so far.
                    // This does not affect output.
                    //
                    case 'n':

                         arg_pint = va_arg( arg, int * );

                         *arg_pint = str - dest;

                    break;

                };

            };

            break;


            // Print every character that was not in the above
            // category
            //
            default:

                *str++ = c;
        }
    }

    // Append zero at the destination string

    *str = '\0';

    return( str - dest );
}


#if 0  

/******************************************************************************
*                                                                             *
*   int printf( const char *format, ... )                                     *
*                                                                             *
*******************************************************************************
*
*   This is the printf function
*
*   Where:
#       format is a printf style format
#       ... is the argument list
*
*   Returns:
*       number of characters printed
*
******************************************************************************/
int printf( const char *format, ... )
{
    va_list arg;
    int i;

    va_start( arg, format );
    i = vsprintf( printbuf, format, arg );
    fputs(printbuf, stdout);
    va_end( arg );

    return i;
}


/******************************************************************************
*                                                                             *
*   int vfprintf( FILE *fp, const char *format, va_list arg )                 *
*                                                                             *
*******************************************************************************
*
*   This is the printf function with variable number of arguments
*
*   Where:
#       fp is the file stream
#       format is a printf style format
#       arg is the argument list
*
*   Returns:
*       number of characters printed
*
******************************************************************************/
int vfprintf( FILE *fp, const char *format, va_list arg )
{
    int i = vsprintf( printbuf, format, arg );
    fputs(printbuf, fp);

    return i;
}


/******************************************************************************
*                                                                             *
*   int fprintf( FILE *fp, const char *format, ... )                          *
*                                                                             *
*******************************************************************************
*
*   This is the printf function to print to a file stream
*
*   Where:
#       fp is the file stream
#       format is a printf style format
#       ... is the argument list
*
*   Returns:
*       number of characters printed
*
******************************************************************************/
int fprintf( FILE *fp, const char *format, ... )
{
    va_list arg;
    int i;

    va_start( arg, format );
    i = vsprintf( printbuf, format, arg );
    fputs(printbuf, fp);
    va_end( arg );

    return i;
}

#endif

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

