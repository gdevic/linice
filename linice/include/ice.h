/******************************************************************************
*                                                                             *
*   Module:     ice.h                                                         *
*                                                                             *
*   Date:       10/27/2000                                                    *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file contains global Ice data structures

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 10/27/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_H_
#define _ICE_H_

#include "intel.h"                      // Include Intel x86 specific defines
#include "ice-limits.h"                 // Include our limits

/******************************************************************************
*                                                                             *
*   Linux kernel Extern functions                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/
// Forward declarations, a strange C thing...
struct TSYMTAB;          
             
/////////////////////////////////////////////////////////////////
// THE MAIN DEBUGGER STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    BYTE *hSymbolBuffer;                // Handle to symbol buffer pool
    struct TSYMTAB *pSymTab;            // Linked list of symbol tables

    BYTE *pHistoryBuffer;               // Pointer to a history buffer
    DWORD nHistorySize;                 // History buffer size

    BOOL fLowercase;                    // Is output disassembly lowercased?

    BOOL fRunningIce;                   // Is ICE running?

    int layout;                         // Keyboard layout
    char keyFn[4][12][MAX_STRING];      // Key assignments for function keys

} TICE, *PTICE;

extern TICE Ice;                        // Global data structure
extern PTICE pIce;                      // and a pointer to it

/////////////////////////////////////////////////////////////////
// STRUCTURE DESCRIBING THE LIVE DEBUGEE
/////////////////////////////////////////////////////////////////
// Define structure that holds debugee state after an interrupt

typedef struct tagTRegs
{
    DWORD   esp;
    DWORD   ss;
    DWORD   es;
    DWORD   gs;
    DWORD   fs;
    DWORD   ds;
    DWORD   edi;
    DWORD   esi;
    DWORD   ebp;
    DWORD   temp;
    DWORD   ebx;
    DWORD   edx;
    DWORD   ecx;
    DWORD   eax;
    DWORD   ErrorCode;
    DWORD   eip;
    DWORD   cs;
    DWORD   eflags;
} TREGS, *PTREGS;

// Root structure describing the debugee state

typedef struct
{
    TDescriptor idt;                    // Linux idt descriptor
    TDescriptor gdt;                    // Linux gdt descriptor
    PTREGS r;                           // pointer to live registers
    TREGS  r_prev;                      // previous registers (for color coding of changes)
    int nInterrupt;                     // Interrupt that occurred

} TDEB, *PTDEB;

extern TDEB deb;                        // Global data structure

/////////////////////////////////////////////////////////////////
// SCREEN WINDOW STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    // These are set by the user
    BOOL fVisible;                      // Whether a window is visible or not
    DWORD nLines;                       // How many lines a window occupies

    // The following are calculated on a fly
    DWORD Top;                          // Top coordinate holding the header line
    DWORD Bottom;                       // Bottom coordinate (inclusive)
    void (*draw)();                     // Function that draws inside window

} TWND, *PTWND;

typedef struct
{
    TWND r;                             // Register window
    TWND d;                             // Data window
    TWND c;                             // Code window
    TWND h;                             // Command (history) window

} TWINDOWS, *PTWINDOWS;

extern TWINDOWS Win;
extern PTWINDOWS pWin;

/////////////////////////////////////////////////////////////////
// DEBUGGER COMMAND STRUCTURE
/////////////////////////////////////////////////////////////////

typedef BOOL (*TFNCOMMAND)(char **args, int subClass);

typedef struct
{
    char *sCmd;                         // Command name
    DWORD nLen;                         // Length of the command string in characters
    int subClass;                       // Optional command argument (subclass)
    TFNCOMMAND pfnCommand;              // Pointer to a function for the command
    char *sSyntax;                      // Syntax string
    char *sExample;                     // Example string
    DWORD iHelp;                        // Index to description string
    
} TCommand;

extern TCommand Cmd[];                  // Command structure array
extern char *sHelp[];                   // Help lines

/////////////////////////////////////////////////////////////////
// OUTPUT SUBSYSTEM DEFINITION
/////////////////////////////////////////////////////////////////
// This structure is instantiated by every output module

typedef struct
{
    BYTE x, y;                          // Current cursor coordinates
    BYTE width, height;                 // Current screen width and height
    BYTE start_x, start_y;              // Display start coordinates

    void (*sprint)(char *c);            // Effective print string function

} TOUT, *PTOUT;

extern PTOUT pOut;                      // Pointer to a current output device

#define DP_SAVEBACKGROUND       0xFF    // Save background into a private buffer
#define DP_RESTOREBACKGROUND    0xFE    // Restore saved background
#define DP_CLS                  0xFD    // Clear screen
#define DP_SETCURSORXY          0xFC    // Set cursor coordinates (1..x, 1..x)
#define DP_SAVEXY               0xFB    // Single-level save XY cursor coords
#define DP_RESTOREXY            0xFA    // Single-level restore XY cursor coords
#define DP_SETSCROLLREGIONYY    0xF9    // Set top and bottom scroll region
#define DP_SCROLLUP             0xF8    // Scroll up a scroll region
#define DP_SCROLLDOWN           0xF7    // Scroll down a scroll region
#define DP_SETWRITEATTR         0xF6    // Set the current write attribute index


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

#define LAYOUT_US               0       // US keyboard layout
#define LAYOUT_GERMAN           1       // German keyboard layout
#define MAX_LAYOUT              2

#define MAX_INPUT_QUEUE         32      // Set max input queue size

extern CHAR GetKey( BOOL fBlock );      // Read a key code from the input queue
#define CHAR_SHIFT              0x0100  // <key> + SHIFT
#define CHAR_ALT                0x0200  // <key> + ALT
#define CHAR_CTRL               0x0400  // <key> + CTRL

extern CHAR PutKey( CHAR Key );

extern int dprint( char *format, ... );
extern BOOL dprinth( int nLineCount, char *format, ... );
extern void dputc(char c);


extern void HistoryDraw(void);
extern DWORD HistoryDisplay(DWORD hView, int nDir);
extern DWORD HistoryGetTop(void);
extern void HistoryAdd(char *sLine);
extern void ClearHistory(void);

extern int GetByte(DWORD absOffset);

#endif //  _ICE_H_
