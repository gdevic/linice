/******************************************************************************
*                                                                             *
*   Module:     clib.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/28/00                                                      *
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
            errno.h
            limits.h
            stdarg.h
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


///////////////////////////////////////////////////////////////////////////////
//
// types.h
//
///////////////////////////////////////////////////////////////////////////////

#ifndef offsetof
#define offsetof(s,m) (int)&(((s*)0)->m)
#endif

typedef unsigned int   size_t;

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

#define NULL    0L


///////////////////////////////////////////////////////////////////////////////
//
// assert.h
//
///////////////////////////////////////////////////////////////////////////////

extern void __assert( char *file, int line );

#define assert( expr )   ((expr)? (void)0: __assert(__FILE__, __LINE__))
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

#define isalnum(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER | _DIGIT))
#define isalpha(c)     ((_ctype_[(c)+1])&(_LOWER | _UPPER))
#define iscntrl(c)     ((_ctype_[(c)+1])&(_CONTROL))
#define isdigit(c)     ((_ctype_[(c)+1])&(_DIGIT))
#define isgraph(c)     ((_ctype_[(c)+1])&(_PUNCT | _DIGIT | _UPPER | _LOWER))
#define islower(c)     ((_ctype_[(c)+1])&(_LOWER))
#define isprint(c)     ((_ctype_[(c)+1])&(_SPACE | _PUNCT | _DIGIT | _UPPER | _LOWER))  
#define ispunct(c)     ((_ctype_[(c)+1])&(_PUNCT))
#define isspace(c)     ((_ctype_[(c)+1])&(_SPACE | _LOWCT))
#define isupper(c)     ((_ctype_[(c)+1])&(_UPPER))
#define isxdigit(c)    ((_ctype_[(c)+1])&(_XDIGIT))

#define tolower(c)     (isupper(c)? (c)|_CASE : (c))
#define toupper(c)     (islower(c)? (c)&~_CASE : (c))

#define isascii(c)     ((unsigned)(c) < 128)


///////////////////////////////////////////////////////////////////////////////
//
// errno.h
//
///////////////////////////////////////////////////////////////////////////////

#define E2BIG        (-1)   // Arguments and environment is larger than ARG_MAX
#define EACCESS      (-2)   // Search file permission is denied
#define EAGAIN       (-3)   // O_NONBLOCK flag is set; not enough resources
#define EBADF        (-4)   // Invalid file descriptor
#define EBUSY        (-5)   // The directory is in use
#define ECHILD       (-6)   // There are no children
#define EDEADLK      (-7)   // An fcntl would cause a deadlock
#define EDOM         (-8)   // Input argument is outside math domain
#define EEXIST       (-9)   // The name file already exists
#define EFAULT       (-10)  // Invalid address in an arg to a function call
#define EFBIG        (-11)  // File exceeds maximum file size
#define EINTR        (-12)  // Function was interrupted by a signal
#define EINVAL       (-13)  // Invalid argument
#define EIO          (-14)  // Input or output error
#define EISDIR       (-15)  // A name is a directory
#define EMFILE       (-16)  // Too many file descriptors used by a process
#define EMLINK       (-17)  // The number of links exceeds LINK_MAX
#define ENAMETOOLONG (-18)  // Length of a filename exceeds PATH_MAX
#define ENFILE       (-19)  // Too many files are open in the system
#define ENODEV       (-20)  // No such device
#define ENOENT       (-21)  // A file or directory does not exist
#define ENOEXEC      (-22)  // File is not an executable
#define ENOLCK       (-23)  // No locks available
#define ENOMEM       (-24)  // No memory available
#define ENOSPC       (-25)  // No space left on disk
#define ENOSYS       (-26)  // Function not implemented
#define ENOTDIR      (-27)  // A component of a path is not a directory
#define ENOTEMPTY    (-28)  // Cant delete or rename a non-empty directory
#define ENOTTY       (-29)  // A file is not a terminal
#define ENXIO        (-30)  // No such device
#define EPERM        (-31)  // Operation is not permitted
#define EPIPE        (-32)  // Attempt to write to a pipe with no reader
#define ERANGE       (-33)  // Result is too large
#define EROFS        (-34)  // Read-only file system
#define ESPIPE       (-35)  // An lseek was issued on a pipe or fifo
#define ESRCH        (-36)  // No such process
#define EXDEV        (-37)  // Attempt to link a file to another file system

#define MAX_ERRNO    37     // Number of error codes

extern int errno;


///////////////////////////////////////////////////////////////////////////////
//
// limits.h
//
///////////////////////////////////////////////////////////////////////////////

#define CHAR_BIT                8
#define CHAR_MIN                0
#define CHAR_MAX                255
#define INT_MIN                 (signed)0x80000000
#define INT_MAX                 (signed)0x7FFFFFFF
#define LONG_MIN                0x80000000
#define LONG_MAX                0x7FFFFFFF
#define ULONG_MAX               0xFFFFFFFF

#define MAX_CANON               255     // Size of the canonical input buffer
#define MAX_INPUT               32      // Size of the type-ahead buffer
#define NAME_MAX                13      // Characters in a file name


///////////////////////////////////////////////////////////////////////////////
//
// stdarg.h
//
///////////////////////////////////////////////////////////////////////////////

typedef char *va_list[1];

// These are really fun !

#define va_start(ap,pn) ((ap)[0]=(char *)&pn+((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)

#define va_arg(ap,type)     ((ap)[0]+=  ((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),(*(type *)((ap)[0]-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))

#define va_end(ap)      ((ap)[0]=0,(void)0)


///////////////////////////////////////////////////////////////////////////////
//
// stdio.h
//
///////////////////////////////////////////////////////////////////////////////

typedef char *__va_list[1];

extern int sprintf( char *str, const char *format, ... );
extern int vsprintf( char *dest, const char *format, __va_list arg );

extern int scanf( const char *fmt, ... );
extern int vscanf( const char *fmt, __va_list arg );


///////////////////////////////////////////////////////////////////////////////
//
// stdlib.h
//
///////////////////////////////////////////////////////////////////////////////

extern void *malloc( size_t size );
extern void *calloc( size_t nmemb, size_t size );
extern void *realloc( void *ptr, size_t size );
extern void free( void *ptr );


///////////////////////////////////////////////////////////////////////////////
//
// string.h
//
///////////////////////////////////////////////////////////////////////////////

extern void * memchr( const void *s, int c, size_t n);
extern int    memcmp( const void *s1, const void *s2, size_t n);
extern void * memcpy( void *s1, const void *s2, size_t n);
extern void * memmove( void *s1, const void *s2, size_t n);
extern void * memset( void *s, int c, size_t n);
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



#endif //  _CLIB_H_
