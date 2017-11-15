/******************************************************************************
*                                                                             *
*   Module:     data.c                                                        *
*                                                                             *
*   Date:       05/15/00                                                      *
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

        This module contains memory dump functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/15/00   Original                                             Goran Devic *
* 09/11/00   Second edition                                       Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures

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

typedef struct
{
    BYTE nF;                            // Number of fields
    BYTE FWidth;                        // Width of a field in bytes
    BYTE xStart[DATA_BYTES];            // Start X coordinate of up to 16 fields
    BYTE xEnd[DATA_BYTES];              // End X coordinate of up to 16 fields
    BYTE xAsciiStart;                   // Start X coordinate of the ASCII field, 0 if no ascii allowed
    BYTE xAsciiEnd;                     // End X coordinate of the ASCII field

} TDataField;

// Descriptions for editing BYTEs (1), WORDs (2) and DWORDs (4)
static TDataField DataField[] = {
    // 0 - not a valid entry
    { 0, },
    // 1 - 16 BYTE position descriptors
    {
        15,
        1,
        { 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59 },
        { 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60 },
        63, 63+15
    },
    // 2 - 8 WORD position descriptors
    {
        7,
        2,
        { 14, 19, 24, 29, 34, 39, 44, 49 },
        { 17, 22, 27, 32, 37, 42, 47, 52 },
        55, 55+15
    },
    // 3 - not a valid entry
    { 0, },
    // 4 - 4 DWORD position descriptors
    {
        3,
        4,
        { 14, 23, 32, 41 },
        { 21, 30, 39, 48 },
        51, 51+15
    }
};


static union
{
    BYTE byte[DATA_BYTES];
    WORD word[DATA_BYTES/2];
    DWORD dword[DATA_BYTES/4];

} MyData;

static BOOL fValid[DATA_BYTES];

static char buf[MAX_STRING];

static char *sSize[4] = { "byte", "word", "", "dword" };

// These defines select a method of access to a field string provided by the
// DataGetSet function on access to data element:

#define DATAGETSET_READ     1
#define DATAGETSET_WRITE    2
#define DATAGETSET_INSERT   3


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   int PrintAscii(int pos)                                                   *
*                                                                             *
*******************************************************************************
*
*   Prints the ASCII field in the buffer for a single data line.
*
******************************************************************************/
static int PrintAscii(int pos)
{
    int i;

    buf[pos++] = ' ';

    for( i=0; i<DATA_BYTES; i++, pos++)
    {
        if( (fValid[i]==TRUE) && MyData.byte[i]>=DP_AVAIL )
            buf[pos] = MyData.byte[i];
        else
            buf[pos] = '.';
    }

    return( i + 1 );
}

/******************************************************************************
*                                                                             *
*   void GetDataLine(PTADDRDESC pAddr)                                        *
*                                                                             *
*******************************************************************************
*
*   Formats the buffer with a single data line.
*
******************************************************************************/
void GetDataLine(PTADDRDESC pAddr)
{
    int i, pos;

    // Fetch a lineful of bytes at a time and get their present flags
    for( i=0; i<DATA_BYTES; i++)
    {
        fValid[i] = AddrIsPresent(pAddr);
        if( fValid[i]==TRUE )
            MyData.byte[i] = AddrGetByte(pAddr);
        pAddr->offset++;
    }

    pos = sprintf(buf, "%04X:%08X ", pAddr->sel, pAddr->offset - DATA_BYTES);

    switch( deb.nDumpSize[deb.nData] )
    {
    //=========================================================
        case 1:             // BYTE
    //=========================================================

            for( i=0; i<DATA_BYTES; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%02X ", MyData.byte[i]);
                else
                    pos += sprintf(buf+pos, "?? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;

    //=========================================================
        case 2:             // WORD
    //=========================================================

            for( i=0; i<DATA_BYTES/2; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%04X ", MyData.word[i]);
                else
                    pos += sprintf(buf+pos, "???? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;

    //=========================================================
        case 4:             // DWORD
    //=========================================================

            for( i=0; i<DATA_BYTES/4; i++)
            {
                if( fValid[i]==TRUE )
                    pos += sprintf(buf+pos, "%08X ", MyData.dword[i]);
                else
                    pos += sprintf(buf+pos, "???????? ");
            }

            // ... and the ASCII representation
            pos += PrintAscii(pos);

        break;
    }
    // Terminate the line
    buf[pos] = 0;
}

/******************************************************************************
*                                                                             *
*   DWORD GetDataLines()                                                      *
*                                                                             *
*******************************************************************************
*
*   Returns the number of data lines on the screen.
*
******************************************************************************/
static DWORD GetDataLines()
{
    // If data frame is visible, we will advance so many lines of data
    if( pWin->data[deb.nData].fVisible )
        return( pWin->data[deb.nData].nLines - 1 );

    // Data window is not visible, so advance 8 or (history height-1) data lines
    if( pWin->h.nLines > 8 )
        return( 8 );

    return( pWin->h.nLines - 1 );
}

/******************************************************************************
*                                                                             *
*   void DataDraw(BOOL fForce, DWORD newOffset, BOOL fCurrent)                *
*                                                                             *
*******************************************************************************
*
*   Draws the current data window.
*
******************************************************************************/
void DataDraw(BOOL fForce, DWORD newOffset, BOOL fCurrent)
{
    TADDRDESC Addr;
    int maxLines;
    int nLine = 1;
    char *pSymName;                     // Symbol that was found
    UINT range;                         // Range variable
    char *pBuf = buf;                   // Pointer to the output buffer


    if( pWin->data[deb.nData].fVisible==TRUE )
    {
        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSORXY, 0+1, pWin->data[deb.nData].Top+1);

        // If the expression had been set for this data window, print it;
        // otherwise, try to find the closest symbol
        if( deb.Dex[deb.nData][0] )
        {
            // We have an expression set via the DEX command (special format for expression error)
            if( deb.DexError[deb.nData] )
                pBuf += sprintf(pBuf, "?%-39s", deb.Dex[deb.nData]);
            else
                pBuf += sprintf(pBuf, "%-40s", deb.Dex[deb.nData]);
        }
        else
        {
            // If there is a symbol that we can use, print it out,
            // unless it is within first 4K - there are no symbols at those addresses in Linux
            pSymName = SymAddress2Name(newOffset, &range);

            pBuf += sprintf(pBuf, "%-40s", pSymName && (newOffset>=4096)? pSymName : "Data");
        }

        pBuf += sprintf(pBuf, "   %5s                   %c%c(%d)%c%c",
            sSize[deb.nDumpSize[deb.nData]-1],
            DP_SETCOLINDEX, fCurrent? COL_BOLD:COL_LINE,
            deb.nData,
            DP_SETCOLINDEX, COL_LINE);

        PrintLine(buf);
    }
    else
        if( fForce==FALSE )
            return;

    deb.dataAddr[deb.nData].offset = newOffset;    // Store the new offset to dump
    Addr = deb.dataAddr[deb.nData];                // Copy the current data address
    maxLines = GetDataLines();

    while( nLine <= maxLines )
    {
        GetDataLine(&Addr);
        if(dprinth(nLine++, "%s\r", buf)==FALSE)
            break;
    }

    if( pWin->data[deb.nData].fVisible==TRUE )
        dprint("%c", DP_RESTOREXY);
}

/******************************************************************************
*                                                                             *
*   static BYTE ReadByte(char *pStr)                                          *
*                                                                             *
*******************************************************************************
*
*   Evaluates two ASCII codes into a BYTE value.
*
******************************************************************************/
static BYTE ReadByte(char *pStr)
{
    BYTE value, nibble;

    nibble = (*pStr>'9')? *pStr - 'A' + 10 : *pStr - '0';
    value = nibble << 4;
    pStr++;
    nibble = (*pStr>'9')? *pStr - 'A' + 10 : *pStr - '0';
    value |= nibble;

    return( value );
}

/******************************************************************************
*                                                                             *
*   BOOL DataGetSet(int cmd, TADDRDESC *pAddr, int pos, char key)             *
*                                                                             *
*******************************************************************************
*
*   Helper function to DataEdit; We keep a current field in ascii representation
*   abstracted within this function. The access methods are:
*       cmd = 1  - Load string from address pAddr
*             2  - Save string to address pAddr
*             3  - Set a sinble nibble within a string at position 'pos' = 'key'
*
*   Returns:
*       TRUE if a key is a valid value for the current data format
*       FALSE if the data entry should be ignored (invalid ascii key)
*
******************************************************************************/
static BOOL DataGetSet(int cmd, TADDRDESC *pAddr, int pos, char key)
{
    static char sField[80];             // Field ASCII value
    BYTE bValue;
    DWORD temp;
    int i;

    // Depending on the command, manage data buffer
    switch( cmd )
    {
        case DATAGETSET_READ: // Read in a field into the internal ASCII buffer

            // Read bytes first
            temp = pAddr->offset;

            for(i=0; i<DataField[deb.nDumpSize[deb.nData]].FWidth; i++ )
            {
                MyData.byte[i] = AddrGetByte(pAddr);
                pAddr->offset++;
            }

            pAddr->offset = temp;

            // Translate into ASCII representation
            switch( deb.nDumpSize[deb.nData] )
            {
                case 1:     // BYTE
                    sprintf(sField, "%02X", MyData.byte[0]);
                    break;

                case 2:     // WORD
                    sprintf(sField, "%04X", MyData.word[0]);
                    break;

                case 4:     // DWORD
                    sprintf(sField, "%08X", MyData.dword[0]);
                    break;
            }
            break;

        case DATAGETSET_WRITE: // Write out a field from the internal ASCII buffer
            switch( deb.nDumpSize[deb.nData] )
            {
                case 1:     // BYTE
                case 2:     // WORD
                case 4:     // DWORD

                    temp = pAddr->offset;

                    for(i=deb.nDumpSize[deb.nData]-1; i>=0; i--)
                    {
                        bValue = ReadByte(&sField[i*2]);
                        AddrSetByte(pAddr, bValue, TRUE);
                        pAddr->offset++;
                    }

                    pAddr->offset = temp;
                    break;
            }
            break;

        case DATAGETSET_INSERT: // Write individual ASCII nibble to a location

            // Accept only HEX characters
            if( (key>='0' && key<='9') || (key>='A' && key<='F') )
                sField[pos] = key;
            else
                return( FALSE );

            break;
    }

    return( TRUE );                     // Accept the character
}

/******************************************************************************
*                                                                             *
*   void DataEvaluateDex(void)                                                *
*                                                                             *
*******************************************************************************
*
*   This function is called from the context set to recalculate data window
*   expressions for those windows that have it.
*
******************************************************************************/
void DataEvaluateDex(void)
{
    UINT window;                        // Data window counter
    DWORD offset;                       // Offset part of the new address
    char *pExp;                         // Pointer to the expression

    for(window=0; window<MAX_DATA; window++)
    {
        pExp = &deb.Dex[window][0];
        if( *pExp )
        {
            // Run the evaluator to see if we can arrive to a new address value

            evalSel = deb.dataAddr[window].sel;
            if( Expression(&offset, pExp, &pExp) )
            {
                // Verify the selector value
                if( VerifySelector(evalSel) )
                {
                    deb.DexError[window] = FALSE;
                    deb.dataAddr[window].offset = offset;
                    deb.dataAddr[window].sel = evalSel;

                    continue;           // Do next data expression
                }
            }

            // Expression is not valid, clear the evaluation error
            deb.DexError[window] = TRUE;
            deb.errorCode = NOERROR;
        }
    }
}

/******************************************************************************
*                                                                             *
*   static void DataEdit()                                                    *
*                                                                             *
*******************************************************************************
*
*   Edit data in place using the data window and cursor focus.
*
******************************************************************************/
static void DataEdit()
{
    BOOL fContinue = TRUE;              // Continuation flag
    int xOrig, yOrig, iOrig;            // Cursor temporary store and insert state store
    int xCur = 0, yCur = 0;             // Current cursor coordinates
    WCHAR Key;                          // Current key pressed
    TDataField *pDataDesc;              // Pointer to data type descriptor
    int index;                          // Current index of the data (x-direction)
    BOOL fAscii = FALSE;                // Edit mode: data or ASCII
    TADDRDESC Addr;                     // Current address that the cursor edits
    int offs;                           // Temporary offset value

    // Make sure that data window is open and points to the correct address
    // specified in the deb.data address structure, so we can edit there

    // Make data window visible and redraw if necessary
    if( pWin->data[deb.nData].fVisible==FALSE )
    {
        pWin->data[deb.nData].fVisible = TRUE;
        RecalculateDrawWindows();
    }

    // Print a bottom line message on edit keys. We offer option Tab only
    // for subClass 0-4, that is standard byte, word or dword
    dprint("%c%c%c%c%c%cValid control keys: %s %s %s %s Enter Esc    %s\r%c",
    DP_SAVEXY,
    DP_SETCURSORXY, 1+0, 1+pOut->sizeY-1,
    DP_SETCOLINDEX, COL_HELP,
    "Left", "Right", "Up", "Dn",
    deb.nDumpSize[deb.nData] > 4? "":"Tab: Toggle ASCII",
    DP_RESTOREXY);

    // Set up initial parameters to edit in DATA mode the first field
    pDataDesc = &DataField[deb.nDumpSize[deb.nData]];
    index = 0;

    xCur = pDataDesc->xStart[index];
    yCur = pWin->data[deb.nData].Top + 1;

    Addr = deb.dataAddr[deb.nData];
    DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

    // Save cursor coordinates manually since we can't have more than one
    // level of save & restore and DataDraw() is doing it
    xOrig = pOut->x;
    yOrig = pOut->y;
    iOrig = deb.fOvertype;

    // Set the cursor appearance to overtype cursor
    if( iOrig==FALSE )
        dprint("%c%c", DP_SETCURSORSHAPE, 2 );

    do
    {
        // Position the cursor at the right data field coordinates
        dprint("%c%c%c", DP_SETCURSORXY, xCur+1, yCur+1);

        Key = GetKey(TRUE);

        if( fAscii )
        {
            //=================================================================
            //  ASCII mode of edit
            //
            // Addr.offset is *not* following the cursor position since we edit
            // only 1 byte in place, and we can't undo it.
            // index is also not following.
            // The only relevant variables are xCur and yCur.
            //=================================================================

            switch( Key )
            {
                case ESC:       // ESC and ENTER quit edit
                case ENTER:
                    fContinue = FALSE;
                    break;

                case TAB:       // Tab key toggles back to editing DATA format
                    fAscii = FALSE;

                    // We need to recompute Addr since the other editing loop is using it
                    offs = xCur - pDataDesc->xAsciiStart;
                    index = offs / pDataDesc->FWidth;
                    xCur = pDataDesc->xStart[index];
                    Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                    DataGetSet(DATAGETSET_READ, &Addr, 0, 0);
                    break;

                case BACKSPACE: // For added convinience, Backspace key acts as left

                case LEFT:      // Left key moves cursor, possibly changing the line
                    if( xCur <= pDataDesc->xAsciiStart )
                    {
                        // Was that the first line in the data window?
                        if( yCur==pWin->data[deb.nData].Top+1 )
                        {
                            // Scroll whole window one byte to the left (decrement address)
                            deb.dataAddr[deb.nData].offset--;
                            DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                        }
                        else
                        {
                            // Decrement the Y coordinate and go to the end of the previous line
                            yCur--;
                            xCur = pDataDesc->xAsciiEnd;
                        }
                    }
                    else
                    {
                        // Freely decrement cursor
                        xCur--;
                    }
                    break;

                case UP:        // Up key accepts and cycles one line up
                    // Was that the first line in the data window?
                    if( yCur==pWin->data[deb.nData].Top+1 )
                    {
                        // Scroll whole window one line up
                        deb.dataAddr[deb.nData].offset -= DATA_BYTES;
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    }
                    else
                    {
                        // Simply move cursor up
                        yCur--;
                    }
                    break;

                case DOWN:      // Down key accepts and cycles one line down
                    // Was that the last line in the data window?
                    if( yCur==pWin->data[deb.nData].Bottom )
                    {
                        // Scroll whole window one line down
                        deb.dataAddr[deb.nData].offset += DATA_BYTES;
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    }
                    else
                    {
                        // Simply move cursor down
                        yCur++;
                    }
                    break;

                case PGUP:      // Page down in the data window
                        deb.dataAddr[deb.nData].offset -= DATA_BYTES * (pWin->data[deb.nData].nLines - 1);
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                case PGDN:      // Page up in the data window
                        deb.dataAddr[deb.nData].offset += DATA_BYTES * (pWin->data[deb.nData].nLines - 1);
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                default:        // Anything else is accepted
                        dprint("%c", Key);

                        // Store the ASCII character
                        Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + (xCur - pDataDesc->xAsciiStart);
                        AddrSetByte(&Addr, Key, TRUE);

                        // Redraw the window to display data in both panes
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);

                        // No break.. Continue similar to the RIGHT key:

                case RIGHT:     // Right key moves cursor, always changing the field (accepting)
                    if( xCur >= pDataDesc->xAsciiEnd )
                    {
                        // Was that the last line in the data window?
                        if( yCur==pWin->data[deb.nData].Bottom )
                        {
                            // Scroll whole window one BYTE to the right (increment address)
                            deb.dataAddr[deb.nData].offset++;
                            DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                        }
                        else
                        {
                            // Increment the Y coordinate and go to the beginning of the next line
                            yCur++;
                            xCur = pDataDesc->xAsciiStart;
                        }
                    }
                    else
                    {
                        // Freely increment cursor
                        xCur++;
                    }
                    break;
            }
        }
        else
        {
            //=================================================================
            //  DATA mode of edit
            //
            // Addr.offset is addressing a field that the cursor is on
            // index is the field number in a current line
            //=================================================================

            Key = toupper(Key);

            switch( Key )
            {
                case ESC:       // ESC key aborts change and quits edit
                    fContinue = FALSE;
                    break;

                case ENTER:     // Enter key accepts change and quits edit
                    DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);
                    fContinue = FALSE;
                    break;

                case TAB:       // Tab key accepts change and toggles ASCII, if allowed
                    DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);
                    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    if( pDataDesc->xAsciiStart )
                    {
                        fAscii = TRUE;

                        // We are trying to position cursor at exactly the same byte where the
                        // current DATA byte belings to. On little-endians, that needs some math..
                        offs = pDataDesc->FWidth - ((xCur-pDataDesc->xStart[index])/2) - 1;
                        xCur = pDataDesc->xAsciiStart + index * pDataDesc->FWidth + offs;
                    }
                    break;

                case BACKSPACE: // For added convinience, Backspace key acts as left, but
                                // only for the current field
                    if( xCur <= pDataDesc->xStart[index] )
                        break;

                case LEFT:      // Left key moves cursor, possibly changing the field (accepting)
                    if( --xCur < pDataDesc->xStart[index] )
                    {
                        // End of field reached.. Accept the new value and go to the next field
                        DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                        // Was that the first index in a line?
                        if( index==0 )
                        {
                            // Was that the first line in the data window?
                            if( yCur==pWin->data[deb.nData].Top+1 )
                            {
                                // Scroll whole window one field to the left (decrement address)
                                deb.dataAddr[deb.nData].offset -= deb.nDumpSize[deb.nData];
                            }
                            else
                            {
                                // Decrement the Y coordinate and go to the end of the previous line
                                yCur--;
                                index = pDataDesc->nF;
                            }
                        }
                        else
                        {
                            // Simply move to the index next to the left
                            index--;
                        }
                        xCur = pDataDesc->xStart[index];

                        // Reload new field
                        Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                        DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                        // Redisplay data
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    }
                    break;

                case UP:        // Up key accepts and cycles one line up
                    DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                    // Was that the first line in the data window?
                    if( yCur==pWin->data[deb.nData].Top+1 )
                    {
                        // Scroll whole window one line up
                        deb.dataAddr[deb.nData].offset -= DATA_BYTES;
                    }
                    else
                    {
                        // Simply move cursor up
                        yCur--;
                    }
                    // Reload new field
                    Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                    DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                    // Redisplay data
                    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                case DOWN:      // Down key accepts and cycles one line down
                    DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                    // Was that the last line in the data window?
                    if( yCur==pWin->data[deb.nData].Bottom )
                    {
                        // Scroll whole window one line down
                        deb.dataAddr[deb.nData].offset += DATA_BYTES;
                    }
                    else
                    {
                        // Simply move cursor down
                        yCur++;
                    }
                    // Reload new field
                    Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                    DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                    // Redisplay data
                    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                case PGUP:      // Page down in the data window
                        DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                        deb.dataAddr[deb.nData].offset -= DATA_BYTES * (pWin->data[deb.nData].nLines - 1);

                        // Reload new field
                        Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                        DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                        // Redisplay data
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                case PGDN:      // Page up in the data window
                        DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                        deb.dataAddr[deb.nData].offset += DATA_BYTES * (pWin->data[deb.nData].nLines - 1);

                        // Reload new field
                        Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                        DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                        // Redisplay data
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    break;

                default:        // All other characters are considered for input

                        if( DataGetSet(DATAGETSET_INSERT, &Addr, xCur-pDataDesc->xStart[index], Key) )
                        {
                            // Character was accepted, print it out
                            dprint("%c", Key);
                        }
                        else
                            fContinue = FALSE;      // Anything else implicitly quite edit

                        // No break.. Continue similar to the RIGHT key:

                case RIGHT:     // Right key moves cursor, possibly changing the field (accepting)
                    if( ++xCur > pDataDesc->xEnd[index] )
                    {
                        // End of field reached.. Accept the new value and go to the next field
                        DataGetSet(DATAGETSET_WRITE, &Addr, 0, 0);

                        // Was that the last index in a line?
                        if( index >= pDataDesc->nF )
                        {
                            // Was that the last line in the data window?
                            if( yCur==pWin->data[deb.nData].Bottom )
                            {
                                // Scroll whole window one field to the right (increment address)
                                deb.dataAddr[deb.nData].offset += deb.nDumpSize[deb.nData];
                            }
                            else
                            {
                                // Increment the Y coordinate and go to the beginning of the next line
                                yCur++;
                                index = 0;
                            }
                        }
                        else
                        {
                            // Simply move to the next right index
                            index++;
                        }
                        xCur = pDataDesc->xStart[index];

                        // Reload new field
                        Addr.offset = deb.dataAddr[deb.nData].offset + DATA_BYTES * (yCur - pWin->data[deb.nData].Top - 1) + index * pDataDesc->FWidth;
                        DataGetSet(DATAGETSET_READ, &Addr, 0, 0);

                        // Redisplay data
                        DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);
                    }
                    break;
            }
        }
    }
    while( fContinue );

    // Restore cursor appearance if it was 'insert' (we had it to be 'overtype', that is TRUE)
    if( iOrig==FALSE )
    {
        deb.fOvertype = FALSE;
        dprint("%c%c", DP_SETCURSORSHAPE, 1 );
    }

    // Redisplay data
    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);

    // Restore cursor coordinates
    pOut->x = xOrig;
    pOut->y = yOrig;
}

/******************************************************************************
*                                                                             *
*   BOOL cmdDdump(char *args, int subClass)                                   *
*                                                                             *
*******************************************************************************
*
*   Data dump command. Opend data window if it is not already open.
*
*   subClass is:
*       0   D  (the last data size)
*       1   DB (dump BYTE)
*       2   DW (dump WORD)
*       4   DD (dump DWORD)
*
******************************************************************************/
BOOL cmdDdump(char *args, int subClass)
{
    DWORD offset;                       // Temporary offset portion

    // If data size was explicitly specified, set it as default
    if( subClass )
        deb.nDumpSize[deb.nData] = subClass;

    if( *args!=0 )
    {
        // Argument present: D <address> [L <len>]
        evalSel = deb.dataAddr[deb.nData].sel;
        if( Expression(&offset, args, &args) )
        {
            // Verify the selector value
            if( VerifySelector(evalSel) )
            {
                deb.dataAddr[deb.nData].offset = offset;
                deb.dataAddr[deb.nData].sel = evalSel;
            }
            else
                return( TRUE );
        }
    }
    else
    {
        // No arguments - advance current address
        deb.dataAddr[deb.nData].offset += DATA_BYTES * GetDataLines();
    }

    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdEdit(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Edit command. If address and a set of data is given, edit stores it.
*   Otherwise, it opens data window (if it is not already open), and lets
*   user edit it in-place.
*
*   subClass is:
*       0   E  (the last data size)
*       1   EB (edit BYTE)
*       2   EW (edit WORD)
*       4   ED (edit DWORD)
*
******************************************************************************/
BOOL cmdEdit(char *args, int subClass)
{
    TADDRDESC EditAddr;                 // Effective edit address

    // If data size was explicitly specified, redraw data window if it is
    // different from the default, then set it to be the current format
    if( subClass )
    {
        if( deb.nDumpSize[deb.nData] != subClass )
        {
            deb.nDumpSize[deb.nData] = subClass;

            RecalculateDrawWindows();
        }
    }


    if( *args==0 )
    {
        // No arguments, if the data window is not visible, make it visible,
        // then edit in place
        DataEdit();
    }
    else
    {
        // Argument can be either:
        // E [address]                  - open data window and edit in place
        // E [address] [data-list]      - store data without opening data window

        // Set the default selector to current data window
        evalSel = deb.dataAddr[deb.nData].sel;

        if( Expression(&EditAddr.offset, args, &args) )
        {
            // Assign the complete start address
            EditAddr.sel = evalSel;

            // Skip possible spaces to either reach the end of line or data string(s)
            while( *args==' ' ) args++;

            // Optionally we have some data following
            if( *args )
            {
                // Some data was following the address, edit that data instead of using data window

                // TODO: Finish edit set of immediate strings.
                //       Perhaps combine functions that take a set of variable arguments
                //       like 'string',num,... for search and edit etc. ?

                dprinth(1, "Edit immediate data not implemented yet.");
            }
            else
            {
                // No data was following the address, open the data window for in place edit
                deb.dataAddr[deb.nData] = EditAddr;

                DataEdit();
            }
        }
        else
        {
            // Something was not right with the address
            dprinth(1, "Syntax error");
            return( TRUE );
        }
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdFormat(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Changes format of the data window (cycles).
*
******************************************************************************/
BOOL cmdFormat(char *args, int subClass)
{
    switch( deb.nDumpSize[deb.nData] )
    {
        case 1:     deb.nDumpSize[deb.nData] = 2;   break;
        case 2:     deb.nDumpSize[deb.nData] = 4;   break;
        case 4:     deb.nDumpSize[deb.nData] = 1;   break;
    }

    DataDraw(TRUE, deb.dataAddr[deb.nData].offset, TRUE);

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdDex(char *args, int subClass)                                     *
*                                                                             *
*******************************************************************************
*
*   Displays or assigns an expression to the data window
*
*   DEX         - lists expressions assigned to every data window
*   DEX # <exp> - assigns an expression to a data window
*   DEX #       - removes the expression for that data window
*
******************************************************************************/
BOOL cmdDex(char *args, int subClass)
{
    int nLine = 1;                      // Print line counter
    UINT window;                        // Data window counter

    if( *args )
    {
        // Get the window number and copy the expression string

        if( GetDecB(&window, &args) )
        {
            if( window < MAX_DATA )
            {
                while( *args==' ' ) *args++;    // Skip optional spaces

                if( *args )
                {
                    // Make sure the selected data window is visible (but dont switch to it!)
                    pWin->data[window].fVisible = TRUE;

                    // Copy the expression string
                    strcpy(&deb.Dex[window][0], args);
                }
                else
                {   // If there are no more parameters, remove the expression

                    deb.Dex[window][0] = 0;
                }

                // Run the initial evaluation and redraw windows on a way back
                DataEvaluateDex();

                deb.fRedraw = TRUE;
            }
            else
                PostError(ERR_DATAWIN, 0);
        }
        else
            PostError(ERR_SYNTAX, 0);
    }
    else
    {
        // List expressions assigned to every data window

        for(window=0; window<MAX_DATA; window++)
        {
            if( deb.Dex[window][0] )
                if( dprinth(nLine++, " %d  %s %s", window, deb.Dex[window], deb.DexError[window]?"(error evaluating)":"")==FALSE )
                    break;
        }
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   DWORD fnDataAddr(DWORD arg)                                               *
*                                                                             *
*******************************************************************************
*
*   Expression evaluator helper function to return the address of the first
*   data item displayed in the data window.
*
******************************************************************************/
DWORD fnDataAddr(DWORD arg)
{
    return( deb.dataAddr[deb.nData].offset );
}
