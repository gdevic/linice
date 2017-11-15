/******************************************************************************
*                                                                             *
*   Module:     history.c                                                     *
*                                                                             *
*   Date:       11/20/00                                                      *
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

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

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

#define MAX_HISTORY_BUF     (16 * 1024)

typedef struct tagLine
{
    struct tagLine *next;           // Next line record
    struct tagLine *prev;           // Previous line record
    BYTE bSize;                     // Size of the whole record
    char line[1];                   // ASCIIZ line itself
    
} PACKED TLine;

static BYTE HistoryBuf[MAX_HISTORY_BUF] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, };

static TLine *pHead = (TLine *) HistoryBuf;
static TLine *pTail = (TLine *) HistoryBuf;

static DWORD avail = MAX_HISTORY_BUF;


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

DWORD Check(void);

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
    pHead = pTail = (TLine *) HistoryBuf;
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
void AddHistory(char *sLine)
{
    DWORD size;

#if 0
    if( *(sLine+1)=='?' )
    {
        Check();
        return;
    }
#endif

    // Add the line to the head of the histore buffer, possibly releasing
    // lines from the tail until we get enough space

    size = strlen(sLine) + 1 + sizeof(TLine) - 1;
//dprint(">%d<", size);
    // Give it some hefty margin since lines at the end of the buffer 
    // can not be split

    while( avail < size + 512 )
    {
#if 0
        ClearHistory();
#else
        // Ok, need to release line by line from the tail
//dprint("*%d*", avail);
        avail += pTail->bSize;
        pTail = pTail->next;
//memset((void *)pTail->prev, 0xCC, pTail->prev->bSize);
        pTail->prev = NULL;
#endif
    }

    // If the new line record can not fit at the end of the buffer, wrap 
    // around by changing previous line `next` pointer

    if( (BYTE *)pHead + size + 32 >= HistoryBuf + MAX_HISTORY_BUF )
    {
        TLine * prev_save;
//dprint("&w&");
        prev_save = pHead->prev;
        pHead = (TLine *) HistoryBuf;
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
DWORD GetCmdViewTop(void)
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
DWORD PrintCmd(DWORD hView, int nDir)
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

    dprint("%c%c%c", DP_SETCURSOR, 0, deb.wcmd.yTop);

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


DWORD Check(void)
{
    TLine *p;

    dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSOR, 0, deb.nLines + 1);
    dprint("av=%d  tail=%08X  head=%08X\n", avail, (DWORD)pTail, (DWORD)pHead);

    p = pTail;

    do
    {
        dprint("(%08X) %08X %08X %2d %s\n", (DWORD) p, p->prev, p->next, p->bSize, p->line);
        p = p->next;
    }
    while( p!=pHead );
    dprint(" %08X  %08X %08X %2d %s\n", (DWORD) p, p->prev, p->next, p->bSize, p->line);

    dputc(DP_RESTOREXY);
}    

void DumpHistory(void)
{
    DWORD count = 1;
    TLine *p;

    dprint("%c%c%c", DP_SETSCROLLREGION, 255, 255 );

    p = pTail;
    while( p != pHead )
    {
        dprint("%02X %s\n", count, p->line );
        count++;
        p = p->next;
    }
}    

