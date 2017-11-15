/******************************************************************************
*                                                                             *
*   Module:     customization.c                                               *
*                                                                             *
*   Date:       04/15/01                                                      *
*                                                                             *
*   Copyright (c) 2001 - 2001 Goran Devic                                     *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 04/15/01   Original                                             Goran Devic *
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

extern int nEvalDefaultBase;

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
} TSETVAR, *PTSETVAR;

static TSETVAR SetVar[] = {
{ "altscr",   6, &deb.fAltscr },
{ "code" ,    4, &deb.fCode },
{ "faults",   6, &deb.fFaults },
{ "i1here",   6, &deb.fI1Here },
{ "i3here",   6, &deb.fI3Here },
{ "lowercase",9, &deb.fLowercase },
{ "pause",    5, &deb.fPause },
{ "symbols",  7, &deb.fSymbols },
{ NULL, }
};

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int GetOnOff(char *args);

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

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
    int nOnOff;

    nOnOff = GetOnOff(args);
    if( nOnOff==0 )
    {
        // Display the state of the CODE view
        dprinth(0, "Code is %s\n", deb.fCode? "on":"off");
    }
    else
    {
        // Set or reset code view variable
        if( nOnOff==1 )
            deb.fCode = TRUE;
        else
        if( nOnOff==2 )
            deb.fCode = FALSE;

        // Repaint the code window using the new settings



    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdSet(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Multiple SET variable handling:
*       SET ALTSCR [ON | OFF]
*       SET CODE   [ON | OFF]
*       SET FAULTS [ON | OFF]
*       SET I1HERE [ON | OFF]
*       SET I3HERE [ON | OFF]
*       SET LOWERCASE [ON | OFF]
*       SET PAUSE  [ON | OFF]
*       SET SYMBOLS [ON | OFF]
*
******************************************************************************/
BOOL cmdSet(char *args, int subClass)
{
    int nOnOff, nLine = 0;
    PTSETVAR pVar;

    pVar = SetVar;

    if( *args==0 )
    {
        // Simple SET command without parameters - list all variables
        while( pVar->sVar )
        {
            dprinth(nLine++, "%s is %s\n", pVar->sVar, pVar->pVal? "on":"off");
            pVar++;
        }
    }
    else
    {
        // Set <VARIABLE> [ON | OFF]
        // Find the variable name
        while( pVar->sVar )
        {
            if( strnicmp(args, pVar->sVar, pVar->sLen)==0 )
                break;

            pVar++;
        }

        // If we did not find a predefined variable...
        if( pVar->sVar==NULL )
        {
            dprinth(0, "Set variable not found\n");
        }
        else
        {
            // Advance the argument pointer pass the var name and into the
            // possible argument [ON | OFF]
            args += pVar->sLen;

            nOnOff = GetOnOff(args);
            if( nOnOff==0 )
            {
                // Display the state of the variable
                dprinth(0, "%s is %s\n", pVar->sVar, pVar->pVal? "on":"off");
            }
            else
            {
                // Set or reset the value of the variable
                if( nOnOff==1 )
                    *pVar->pVal = TRUE;
                else
                if( nOnOff==2 )
                    *pVar->pVal = FALSE;
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdLines(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Change number of display lines
*
******************************************************************************/
BOOL cmdLines(char *args, int subClass)
{
    int nLines;

    nEvalDefaultBase = 10;

    nLines = Evaluate( args, &args );
    switch( nLines )
    {
        case 25:
            break;

        case 43:
            break;

        case 50:
            break;
    }

    return( TRUE );
}

