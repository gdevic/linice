/******************************************************************************
*                                                                             *
*   Module:     clib.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/28/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        Header file containing all supported c-library function headers
        Selection includes:
            types.h
            assert,h
            ctype.h
            stdio.h
            stdlib.h
            string.h

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/28/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _CLIB_H_
#define _CLIB_H_

#include <linux/fs.h>                   // Include file operations file

///////////////////////////////////////////////////////////////////////////////
//
// types.h
//
///////////////////////////////////////////////////////////////////////////////

#ifndef offsetof
#define offsetof(s,m) (int)&(((s*)0)->m)
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define MIN(a,b)        ((a)<(b)?(a):(b))
#define MAX(a,b)        ((a)<(b)?(b):(a))

#define LONG_MIN                0x80000000

///////////////////////////////////////////////////////////////////////////////
//
// assert.h
//
///////////////////////////////////////////////////////////////////////////////

extern void ice_assert( char *file, int line );

#define assert( expr )   ((expr)? (void)0: ice_assert(__FILE__, __LINE__))
#define ASSERT( expr )   assert( expr )

///////////////////////////////////////////////////////////////////////////////
//
// ctype.h
//
///////////////////////////////////////////////////////////////////////////////

extern char _ctype_[257];

#define _LOWER         0x01
#define _UPPER         0x02
#define _DIGIT         0x04
#define _XDIGIT        0x08
#define _CONTROL       0x10
#define _LOWCT         0x20
#define _SPACE         0x40
#define _PUNCT         0x80

#define _CASE          0x20

#define ice_isalnum(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER | _DIGIT))
#define ice_isalpha(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER))
#define ice_iscntrl(c)     ((_ctype_[(c)+1])&(_CONTROL))
#define ice_isdigit(c)     ((_ctype_[(c)+1])&(_DIGIT))
#define ice_isgraph(c)     ((_ctype_[(c)+1])&(_PUNCT | _DIGIT | _UPPER | _LOWER))
#define ice_islower(c)     ((_ctype_[(c)+1])&(_LOWER))
#define ice_isprint(c)     ((_ctype_[(c)+1])&(_SPACE | _PUNCT | _DIGIT | _UPPER | _LOWER))  
#define ice_ispunct(c)     ((_ctype_[(c)+1])&(_PUNCT))
#define ice_isspace(c)     ((_ctype_[(c)+1])&(_SPACE | _LOWCT))
#define ice_isupper(c)     ((_ctype_[(c)+1])&(_UPPER))
#define ice_isxdigit(c)    ((_ctype_[(c)+1])&(_XDIGIT))

#define ice_tolower(c)     (ice_isupper(c)? (c)|_CASE : (c))
#define ice_toupper(c)     (ice_islower(c)? (c)&~_CASE : (c))

#define ice_isascii(c)     ((unsigned)(c) < 128)

///////////////////////////////////////////////////////////////////////////////
//
// stdio.h
//
///////////////////////////////////////////////////////////////////////////////

typedef char *__va_list[1];

extern int ice_sprintf( char *str, char *format, ... );
extern int ice_vsprintf( char *dest, char *format, __va_list arg );

extern int ice_scanf( const char *fmt, ... );
extern int ice_vscanf( const char *fmt, __va_list arg );

///////////////////////////////////////////////////////////////////////////////
//
// stdlib.h
//
///////////////////////////////////////////////////////////////////////////////

extern BYTE *ice_init_heap( size_t size );
extern void * _kMalloc( BYTE *pHeap, DWORD size );
extern void _kFree( BYTE *pHeap, void *mPtr );

///////////////////////////////////////////////////////////////////////////////
//
// string.h
//
///////////////////////////////////////////////////////////////////////////////

extern void * ice_memchr( const void *s, int c, size_t n);
extern int    ice_memcmp( const void *s1, const void *s2, size_t n);
extern void * ice_memcpy( void *s1, const void *s2, size_t n);
extern void * ice_memmove( void *s1, const void *s2, size_t n);
extern void * ice_memset( void *s, int c, size_t n);
extern char * ice_strcat( char *s1, const char *s2 );
extern char * ice_strchr( const char *s, int c );
extern int    ice_strcmp( const char *s1, const char *s2 );
extern int    ice_strcoll( const char *s1, const char *s2 );
extern char * ice_strcpy( char *s1, const char *s2 );
extern size_t ice_strcspn( const char *s1, const char *s2 );
extern char * ice_strerror( int errnum );
extern size_t ice_strlen( const char *s );
extern char * ice_strncat( char *s1, const char *s2, size_t n );
extern int    ice_strncmp( const char *s1, const char *s2, size_t n );
extern char * ice_strncpy( char *s1, const char *s2, size_t n );
extern char * ice_strpbrk( const char *s1, const char *s2 );
extern char * ice_strrchr( const char *s, int c );
extern size_t ice_strspn( const char *s1, const char *s2 );
extern char * ice_strstr( const char *s1, const char *s2 );
extern char * ice_strtok( char *s1, const char *s2 );
extern size_t ice_strxfrm( char *s1, const char *s2, size_t n );

extern void * ice_memccpy( void *s1, const void *s2, int c, size_t n );
extern int    ice_memicmp( const void *s1, const void *s2, size_t n );
extern int    ice_strcmpi( const char *s1, const char *s2 );
extern char * ice_strdup( const char *s );
extern int    ice_stricmp( const char *s1, const char *s2 );
extern char * ice_strlwr( char *s );
extern int    ice_strnicmp( const char *s1, const char *s2, size_t n );
extern char * ice_strnset( char *s, int c, size_t len );
extern char * ice_strrev( char *s );
extern char * ice_strset( char *s, int c );
extern char * ice_strupr( char *s );


#endif //  _CLIB_H_
