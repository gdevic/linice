/******************************************************************************
*
*   Module:     String.c
*
*   Revision:   1.00
*
*   Date:       08/24/96
*
*   Author:     Goran Devic
*
*******************************************************************************
.-
    Module Description:

          This module contains functions that are defined in "string.h"
          header file.

          All the messages for strerror() function are defined here.

    Note: This file is taken from Yaos project, 2.0 string C library and
          slightly trimmed down
-.
*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/24/96   1.00  Original                                        Goran Devic
* 08/14/02   2.00  Trimmed down for Linice project                 Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/

#include "string.h"                     // Include its own header

#include "ctype.h"                      // Include character types header

/******************************************************************************
*   Global Variables
******************************************************************************/

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

/******************************************************************************
*   Functions
******************************************************************************/

/******************************************************************************
*
*   void * memchr( const void *s, int c, size_t n)
*
*      Scans memory for a byte
*
*   Where:
*      s - pointer to the source string
*      c - character to look for
*      n - maximum number of bytes to examine
*
*   Returns:
*      A pointer to the located character or NULL
*
******************************************************************************/
void * memchr( const void *s, int c, size_t n)
{
    if( n==0 ) return( NULL );

    while( *(unsigned char *)s != c )
    {
        s = (unsigned char *)s + 1;
        if( --n == 0 ) return( NULL );
    }

    return( (void *)s );
}

/******************************************************************************
*
*   int memcmp( const void *s1, const void *s2, size_t n)
*
*      Compares the first n bytes of s1 with the first n bytes of s2.  Returns
*      an int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*   Where:
*      s1 - pointer to object 1
*      s2 - pointer to object 2
*      n number of bytes to compare
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int memcmp( const void *s1, const void *s2, size_t n)
{
    if( n==0 ) return( 0 );

    while( n-- )
    {
        if (*(unsigned char *)s1 != *(unsigned char *)s2)
            return(*(unsigned char *)s1 - *(unsigned char *)s2);
        s1 = (unsigned char *)s1 + 1;
        s2 = (unsigned char *)s2 + 1;
    }

    return( 0 );
}

/******************************************************************************
*
*   void * memcpy( void *s1, const void *s2, size_t n)
*
*      Copies non-overlapping memory objects.
*
*   Where:
*      s1 - pointer to the destination
*      s2 - pointer to the source
*      n number of bytes to copy
*
*   Returns:
*      Pointer to the destination, s1.
*
******************************************************************************/
void * memcpy( void *s1, const void *s2, size_t n)
{
    char *original = s1;

    if( n==0 ) return( s1 );

    while( n-- )
    {
        *(unsigned char *)s1 = *(unsigned char *)s2;
        s1 = (unsigned char *)s1 + 1;
        s2 = (unsigned char *)s2 + 1;
    }

    return(original);
}

/******************************************************************************
*
*   void * memmove( void *s1, const void *s2, size_t n)
*
*      Copies memory objects.  Memory may overlap.
*
*   Where:
*      s1 - pointer to the destination
*      s2 - pointer to the source
*      n number of bytes to move
*
*   Returns:
*      Pointer to the destination, s1.
*
******************************************************************************/
void * memmove( void *s1, const void *s2, size_t n)
{
    char *original = s1;

    if( n==0 ) return( s1 );
    if( s1 == s2 ) return( s1 );
    if( s2 > s1 )
    {
        while( n-- )
        {
            *(unsigned char *)s1 = *(unsigned char *)s2;
            s1 = (unsigned char *)s1 + 1;
            s2 = (unsigned char *)s2 + 1;
        }

        return( original );
    }

    s1 = (unsigned char *)s1 + n-1;
    s2 = (unsigned char *)s2 + n-1;

    while( n-- )
    {
        *(unsigned char *)s1 = *(unsigned char *)s2;
        s1 = (unsigned char *)s1 - 1;
        s2 = (unsigned char *)s2 - 1;
    }

    return( s1 );
}

/******************************************************************************
*
*   void * memset( void *s, int c, size_t n)
*
*      Copies c into the first n characters of s.
*
*   Where:
*      s - pointer to the region of memory to fill
*      c - fill byte
*      n number of bytes to store
*
*   Returns:
*      Pointer to the region, s.
*
******************************************************************************/
void * memset( void *s, int c, size_t n)
{
    char *fill = s;

    if( n==0 ) return( s );
    while( n-- ) *fill++ = c;

    return( s );
}

/******************************************************************************
*
*   char * strcat( char *s1, const char *s2 )
*
*      Appends a copy of the source (including the terminating null character)
*      to the destination so that the first character of s2 overwrites the
*      trailing null character at the end of s1.  The source and destination
*      may not overlap.
*
*   Where:
*      s1 - pointer to destination
*      s2 - pointer to source
*
*   Returns:
*      Pointer to the destination, s1.
*
******************************************************************************/
char * strcat( char *s1, const char *s2 )
{
    char *original = s1;

    /* Find the end of s1. */
    while( *s1 != 0 ) s1++;

    /* Now copy s2 to the end of s1. */
    while (*s2 != 0) *s1++ = *s2++;
    *s1 = 0;

    return(original);
}

/******************************************************************************
*
*   char * strchr( const char *s, int c )
*
*      Returns a pointer of the first occurrence of c in s.  If c is zero,
*      a pointer to the terminating null character is returned.
*
*   Where:
*      s - pointer to the source string
*      c - character to look for
*
*   Returns:
*      A pointer to the matched character or NULL if no character is matched
*
******************************************************************************/
char * strchr( const char *s, int c )
{
    while( *s != c )
    {
        if( *s == 0 )
            return( NULL );

        s++;
    }

    return( (char *) s );
}

/******************************************************************************
*
*   int strcmp( const char *s1, const char *s2 )
*
*      Compares s1 with s2.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strcmp( const char *s1, const char *s2 )
{
    while( 1 )
    {
        if (*s1 != *s2) return(*s1 - *s2);
        if (*s1 == 0) return(0);
        s1++;
        s2++;
    }
}

/******************************************************************************
*
*   int strcoll( const char *s1, const char *s2 )
*
*      Compares s1 with s2 using the current locale.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strcoll( const char *s1, const char *s2 )
{
    // This implementation does not support locale
    return( strcmp(s1,s2) );
}

/******************************************************************************
*
*   char * strcpy( char *s1, const char *s2 )
*
*      Copies non-overlapping string.
*
*   Where:
*      s1 - pointer to the destination
*      s2 - pointer to the source
*
*   Returns:
*      Pointer to the destination, s1.
*
******************************************************************************/
char * strcpy( char *s1, const char *s2 )
{
    char *original = s1;

    while (*s2 != 0) *s1++ = *s2++;
    *s1 = 0;

    return(original);
}

/******************************************************************************
*
*   size_t strcspn( const char *s1, const char *s2 )
*
*      Searches a string for characters which are not in the second string.
*
*   Where:
*      s1 - pointer to the subject string
*      s2 - pointer to the set of break characters
*
*   Returns:
*      The number of initial characters in s1 that are not in the string
*      pointed to by s2.
*
******************************************************************************/
size_t strcspn( const char *s1, const char *s2 )
{
    char *scan1;
    char *scan2;
    int count;

    count = 0;
    for( scan1 = (char*) s1; *scan1 != 0; scan1++ )
    {
        for( scan2 = (char*) s2; *scan2 != 0; )
                if( *scan1 == *scan2++ )
                        return(count);
        count++;
    }

    return(count);
}

#if 0
/******************************************************************************
*
*   char * strerror( int errnum )
*
*      Maps the error code into an error message string
*
*   Where:
*      errnum - error number
*
*   Returns:
*      A pointer to the string describing the error
*
******************************************************************************/
char * strerror( int errnum )
{
    errnum = -errnum;

    if( (errnum>MAX_ERRNO) || (errnum<0) )  errnum = 0;

    return( _sErrors[errnum] );
}
#endif

/******************************************************************************
*
*   size_t strlen( const char *s )
*
*      Computes the length of a string
*
*   Where:
*      s - pointer to a string
*
*   Returns:
*      The length of string s
*
******************************************************************************/
size_t strlen( const char *s )
{
    char *original = (char*) s;

    while( *s != 0 ) s++;

    return(s - original);
}

/******************************************************************************
*
*   char * strncat( char *s1, const char *s2, size_t n )
*
*      Concatenates two counted strings.
*
*   Where:
*      s1 - pointer to a string
*      s2 - pointer to a string to be appended to s1
*      n - number of characters
*
*   Returns:
*      s1
*
******************************************************************************/
char * strncat( char *s1, const char *s2, size_t n )
{
    char *original = s1;

    if( n == 0 ) return(s1);

    /* Find the end of s1. */
    while (*s1 != 0) s1++;

    /* Now copy s2 to the end of s1. */
    while (*s2 != 0)
    {
        *s1++ = *s2++;
        if (--n == 0) break;
    }

    *s1 = 0;

    return(original);
}

/******************************************************************************
*
*   int strncmp( const char *s1, const char *s2, size_t n )
*
*      Compares two counted strings.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*      n - maximum number of characters to compare
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strncmp( const char *s1, const char *s2, size_t n )
{
    if( n==0 ) return( 0 );
    while( 1 )
    {
        if (*s1 != *s2) return(*s1 - *s2);
        if (*s1 == 0 || --n == 0) return(0);
        s1++;
        s2++;
    }
}

/******************************************************************************
*
*   char * strncpy( char *s1, const char *s2, size_t n )
*
*      Copy up to n characters from s2 to s1.  The copy operation stops when a
*      null character is copied.  If there is no null in the first n characters
*      of s2, the result will not be null-terminated.
*
*   Where:
*      s1 - pointer to the destination
*      s2 - pointer to the source
*      n - number of bytes to copy
*
*   Returns:
*      Pointer to the destination, s1.
*
******************************************************************************/
char * strncpy( char *s1, const char *s2, size_t n )
{
    char *original = s1;

    if( n==0 ) return( s1 );
    while( *s2 != 0 )
    {
        *s1++ = *s2++;
        if(--n == 0)
            return( original );
    }

    *s1 = 0;

    return( original );
}

/******************************************************************************
*
*   char * strpbrk( const char *s1, const char *s2 )
*
*      Searches a string for any of a set of characters.  It locates the first
*      occurrence in s1 of any of the characters in s2.
*
*   Where:
*      s1 - pointer to the subject string
*      s2 - pointer to the list of delimiters
*
*   Returns:
*      Pointer to a character in s1 which matches one of the characters in s2
*      or NULL if there is no match.
*
******************************************************************************/
char * strpbrk( const char *s1, const char *s2 )
{
    char *scan1;
    char *scan2;

    for( scan1 = (char*) s1; *scan1 != 0; scan1++ )
        for( scan2 = (char*) s2; *scan2 != 0; )
                if( *scan1 == *scan2++ )
                        return( scan1 );
    return( NULL );
}

/******************************************************************************
*
*   char * strrchr( const char *s, int c )
*
*      Returns a pointer of the last occurrence of c in s or NULL if there
*      is no match.  The terminating null character is considered to be part
*      of the string.
*
*   Where:
*      s - pointer to the string to scan
*      c - character to look for
*
*   Returns:
*      A pointer to the last matched character or NULL.
*
******************************************************************************/
char * strrchr( const char *s, int c )
{
    char *last = NULL;

    while( *s )
    {
        if( *(char*) s == c ) last = (char*) s;
        s++;
    }

    return( last );
}

/******************************************************************************
*
*   size_t strspn( const char *s1, const char *s2 )
*
*      Computes the length of the maximum initial segment of s1 which consists
*      entirely of characters from s2.
*
*   Where:
*      s1 - pointer to the subject string
*      s2 - pointer to the characters to search for
*
*   Returns:
*      The number of characters in the initial segment of s1 which consists
*      only of characters from s2, excluding the trailing null character
*
******************************************************************************/
size_t strspn( const char *s1, const char *s2 )
{
    char *scan;
    int len;

    len = 0;
    while( *s1 != 0 )
    {
        for( scan = (char*) s2; *scan != 0; scan++ )
        {
            if( *s1==*scan )
                break;
        }

        if( *scan==0 )
            return( len );

        len++;
        s1++;
    }

    return( len );
}

/******************************************************************************
*
*   char * strstr( const char *s1, const char *s2 )
*
*      Locates the first occurrence in s1 of the substring s2.  The terminating
*      null characters are not compared.
*
*   Where:
*      s1 - pointer to the subject string
*      s2 - pointer to the substring to locate
*
*   Returns:
*      A pointer to the located substring in s1 or NULL.
*
******************************************************************************/
char * strstr( const char *s1, const char *s2 )
{
    char *orig;
    char *scan;

    while( *s1 != 0 )
    {
        orig = (char*) s1;
        scan = (char*) s2;

        for( ; *scan != 0; scan++, orig++ )
        {
            if( *orig != *scan )
                break;
        }

        if( *scan == 0 )
            return( (char*) s1 );

        s1++;
    }

    return( NULL );
}

/******************************************************************************
*
*   char * strtok( char *s1, const char *s2 )
*
*      Breaks a string into tokens.  The first call should have s1 as the first
*      argument.  Subsequent calls should have NULL as the first argument.  The
*      separator string, s2, may be different from call to call.
*
*   Where:
*      s1 - pointer to the string to search. If s1 is NULL, strtok() uses a
*          saved pointer from the previous call.
*      s2 - delimiter list
*
*   Returns:
*      A pointer to the first character of the token or NULL if there are no
*      more tokens
*
******************************************************************************/
char * strtok( char *s1, const char *s2 )
{
    char *last;
    static char *str;

    if( s1 != NULL )
        str = s1;
    else
        s1 = str;

    // Find the beginning of the token
    str += strspn( s1, s2 );
    if( *str==0 ) return( NULL );

    // Find the end of the token
    last = strpbrk( str, s2 );
    s1 = str;
    str = last;
    *str++ = 0;

    return( s1 );
}

/******************************************************************************
*
*   size_t strxfrm( char *s1, const char *s2, size_t n )
*
*      Transforms string using rules for locale.
*
*   Where:
*      s1 - pointer to output string
*      s2 - pointer to input string
*      n - Maximum number of bytes to store into s1
*
*   Returns:
*      The number of bytes required to store the output string
*
******************************************************************************/
size_t strxfrm( char *s1, const char *s2, size_t n )
{
    // This implementation does not support locale
    strncpy( s1, s2, n );
    return( strlen(s2) );
}


/******************************************************************************
* NON-POSIX FUNCTIONS:  ANSI / STANDARD C
******************************************************************************/


/******************************************************************************
*
*   void * memccpy( void *s1, const void *s2, int c, size_t n )
*
*      Copies non-overlapping memory objects from s2 to s1 up to and including
*      the first occurence of character c or until n bytes have been copied,
*      whichever comes first.
*
*   Where:
*      s1 - pointer to the destination
*      s2 - pointer to the source
*      c - copy break character
*      n number of bytes to copy
*
*   Returns:
*      Pointer to the byte following the character c, if one is found and
*      copied, otherwise it returns NULL
*
******************************************************************************/
void * memccpy( void *s1, const void *s2, int c, size_t n )
{
    if( n==0 ) return( NULL );

    while( (*(unsigned char *)s1 = *(unsigned char *)s2) != c )
    {
        s1 = (unsigned char *)s1 + 1;
        s2 = (unsigned char *)s2 + 1;
        if( --n == 0 ) return( NULL );
    }

    return( (void *)((unsigned char *)s1 + 1) );

}

/******************************************************************************
*
*   int memicmp( const void *s1, const void *s2, size_t n )
*
*       Compares, with case insensitivity, the first n characters of the object
*       pointed to by s1 to the object pointed to by s2.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*      n number of bytes to compare
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int memicmp( const void *s1, const void *s2, size_t n )
{
    if( n==0 ) return( 0 );

    while( n-- )
    {
        if (tolower(*(unsigned char *)s1) != tolower(*(unsigned char *)s2))
            return(tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2));
        s1 = (unsigned char *)s1 + 1;
        s2 = (unsigned char *)s2 + 1;
    }

    return( 0 );
}

/******************************************************************************
*
*   int strcmpi( const char *s1, const char *s2 )
*
*      Compares s1 with s2 with case insensitivity.
*      This function is identical to stricmp()
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strcmpi( const char *s1, const char *s2 )
{
    while( 1 )
    {
        if (tolower(*s1) != tolower(*s2)) return(tolower(*s1) - tolower(*s2));
        if (*s1 == 0) return(0);
        s1++;
        s2++;
    }
}

#if 0
/******************************************************************************
*
*   char * strdup( const char *string )
*
*      Create a duplicate copy of the string pointed to by string and returns
*      a pointer to the new copy.  The memory is allocated using the malloc()
*      call and can be freed using the free() function.
*
*   Where:
*      string - pointer to a source string to be cloned
*
*   Returns:
*       Pointer to the new string or NULL if failed
*
******************************************************************************/
char * strdup( const char *string )
{
    char *new;

    new = malloc( strlen(string) + 1);

    if( new == NULL )
        return( NULL );

    return( strcpy(new, string) );
}
#endif

/******************************************************************************
*
*   int stricmp( const char *s1, const char *s2 )
*
*      Compares s1 with s2 with case insensitivity.
*      This function is identical to strcmpi()
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int stricmp( const char *s1, const char *s2 )
{
    while( 1 )
    {
        if (tolower(*s1) != tolower(*s2)) return(tolower(*s1) - tolower(*s2));
        if (*s1 == 0) return(0);
        s1++;
        s2++;
    }
}

/******************************************************************************
*
*   char * strlwr( char *s )
*
*      Replace the string with lowercase characters.
*
*   Where:
*      s - string to be replaced
*
*   Returns:
*      The address of the original string
*
******************************************************************************/
char * strlwr( char *s )
{
    char *scan;

    scan = s;
    while( *scan != 0 )
    {
        *scan = tolower( *scan );
        scan++;
    }

    return( s );
}

/******************************************************************************
*
*   int strnicmp( const char *s1, const char *s2, size_t n )
*
*      Compares two counted strings with case insensitivity.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*      n - maximum number of characters to compare
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strnicmp( const char *s1, const char *s2, size_t n )
{
    if( n==0 ) return( 0 );
    while( 1 )
    {
        if (tolower(*s1) != tolower(*s2))
            return(tolower(*s1) - tolower(*s2));
        if (*s1 == 0 || --n == 0) return(0);
        s1++;
        s2++;
    }
}

/******************************************************************************
*
*   char * strnset( char *s, int c, size_t n )
*
*      Copies c into the first n characters of s.
*
*   Where:
*      s - pointer to the string to fill
*      c - fill byte
*      n number of bytes to store
*
*   Returns:
*      Pointer to the string s.
*
******************************************************************************/
char * strnset( char *s, int c, size_t n )
{
    char *scan;

    scan = s;
    while( n-- )
    {
        if( *scan == 0 )
            return( s );

        *scan++ = c;
    }

    return( s );
}

/******************************************************************************
*
*   char * strrev( char *s )
*
*       Replaces the string s with a string whose characters are in the reverse
*       order.
*
*   Where:
*       s - string to be replaced
*
*   Returns:
*       The address of the original string s
*
******************************************************************************/
char * strrev( char *s )
{
    char *front;
    char *back;
    char swap;

    front = s;
    back = s + strlen(s) - 1;

    while( front < back )
    {
        swap = *front;
        *front = *back;
        *back = swap;

        front++;
        back--;
    }

    return( s );
}

/******************************************************************************
*
*   char * strset( char *s, int c )
*
*      Fills the string pointed to by s with the character c.
*
*   Where:
*      s - pointer to the string to fill
*      c - fill byte
*
*   Returns:
*      Pointer to the string s.
*
******************************************************************************/
char * strset( char *s, int c )
{
    char *scan;

    scan = s;
    while( *scan != 0 )
        *scan++ = c;

    return( s );
}

/******************************************************************************
*
*   char * strupr( char *s )
*
*      Replace the string with uppercase characters.
*
*   Where:
*      s - string to be replaced
*
*   Returns:
*      The address of the original string s
*
******************************************************************************/
char * strupr( char *s )
{
    char *scan;

    scan = s;
    while( *scan != 0 )
    {
        *scan = toupper( *scan );
        scan++;
    }

    return( s );
}


