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

static char sCmd[MAX_STRING];


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
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
    BOOL fContinue = TRUE;

    // Enable windowing and save background
    pWin->fEnable = TRUE;
    dputc(DP_SAVEBACKGROUND);

    // Set the new CS:EIP for code disasembly
    deb.codeAddr.sel = deb.r->cs;
    deb.codeAddr.offset = deb.r->eip;

    memcpy(&deb.r_prev, deb.r, sizeof(TREGS));

    // Recalculate window locations based on visibility and number of lines
    // and repaint all windows

    RecalculateDrawWindows();

    // Print the reason for break into the debugger
//    if( deb.nInterrupt==1 )
//        dprinth(1, "Breakpoint due to INT1\n");

    dprinth(1, "Breakpoint due to INT%02X\n", deb.nInterrupt);

//    if( deb.nInterrupt==3 )
//        dprinth(1, "Breakpoint due to INT3\n");

    while( fContinue )
    {
        EdLin( sCmd );

        fContinue = CommandExecute( sCmd+1 );   // Skip the prompt
    }

memcpy(deb.r, &deb.r_prev, sizeof(TREGS));


    // Disable windowing and restore background
    pWin->fEnable = FALSE;
//    dputc(DP_RESTOREBACKGROUND);
}

