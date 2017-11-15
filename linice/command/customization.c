/******************************************************************************
*                                                                             *
*   Module:     customization.c                                               *
*                                                                             *
*   Date:       10/15/00                                                      *
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

        Customization commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/15/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

#include "debug.h"                      // Include our dprintk()

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern PTOUT pOut;                      // Pointer to a current Out class
extern TOUT outVga;                     // VGA output device
extern TOUT outMda;                     // MDA output device
extern TOUT outVT100;                   // Serial VT100 output device
extern TOUT outDga;                     // DGA X-Window frame buffer output device

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Structure used by SET command
typedef struct
{
    char *sVar;                         // Variable name
    int  sLen;                          // Length of the name
    BOOL *pVal;                         // Address of the value variable
    BYTE type;                          // Type of the variable: VAR_*
    BYTE test;                          // Test condition number for the new value
} TSETVAR, *PTSETVAR;

#define VAR_BOOL        0               // Boolean variable (on/off)
#define VAR_DWORD       1               // Dword variable

static TSETVAR SetVar[] = {
{ "altscr",   6, &deb.fAltscr,      VAR_BOOL , 0 },
{ "code" ,    4, &deb.fCode,        VAR_BOOL , 0 },
{ "faults",   6, &deb.fFaults,      VAR_BOOL , 0 },
{ "i1here",   6, &deb.fI1Here,      VAR_BOOL , 0 },
{ "i3here",   6, &deb.fI3Here,      VAR_BOOL , 0 },
{ "lowercase",9, &deb.fLowercase,   VAR_BOOL , 0 },
{ "pause",    5, &deb.fPause,       VAR_BOOL , 0 },
{ "symbols",  7, &deb.fSymbols,     VAR_BOOL , 0 },
{ "flash",    5, &deb.fFlash,       VAR_BOOL , 0 },
{ "font",     4, &deb.nFont,        VAR_DWORD, 1 },   // Test condition 1 - font size
{ "framex",   6, &deb.FrameX,       VAR_DWORD, 2 },   // Test condition 2 - frame X coordinate
{ "framey",   6, &deb.FrameY,       VAR_DWORD, 3 },   // Test condition 3 - frame Y coordinate
{ "autoon",   6, &deb.fTableAutoOn, VAR_BOOL , 0 },   // TABLE AUTOON | AUTOOFF
{ "pfprotect",9, &deb.fPfProtect,   VAR_BOOL , 0 },   // Internal: PF Protect
{ "syscall",  7, &deb.fSyscall,     VAR_BOOL , 0 },   // Display system calls from with our hook
{ NULL, }
};

// User variables and macros structure
typedef struct
{
    char *pName;                        // Name string in the heap
    char *pValue;                       // Value string in the heap
} TNAMEVAL;

static TNAMEVAL *pVars = NULL;          // User variables
static TNAMEVAL *pMacros = NULL;        // User macros


extern char *pCmdEdit;                  // Push edit line string
static char Buf[MAX_STRING];            // Temp line buffer to send line to edit

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void MdaInit();
extern BOOL SerialInit(int com, int baud);
extern int InitVT100(void);
extern void SerialPrintStat();

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL InitUserVars(int nVars)
{
    if( (pVars = (TNAMEVAL *) mallocHeap(deb.hHeap, nVars * sizeof(TNAMEVAL)))==NULL )
        return(FALSE);

    memset(pVars, 0, nVars * sizeof(TNAMEVAL));

    return(TRUE);
}


BOOL InitMacros(int nMacros)
{
    if( (pMacros = (TNAMEVAL *) mallocHeap(deb.hHeap, nMacros * sizeof(TNAMEVAL)))==NULL )
        return(FALSE);

    memset(pMacros, 0, nMacros * sizeof(TNAMEVAL));

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL GetUserVar(DWORD *pValue, char *sStart, int nLen)                    *
*                                                                             *
*******************************************************************************
*
*   Finds user variable and returns its numerical evaluation.
*
*   Where:
*       pValue - the address to store value if successful
*       sStart - variable name
*       nLen - variable name len
*
*   Returns:
*       TRUE if variable found, evaluated and stored
*       FALSE if variable not found
*
******************************************************************************/
BOOL GetUserVar(DWORD *pValue, char *sStart, int nLen)
{
    int i;

    // Search for that variable name
    for(i=0; i<deb.nVars; i++ )
    {
        if( pVars[i].pName )
            if( strlen(pVars[i].pName)==nLen )
                if( !strnicmp(pVars[i].pName, sStart, nLen) )
                    break;
    }

    // If found, evaluate it and store the result
    if( i < deb.nVars )
    {
        Expression(pValue, pVars[i].pValue, NULL);

        return(TRUE);
    }

    return(FALSE);
}

/******************************************************************************
*                                                                             *
*   void VarMacro(char *sOp, char *args, TNAMEVAL *pObject, int nElem)        *
*                                                                             *
*******************************************************************************
*
*   This code handles both user macros and variables. Variables are extension
*   of the legacy macros; where macros can be used to perform a set of operations,
*   variables can be used to evaluate expression (lparm vs. rparm).
*
*   We keep macros and variables in arrays of TNAMEVAL structures.
*
*   Where:
*       sOp - operation string ("VAR" or "MACRO")
*       args - original args argument
*       pObject - pointer to object array
*       nElem - number of elements in that object
*
******************************************************************************/
static void VarMacro(char *sOp, char *args, TNAMEVAL *pObject, int nElem)
{
    int nLine = 1;
    int i, nameLen, valueLen;
    char *ptr;

    if( *args==0 )
    {
        // No parameters - display all macros
        //--------------------------------------------------------------------
        for(i=0; i<nElem; i++)
        {
            if( pObject[i].pName )
                if( dprinth(nLine++, "%s = %s", pObject[i].pName, pObject[i].pValue)==FALSE )
                    break;
        }
    }
    else
    {
        // Set or change a macro, if '*', delete all macros
        if( *args=='*' )
        {
            // Parameter '*' - delete all macros
            //----------------------------------------------------------------
            for(i=0; i<nElem; i++ )
            {
                if( pObject[i].pName != NULL )
                {
                    freeHeap(deb.hHeap, pObject[i].pName);
                    freeHeap(deb.hHeap, pObject[i].pValue);
                    pObject[i].pName = NULL;
                    pObject[i].pValue = NULL;
                }
            }
        }
        else
        {
            // Assignment of a new macro or a redefinition
            //----------------------------------------------------------------
            // Look for the macro name length
            ptr = args;
            while( *ptr && *ptr!=' ' && *ptr!='=' ) ptr++;  // End of token
            nameLen = ptr - args;
            while( *ptr && *ptr==' ' ) ptr++;               // Skip spaces

            // Search for the macro of that name and store its index into 'i'
            for(i=0; i<nElem; i++ )
            {
                if( pObject[i].pName )
                    if( strlen(pObject[i].pName)==nameLen )
                        if( !strnicmp(pObject[i].pName, args, nameLen) )
                            break;
            }

            // If only the name is given, edit that particular macro
            //----------------------------------------------------------------
            if( *ptr==0 )
            {
                // EDIT
                if( i < nElem )
                {
                    sprintf(Buf, "%s %s = %s", sOp, pObject[i].pName, pObject[i].pValue);
                    pCmdEdit = Buf;
                }
                else
                {
                    dprinth(nLine++, "%s %s not defined", sOp, args);
                }
            }
            else
            {
                // Skip the assignment character
                if( *ptr=='=' ) ptr++;

                while( *ptr && *ptr==' ' ) ptr++;           // Skip spaces
                valueLen = strlen(ptr);

                // If the value is not given, delete that particular macro
                //------------------------------------------------------------
                if( valueLen==0 )
                {
                    if( i < nElem )
                    {
                        freeHeap(deb.hHeap, pObject[i].pName);
                        freeHeap(deb.hHeap, pObject[i].pValue);
                        pObject[i].pName = NULL;
                        pObject[i].pValue = NULL;
                    }
                    else
                        PostError(ERR_SYNTAX, 0);
                }
                else
                {
                    // If that macro exists, redefine it
                    //--------------------------------------------------------
                    if( i < nElem )
                    {
                        // We found a macro with that name.. Delete its value
                        freeHeap(deb.hHeap, pObject[i].pValue);
                        pObject[i].pValue = NULL;
                    }
                    else
                    {
                        // New macro.. search for an empty slot
                        for(i=0; i<nElem; i++ )
                        {
                            if( pObject[i].pName==NULL )
                                break;
                        }

                        if( i==nElem )
                        {
                            dprinth(nLine++, "Out of entry slots.");
                            return;
                        }

                        // Allocate space for the name and copy it there
                        pObject[i].pName = mallocHeap(deb.hHeap, nameLen + 1);
                        if( pObject[i].pName==NULL )
                        {
                            PostError(ERR_MEMORY, 0);
                            return;
                        }

                        memcpy(pObject[i].pName, args, nameLen);
                        pObject[i].pName[nameLen] = 0;
                    }

                    // We have name, we just need to add value string
                    pObject[i].pValue = mallocHeap(deb.hHeap, valueLen + 1);
                    if( pObject[i].pValue==NULL )
                    {
                        freeHeap(deb.hHeap, pObject[i].pName);
                        pObject[i].pName = NULL;

                        PostError(ERR_MEMORY, 0);
                        return;
                    }

                    memcpy(pObject[i].pValue, ptr, valueLen + 1);
                }
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   char *MacroExpand(char *pCmd)                                             *
*                                                                             *
*******************************************************************************
*
*   This function is used from within the command line interpreter to detect
*   and expand macro commands. pCmd points to the original string with the
*   macro command at the start of it. Macro is being expanded into a global Buf
*   buffer.
*
*   Where:
*       pCmd - buffer containing the command and arguments
*
*   Returns:
*       NULL - if the command is not a recognized macro
*       pointer to a static buffer with expanded macro body and arguments
*               substituted.
*
******************************************************************************/
char *MacroExpand(char *pCmd)
{
    int i, nLen, src, arg;
    char *ptr;
    char *args[8];                      // Pointers to macro arguments
    int arglen[8];                      // Argument length in characters

    // Get the length of the potential macro command and advance ptr to the
    // first argument
    ptr = pCmd;
    while( *ptr && *ptr!=' ' ) ptr++;
    nLen = ptr - pCmd;

    // Scan the rest of the command line and fill in the macro argument pointers
    for(i=0; i<8; i++ )
    {
        // Skip the heading spaces
        while( *ptr && *ptr==' ' ) ptr++;

        if( *ptr )
            args[i] = ptr;
        else
            args[i] = NULL;

        // Skip the argument token; either a nonspace token or a string
        if( *ptr=='"' )
        {
            ptr++;
            args[i]++;
            while( *ptr && *ptr!='"' ) ptr++;
            arglen[i] = ptr - args[i];
            ptr++;
        }
        else
        {
            while( *ptr && *ptr!=' ' ) ptr++;
            arglen[i] = ptr - args[i];
        }
    }

    // Look if the command is a defined macro
    for(i=0; i<deb.nMacros; i++ )
    {
        if( pMacros[i].pName )
            if( strlen(pMacros[i].pName)==nLen )
                if( !strnicmp(pMacros[i].pName, pCmd, nLen) )
                    break;
    }

    // If found the macro, start copying it to the temp buffer
    if( i < deb.nMacros )
    {
        for(src = nLen = 0; nLen<MAX_STRING; src++ )
        {
            // Special cases are \% -> %, \\ -> \, \" -> "
            if( pMacros[i].pValue[src]=='\\' )
            {
                if( pMacros[i].pValue[src+1]=='\\'
                 || pMacros[i].pValue[src+1]=='%'
                 || pMacros[i].pValue[src+1]=='"')
                {
                    src++;
                    Buf[nLen++] = pMacros[i].pValue[src];

                    continue;
                }
            }

            // If the source character is '%n', do argument substitution
            if( pMacros[i].pValue[src]=='%'
                && pMacros[i].pValue[src+1]>='1'        // Between %1 - %8
                && pMacros[i].pValue[src+1]<='8')
            {
                // Substitute a given argument number
                arg = pMacros[i].pValue[src+1] - '1';

                // If the argument is given, use it. Otherwise, ignore it
                if( args[arg] != NULL )
                {
                    if( nLen+arglen[arg] < MAX_STRING )
                    {
                        memcpy(&Buf[nLen], args[arg], arglen[arg]);
                        nLen += arglen[arg];
                    }
                }

                // Skip '%n' in the macro body
                src += 2 - 1;
            }
            else
            {
                // Copy a character of the macro body, skip "
                if( pMacros[i].pValue[src]!='"' )
                    Buf[nLen++] = pMacros[i].pValue[src];
            }
        }

        // Shorten the final line as much as we can
        i=MAX_STRING-1;
        while( i && Buf[i]==' ' ) i--;
        Buf[i+1] = 0;

        return(Buf);
    }

    // Did not find the macro, return NULL
    return(NULL);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdVar(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Set or change a user variable. Without parameters, display all user variables.
*   With parameters, set or change a user variable.
*   With parameter '*', delete all entries.
*
******************************************************************************/
BOOL cmdVar(char *args, int subClass)
{
    if( pVars )
        VarMacro("VAR", args, pVars, deb.nVars);

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdVar(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Set or change a user macro. Without parameters, display all user macros.
*   With parameters, set or change a user macro.
*   With parameter '*', delete all entries.
*
******************************************************************************/
BOOL cmdMacro(char *args, int subClass)
{
    if( pMacros )
        VarMacro("MACRO", args, pMacros, deb.nMacros);

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdAltkey(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   ALTKEY [Alt letter | Ctrl letter]
*
******************************************************************************/
BOOL cmdAltkey(char *args, int subClass)
{
    WCHAR Key = 0;

    if( *args==0 )
    {
        // No arguments - display key sequence
        dprinth(1, "Key sequence is %s %c",
            deb.BreakKey & CHAR_CTRL? "CTRL" : "ALT", deb.BreakKey & 0x7F);
    }
    else
    {
        // Argument is Ctrl/Alt letter
        if( !strnicmp("ctrl", args, 4) )
            Key |= CHAR_CTRL, args += 4;
        else
        if( !strnicmp("alt", args, 3) )
            Key |= CHAR_ALT, args += 3;

        // Skip spaces before a key letter
        while( *args==' ' ) args++;
        Key |= toupper(*args);

        if( Key==0 || !isalpha(Key & 0x7F) )
            PostError(ERR_SYNTAX, 0);
        else
            deb.BreakKey = Key;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdPause(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   PAUSE [on | off]    - Pause after each screen of scrolling information
*   PAUSE               - Display state of the pause setting
*
******************************************************************************/
BOOL cmdPause(char *args, int subClass)
{
    switch( GetOnOff(args) )
    {
        case 1:         // On
            deb.fPause = TRUE;
        break;

        case 2:         // Off
            deb.fPause = FALSE;
        break;

        case 3:         // Display the state of the PAUSE variable
            dprinth(1, "Pause is %s", deb.fPause? "on":"off");
        break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdCode(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   CODE [on | off]     - Set code view in disassembly output
*   CODE                - Display state of the code setting
*
******************************************************************************/
BOOL cmdCode(char *args, int subClass)
{
    switch( GetOnOff(args) )
    {
        case 1:         // On
            deb.fCode = TRUE;
            deb.fRedraw = TRUE;
        break;

        case 2:         // Off
            deb.fCode = FALSE;
            deb.fRedraw = TRUE;
        break;

        case 3:         // Display the state of the CODE view
            dprinth(1, "Code is %s", deb.fCode? "on":"off");
        break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdSet(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   SET internal variable list or set a new value
*
******************************************************************************/
BOOL DisplayVariable(PTSETVAR pVar, int *pnLine)
{
    switch( pVar->type )
    {
        case VAR_BOOL:
            if(dprinth(*pnLine++, "%s is %s", pVar->sVar, *pVar->pVal? "on":"off")==FALSE)
                return( TRUE );
            break;

        case VAR_DWORD:
            if(dprinth(*pnLine++, "%s is %d", pVar->sVar, *pVar->pVal)==FALSE)
                return( TRUE );
            break;
    }

    return( FALSE );
}

BOOL cmdSet(char *args, int subClass)
{
    int nLine = 1;
    PTSETVAR pVar;                      // Pointer to current variable
    DWORD value;                        // New value

    pVar = SetVar;

    if( *args==0 )
    {
        // Simple SET command without parameters - list all variables
        while( pVar->sVar )
        {
            if( DisplayVariable(pVar, &nLine) )
                return( TRUE );
            pVar++;
        }
    }
    else
    {
        // Set a variable
        // Find the variable name
        while( pVar->sVar )
        {
            if( !strnicmp(args, pVar->sVar, pVar->sLen) )
                break;

            pVar++;
        }

        // If we did not find a predefined variable...
        if( pVar->sVar==NULL )
        {
            dprinth(1, "Set variable not found");
        }
        else
        {
            // Advance the argument pointer pass the var name and into the
            // possible argument of variable type
            args += pVar->sLen;
            while( *args==' ' ) args++;

            if( *args )
            {
                // Set new value depending on the variable type
                switch( pVar->type )
                {
                    case VAR_BOOL:

                        switch( GetOnOff(args) )
                        {
                            case 1:         // On
                                *pVar->pVal = TRUE;
                                deb.fRedraw = TRUE;
                            break;

                            case 2:         // Off
                                *pVar->pVal = FALSE;
                                deb.fRedraw = TRUE;
                            break;
                        }
                    break;

                    case VAR_DWORD:
                            Expression(&value, args, NULL);

                            // Run the appropriate test condition on the new variable value
                            switch( pVar->test )
                            {
                                case 1:     // FONT size
                                    if( value>=0 && value <MAX_FONTS )
                                    {
                                        // We have specified a new font size

                                        // It is the responsibility of the output device to actually
                                        // set new sizes in its variables after verifying them...
                                        if( (pOut->resize)(pOut->sizeX, pOut->sizeY, value)==TRUE )
                                        {
                                            // New font value was accepted in the resize(), just repaint windows
                                            deb.fRedraw = TRUE;
                                        }
                                    }
                                    else
                                        dprinth(nLine++, "FONT index has to be between 0 and %d", MAX_FONTS-1);
                                    break;

                                // TODO: Since setting the frame coordinates could be done at any time,
                                //       without calling the code to check it, one can intentionally crash
                                //       the DGA output by moving the window out while on some other output
                                //       device...
//                                case 2:     // FRAMEX initial coordinate
//                                    break;

//                                case 3:     // FRAMEY initial coordinate
//                                    break;

                                // By default, accept a value with no test
                                default:
                                    *pVar->pVal = value;
                                    break;
                            }
                        break;
                }
            }
            else
            {
                // If there were no arguments, display that variable value

                DisplayVariable(pVar, &nLine);
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdResize(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display or change number of display lines or width
*
*   Where:
*       subClass = 0  command WIDTH (X-size)
*       subClass = 1  command LINES (Y-size)
*
******************************************************************************/
BOOL cmdResize(char *args, int subClass)
{
    DWORD value;
    int x, y;

    x = pOut->sizeX;
    y = pOut->sizeY;

    if( *args==0 )
    {
        // No arguments - display the current screen size
        dprinth(1, "Screen size is: LINES=%d WIDTH=%d", y, x);
    }
    else
    {
        // Set the new number of lines or width
        if( GetDecB(&value, &args) && !*args )
        {
            if( subClass==0 )
                x = value;              // Command was WIDTH
            else
                y = value;              // Command was LINES

            if( y >= 24 && y <= MAX_OUTPUT_SIZEY )
            {
                if( x >= 80 && x <= MAX_OUTPUT_SIZEX )
                {
                    // Check if the parameters are the same as active, so we
                    // dont call function if we don't have to
                    if( x!=pOut->sizeX || y!=pOut->sizeY )
                    {
                        // Call the resize function of the output device
                        // This function will do all necessary checking depending
                        // on the device's capability.

                        // It is the responsibility of the output device to actually
                        // set new sizes in its variables after verifying them...
                        if( (pOut->resize)(x, y, deb.nFont)==TRUE )
                        {
                            // If the resize returned true, repaint windows
                            deb.fRedraw = TRUE;
                        }
                    }
                }
                else
                    dprinth(1, "Width has to be in the range [80, %d]", MAX_OUTPUT_SIZEX);
            }
            else
                dprinth(1, "Lines have to be in the range [24, %d]", MAX_OUTPUT_SIZEY);
        }
        else
            PostError(ERR_SYNTAX, 0);
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdColor(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Display or set the screen colors:
*   COLOR [normal bold reverse help line]
*
*   Reset default screen colors if argument is '-'
*
******************************************************************************/
BOOL cmdColor(char *args, int subClass)
{
    DWORD mycol[6];
    BOOL fValid;
    int i;

    if( *args==0 )
    {
        // No arguments - display screen colors
#if 0
        dprinth(1, "Colors are %02X %02X %02X %02X %02X",
            deb.col[COL_NORMAL],
            deb.col[COL_BOLD],
            deb.col[COL_REVERSE],
            deb.col[COL_HELP],
            deb.col[COL_LINE] );
#endif
        // IMPROVEMENT: Displayed colors are described
        dprinth(1, "Colors are: Normal: %02X  Bold: %02X  Reverse: %02X  Help: %02X  Lines: %02X",
            deb.col[COL_NORMAL],
            deb.col[COL_BOLD],
            deb.col[COL_REVERSE],
            deb.col[COL_HELP],
            deb.col[COL_LINE] );
    }
    else
    {
        // Set screen colors
        if( *args=='-' )
        {
            // Reset default screen colors and repaint the window

            deb.col[COL_NORMAL]  = 0x07;
            deb.col[COL_BOLD]    = 0x0B;
            deb.col[COL_REVERSE] = 0x71;
            deb.col[COL_HELP]    = 0x30;
            deb.col[COL_LINE]    = 0x02;

            deb.fRedraw = TRUE;
        }
        else
        {
            fValid = TRUE;              // Assume colors are valid

            // Read colors from the command line and set them
            for( i=COL_NORMAL; i<=COL_LINE; i++ )
            {
                // Loop and pick up 5 colors in a row
                if( Expression(&mycol[i], args, &args) )
                {
                    // Make sure they are byte-len
                    if( mycol[i] > 0xFF )
                        fValid = FALSE;
                }
                else
                    fValid = FALSE;
            }

            if( fValid )
            {
                // Assign colors and repaint the window

                for( i=COL_NORMAL; i<=COL_LINE; i++)
                    deb.col[i] = mycol[i];

                deb.fRedraw = TRUE;
            }
            else
                PostError(ERR_SYNTAX, 0);
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdSerial(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Sets serial communication parameters. Special serial port with the IO
*   address of 1E0 is supported by specifying the serial port number of 5.
*   This port is used on some Toshiba notebooks.
*
*   Syntax:
*
*   SERIAL [ON|VT100 [com-port] [baud-rate] | OFF]", "ex: SERIAL ON 2 19200"
*
******************************************************************************/
BOOL cmdSerial(char *args, int subClass)
{
    static UINT com = 0;                // Use whatever default is in the serial module
    static UINT baud = 0x9600;          // If we dont specify it, the default rate is 9600 baud

    if( *args )
    {
        if( !strnicmp(args, "off", 3) )
        {
            // Turn the serial interface OFF:
            // Link the VGA back to be the default output device
            pOut = &outVga;

            // Back to debugger, save the background
            dputc(DP_ENABLE_OUTPUT);
            dputc(DP_SAVEBACKGROUND);

            // Redraw all windows
            deb.fRedraw = TRUE;
        }
        else
        {
            if( !strnicmp(args, "vt100", 5) )
            {
                // Set up the terminal communication
                args += 5;
            }
            else
            if( !strnicmp(args, "on", 2) )
            {
                // Regular serial port communication
                args += 2;
            }

            while( *args==' ' ) args++;      // Skip spaces before checking for the next parameter

            if( *args )
            {
                // Read the optional com port and baud rate
                if( Expression(&com, args, &args) )
                {
                    if( com>=1 && com<=MAX_SERIAL )
                    {
                        if( Expression(&baud, args, &args) )
                        {
                            if( baud==0x2400 || baud==0x9600 || baud==0x19200 || baud==0x38400 || baud==0x57600 || baud==0x115200 )
                            {
                                goto Proceed;       // All right!
                            }
                        }
                        // Could not evaluate expression into a baud rate, or the rate was illegal

                        dprinth(1, "Invalid baud rate (2400, 9600, 19200, 38400, 57600, 115200)");
                        return(TRUE);
                    }
                }
                // Either we could not evaluate the serial port number or the port number was illegal
                dprinth(1, "Invalid com port (valid ports are: 1, 2, 3, 4, 5)");
                return( TRUE );
            }
            // No more arguments - use the defaults or the last selection
Proceed:
            // Restore client window since we are switching from the VGA out
            dputc(DP_RESTOREBACKGROUND);
            dputc(DP_DISABLE_OUTPUT);

            // Link the serial VT100 to be the default output device
            pOut = &outVT100;

            // Reset the serial interface
            SerialInit(com, baud);

            // Now we can initialize and start using a VT100 terminal
            InitVT100();

            // Redraw all windows
            deb.fRedraw = TRUE;
        }
    }
    else
    {
        // Display the state of the serial interface
        SerialPrintStat();
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdDisplay(char *args, int subClass)                                 *
*                                                                             *
*******************************************************************************
*
*   Switches to a different display output device:
*
*   Where:
*       subClass is one of the following:
*           0   - VGA
*           1   - MDA (Monochrome display)
*           2   - XWIN (DGA X-Window compatible screen)
*           3   - ALTSCR command - alternate way to switch displays
*
******************************************************************************/
BOOL cmdDisplay(char *args, int subClass)
{
    if( subClass==3 )
    {
        // ALTSCR command - translate into one of the basic displays
        if( !strcmp(args, "vga") ) subClass = 0;        // VGA?
        else
        if( !strcmp(args, "mono")) subClass = 1;        // MONO?
        else
        if( !strcmp(args, "xwin")) subClass = 2;        // XWIN
        else
        if( !strcmp(args, "off") ) subClass = 0;        // OFF? -> VGA
        else
        {
            // No arguments to ALTSCR command - display current display assignment

            if( pOut==&outVga )
                strcpy(Buf, "VGA");
            else
            if( pOut==&outMda )
                strcpy(Buf, "MONO");
            else
            if( pOut==&outDga )
                strcpy(Buf, "XWIN");
            else
                strcpy(Buf, "SERIAL");  // Otherwise it is a serial connection..

            dprinth(1, "Current display is %s", Buf);

            return( TRUE );
        }
    }

    // Restore user screen from the old display device
    dputc(DP_RESTOREBACKGROUND);
    dputc(DP_DISABLE_OUTPUT);

    switch( subClass )
    {
        case 0:     // Standard VGA
            pOut = &outVga;
            break;

        case 1:     // Standard MDA (Monochrome or Hercules)
            MdaInit();
            pOut = &outMda;
            break;

        case 2:     // DGA-compatible X-Windows display
            // We can switch to this driver only if xice had passed init packet,
            // we allocated memory and initialized this subsystem
            if( deb.pXDrawBuffer )
                pOut = &outDga;
            else
                dprinth(1, "Error: XWIN not initialized. Please run 'xice' to send parameters..");
            break;

        default:;
    }

    // Save user screen from the new display device
    dputc(DP_ENABLE_OUTPUT);
    dputc(DP_SAVEBACKGROUND);

    // Redraw all windows
    deb.fRedraw = TRUE;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdSrc(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Toggles between source code, assembly and mixed
*
******************************************************************************/
BOOL cmdSrc(char *args, int subClass)
{
    // Cycle source mode:

    switch( deb.eSrc )
    {
        case SRC_OFF    : deb.eSrc = SRC_ON;    break;
        case SRC_ON     : deb.eSrc = SRC_MIXED; break;
        case SRC_MIXED  : deb.eSrc = SRC_OFF;   break;
    }

    // TODO: Reset the symbol context
//    SetSymbolContext(deb.codeTopAddr.sel, deb.codeTopAddr.offset);
//    SetSymbolContext(deb.r->cs, deb.r->eip);

    // Redraw all windows
    deb.fRedraw = TRUE;

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdTabs(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display or set the tabs setting for the source output
*
******************************************************************************/
BOOL cmdTabs(char *args, int subClass)
{
    UINT nTabs;

    if( *args )
    {
        // Argument is supplied - set the tabs value
        if( Expression(&nTabs, args, &args) )
        {
            if( nTabs>=1 && nTabs<=8 )
            {
                deb.nTabs = nTabs;

                deb.fRedraw = TRUE;
            }
            else
                dprinth(1, "Tabs value should be 1 - 8");
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - display the tabs value
        dprinth(1, "TABS are %d", deb.nTabs);
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdAscii(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Utility function that dumps the ASCII character table.
*
******************************************************************************/
BOOL cmdAscii(char *args, int subClass)
{
    int nLine = 1;                      // Standard line counter
    int nibble;                         // ASCII index

    if(dprinth(nLine++, "         0 1 2 3 4 5 6 7  8 9 A B C D E F")==FALSE) return( TRUE );

    for(nibble=0; nibble<256; nibble+=16)
    {
        if(dprinth(nLine++, " %3d %02X  %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c  %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c",
                nibble,
                nibble,
                DP_ESCAPE, (nibble==0)? ' ' : nibble+0,
                DP_ESCAPE, nibble+1,
                DP_ESCAPE, nibble+2,
                DP_ESCAPE, nibble+3,
                DP_ESCAPE, nibble+4,
                DP_ESCAPE, nibble+5,
                DP_ESCAPE, nibble+6,
                DP_ESCAPE, nibble+7,
                DP_ESCAPE, nibble+8,
                DP_ESCAPE, nibble+9,
                DP_ESCAPE, nibble+10,
                DP_ESCAPE, nibble+11,
                DP_ESCAPE, nibble+12,
                DP_ESCAPE, nibble+13,
                DP_ESCAPE, nibble+14,
                DP_ESCAPE, nibble+15 )==FALSE) break;
    }

    return( TRUE );
}
