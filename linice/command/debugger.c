/******************************************************************************
*                                                                             *
*   Module:     debugger.c                                                    *
*                                                                             *
*   Date:       10/31/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains debugger main loop.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/31/00   Original                                             Goran Devic *
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

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern void RecalculateDrawWindows();
extern void EdLin( char *sCmdLine );

/******************************************************************************
*                                                                             *
*   void DebuggerEnter(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop
*
******************************************************************************/
void DebuggerEnter(void)
{
    char sCmd[MAX_STRING];
    BOOL fContinue = TRUE;

    dputc(DP_SAVEBACKGROUND);

    HistoryAdd("LinIce (C) 2000 by Goran Devic");

    // Recalculate window locations based on visibility and number of lines
    // and repaint all windows

    RecalculateDrawWindows();

    while( fContinue )
    {
        EdLin( sCmd );
        
        fContinue = CommandExecute( sCmd );
    }

    dputc(DP_RESTOREBACKGROUND);
}    

