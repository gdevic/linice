/******************************************************************************
*                                                                             *
*   Module:     LiniceExt.h                                                   *
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

        Define Linice debugger extension interface.

*******************************************************************************
*   Defines                                                                   *
******************************************************************************/
#ifndef _LINEXT_H_
#define _LINEXT_H_

//////////////////////////////////////////////////////////////////////////////
// PRIVATE STRUCTURES
//////////////////////////////////////////////////////////////////////////////
#define LINICEEXTVERSION        0x100           // Current extension interface version
#define LINICEEXTSIZE       sizeof(TLINICEEXT)  // Current extension interface structure size

//////////////////////////////////////////////////////////////////////////////
// TYPED STRUCTURE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////
// Define structure that holds CPU register state after an interrupt

typedef struct tagLiniceRegs
{
    unsigned int   esp;
    unsigned int   ss;
    unsigned int   es;
    unsigned int   ds;
    unsigned int   fs;
    unsigned int   gs;
    unsigned int   edi;
    unsigned int   esi;
    unsigned int   ebp;
    unsigned int   res0;
    unsigned int   ebx;
    unsigned int   edx;
    unsigned int   ecx;
    unsigned int   eax;
    unsigned int   res1;
    unsigned int   res2;
    unsigned int   eip;
    unsigned int   cs;
    unsigned int   eflags;
} TLINICEREGS;

//////////////////////////////////////////////////////////////////////////////
// CALLBACK FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
// Functions called by Linice to the extension module:

typedef int  (*PEXT_DOT)(char *pCommand);
typedef void (*PEXT_NOTIFY)(int Notification);

#define PEXT_NOTIFY_ENTER       1       // Linice got control
#define PEXT_NOTIFY_LEAVE       2       // Linice lost control

typedef int (*PEXT_QUERYTOKEN)(int *pResult, char *pToken, int len);

// Function called by the extension module to Linice:

typedef TLINICEREGS *(*PLINICEEXT_GETREGS)(void);
typedef int (*PLINICEEXT_EVAL)(int *pValue, char *pExpr, char **ppNext);
typedef int (*PLINICEEXT_DISASM)(char *pBuffer, int sel, int offset);
typedef int (*PLINICEEXT_DPRINT)(char *format,...);
typedef int (*PLINICEEXT_EXEC)(char *pCommand);
typedef int (*PLINICEEXT_GETCH)(int fPolled);
typedef int (*PLINICEEXT_MEMVERIFY)(int sel, int offset, int size);


typedef struct tagLiniceExt
{
//------------------------------------------------------------------
// These need to be filled in by the registrar of the extension
//------------------------------------------------------------------
    int     version;                    // version number
    int     size;                       // size of this structure
    char   *pDotName;                   // Name of the extension dot command
    char   *pDotDescription;            // Optional description string
    struct TLINICEEXT *pNext;           // Ignore this one, for internal use

    // The extension interface module sets these as desired handlers (some may be NULL):
    PEXT_DOT             Command;       // Dot-command evaluator function
    PEXT_NOTIFY          Notify;        // Notification function
    PEXT_QUERYTOKEN      QueryToken;    // Expression evaluator helper

//------------------------------------------------------------------
// Linice fills these in after a successful registration with its own service handlers
//------------------------------------------------------------------
    PLINICEEXT_GETREGS   GetRegs;       // Return the global address of the registers
    PLINICEEXT_EVAL      Eval;          // Expression evaluation function
    PLINICEEXT_DISASM    Disasm;        // Disassemble into a text buffer
    PLINICEEXT_DPRINT    dprint;        // Print into the Linice command line
    PLINICEEXT_EXEC      Execute;       // Execute command
    PLINICEEXT_GETCH     Getch;         // Get input character
    PLINICEEXT_MEMVERIFY MemVerify;     // Verify memory range for access

} TLINICEEXT;

// These functions are exported by Linice and should link with the module
// when it loads into the running kernel by insmod

extern int LiniceRegisterExtension(TLINICEEXT *pExt);
extern void LiniceUnregisterExtension(TLINICEEXT *pExt);

// Return value from LiniceRegisterExtension is one of the following:
#define EXTREGISTER_OK              0   // Extension registered successfully
#define EXTREGISTER_BADVERSION      1   // Invalid version
#define EXTREGISTER_BADHEADER       2   // Invalid size or dot name is null
#define EXTREGISTER_DUPLICATE       3   // This extension has already been registered


#endif // _LINEXT_H_
