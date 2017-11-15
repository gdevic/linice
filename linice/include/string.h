/******************************************************************************
*
*   Module:     string.h
*
*   Revision:   1.00
*
*   Date:       08/24/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          ANSI C / POSIX string header file

    Note: This file is taken from Yaos project, 2.0 string C library and
          slightly trimmed down

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
#ifndef _STRING_H_
#define _STRING_H_

/******************************************************************************
*   Global Variables, Macros and Defines
******************************************************************************/

// Protect size_t for historical reasons..
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#define NULL               0L


/******************************************************************************
*   Functions
******************************************************************************/
extern void * memchr( const void *s, int c, size_t n);
extern int    memcmp( const void *s1, const void *s2, size_t n);
//extern void * memcpy( void *s1, const void *s2, size_t n);      // I think we still picked up these two somewhere in the
extern void * memmove( void *s1, const void *s2, size_t n);
//extern void * memset( void *s, int c, size_t n);                // few linux include files that we are getting...
extern char * strcat( char *s1, const char *s2 );
extern char * strchr( const char *s, int c );
extern int    strcmp( const char *s1, const char *s2 );
extern int    strcoll( const char *s1, const char *s2 );
extern char * strcpy( char *s1, const char *s2 );
extern size_t strcspn( const char *s1, const char *s2 );
extern char * strerror( int errnum );
extern size_t strlen( const char *s );
extern char * strncat( char *s1, const char *s2, size_t n );
extern int    strncmp( const char *s1, const char *s2, size_t n );
extern char * strncpy( char *s1, const char *s2, size_t n );
extern char * strpbrk( const char *s1, const char *s2 );
extern char * strrchr( const char *s, int c );
extern size_t strspn( const char *s1, const char *s2 );
extern char * strstr( const char *s1, const char *s2 );
extern char * strtok( char *s1, const char *s2 );
extern size_t strxfrm( char *s1, const char *s2, size_t n );

// We dont care for POSIX in this incarnation, but let's keep the marker
//#ifndef _POSIX_SOURCE

extern void * memccpy( void *s1, const void *s2, int c, size_t n );
extern int    memicmp( const void *s1, const void *s2, size_t n );
extern int    strcmpi( const char *s1, const char *s2 );
extern char * strdup( const char *s );
extern int    stricmp( const char *s1, const char *s2 );
extern char * strlwr( char *s );
extern int    strnicmp( const char *s1, const char *s2, size_t n );
extern char * strnset( char *s, int c, size_t len );
extern char * strrev( char *s );
extern char * strset( char *s, int c );
extern char * strupr( char *s );

//#endif // _POSIX_SOURCE

#endif // _STRING_H_
