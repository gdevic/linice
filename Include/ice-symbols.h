/******************************************************************************
*                                                                             *
*   Module:     ice-symbols.h                                                 *
*                                                                             *
*   Date:       11/26/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

        This header file contains definitions for debugger symbol file .

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

//////////////////////////////////////////////////////////////////////////////
// SYMBOL TABLE STRUCTURE
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Public symbols:
//  Global variables and functions from ELF header
//----------------------------------------------------------------------------
#define SECTION_TEXT        1           // Text section ID
#define SECTION_DATA        2           // Data section ID
#define SECTION_BSS         3           // BSS section ID

typedef struct
{
    DWORD dwAddress;                    // Address or offset
    DWORD Section;                      // Section number (.text, .data, .bss,..)
    char *pName;                        // Offset/Pointer to a variable name

} PACKED TSYM_PUB;

typedef struct
{
    DWORD dwAddrStart;                  // Symbol's starting address (bottom)
    DWORD dwAddrEnd;                    // Symbol's ending address (top)
    DWORD nSyms;                        // How many global symbols are defined
    DWORD nHash;                        // Number of entries in a symbol hash table

    TSYM_PUB *pSym;                     // Offset/Pointer to global symbols array
    WORD *pHash;                        // Offset/Pointer to symbols hash table

} PACKED TGLOBAL;

//----------------------------------------------------------------------------
// Type information symbols:
//  Type information from STABS
//----------------------------------------------------------------------------
typedef struct                          // Define type array entry structure
{
    char *pName;                        // Offset/Pointer to type name string
    char *pDef;                         // Offset/Pointer to type definition string

} PACKED TONETYPE;

typedef struct
{
    DWORD nTypes;                       // Number of type declarations
    TONETYPE *pType;                    // Offset/Pointer to type decl. array

} PACKED TTYPE;

//----------------------------------------------------------------------------
// Source information symbols:
//  Source line numbers
//  Function scope and lexical blocks
//  Local variables and function parameter variables
//----------------------------------------------------------------------------
typedef struct                          // Define file database entry structure
{
    char *pFileName;                    // Offset/Pointer to a file name
    WORD  nLine;                        // Number of lines
    char **pLine;                       // Array of line numbers -> Offset/Pointer to line

} PACKED TFILE;

typedef struct                          // Define line number information array element
{
    DWORD dwAddress;                    // Address of the instruction
    WORD dwLine;                        // Line number
    WORD FileNo;                        // File number

} PACKED TLINE;

// Lexical scope array lists all local variables (automatic and parameters) along
// with their scope offsets from the start of the function. When you are inside the
// function it is easy to find which variables are in scope by traversing this array and
// comparing the EIP to it's bounds.

typedef struct                          // Define lexical scope entry structure
{
#define LEX_PARAM       0x01            // Parameter (argument) to a function
#define LEX_LOCAL       0x02            // Local variable
#define LEX_RBRAC       0x04            // Open lexical scope
#define LEX_LBRAC       0x08            // Close lexical scope
    BYTE token;                         // Token type
    DWORD value;                        // Different value store
    char *pName;                        // Offset/Pointer to a variable name/definition string

} PACKED TLEX;

// Function domain. Each function is defined by one of these structures. They are
// placed in an array.

typedef struct                          // Define function domain structure
{
    DWORD dwAddress;                    // Start address
    DWORD dwEndAddress;                 // End address
    char  *pName;                       // Offset/Pointer to a function name
    WORD   nLine;                       // Number of line number entries
    TLINE *pLine;                       // Offset/Pointer to line info array
    WORD   nLex;                        // Number of lexical scope variables
    TLEX  *pLex;                        // Offset/Pointer to lexical scope array

} PACKED TFUN;


typedef struct                          // Define head symbol structure
{
    WORD nFun;                          // Number of entries in the function def. table
    TFUN *pFun;                         // Offset/Pointer to a function def. table

} PACKED TSYMBOL;

typedef struct                          // Define source file structure
{
    WORD nFile;                         // Number of files in the array
    TFILE *pFile;                       // Offset/Pointer to file array

} PACKED TSRC;


//============================================================================
// Symbol file header
//============================================================================
typedef struct tagSYMTAB
{
    // Symbol file signature - this field must appear first in the structure
    char Sig[16];                       // Symbol file signature

    DWORD size;                         // Size of the complete symbol table
    char name[MAX_MODULE_NAME];         // Name of the symbol table module
    struct TSYMTAB *next;               // Next table in a list

    char *pStrings;                     // Offset/Pointer to strings

    TGLOBAL *pGlobals;                  // Offset/Pointer to globals structure
    TTYPE   *pTypes;                    // Offset/Pointer to type structure
    TSYMBOL *pSymbols;                  // Offset/Pointer to symbol structure
    TSRC    *pSrc;                      // Offset/Pointer to src structure

} PACKED TSYMTAB;


#endif //  _ICE_SYMBOLS_H_

