/******************************************************************************
*                                                                             *
*   Module:     blockops.c                                                    *
*                                                                             *
*   Date:       12/01/00                                                      *
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

        This module contains code for block memory access commands

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 12/30/00   Original                                             Goran Devic *
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

#define MAX_AUXBUF       256            // Maximum fill/search string len

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   BOOL cmdFill(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Fills memory with data.
*
******************************************************************************/
BOOL cmdFill(char *args, int subClass)
{
    static BYTE auxBuf[MAX_AUXBUF];     // Buffer to collect byte-values for fill
    TADDRDESC Addr;                     // Final address to fill
    DWORD value, len = 0;               // Default len is 0 (or not specified)
    DWORD index = 0, i;

    // Set the default selector to kernel DS
    evalSel = deb.r->ds;

    //===========================================================
    // Get the fill starting address (evalSel:offset)
    //===========================================================
    if( Expression(&Addr.offset, args, &args) )
    {
        // Verify that the selector is readable and valid
        if( VerifySelector(evalSel) )
        {
            Addr.sel = evalSel;

            // If the optional 'L' length token is given, read the length
            if( *args=='l' || *args=='L' )
            {
                args++;

                // Read the length parameter
                if( !Expression(&len, args, &args) )
                    goto SyntaxError;
            }

            //======================================================================
            // Get the list of bytes or quoted strings separated by spaces or commas
            // and store them in the aux buffer
            //======================================================================
            while( *args )
            {
                // Skip spaces, commas
                while( *args==' ' || *args==',' ) args++;

                if( *args=='\"' || *args=='\'' )
                {
                    args++;

                    // It is a quoted string.. copy it in
                    while( *args && *args!='\"' && *args!='\'' )
                        auxBuf[index++] = *args++;

                    // Skip the closing quote if not the end of line
                    if( *args ) args++;
                }
                else
                {
                    // It is a number (value)
                    if( !Expression(&value, args, &args) || value > 0xFF )
                        goto SyntaxError;

                    auxBuf[index++] = value;
                }
            }

            // Fill length string can not be 0!
            if( index==0 )
                goto SyntaxError;

            //===========================================================
            // Do the actual memory fill
            //===========================================================

            // If len was not specified, use as many bytes as given
            if( len==0 )
                len = index;
#if 0
            {
                // Test of how did we process those arguments
                static char str[256];
                char *p = str;

                p += sprintf(p, "%04X:%08X [%X,%X] ", Addr.sel, Addr.offset, len, index);
                for(i=0; i<index; i++ )
                {
                    p += sprintf(p, "%02X ", auxBuf[i]);
                }
                dprinth(1, "%s", str);
            }
#endif
            i = 0;
            while( len-- )
            {
                AddrSetByte(&Addr, auxBuf[i], FALSE);
                Addr.offset++;

                // Wrap the index around the fill buffer
                if( ++i==index )
                    i = 0;
            }
        }
    }
    else
    {
        SyntaxError:
        dprinth(1, "Syntax error");
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdSearch(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Search memory for data.
*
******************************************************************************/
BOOL cmdSearch(char *args, int subClass)
{
    static BYTE auxBuf[MAX_AUXBUF];     // Buffer to collect byte-values for search
    static TADDRDESC Addr;              // Starting address to search
    static DWORD searchLen = 0;         // Length of memory to search
    static DWORD index;                 // Length of the given search string
    static BOOL fIgnoreCase;            // Case-insensitive ASCII search?

    DWORD value, remains, matching;


    // Did we ask for a subsequent search?
    if( *args )
    {
        // Parameters given...New search
        index = 0;

        // Check for case-insensitive search
        if( !strnicmp(args, "-c", 2) )
        {
            args += 2;
            fIgnoreCase = TRUE;
        }
        else
            fIgnoreCase = FALSE;

        // Set the default selector to kernel DS
        evalSel = deb.r->ds;

        //===========================================================
        // Get the search starting address (evalSel:offset)
        //===========================================================
        if( Expression(&Addr.offset, args, &args) )
        {
            // Verify that the selector is readable and valid
            if( VerifySelector(evalSel) )
            {
                Addr.sel = evalSel;

                // Get the mandatory 'L' length token and the length
                if( *args=='l' || *args=='L' )
                {
                    args++;

                    // Read the length parameter. can not be zero
                    if( !Expression(&searchLen, args, &args) || searchLen==0 )
                        goto SyntaxError;
                }
                else
                    goto SyntaxError;

                //======================================================================
                // Get the list of bytes or quoted strings separated by spaces or commas
                // and store them in the aux buffer
                //======================================================================
                while( *args )
                {
                    // Skip spaces, commas
                    while( *args==' ' || *args==',' ) args++;

                    if( *args=='\"' || *args=='\'' )
                    {
                        args++;

                        // It is a quoted string.. copy it in
                        while( *args && *args!='\"' && *args!='\'' )
                            auxBuf[index++] = *args++;

                        // Skip the closing quote if not the end of line
                        if( *args ) args++;
                    }
                    else
                    {
                        // It is a number (value)
                        if( !Expression(&value, args, &args) || value > 0xFF )
                            goto SyntaxError;

                        auxBuf[index++] = value;
                    }
                }

                // Search length string can not be 0!
                if( index==0 )
                    goto SyntaxError;

                // If we are doing a case-insensitive search, we will first
                // make all ASCII characters inside buffer lowercased
                if( fIgnoreCase )
                    for(value=0; value<index; value++)
                        if( auxBuf[value]>='A' && auxBuf[value]<='Z' )
                            auxBuf[value] += 'a' - 'A';

                // Start a new search with the values that we just set up
                goto DoSearch;
            }
        }
        else
        {
            SyntaxError:
            dprinth(1, "Syntax error");
        }
    }
    else
    {
        // Reuse previous search
        if( searchLen )
        {
            // Do the actual search reusing the values from the previous search
DoSearch:
            matching = 0;

            while( searchLen )
            {
                if( AddrIsPresent(&Addr) )
                {
                    value = AddrGetByte(&Addr);
                    // If we are doing a case-insensitive search, make ASCII a lowercased
                    // to match what we got in the buffer
                    if( fIgnoreCase )
                        if( value>='A' && value<='Z' )
                            value += 'a' - 'A';

                    if( value==auxBuf[matching] )
                        matching++;     // If a value matches, increment matching count
                    else
                    {
                        if( matching > 0 )
                        {
                            // Fixup address and len to go back to the start of the string
                            // that matched incompletely, so we dont lose any substrings
                            Addr.offset -= matching;
                            searchLen += matching;
                            matching = 0;   // Otherwise, reset it back to zero
                        }
                    }

                    searchLen--;        // Decrement search length
                    Addr.offset++;      // Increment address

                    // If we matched complete string, print the address, open the data window
                    // and point the data window to it
                    if( matching==index )
                    {
                        dprinth(1, "PATTERN FOUND AT %04X:%08X", Addr.sel, Addr.offset - matching);

                        // Open the data window if it is closed
                        if( pWin->data[deb.nData].fVisible==FALSE )
                        {
                            pWin->data[deb.nData].fVisible = TRUE;
                            RecalculateDrawWindows();
                        }

                        // Point the data window to the search address
                        DataDraw(FALSE, Addr.offset - matching, TRUE);

                        break;
                    }

                    // If we pressed ESC, break the search (if it's taking too long)
                    if( GetKey(FALSE)==ESC )
                        break;
                }
                else
                {
                    // Page is not present.. skip that page if enough length was left
                    remains = 4096 - (Addr.offset & 0xFFF);
                    if( searchLen <= remains )
                        searchLen = 0;
                    else
                        searchLen = searchLen - remains;

                    Addr.offset += remains;
                }
            }
        }
        else
        {
            dprinth(1, "No previous incomplete searches");
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdCompare(char *args, int subClass)                                 *
*                                                                             *
*******************************************************************************
*
*   Compare two data blocks.
*
******************************************************************************/
BOOL cmdCompare(char *args, int subClass)
{
    TADDRDESC Addr1, Addr2;             // Block 1 and 2 addresses
    DWORD len;                          // Length of the block to compare
    BYTE b1, b2;                        // Temp bytes from each block
    BOOL fExtended = FALSE;             // Extended option?
    int nLine = 1;                      // Standard dprinth line counter

    // The format is: [-e] addr1 L len addr2. All parameters are mandatory.

    // Check for extended output parameter
    if( !strnicmp(args, "-e", 2) )
    {
        args += 2;
        fExtended = TRUE;
    }

    // Set the default selector to kernel DS
    evalSel = deb.r->ds;

    // Get the first address
    if( Expression(&Addr1.offset, args, &args) )
    {
        // Verify that the selector is readable and valid
        if( VerifySelector(evalSel) )
        {
            Addr1.sel = evalSel;

            // Get the mandatory 'L' length token and the length
            if( *args=='l' || *args=='L' )
            {
                args++;

                if( Expression(&len, args, &args) && (len!=0) )
                {
                    // Get the second address
                    evalSel = deb.r->ds;

                    if( Expression(&Addr2.offset, args, &args) )
                    {
                        // Verify that the selector is readable and valid
                        if( VerifySelector(evalSel) )
                        {
                            Addr2.sel = evalSel;

                            // Start block compare
                            while( len-- )
                            {
                                b1 = AddrGetByte(&Addr1);
                                b2 = AddrGetByte(&Addr2);

                                // Two ways of displaying data: by default display only
                                // addresses that dont match. Extended way displays all.
                                if( fExtended )
                                {
                                    if( dprinth(nLine++, "%04X:%08X  %02X %c %02X  %04X:%08X",
                                        Addr1.sel, Addr1.offset, b1, b1==b2? ' ':'*',
                                        b2, Addr2.sel, Addr2.offset)==FALSE )
                                            break;
                                }
                                else
                                if( b1 != b2 )
                                {
                                    if( dprinth(nLine++, "%04X:%08X  %02X  %02X  %04X:%08X",
                                        Addr1.sel, Addr1.offset, b1,
                                        b2, Addr2.sel, Addr2.offset)==FALSE )
                                            break;
                                }

                                Addr1.offset++;
                                Addr2.offset++;
                            }
                        }

                        return(TRUE);
                    }
                }
            }
        }
    }

    dprinth(1, "Syntax error");

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL cmdMove(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Move a memory block.
*
******************************************************************************/
BOOL cmdMove(char *args, int subClass)
{
    TADDRDESC AddrSrc, AddrDest;        // Source and destination address
    DWORD len;                          // Length in bytes to move

    // The format is: source L len dest. All parameters are mandatory.

    // Set the default selector to kernel DS
    evalSel = deb.r->ds;

    if( Expression(&AddrSrc.offset, args, &args) )
    {
        // Verify that the selector is readable and valid
        if( VerifySelector(evalSel) )
        {
            AddrSrc.sel = evalSel;

            // Get the mandatory 'L' length token and the length
            if( *args=='l' || *args=='L' )
            {
                args++;

                if( Expression(&len, args, &args) && (len!=0) )
                {
                    // Get the second address
                    evalSel = deb.r->ds;

                    if( Expression(&AddrDest.offset, args, &args) )
                    {
                        // Verify that the selector is readable and valid
                        if( VerifySelector(evalSel) )
                        {
                            AddrDest.sel = evalSel;

                            // Start a smart block move: If the destination address is
                            // after the source, start from the tail and decrement.
                            // Otherwise, start from the head and increment. That will
                            // take care of overlapping blocks.

                            if( AddrDest.offset > AddrSrc.offset )
                            {
                                AddrSrc.offset += len - 1;
                                AddrDest.offset += len - 1;

                                while( len-- )
                                {
                                    AddrSetByte(&AddrDest, AddrGetByte(&AddrSrc), FALSE);
                                    AddrSrc.offset--;
                                    AddrDest.offset--;
                                }
                            }
                            else
                            {
                                while( len-- )
                                {
                                    AddrSetByte(&AddrDest, AddrGetByte(&AddrSrc), FALSE);
                                    AddrSrc.offset++;
                                    AddrDest.offset++;
                                }
                            }
                        }

                        return(TRUE);
                    }
                }
            }
        }
    }

    dprinth(1, "Syntax error");

    return(TRUE);
}


