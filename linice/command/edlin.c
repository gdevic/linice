/******************************************************************************
*                                                                             *
*   Module:     edlin.c                                                       *
*                                                                             *
*   Date:       09/10/00                                                      *
*                                                                             *
*   Copyright (c) 1997, 2001 Goran Devic                                      *
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
* 05/17/00   Modified for LinIce                                  Goran Devic *
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

extern void CodeScroll(int Xdir, int Ydir);
extern char *MacroExpand(char *pCmd);

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
    CHAR Key;
    int i, map;
    char *pMacro;                       // Macro string that we found to match
    char *pSource;                      // String source to copy from a Fn key
    BOOL fSilent;                       // Echo the line or not - used with F-keys

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

        // Print current edited line and position the cursor

        dprint("%c%c%c%s%c%c%c",
            DP_SETCURSORXY, 1+0, 1+yCur, sCmd,
            DP_SETCURSORXY, 1+xCur, 1+yCur );

        // Wait for the input character and process it

        Key = GetKey( TRUE );

        // If the imput key is one of the Function key combinations, copy or exec the string
        // THIS CODE ASSUMES F1 CODE STARTS AT 0x80 !!!!

        pSource = "";

        if( Key & 0x80 )
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

            if( pIce->keyFn[map][Key & 0x1F][0]=='^' )
            {
                // First character was ^ execute immediately and silently.
                // Now, we should really preserve the current line for the next edit pass,
                // but I can't think of an elegant way to do that yet

                fSilent = TRUE;
                ClearLine();     // Clear the current command line and reset the cursor

                // Copy the string into our edit line but skip leading ^
                pSource = pIce->keyFn[map][Key & 0x1F] + 1;
            }
            else
                // Copy the string into our edit line
                pSource = pIce->keyFn[map][Key & 0x1F];
        }

        do
        {
            // Make our Fn-key string being accepted
            if( *pSource )
                Key = *pSource++;

            switch( Key )
            {
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

                    newOffset = deb.dataAddr.offset - DATA_BYTES * (pWin->d.nLines - 1);
                    DataDraw(FALSE, newOffset);

                    break;

                case CHAR_ALT + PGDN:
                    // Alt + PgDown scrolls data window forward

                    newOffset = deb.dataAddr.offset + DATA_BYTES * (pWin->d.nLines - 1);
                    DataDraw(FALSE, newOffset);

                    break;

                case CHAR_ALT + UP:
                    // Alt + CursorUp scrolls data window one line back

                    newOffset = deb.dataAddr.offset - DATA_BYTES;
                    DataDraw(FALSE, newOffset);

                    break;

                case CHAR_ALT + DOWN:
                    // Alt + CursorDown scrolls data window one line forward

                    newOffset = deb.dataAddr.offset + DATA_BYTES;
                    DataDraw(FALSE, newOffset);

                    break;

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

                    PrevHistoryLine();

                    break;

                case DOWN:
                    // DOWN key retrieves the next line from the history buffer

                    NextHistoryLine();

                    break;

                case PGUP:
                    // PGUP key scrolls history buffer one screenful up

                    fCmdBuf = TRUE;
                    hView = HistoryDisplay(hView, -1);

                    break;

                case PGDN:
                    // PGDOWN key scrolls history buffer one screenful down

                    fCmdBuf = TRUE;
                    hView = HistoryDisplay(hView, 1);

                    break;

                case INS:
                    // INSERT key toggles the insert mode

                    fInsert ^= 1;

                    dprint("%c%c", DP_SETCURSORSHAPE, fInsert? 1 : 2 );

                    break;

                case '\b':
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

                case '\n':
                    // Enter key accepts the line.  If the line is different from
                    // any history line, it copies it to the writing history line.
                    // If the line is identical, do not copy, but use its prevous
                    // copy.

                    for( i=0; i<MAX_HISTORY; i++ )
                    {
                        // Look for the history line that is identical

                        if( !strcmp( sCmd, sHistory[i] ) )
                            iWriteHistory = i;
                    }

                    if( i==MAX_HISTORY )
                    {
                        // Line is new.  Store it to the history buffer

                        memcpy( sHistory[iWriteHistory], sCmd, MAX_CMD );

                        // Advance write history index

                        iWriteHistory = NEXT_INDEX(iWriteHistory);
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
                    if( Key < 0x7F && isascii(Key) )
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

