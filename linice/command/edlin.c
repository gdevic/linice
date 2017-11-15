/******************************************************************************
*                                                                             *
*   Module:     edlin.c                                                       *
*                                                                             *
*   Date:       09/10/00                                                      *
*                                                                             *
*   Copyright (c) 1997-2005 Goran Devic                                       *
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

    This module contains the code for the command line editor.

    The line editor supports editing a line of up to MAX_STRINGS characters
    in length. The last character is always zero-terminator.
    Cursor can be moved using `Left'/`Right' keys.

    It supports `insert' key for typing modes (insert and overwrite),
    `backspace' and `delete' keys for deletion, `end' and `home' keys for
    position.

    History buffer contains previous unique lines.  History lines are called
    using `Up'/`Down' keys.

    `ESC' key clears the input line.

    Function keys invoke immediate execution of the string definition, if
    string is prefixed by ~, or copy string otherwise.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 8/28/97    Original                                             Goran Devic *
* 05/17/00   Modified for Linice                                  Goran Devic *
* 09/10/00   Second revision                                      Goran Devic *
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

#define MAX_CMD     (pOut->sizeX)

char *pCmdEdit = NULL;                  // Push edit line string

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char *sEnterCommand   = "     Enter a command (H for help)\r";
static char *sInvalidCommand = "Invalid command\r";

static char sBuf[MAX_STRING];
static char sHelpLine[MAX_STRING];

// History buffer is initially set with index [0] = '/0' that is really an
// invalid entry. Only the first line contains valid (spaces) line.  That
// ensures a valid entry for the search!

// If you change this nunber of history lines, sHistory must be changed to
// initialize that many lines to '\0', and that's it!

#define MAX_HISTORY         32          // Number of history buffers (hard coded)


#define PREV_INDEX(i)  ((i)==0? MAX_HISTORY-1 : (i)-1)
#define NEXT_INDEX(i)  ((i)>=MAX_HISTORY-1? 0 : (i)+1)


static int  iHistory;                   // Current history lookup entry index
static int  iWriteHistory = 0;          // Index which history line to update

static char sHistory[MAX_HISTORY][MAX_STRING] = {
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
{ "                                                                               \0" },
          { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" },
{ "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" },
{ "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" },
{ "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" }, { "\0" } };

static char sInitialHistoryLine[] = ":ver";

static char *sCmd;                      // Local pointer to a current cmd line
static int xCur;                        // X coordinate of a cursor
static int yCur;                        // Y coordinate of the edit line
static BOOL fInsert = TRUE;             // Insert mode?
static BOOL fCmdBuf = FALSE;            // Did we use cmd buffer
static DWORD hView;                     // Handle to a history view

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern void XWinControl(WCHAR Key);
extern void FocusInPlace(TLIST *pList, TFRAME *pFrame);
extern void CodeScroll(int Xdir, int Ydir);
extern void CodeCursorScroll(int Ydir);
extern char *MacroExpand(char *pCmd);
extern BOOL CheckNV2(void);

/******************************************************************************
*                                                                             *
*   void InitEdit()                                                           *
*                                                                             *
*******************************************************************************
*
*   Initialize editing system. Will use later.
*
******************************************************************************/
void InitEdit()
{
    // Initialize the edit history with a "ver" command
    memcpy(sHistory[iWriteHistory], sInitialHistoryLine, strlen(sInitialHistoryLine));

    // Advance write history index
    iWriteHistory = NEXT_INDEX(iWriteHistory);
}

/******************************************************************************
*                                                                             *
*   void ClearLine()                                                          *
*                                                                             *
*******************************************************************************
*
*   Clears the current command line buffer.
*
******************************************************************************/
static void ClearLine()
{
    memset( sCmd, ' ', MAX_CMD-1 );
    sCmd[MAX_CMD-1] = '\0';
    sCmd[0] = ':';

    dprint("%c%c%c:", DP_SETCURSORXY, 1+0, 1+yCur);
    xCur = 1;
}


/******************************************************************************
*                                                                             *
*   void CursorEnd()                                                          *
*                                                                             *
*******************************************************************************
*
*   Positions the cursor at the end of the current command line.
*
******************************************************************************/
static void CursorEnd()
{
    int i;

    for( i=MAX_CMD-2; i>=0; i-- )
        if( sCmd[i] != ' ' )
            break;

    xCur = i + 1;
    dprint("%c%c%c", DP_SETCURSORXY, 1+xCur, 1+yCur);
}


/******************************************************************************
*                                                                             *
*   void PrevHistoryLine()                                                    *
*                                                                             *
*******************************************************************************
*
*   Finds the previous history line and copies it to the current line.
*
******************************************************************************/
static void PrevHistoryLine()
{
    // Find the previous valid history line (valid means byte[0] != '\0')
    // We are guarranteed to find one

    for(;;)
    {
        iHistory = PREV_INDEX(iHistory);

        if( sHistory[iHistory][0] != '\0' )
        {
            // Found a good history line, copy it

            strcpy( sCmd, (const char *) &sHistory[iHistory] );

            // Position the cursor at the end of it

            CursorEnd();

            return;
        }
    }
}


/******************************************************************************
*                                                                             *
*   void NextHistoryLine()                                                    *
*                                                                             *
*******************************************************************************
*
*   Finds the next history line and copies it to the current line.
*
******************************************************************************/
static void NextHistoryLine()
{
    // Find the next valid history line (valid means byte[0] != '\0')
    // We are guarranteed to find one

    for(;;)
    {
        iHistory = NEXT_INDEX(iHistory);

        if( sHistory[iHistory][0] != '\0' )
        {
            // Found a good history line, copy it

            strcpy( sCmd, (const char *) &sHistory[iHistory] );

            // Position the cursor at the end of it

            CursorEnd();

            return;
        }
    }
}


/******************************************************************************
*                                                                             *
*   void EdLin(char *sCmdLine )                                               *
*                                                                             *
*******************************************************************************
*
*   This is the main command line function.
*
*   Where:
*       sCmdLine - pointer to a buffer in which the new command is copied.
*       pOut->y is the line editor Y coordinate
*
*   Returns:
*       Command line
*
******************************************************************************/
void EdLin( char *sCmdLine )
{
    DWORD newOffset;
    WCHAR Key;
    int i, map;
    char *pMacro;                       // Macro string that we found to match
    char *pSource;                      // String source to copy from a Fn key
    BOOL fSilent;                       // Echo the line or not - used with F-keys
    BOOL fInCodeWindow;                 // Cursor is effectively in the code window

    yCur = pOut->y;                     // Store line editor Y coordinate
    fCmdBuf = FALSE;                    // We did not scroll history buffer yet
    hView = HistoryGetTop();            // Get a handle to the current history view
    sCmd = sCmdLine;                    // Local pointer to a command line
    iHistory = iWriteHistory;           // Always lookup from the current history line
    fSilent = FALSE;                    // Default print out a line

    ClearLine();                        // Clear the current command line and reset the cursor

    // When we are requested to edit an existing line, we will have pCmdEdit nonzero,
    // and in that case copy it into the sCmd buffer instead of clearing it
    if( pCmdEdit != NULL )
    {
        strcpy(sCmd + 1, pCmdEdit);
        sCmd[strlen(pCmdEdit)+1] = ' ';
        xCur = strlen(pCmdEdit) + 1;
        pCmdEdit = NULL;

        dprint("%c%c%c:", DP_SETCURSORXY, 1+xCur, 1+yCur);
    }

    do
    {
        TCommand *pCmd = NULL;          // Pointer to the last command record
        int nFound;                     // How many matches
        char *tok;                      // Pointer to the first token
        int nLen;                       // Length of the first token

        // Update the help line with the (partial) command typed

        sHelpLine[0] = '\0';
        nFound = 0;
        nLen = 0;

        // Find the first token

        tok = sCmd + 1;
        while( *tok==' ' )
            tok++;

        if( *tok != '\0' )
        {
            // Find the length of the first token

            while( *(tok+nLen)!=' ' && *(tok+nLen)!='\0' )
                nLen++;

            // Find (all the) commands that are superset of that token

            for( i=0; Cmd[i].sCmd != NULL; i++)
            {
                if( strnicmp(Cmd[i].sCmd, tok, nLen)==0 )
                {
                    pCmd = &Cmd[i];
                    if( nFound > 0 )
                        strcat(sHelpLine, ", ");
                    strcat(sHelpLine, Cmd[i].sCmd);
                    nFound++;

                    if( nFound==1 )
                    {
                        // If it is an exact match, more than 2 chars, cursor adjacent, bail out
                        if( pCmd->nLen==nLen && nLen>1 && xCur == tok-sCmd+nLen )
                            break;

                        // If it is an exact match followed by a space, look no further
                        if( pCmd->nLen==nLen && xCur > tok-sCmd+nLen )
                            break;
                    }
                }
                else    // Little shortcut out since our commands are sorted...
                    if( nFound )
                        break;
            }

            if( nFound==0 )
            {
                // If we did not find any match, look for a possible defined macro that
                // would be used
                pMacro = MacroExpand(tok);

                if( pMacro != NULL )
                {
                    strncpy(sHelpLine, tok, nLen);
                    sprintf(sHelpLine+nLen, " = \"%s\"\r", pMacro);
                }
                else
                {
                    // If we did not find any match, print invalid command
                    strcpy(sHelpLine, sInvalidCommand);
                }
            }
            else
            if( nFound==1 )
            {
                // If we found exactly one match, print its syntax or description instead
                if( xCur > tok-sCmd+nLen )
                    strcpy(sHelpLine, pCmd->sSyntax);
                else
                    strcpy(sHelpLine, sHelp[pCmd->iHelp] + 9);
                strcat(sHelpLine, "\r");
            }
            else
            {
                // If we found multiple matches, we've already assembled them...
                strcat(sHelpLine, "\r");
            }
        }
        else
        {
            // Back to the default help line...
            strcpy(sHelpLine, sEnterCommand);
        }

        // Print (new) help line

        dprint("%c%c%c%c%c%s", DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1, DP_SETCOLINDEX, COL_HELP, sHelpLine);

        // Special mode is code edit mode (EC command) where the cursor is in the code window
        // However, this pseudo-mode can be evident only before a single key is pressed, after
        // which the cursor moves down to the edit line

        if( deb.fCodeEdit && xCur==1 && sCmd[1]==' ')
            fInCodeWindow = TRUE;
        else
            fInCodeWindow = FALSE;

        // Print current edited line and position the cursor

        dprint("%c%c%c%s%c%c%c",
            DP_SETCURSORXY, 1+0, 1+yCur, sCmd,
            DP_SETCURSORXY, 1+xCur, 1+yCur );

        // Wait for the input character and process it

        // If we are in the code edit mode, with the cursor at the start of the empty line,
        // display it in the code window instead
        if( fInCodeWindow )
        {
            // Code edit mode and the cursor is at the starting position
            // We add 1 since the setcursor is 1-based, then 1 to skip over the code window header line
            dprint("%c%c%c", DP_SETCURSORXY, 1+0, 1+pWin->c.Top + deb.nCodeEditY + 1);
        }

        Key = GetKey( TRUE );

        // If the imput key is one of the Function key combinations, copy or exec the string

        pSource = "";

        if( (Key & 0xFF)>=F1 && (Key & 0xFF)<=F12 )
        {
            if( Key & CHAR_SHIFT )
                map = 1;
            else
            if( Key & CHAR_ALT )
                map = 2;
            else
            if( Key & CHAR_CTRL )
                map = 3;
            else
                map = 0;

            Key = (Key & 0xFF) - F1;

            if( deb.keyFn[map][Key][0]=='^' )
            {
                // First character was ^ execute immediately and silently.
                // Now, we should really preserve the current line for the next edit pass,
                // but I can't think of an elegant way to do that yet

                // TODO - It is possible to run a macro while typing another line.
                //        In this case it executes silently and returns to the original edited line in place

                fSilent = TRUE;
                ClearLine();     // Clear the current command line and reset the cursor

                // Copy the string into our edit line but skip leading ^
                pSource = deb.keyFn[map][Key] + 1;
            }
            else
                // Copy the string into our edit line
                pSource = deb.keyFn[map][Key];
        }

        do
        {
            // Make our Fn-key string being accepted
            if( *pSource )
                Key = *pSource++;

            // Transform TAB character into a space
            if( Key==DP_TAB ) Key = ' ';

            switch( Key )
            {
                //========================= X - WINDOW =========================

                case CHAR_CTRL + CHAR_ALT + HOME:
                case CHAR_CTRL + CHAR_ALT + 'c':
                case CHAR_CTRL + CHAR_ALT + 'C':
                case CHAR_CTRL + CHAR_ALT + LEFT:
                case CHAR_CTRL + CHAR_ALT + RIGHT:
                case CHAR_CTRL + CHAR_ALT + UP:
                case CHAR_CTRL + CHAR_ALT + DOWN:

                    // Ctrl+Alt+Home repositions the X-Window window to the top-left corner
                    // Ctrl+Alt+'C' centers the window on the X screen
                    // Ctrl+Alt+<arrow keys> move window around

                    XWinControl(Key);

                    break;

                //======================== LOCALS WINDOW =======================
#ifdef SIM // We cannot handle ALT key
                case CHAR_CTRL + 'l':
#else
                case CHAR_ALT + 'l':
#endif // SIM
                    // Temporary switch to manage locals window

                    FocusInPlace(&deb.Local, &pWin->l);

                    break;

                //======================== WATCH WINDOW ========================
#ifdef SIM // We cannot handle ALT key
                case CHAR_CTRL + 'w':
#else
                case CHAR_ALT + 'w':
#endif // SIM
                    // Temporary switch to manage watch window

                    FocusInPlace(&deb.Watch, &pWin->w);

                    break;

                //======================== STACK WINDOW ========================
#ifdef SIM // We cannot handle ALT key
                case CHAR_CTRL + 's':
#else
                case CHAR_ALT + 's':
#endif // SIM
                    // Temporary switch to manage stack window

                    FocusInPlace(&deb.Stack, &pWin->s);

                    break;

                //======================== CODE WINDOW =========================

                case CHAR_CTRL + HOME:
                    // Ctrl + Home positions at the top of source file

                    CodeScroll(0, -3);

                    break;

                case CHAR_CTRL + END:
                    // Ctrl + End positions at the end of source file

                    CodeScroll(0, 3);

                    break;

                case CHAR_CTRL + PGUP:
                    // Ctrl + PgUp page up code window

                    CodeScroll(0, -2);

                    break;

                case CHAR_CTRL + UP:
                    // Ctrl + Up one line up code window

                    CodeScroll(0, -1);

                    break;

                case CHAR_CTRL + PGDN:
                    // Ctrl + PgDown page down code window

                    CodeScroll(0, 2);

                    break;

                case CHAR_CTRL + DOWN:
                    // Ctrl + Down one line down code window

                    CodeScroll(0, 1);

                    break;

                case CHAR_CTRL + RIGHT:
                    // Ctrl + Right shifts source to the left

                    CodeScroll(1, 0);

                    break;

                case CHAR_CTRL + LEFT:
                    // Ctrl + Left shifts source to the right

                    CodeScroll(-1, 0);

                    break;

                //======================== DATA WINDOW =========================

                case CHAR_ALT + PGUP:
                    // Alt + PgUp scrolls data window backward

                    newOffset = deb.dataAddr[deb.nData].offset - DATA_BYTES * (pWin->data[deb.nData].nLines - 1);
                    DataDraw(FALSE, newOffset, TRUE);

                    break;

                case CHAR_ALT + PGDN:
                    // Alt + PgDown scrolls data window forward

                    newOffset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (pWin->data[deb.nData].nLines - 1);
                    DataDraw(FALSE, newOffset, TRUE);

                    break;

                case CHAR_ALT + UP:
                    // Alt + CursorUp scrolls data window one line back

                    newOffset = deb.dataAddr[deb.nData].offset - DATA_BYTES;
                    DataDraw(FALSE, newOffset, TRUE);

                    break;

                case CHAR_ALT + DOWN:
                    // Alt + CursorDown scrolls data window one line forward

                    newOffset = deb.dataAddr[deb.nData].offset + DATA_BYTES;
                    DataDraw(FALSE, newOffset, TRUE);

                    break;

                //==================== COMMAND HISTORY WINDOW ======================

                case ESC:
                    // ESC key clears the current buffer

                    ClearLine();

                    break;

                case HOME:
                    // HOME positions the cursor at the column 1 (col 0 is a prompt)

                    xCur = 1;

                    break;

                case END:
                    // END positions the cursor after the last nonblank character

                    CursorEnd();

                    break;

                case LEFT:
                    // LEFT moves the cursor one character to the left

                    if( xCur > 1 )
                        xCur--;

                    break;

                case RIGHT:
                    // RIGHT moves the cursor one character to the right

                    if( xCur < MAX_CMD-1 )
                        xCur++;

                    break;

                case UP:
                    // UP key retrieves the previous line from the history buffer
                    // If the effective code edit mode, it moves cursor / scrolls code window

                    if( fInCodeWindow )
                        CodeCursorScroll(-1);
                    else
                        PrevHistoryLine();

                    break;

                case DOWN:
                    // DOWN key retrieves the next line from the history buffer
                    // If the effective code edit mode, it moves cursor / scrolls code window

                    if( fInCodeWindow )
                        CodeCursorScroll(1);
                    else
                        NextHistoryLine();

                    break;

                case PGUP:
                    // PGUP key scrolls history buffer one screenful up
                    // If the effective code edit mode, it scrolls code window up one page

                    if( fInCodeWindow )
                        CodeScroll(0, -2);
                    else
                    {
                        fCmdBuf = TRUE;
                        hView = HistoryDisplay(hView, -1);
                    }

                    break;

                case PGDN:
                    // PGDOWN key scrolls history buffer one screenful down
                    // If the effective code edit mode, it scrolls code window down one page

                    if( fInCodeWindow )
                        CodeScroll(0, 2);
                    else
                    {
                        fCmdBuf = TRUE;
                        hView = HistoryDisplay(hView, 1);
                    }

                    break;

                case INS:
                    // INSERT key toggles the insert mode

                    fInsert ^= 1;

                    dprint("%c%c", DP_SETCURSORSHAPE, fInsert? 1 : 2 );

                    break;

                case BACKSPACE:
                    // Backspace key deletes a character to the left of the cursor
                    // and moves the cursor to the left

                    if( xCur > 1 )
                    {
                        memmove( sCmd + xCur - 1,
                                 sCmd + xCur,
                                 (MAX_CMD-1) - xCur );

                        // Set the last char to be space

                        sCmd[MAX_CMD-2] = ' ';

                        // Move the cursor one place to the left

                        xCur--;
                    }

                    break;

                case DEL:
                    // Delete key deletes the current character and moves the rest
                    // right from the cursor to the right

                    if( xCur < 79 )
                    {
                        memmove( sCmd + xCur,
                                 sCmd + xCur + 1,
                                 (MAX_CMD-1) - xCur );

                        // Set the last char to be space

                        sCmd[MAX_CMD-1] = ' ';
                    }

                    break;

                case ENTER:
                    // Enter key accepts the line.  If the line is different from
                    // the strictly previous line, it copies it to the new history line.
                    // If the line is identical, do not copy, but use its prevous
                    // copy.

                    // Write into history buffer only non-silent commands
                    if( !fSilent )
                    {
                        if( strcmp( sCmd, sHistory[PREV_INDEX(iWriteHistory)] ) )
                        {
                            // Line is new, does not match, store it to the history buffer

                            memcpy( sHistory[iWriteHistory], sCmd, MAX_CMD );

                            // Advance write history index

                            iWriteHistory = NEXT_INDEX(iWriteHistory);
                        }
                    }
                    break;

                case ' ':
                    // IMPORTANT: This case has to be followed by the default case
                    //            It has no break statement.

                    // If we pressed SPACE key, and a single command token was found,
                    // complete it if necessary

                    if( nFound==1 && xCur==tok-sCmd+nLen && xCur<MAX_CMD-pCmd->nLen)
                    {
                        // Insert the rest of the suggested command (complete it)

                        if( nLen < pCmd->nLen )
                        {
                            memcpy(&sCmd[xCur], pCmd->sCmd+nLen, pCmd->nLen - nLen);
                            xCur += pCmd->nLen - nLen;
                        }
                    }
                    // Proceed with adding in the space...

                default:
                    if( Key <= 0xFF && Key >= ' ' )
                    {
                        // Any other character is written in the buffer in insert or
                        // overwrite mode

                        if( fInsert && xCur < MAX_CMD-2 )
                        {
                            // Move the buffer right of the cursor to make some space

                            if( xCur < MAX_CMD-2 )
                                memmove( sCmd + xCur + 1,
                                         sCmd + xCur,
                                         (MAX_CMD-2) - xCur );
                        }

                        // Store a new character and advance the cursor

                        if( xCur < MAX_CMD-1 )
                        {
                            sCmd[ xCur ] = Key;

                            if( xCur < MAX_CMD-1 )
                                xCur++;
                        }
                    }
                    else    // Combination keys with Alt/Ctrl/Shift key or a function key
                    {
                        ;
                    }

                    break;
            }

        }
        while( *pSource );

    } while( Key != '\n' );

    // Last thing we need to do with a line is to put a zero-terminator
    // after the last character

    CursorEnd();

    sCmd[ xCur ] = '\0';

    // Restore the last screen of the command window

    if( fCmdBuf==TRUE )
        HistoryDisplay(0, 0);

    // Print the final line and scroll it up.. Add it to the history buffer

    dprint("%c%c%c", DP_SETCURSORXY, 1+0, 1+yCur);

    if( fSilent==FALSE )
        dprinth(1, "%s", sCmd);
}


/******************************************************************************
*                                                                             *
*   void EdDumpHistory(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Helper function that dumps edit history lines
*   This function is accessed via "ver ed-dump" command.
*
******************************************************************************/
void EdDumpHistory(void)
{
    int i;

    for( i=0; i<MAX_HISTORY && (*sHistory[i] || i==iWriteHistory); i++ )
    {
        dprinth(1, "%02d%c%s", i, (i==iWriteHistory)?'>':' ', sHistory[i] );
    }
}

/******************************************************************************
*                                                                             *
*   BOOL cmdFkey(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Shows and edit function key assignments.
*   With no arguments, display function key assignments.
*   To delete a function key assignment, dont specify definition after the name.
*
*   If the definition end with '+', it is incomplete and will cause further edit.
*
******************************************************************************/
BOOL cmdFkey(char *args, int subClass)
{
    int nLine = 1;                      // line counter
    int map, key;                       // Current map and key number
    char *ptr;                          // Generic pointer to string
    static char cMap[4] = { ' ', 's', 'a', 'c' };

    if( *args )
    {
        // Argument is specified in the form of "<fnkey>[=][assignment]"
        // <fnkey> is mandatory
        // [=] is optional
        // [assignment] is optional, If omitted, fkey definition will be cleared
        map = 0;

        if( *args=='s' ) args++, map=1;
        else
        if( *args=='a' ) args++, map=2;
        else
        if( *args=='c' ) args++, map=3;

        if( *args=='f' )
        {
            // We need to parse the function key number
            args++;

            if( *args>='1' && *args<='9' )
            {
                key = *args - '0';
                args++;

                if( *args>='0' && *args<='2' )
                {
                    key = key * 10 + *args-'0';
                    args++;
                }

                if( key < 12 )
                {
                    // We got the right map and the key (1..12) -> (0..11)
                    key--;

                    // Check for the optional '='
                    while( *args==' ' ) args++;

                    if( *args=='=' ) args++;

                    ptr = deb.keyFn[map][key];

                    // If we provided definition, copy it into the store space,
                    // if we did not, delete previous definition
                    if( *args )
                    {
                        // We provided a new definition + append end of line code
                        strcpy(ptr, args);

                        // If the definition ends with a '+' we will not append
                        // newline and the editor will position carret there
                        ptr += strlen(ptr) - 1;
                        if( *ptr!='+' )
                            *(ptr+1) = '\n';
                        else
                            *ptr = 0;               // Else delete '+' character
                    }
                    else
                    {
                        // Delete the previous definition
                        *ptr = 0;
                    }

                    return( TRUE );
                }
            }
        }

        // Everything else is an syntax error
        PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // No arguments - display function key assignment
        for(map=0; map<4; map++)
        {
            for(key=0; key<12; key++)
            {
                // Print out only defined keys
                if( *deb.keyFn[map][key] )
                {
                    sprintf(sBuf, "%cf%-2d = %s", cMap[map], key+1, deb.keyFn[map][key]);

                    // Get rid of newline character at the end
                    if( (ptr = strchr(sBuf, 0x0A)) ) *ptr = 0;
                    if( (ptr = strchr(sBuf, 0x0D)) ) *ptr = 0;

                    // Print the function line definition
                    if( dprinth(nLine++, "%s", sBuf)==FALSE )
                        return( TRUE );
                }
            }
        }
    }

    return( TRUE );
}

