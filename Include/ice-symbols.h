/******************************************************************************
*                                                                             *
*   Module:     ice-symbols.h                                                 *
*                                                                             *
*   Date:       11/26/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains definitions for debugger symbol file.

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/26/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_SYMBOLS_H_
#define _ICE_SYMBOLS_H_

#include "ice-types.h"                  // Include exended data types
#include "ice-limits.h"                 // Include global program limits

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//============================================================================
// 				        LINICE SYMBOL STRUCTURES
//============================================================================

// Define symbol table types (SymTableType)
#define SYMTABLETYPE_UNDEF      0x00    // Undefined symbol type
#define SYMTABLETYPE_KERNEL     0x01    // Linux kernel itself
#define SYMTABLETYPE_MODULE     0x02    // Kernel loadable module
#define SYMTABLETYPE_APP        0x03    // User-level application with "main"

// Define section header types (hType)
#define HTYPE_GLOBALS           0x01    // Global symbols
#define HTYPE_SOURCE            0x02    // Parsed source file
#define HTYPE_FUNCTION_LINES    0x03    // Defines a function line numbers
#define HTYPE_FUNCTION_SCOPE    0x04    // Defines a function variable scope
#define HTYPE_STATIC            0x05    // All static symbols bound to one source file
#define HTYPE_TYPEDEF           0x06    // All typedefs bound to one source file
#define HTYPE_IGNORE            0x07    // This header should be ignored and skipped
#define HTYPE_RELOC             0x08    // Relocation section
#define HTYPE__END              0x00    // (End of the array of headers)

typedef struct
{
//  ------------- constant section -----------------
    BYTE hType;                         // Header type (ID)
    DWORD dwSize;                       // Total size in bytes
//  ------------- type dependent section -----------
} PACKED TSYMHEADER;

typedef struct _SYMTAB
{
    char sSig[4];                       // File signature "SYM"
    char sTableName[MAX_MODULE_NAME];   // Name of this symbol table (app/module)
    WORD Version;                       // Symbol file version number
    BYTE SymTableType;                  // Type of the symbol file
    DWORD dwSize;                       // Total file size
    DWORD dStrings;                     // Offset to the strings section

    struct TSYMTAB *next;               // Next symbol structure in a list

    TSYMHEADER header[1];               // Array of headers []
    //         ...
} PACKED TSYMTAB;


//----------------------------------------------------------------------------
// HTYPE_FUNCTION_SCOPE
// Defines a function run-time execution variable scope
//----------------------------------------------------------------------------

// Token types:
#define TOKTYPE_PARAM       0x01        // Function parameter (argument)
                                        // param: BP+offset to address the parameter
                                        // pName: Pointer to the name definition string

#define TOKTYPE_RSYM        0x02        // Register local variable
                                        // param: register index
                                        // pName: Pointer to the name definition string

#define TOKTYPE_LSYM        0x03        // Local variable kept on the stack
                                        // param: BP-offset to its address
                                        // pName: Pointer to the name definition string

#define TOKTYPE_LCSYM       0x04        // Local static variable
                                        // param: address where is it
                                        // pName: Pointer to the name definition string

#define TOKTYPE_LBRAC       0x05        // LBRAC scope identifier (open scope)
                                        // param: function code offset
                                        // pName: not used, ignore

#define TOKTYPE_RBRAC       0x06        // RBRAC scope identifier (close scope)
                                        // param: function code offset
                                        // pName: not used, ignore

// Function descriptor token:
typedef struct
{
    BYTE TokType;                       // Token type
    DWORD param;                        // Parameter depending on the type
    PSTR  pName;                        // P2 is always a pointer to the name string
    BYTE  bSegment;                     // Segment where the variable resides (TOKTYPE_LCSYM only)

} PACKED TSYMFNSCOPE1;

typedef struct
{
    TSYMHEADER h;                       // Section header

    PSTR  pName;                        // Function name string
    WORD  file_id;                      // Function is defined in this file
    DWORD dwStartAddress;               // Function start address
    DWORD dwEndAddress;                 // Function end address
    WORD nTokens;                       // How many tokens are in the array?

    TSYMFNSCOPE1 list[1];               // Function descriptor tokens
    //     ...
} PACKED TSYMFNSCOPE;


//----------------------------------------------------------------------------
// HTYPE_GLOBALS
// Defines all global symbols: that includes functions and variables
//----------------------------------------------------------------------------

typedef struct
{
    PSTR  pName;                        // Global symbol name string
    PSTR  pDef;                         // Global symbol definition string
    DWORD dwStartAddress;               // Address where the symbol starts
    DWORD dwEndAddress;                 // Address + size of the symbol
    WORD  file_id;                      // Source file ID
    BYTE  bSegment;                     // Segment where the variable resides

} PACKED TSYMGLOBAL1;

typedef struct
{
    TSYMHEADER h;                       // Section header

    DWORD nGlobals;                     // Number of global items in the array
    TSYMGLOBAL1 list[1];                // Array of global symbol descriptors
    //           ...
} PACKED TSYMGLOBAL;


//----------------------------------------------------------------------------
// HTYPE_STATIC
// Defines all static symbols bound to a single source file
//----------------------------------------------------------------------------

typedef struct
{
    PSTR  pName;                        // Static symbol name string
    PSTR  pDef;                         // Static symbol definition string
    DWORD dwAddress;                    // Address of the symbol
    BYTE  bSegment;                     // Segment where the variable resides

} PACKED TSYMSTATIC1;

typedef struct
{
    TSYMHEADER h;                       // Section header

    WORD file_id;                       // Static symbols is defined in this file
    WORD nStatics;                      // How many static symbols are in the array?

    TSYMSTATIC1 list[1];                // Array of static symbol descriptors
    //           ...
} PACKED TSYMSTATIC;


//----------------------------------------------------------------------------
// HTYPE_SOURCE
// Stores a source code file
//----------------------------------------------------------------------------
// pLineArray is the (DWORD dLine[]) array indexing each line in the source code
// They are zero-terminated. The last index is followed by 0. (Also, the nLines
// contains the number of lines).

typedef struct
{
    TSYMHEADER h;                       // Section header

    WORD  file_id;                      // This file's unique ID number
    PSTR  pSourcePath;                  // Source code path/name
    PSTR  pSourceName;                  // Source code name part only
    DWORD nLines;                       // How many lines this file has
    PSTR  pLineArray[1];                // Array of pointers to each ASCIIZ line
    //    ...
} PACKED TSYMSOURCE;

//----------------------------------------------------------------------------
// HTYPE_FUNCTION_LINES
// Defines a function line numbers and offset relation
//----------------------------------------------------------------------------

// Function lines descriptor:
typedef struct
{
    WORD offset;                        // Offset into the function
    WORD line;                          // Line number
    WORD file_id;                       // Source file ID

} PACKED TSYMFNLIN1;

typedef struct
{
    TSYMHEADER h;                       // Section header

    DWORD dwStartAddress;               // Function start address
    DWORD dwEndAddress;                 // Function end address
    WORD nLines;                        // How many lines this function describes

    TSYMFNLIN1 list[1];                 // Line numbers and offsets array
    //     ...
} PACKED TSYMFNLIN;

//----------------------------------------------------------------------------
// HTYPE_TYPEDEF
// Defines all types bound to a single source file
//----------------------------------------------------------------------------
//
// 'Major' typedef number is the file number in which scope the type is defined.
// 'Minor' typedef number is the typedef ordinal number and it starts with 1
//
// There is a special class of typedefs containing the internal basic types
// such is 'int' or 'char' whose definition is not stored as string, but instead
// pDef addresses the index (1..19) (still in the strings section.)

#define TYPEDEF_INT                     1
#define TYPEDEF_CHAR                    2
#define TYPEDEF_LONG_INT                3
#define TYPEDEF_UNSIGNED_INT            4
#define TYPEDEF_LONG_UNSIGNED_INT       5
#define TYPEDEF_LONG_LONG_INT           6
#define TYPEDEF_LONG_LONG_UNSIGNED_INT  7
#define TYPEDEF_SHORT_INT               8
#define TYPEDEF_SHORT_UNSIGNED_INT      9
#define TYPEDEF_SIGNED_CHAR             10
#define TYPEDEF_UNSIGNED_CHAR           11
#define TYPEDEF_FLOAT                   12
#define TYPEDEF_DOUBLE                  13
#define TYPEDEF_LONG_DOUBLE             14
#define TYPEDEF_COMPLEX_INT             15
#define TYPEDEF_COMPLEX_FLOAT           16
#define TYPEDEF_COMPLEX_DOUBLE          17
#define TYPEDEF_COMPLEX_LONG_DOUBLE     18
#define TYPEDEF_VOID                    19
#define TYPEDEF__LAST                   19

typedef struct
{
    WORD maj;                           // Type file number
    WORD min;                           // Type ordinal definition number
    PSTR pName;                         // Typedef name string
    PSTR pDef;                          // Typedef definition string
    WORD file_id;                       // Typedef is defined in this file scope

} PACKED TSYMTYPEDEF1;

typedef struct
{
    SWORD adjust;                       // Major number adjustment value
    WORD file_id;                       // File where this major type is defined

} PACKED TSYMADJUST;

typedef struct
{
    TSYMHEADER h;                       // Section header

    WORD file_id;                       // Typedefs refer to this file unique ID number
    WORD nRel;                          // How many reference adjustment entries?
    WORD nTypedefs;                     // How many typedefs are in the array?

    TSYMADJUST *pRel;                   // External type adjustment array
    TSYMTYPEDEF1 list[1];               // Typedef array
    //           ...
} PACKED TSYMTYPEDEF;


//----------------------------------------------------------------------------
// HTYPE_RELOC
// Relocation information for object files (kernel modules).
//----------------------------------------------------------------------------
//
// Relocation segment type is [0] - .text section (only refOffset used for init_module')
//                            [1] - .data
//                            [2] - .rodata
//                            [3] - .bss
//                            [4...] - { COMMON variables }
// [0].refOffset contains the offset of the init_module symbol from the .text section

typedef struct
{
    DWORD refFixup;                     // Fixup address difference from 'init_module'
    DWORD refOffset;                    // Symbol real offset before relocation
    int reloc;                          // Currently relocated offset - used only within Linice

} PACKED TSYMRELOC1;

typedef struct
{
    TSYMHEADER h;                       // Section header

    WORD nReloc;                        // Number of elements in the relocation list

    TSYMRELOC1 list[1];                 // Array of symbol relocation references
    //     ...
} PACKED TSYMRELOC;

#endif //  _ICE_SYMBOLS_H_

