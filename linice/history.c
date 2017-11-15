/******************************************************************************
*                                                                             *
*   Module:     history.c                                                     *
*                                                                             *
*   Date:       11/20/00                                                      *
*                                                                             *
*   Copyright (c) 1997, 2000 Goran Devic                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
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
* 11/20/00   Original                                             Goran Devic *
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

#define HISTORY_BUFFER      (pIce->pHistoryBuffer)
#define MAX_HISTORY_BUF     (pIce->nHistorySize)

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct tagLine
{
    struct tagLine *next;           // Next line record
    struct tagLine *prev;           // Previous line record
    BYTE bSize;                     // Size of the whole record
    char line[1];                   // ASCIIZ line itself
    
} PACKED TLine;

static TLine *pHead;
static TLine *pTail;
static DWORD avail;

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

    // Backtrack so many lines in the history buffer
    nLines = deb.wcmd.nLines;

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
    DWORD nLines;

    nLines = deb.wcmd.nLines;

    if( nDir==0 )
    {
        // Print the end of the buffer
        pView = (TLine *) GetCmdViewTop();
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

        // This is quite convoluted: if we are anywhere within the last
        // screenful, reset the view to the top line of it..

        nLines = deb.wcmd.nLines;
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

    dprint("%c%c%c", DP_SETCURSORXY, 1+0, 1+deb.wcmd.yTop);

    p = pView;
    nLines = deb.wcmd.nLines;
    while( p != pHead && nLines-- )
    {
        dprint("%s\n", p->line);
        p = p->next;
    }

    if( nDir )
        dputc(DP_RESTOREXY);

    return( (DWORD) pView );
}    


void HistoryDraw(void)
{
    HistoryDisplay(NULL, 0);
}    

