/******************************************************************************
*                                                                             *
*   Module:     output.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/4/97                                                        *
*                                                                             *
*   Copyright (c) 1997, 2000 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

    This module contains the printing code that is independent on the
    type of output.

    dprint() performs the functionality of printf() with some extra control
    characters that are defined in Display.h file.

    ice_sprintf() is also here since it is basically using the same function.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/4/97     1.00  Original                                       Goran Devic *
* 11/13/00         Modified for LinIce                            Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "ice.h"                        // Include global structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TPUTCHAR pfnPutChar;                    // Put character function

int nCharsWritten;                      // Number of written characters

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

static char *sScroll = "    Press any key to continue; Esc to cancel";

static BOOL fMultiline = FALSE;
static BOOL fEsc = FALSE;
static DWORD nMultiline = 0;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   ice_sprintf() helper function that stores a character into a string           *
*                                                                             *
******************************************************************************/

static char *str_buf;                   // String buffer pointer

static void sputchar( char c )
{
    *str_buf++ = c;

    nCharsWritten++;
}


/******************************************************************************
*                                                                             *
*   int _print( char *format, va_list arg )                                   *
*                                                                             *
*******************************************************************************
*
*   This is a generic print function.  It is used by the rest of print
*   functions after they have set up the appropriate (pfnPutChar) function.
*
*   Where:
*       format is the format of the string to be printed, standard "C"
*       arg is a variable length list of arguments needed for printing
*
*   Returns:
*       number of characters actually printed
*
******************************************************************************/
static int _print( char *format, va_list arg )
{
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

    char buf[10];
    char * fmt = format;
    char c;
    int i, j, k, l;
    int flags;
    int width;

    int arg_int;
    unsigned int arg_uint;
    char arg_chr;
    char *arg_str;
    int *arg_pint;


    nCharsWritten = 0;

    while( (c = *fmt++) )
    {
        switch( c )
        {
            // Character \n resets x=0, advances to new line
            // Character \r clears text to the end of line and resets x=0
            // They are sent with the rest of the printable codes

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
                                    (pfnPutChar)('0');
                                else
                                    (pfnPutChar)(' ');

                                width--;
                             }
                         }

                         // Take care of the sign
                         //
                         if( flags&NEGATIVE )
                             (pfnPutChar)('-');
                         else
                             if( flags&PRINT_SIGN )
                                 (pfnPutChar)('+');
                             else
                                 if( flags&PRINT_SPACE )
                                     (pfnPutChar)(' ');

                         // If the left justify is set, print the number
                         // and pad it with spaces if width is greater
                         //
                         if( flags&LEFT_JUSTIFY )
                         {
                            for( i=10-k; i<10; i++ )
                                (pfnPutChar)( buf[i] );

                            while( width-k > 0 )
                            {
                                (pfnPutChar)(' ');
                                width--;
                            }
                            break;
                         }

                         // Finally, put out the digits
                         //
                         for( i=10-k; i<10; i++ )
                             (pfnPutChar)( buf[i] );

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
                            if( (flags&HEX_UPPERCASE) && (j>'9') )
                                j += 'A' - 'a';

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
                                (pfnPutChar)( buf[i] );

                            while( width-k > 0 )
                            {
                                (pfnPutChar)(' ');
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
                                (pfnPutChar)('0');
                            else
                                (pfnPutChar)(' ');

                            width--;
                         }

                         // Finally, put out the digits
                         //
                         for( i=8-k; i<8; i++ )
                             (pfnPutChar)( buf[i] );


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
                                 (pfnPutChar)(' ');
                             }

                         // Print the entire string

                         while( (c = *arg_str++) )
                         {
                            (pfnPutChar)( c );
                         }

                         // If the string was left justified and the width
                         // was greater than the string len, pad with spaces

                         if( (flags&LEFT_JUSTIFY) && (i<width) )
                             for( j=0; j< width-i; j++ )
                             {
                                 (pfnPutChar)(' ');
                             }

                    break;


                    // Simple character
                    //
                    case 'c':

                         // Seems like I had this bug for a long time...
                         //arg_chr = va_arg( arg, char );

                         arg_chr = va_arg( arg, int );

                         (pfnPutChar)( arg_chr );

                    break;


                    // '%' character
                    //
                    case '%':

                         (pfnPutChar)( '%' );

                    break;


                    // Pointer to integer that is written with the
                    // number of characters written so far.
                    // This does not affect output.
                    //
                    case 'n':

                         arg_pint = va_arg( arg, int * );

                         *arg_pint = nCharsWritten;

                    break;

                };

            };

            break;


            // Print every character that was not in the above
            // category
            //
            default:

                (pfnPutChar)( c );
        }
    }

    return( nCharsWritten );
}



/******************************************************************************
*                                                                             *
*   int ice_sprintf( char *buf, const char *format, ... )                         *
*                                                                             *
*******************************************************************************
*
*   Standard ice_sprintf() function that prints into a string.
*
*   Where:
*       buf is a pointer to a destination buffer (where to print)
*       format is the format of the string to be printed, standard "C"
*
*   Returns:
*       number of characters actually printed
*
******************************************************************************/
int ice_sprintf( char *buf, char *format, ... )
{
    TPUTCHAR pfnPutCharSave;
    va_list arg;
    int value;

    // Set the string pointer

    str_buf = buf;

    // Set the output function

    pfnPutCharSave = pfnPutChar;
    pfnPutChar = sputchar;

    va_start( arg, format );

    value = _print( format, arg );

    // Zero-terminate the string

    *str_buf = (char)0;

    // Restore original put character function

    pfnPutChar = pfnPutCharSave;

    return( value );
}


/******************************************************************************
*                                                                             *
*   int dprint( char *format, ... )                                           *
*                                                                             *
*******************************************************************************
*
*   This is the main print function for debugger display.  It also has some
*   added functionality in form of control codes.
*
*   Where:
*       format is the standard printf() format string
*       ... standard printf() list of arguments.
*
*   Returns:
*       number of characters actually printed.
*
******************************************************************************/
int dprint( char *format, ... )
{
    va_list arg;

    va_start( arg, format );

    return _print( format, arg );
}


/******************************************************************************
*                                                                             *
*   void Multiline(BOOL fStart)                                               *
*                                                                             *
*******************************************************************************
*
*   Prepares a multi-line output or stops a multiline output.  Such output is
*   a seria of printline() calls that may need user keypress to proceed or 
*   break.
*
*   Where:
*       fStart is the start/end of multiline block
*
******************************************************************************/
void Multiline(BOOL fStart)
{
    nMultiline = deb.wcmd.nLines;
    fMultiline = fStart;
    fEsc = FALSE;
}    


/******************************************************************************
*                                                                             *
*   int printline( char *format, ... )                                        *
*                                                                             *
*******************************************************************************
*
*   This version of print function adds the printed line to the command
*   history buffer as well as prints it.
*   Multi-line outputs, if enabled, are also counted and appropriate user
*   input is processed.
*
*   Where:
*       format is the standard printf() format string
*       ... standard printf() list of arguments.
*
*   Returns:
*       FALSE if within a multiline block with further output disabled
*
******************************************************************************/
BOOL printline( char *format, ... )
{
    TPUTCHAR pfnPutCharSave;
    va_list arg;
    char buf[256];
    int value = 0;

    // If we escaped further prints until the multiline terminator...

    if( fMultiline && fEsc )
        return( FALSE );
        
    // Set the string pointer

    str_buf = buf;

    // Set the output function

    pfnPutCharSave = pfnPutChar;
    pfnPutChar = sputchar;

    va_start( arg, format );

    _print( format, arg );

    // Zero-terminate the string

    *str_buf = (char)0;

    // Restore original put character function

    pfnPutChar = pfnPutCharSave;

    // Print the string out simply and fast

    while( buf[value] != 0 )
    {
        (pfnPutChar)( buf[value] );
        value++;
    }

    // If multiline output is enabled, check if we need to ask user to continue...

    if( (fMultiline==TRUE) && (nMultiline--==0) )
    {
        dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_HELP]);
        dprint("%c%c%c%c%s%c", DP_SAVEXY, DP_SETCURSOR, 0, deb.nLines - 1, sScroll, DP_RESTOREXY);
        dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_NORMAL]);

        if( GetKey(TRUE)==ESC )
            fEsc = TRUE;

        nMultiline = deb.wcmd.nLines;
    }

    (pfnPutChar)('\n');

    // Add it to the command history

    AddHistory(buf);

    return( fEsc );
}

