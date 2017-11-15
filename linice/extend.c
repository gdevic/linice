/******************************************************************************
*                                                                             *
*   Module:     extend.c                                                      *
*                                                                             *
*   Date:       08/23/04                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This module implements debugger interface extension for custom
        modules.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 08/23/04   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "stdarg.h"                     // Include variable argument header
#include "disassembler.h"               // Include disassembler

#include "LiniceExt.h"                  // Include extension include file

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

static TLINICEEXT *pRootExt = NULL;     // Pointer to the list of extensions

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*   TLINICEREGS *ExtGetRegs(void)
*******************************************************************************
*
*   Returns the address of the debugee register state
*
******************************************************************************/
static TLINICEREGS *ExtGetRegs(void)
{
    return( (TLINICEREGS *) deb.r );
}

/******************************************************************************
*   int ExtEval(char *pExpression)
*******************************************************************************
*
*   Evaluates a string into a value
*
******************************************************************************/
static int ExtEval(int *pValue, char *pExpr, char **ppNext)
{
    return( Expression(pValue, pExpr, ppNext) );
}

/******************************************************************************
*   int ExtDisasm(char *pBuffer, int Address)
*******************************************************************************
*
*   Disassemble into a string
*
******************************************************************************/
static int ExtDisasm(char *pBuffer, int sel, int offset)
{
    TDISASM dis;                        // Disassembler interface structure

    // Disassemble the current instruction
    // If the sel argument is 0, use kernel code selector

    dis.bState   = DIS_DATA32 | DIS_ADDRESS32;
    dis.wSel     = sel? sel : GetKernelCS();
    dis.dwOffset = offset;
    dis.szDisasm = pBuffer;

    // Disassemble and store into the line buffer
    Disassembler( &dis );

    return( dis.bInstrLen );
}

/******************************************************************************
*   int Extdprint(char *format,...)
*******************************************************************************
*
*   Prints into Linice console
*
*   NOTE: It is possible to overrun the buffer !!
*
******************************************************************************/
static int Extdprint(char *format,...)
{
    va_list arg;
    int written;                        // Number of characters written
    char buf[MAX_STRING];               // Temp print buffer

    // Print into our buffer first, then use standard print function
    va_start( arg, format );
    ivsprintf(buf, format, arg);
    va_end(arg);

    written = dprinth(1, buf);

    return( written );
}

/******************************************************************************
*   int ExtExecute(char *pCommand)
*******************************************************************************
*
*   Executes a Linice command
*
******************************************************************************/
static int ExtExecute(char *pCommand)
{
    return( CommandExecute(pCommand) );
}

/******************************************************************************
*   int ExtGetch(int fPolled)
*******************************************************************************
*
*   Returns a character from the input stream
*
******************************************************************************/
static int ExtGetch(int fPolled)
{
    return( GetKey(fPolled) );
}

/******************************************************************************
*   int ExtMemVerify(int sel, int offset, int size)
*******************************************************************************
*
*   Verifies a memory range for access
*
******************************************************************************/
static int ExtMemVerify(int sel, int offset, int size)
{
    TADDRDESC Addr;                     // Address descriptor

    // If the sel argument is 0, use the kernel DS
    Addr.sel    = sel? sel : GetKernelDS();
    Addr.offset = offset;

    return( VerifyRange(&Addr, size) );
}


/******************************************************************************
*   void ExtZapCallbacks(TLINICEEXT *pExt)
*******************************************************************************
*
*   Zaps all the Linice callbacks of an extension structure
*
******************************************************************************/
static void ExtZapCallbacks(TLINICEEXT *pExt)
{
    // Store the RET opcode into the pNext field and point all callbacks to it

    pExt->pNext = (struct TLINICEEXT *)(0xC3909090);

    pExt->GetRegs   = (PLINICEEXT_GETREGS)   (&pExt->pNext);
    pExt->Eval      = (PLINICEEXT_EVAL)      (&pExt->pNext);
    pExt->Disasm    = (PLINICEEXT_DISASM)    (&pExt->pNext);
    pExt->dprint    = (PLINICEEXT_DPRINT)    (&pExt->pNext);
    pExt->Execute   = (PLINICEEXT_EXEC)      (&pExt->pNext);
    pExt->Getch     = (PLINICEEXT_GETCH)     (&pExt->pNext);
    pExt->MemVerify = (PLINICEEXT_MEMVERIFY) (&pExt->pNext);
}

/******************************************************************************
*                                                                             *
*   int LiniceRegisterExtension(TLINICEEXT *pExt)                             *
*                                                                             *
*******************************************************************************
*
*   Registers a new debugger extension.
*
*   Where:
*       pExt is the pointer to the extension structure
*
*   Returns:
*       One of the return (error) codes
*
******************************************************************************/
int LiniceRegisterExtension(TLINICEEXT *pExt)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    int retval = EXTREGISTER_OK;        // Assume interface OK

    // Check the validity of the interface header
    if( pExt )
    {
        if( pExt->version==LINICEEXTVERSION )
        {
            if( pExt->size==sizeof(TLINICEEXT) && pExt->pDotName!=NULL )
            {
                // Add extension to the list of extensions (to the head of the list)
                // But first we need to make sure that block has not been already
                // registered (as we may get broken linked list)
                while( p )
                {
                    if( p==pExt )
                        return( EXTREGISTER_DUPLICATE );

                    (struct TLINICEEXT *) p = p->pNext;
                }

                // Next we need to check that this particular command has not been
                // registered. Although harmless to us, it would make no sense.
                p = pRootExt;           // Rewind the pointer
                while( p )
                {
                    if( p->pDotName==pExt->pDotName )
                        return( EXTREGISTER_DUPLICATE );

                    (struct TLINICEEXT *) p = p->pNext;
                }

                // We are ok to register this handler

                pExt->pNext = (struct TLINICEEXT *) pRootExt;
                pRootExt = pExt;

                // Fill up the callback pointers
                pExt->GetRegs   = (PLINICEEXT_GETREGS)   ExtGetRegs;
                pExt->Eval      = (PLINICEEXT_EVAL)      ExtEval;
                pExt->Disasm    = (PLINICEEXT_DISASM)    ExtDisasm;
                pExt->dprint    = (PLINICEEXT_DPRINT)    Extdprint;
                pExt->Execute   = (PLINICEEXT_EXEC)      ExtExecute;
                pExt->Getch     = (PLINICEEXT_GETCH)     ExtGetch;
                pExt->MemVerify = (PLINICEEXT_MEMVERIFY) ExtMemVerify;

                dprinth(1, "EXT Register: %8s", pExt->pDotName);
            }
            else
                retval = EXTREGISTER_BADHEADER;
        }
        else
            retval = EXTREGISTER_BADVERSION;
    }
    else
        retval = EXTREGISTER_BADHEADER;

    return( retval );
}

/******************************************************************************
*                                                                             *
*   void LiniceUnregisterExtension(TLINICEEXT *pExt)                          *
*                                                                             *
*******************************************************************************
*
*   Unregisters a debugger extension.
*
*   Where:
*       pExt is the pointer to the extension structure that was already registered
*
*   Returns:
*       None
*
******************************************************************************/
void LiniceUnregisterExtension(TLINICEEXT *pExt)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    TLINICEEXT *pPrev = NULL;           // Previous node (need for deletion)

    // Traverse the list and find the extension by the address
    while( p )
    {
        if( p==pExt )
        {
            // Found the address of the structure, unlink it from the linked list
            if( pExt==pRootExt )
            {
                // The extension is the first node on the list
                (struct TLINICEEXT *) pRootExt = pExt->pNext;
            }
            else
            {
                // The extension is not the first node on the list
                pPrev->pNext = pExt->pNext;
            }

            // Set the callback functions to its dummy RET, so if they are
            // called by mistake, nothing will happen.
            ExtZapCallbacks(pExt);

            dprinth(1, "EXT Unregister: %s", pExt->pDotName);

            return;
        }

        pPrev = p;
        (struct TLINICEEXT *) p = p->pNext;
    }
}

/******************************************************************************
*                                                                             *
*   BOOL cmdExtList (char *args, int subClass)                                *
*                                                                             *
*******************************************************************************
*
*   List all registered DOT commands.
*
******************************************************************************/
BOOL cmdExtList (char *args, int subClass)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    int nLine = 1;                      // Standard dprinth line counter

    while( p )
    {
        dprinth(nLine++, "  %-8s %s", p->pDotName, p->pDotDescription? p->pDotDescription : "");

        (struct TLINICEEXT *) p = p->pNext;
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   int DispatchExtCommand(char *pCommand)                                    *
*                                                                             *
*******************************************************************************
*
*   Dispatch DOT command to the registered handler
*
*   Where:
*       pCommand is the extension command name followed by arguments
*
*   Returns:
*       TRUE
*
******************************************************************************/
int DispatchExtCommand(char *pCommand)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    int nLen = 0;                       // Length of the command token

    // Find the total length of the command token
    while( isalnum(*(pCommand+nLen)) )
        nLen++;

    if( nLen )
    {
        // We have a valid DOT-command token. Search all registered commands for it.

        while( p )
        {
            if( !strnicmp(pCommand, p->pDotName, nLen) && *(p->pDotName+nLen)=='\0' )
            {
                // We found the extension handler

                if( p->Command )
                    p->Command(pCommand+nLen);

                return( TRUE );
            }

            (struct TLINICEEXT *) p = p->pNext;
        }
        // TODO: error here
    }
    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   void DispatchExtNotify(int signal)                                        *
*                                                                             *
*******************************************************************************
*
*   Dispatch various signals to all extended handlers.
*
*   Where:
*       signal is the notify message.
*
******************************************************************************/
static void DispatchExtNotify(int signal)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    TLINICEEXT *pNext;                  // Prefetch next pointer

    while( p )
    {
        // Prefetch pNext pointer so we are safe if something happens with the
        // extension structure within their handlers
        (struct TLINICEEXT *) pNext = p->pNext;

        if( p->Notify )
            p->Notify(signal);

        p = pNext;
    }
}

/******************************************************************************
*   void DispatchExtEnter()                                                   *
*******************************************************************************
*
*   Dispatch the debugger enter signal
*
******************************************************************************/
void DispatchExtEnter()
{
    DispatchExtNotify(PEXT_NOTIFY_ENTER);
}

/******************************************************************************
*   void DispatchExtLeave()                                                   *
*******************************************************************************
*
*   Dispatch the debugger leave signal
*
******************************************************************************/
void DispatchExtLeave()
{
    DispatchExtNotify(PEXT_NOTIFY_LEAVE);
}

/******************************************************************************
*                                                                             *
*   int QueryExtModule()                                                      *
*                                                                             *
*******************************************************************************
*
*   Returns TRUE if there are any modules registered.
*
******************************************************************************/
int QueryExtModule()
{
    return( (int) pRootExt );
}

/******************************************************************************
*                                                                             *
*   BOOL QueryExtToken(DWORD *pResult, char **pToken, int len)                *
*                                                                             *
*******************************************************************************
*
*   Query the extensions for the token.
*
*   Where:
*       pResult is where the final value should be stored
*       pToken is the pointer to a token address
*       len is the initial suggested token len
*
******************************************************************************/
BOOL QueryExtToken(DWORD *pResult, char **pToken, int len)
{
    TLINICEEXT *p = pRootExt;           // Traverse pointer
    int Result;                         // Local result value
    int retLen;                         // Return length parameter

    // Traverse all external commands, call their query token function,
    // return when one of them is able to evaluate it
    while( p )
    {
        if( p->QueryToken )
        {
            retLen = p->QueryToken(&Result, *pToken, len);

            // If the retLen is non-zero, the token has been evaluated
            if( retLen )
            {
                *pResult = Result;      // Store the result
                *pToken += retLen;      // Advance our pointer

                return( TRUE );
            }
        }

        (struct TLINICEEXT *) p = p->pNext;
    }

    return( FALSE );
}
