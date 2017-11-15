/******************************************************************************
*                                                                             *
*   Module:     history.c                                                     *
*                                                                             *
*   Date:       05/20/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        This module contains History buffer functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/20/00   Original                                             Goran Devic *
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

#define HISTORY_BUFFER      (deb.pHistoryBuffer)
#define MAX_HISTORY_BUF     (deb.nHistorySize)

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct tagLine
{
    struct tagLine *next;               // Next line record
    struct tagLine *prev;               // Previous line record
    BYTE bSize;                         // Size of the whole record
    char line[1];                       // ASCIIZ line itself

} PACKED TLine;

static TLine *pHead;                    // Pointer to the head line
static TLine *pTail;                    // Pointer to the tail line
static DWORD avail;                     // Number of bytes available in the buffer
static TLine *pRead = NULL;             // Read history ioctl pointer

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int HistoryReadReset()                                                    *
*                                                                             *
*******************************************************************************
*
*   This function should be called once before calling HistoryReadNext()
*   multiple times. It resets the read pointer to the start of the history
*   buffer.
*
*   Returns: 0
*
******************************************************************************/
int HistoryReadReset()
{
    // pTail points to the last line (the oldest) in the buffer

    pRead = pTail;

    return( 0 );
}

/******************************************************************************
*                                                                             *
*   char *HistoryReadNext(void)                                               *
*                                                                             *
*******************************************************************************
*
*   This is the counterpart to HistoryReadReset(). It needs to be called
*   multiple times, one time per line. When no more lines are to be returned,
*   it will return NULL.
*
******************************************************************************/
char *HistoryReadNext(void)
{
    char *pRet;                         // Returning pointer

    if( pRead )
    {
        // Return the address of the ASCIIZ line
        pRet = pRead->line;

        // Advance the read pointer if there are still more lines left
        if( pRead != pHead )
        {
            pRead = pRead->next;

            return( pRet );
        }
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void ClearHistory(void)                                                   *
*                                                                             *
*******************************************************************************
*
*   Clears the history buffer
*
******************************************************************************/
void ClearHistory(void)
{
    pHead = pTail = (TLine *) HISTORY_BUFFER;
    memset(pHead, 0, sizeof(TLine));
    avail = MAX_HISTORY_BUF;
    pHead->next = pHead->prev = NULL;
}

/******************************************************************************
*                                                                             *
*   void AddHistory(char *sLine)                                              *
*                                                                             *
*******************************************************************************
*
*   Call this function to store a line into the end of command history buffer,
*
*   Where:
*       sLine is the line to store
*
******************************************************************************/
void HistoryAdd(char *sLine)
{
    DWORD size;

    // Add the line to the head of the histore buffer, possibly releasing
    // lines from the tail until we get enough space

    size = strlen(sLine) + 1 + sizeof(TLine) - 1;

    // Give it some hefty margin since lines at the end of the buffer
    // can not be split

    while( avail < size + 512 )
    {
        // Ok, need to release line by line from the tail

        avail += pTail->bSize;
        pTail = pTail->next;
        pTail->prev = NULL;
    }

    // If the new line record can not fit at the end of the buffer, wrap
    // around by changing previous line `next` pointer

    if( (BYTE *)pHead + size + 32 >= HISTORY_BUFFER + MAX_HISTORY_BUF )
    {
        TLine * prev_save;

        prev_save = pHead->prev;
        pHead = (TLine *) HISTORY_BUFFER;
        prev_save->next = pHead;
        pHead->prev = prev_save;
        pHead->next = NULL;
    }

    // Add the line to pHead

    pHead->bSize = (BYTE) size;

    avail -= size;
    strcpy(pHead->line, sLine);

    pHead->next = (TLine *)( (BYTE *)pHead + size);
    pHead->next->prev = pHead;
    pHead = pHead->next;
    pHead->next = NULL;
}


/******************************************************************************
*                                                                             *
*   DWORD GetCmdViewTop(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   Returns a handle to the top line from the last screen of a buffer.
*   Use this handle in PrintCmd()
*
******************************************************************************/
DWORD HistoryGetTop(void)
{
    TLine *p;
    DWORD nLines;

    // Read the linked list of history lines and backtrack either to the first
    // one or count number of lines in the command history and find the top
    // one, and that is number of lines-2 (1 for the header and 1 for the cursor)

    nLines = pWin->h.nLines - 2;
    p = pHead;

    while( (p!=pTail) && nLines-- )
        p = p->prev;

    return( (DWORD) p );
}


/******************************************************************************
*                                                                             *
*   DWORD PrintCmd(DWORD hView, int nDir)                                     *
*                                                                             *
*******************************************************************************
*
*   Displays command window from the location given handle hView.
*
*   Where:
*       hView is a view handle returned by this function or GetCmdViewTop()
*       nDir is the direction of scroll for the return value handle
*           0   print only last screenful
*           -1  page up
*           1   page down
*
******************************************************************************/
DWORD HistoryDisplay(DWORD hView, int nDir)
{
    TLine *p;
    TLine *pView = (TLine *) hView;
    int nLines;

    nLines = pWin->h.nLines - 2;

    if( nDir==0 )
    {
        // Print the end of the buffer
        pView = (TLine *) HistoryGetTop();
    }
    else
    if( nDir < 0 )
    {
        // Backtrack another screenful of buffer lines
        while( (pView!=pTail) && nLines-- )
            pView = pView->prev;
    }
    else
    {
        // One screen forward
        while( (pView!=pHead) && nLines-- )
            pView = pView->next;

        // This is quite confusing: if we are anywhere within the last
        // screenful, reset the view to the top line of it..

        nLines = pWin->h.nLines - 2;
        p = pHead;

        while( (p!=pTail) && nLines-- )
        {
            if( p==pView )
                pView = p->prev;
            p = p->prev;
        }
    }

    if( nDir )
        dputc(DP_SAVEXY);

    // Position the curson on the first line of the history frame,
    // excluding the header line

    dprint("%c%c%c", DP_SETCURSORXY, 1+0, 1+pWin->h.Top+1);

    p = pView;
    nLines = pWin->h.nLines - 2;
    while( p != pHead && nLines-- )
    {
        dprint("%s\r\n", p->line);
        p = p->next;
    }

    if( nDir )
        dputc(DP_RESTOREXY);

    return( (DWORD) pView );
}


void HistoryDraw(void)
{
    dprint("%c%c%c", DP_SETCURSORXY, 0+1, pWin->h.Top+1);

    PrintLine("");

    // Print the last screenful of the command history
    HistoryDisplay(0, 0);
}

