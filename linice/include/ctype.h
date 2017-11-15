/*********************************************************************
*                                                                    *
*   Module:     ctype.h                                              *
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       04/13/96                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

        ANSI C / POSIX ctype header file
       
    Note: This file is taken from Yaos project, 2.0 string C library and
          slightly trimmed down

**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  04/13/96   1.00  Original                              Goran Devic
* 08/14/02   2.00  Trimmed down for Linice project       Goran Devic
* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/
#ifndef _CTYPE_H_
#define _CTYPE_H_

/*********************************************************************
*   Local Variables and Defines
**********************************************************************/

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

/* POSIX */

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


/* Non-POSIX */

#define isascii(c)     ((unsigned)(c) < 128)


/*********************************************************************
*   Global Functions
**********************************************************************/


#endif // _CTYPE_H_
