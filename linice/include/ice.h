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

    // Statistics variables
    int nIntsPass[0x40];                // Number of interrupts received
    int nIntsIce[0x40];                 // While debugger run

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
    DWORD   ds;
    DWORD   fs;
    DWORD   gs;
    DWORD   edi;
    DWORD   esi;
    DWORD   ebp;
    DWORD   temp;
    DWORD   ebx;
    DWORD   edx;
    DWORD   ecx;
    DWORD   eax;
    DWORD   ChainAddress;
    DWORD   ErrorCode;
    DWORD   eip;
    DWORD   cs;
    DWORD   eflags;
} TREGS, *PTREGS;

// Generic address descriptor - used to describe any memory access

typedef struct
{
    WORD sel;
    DWORD offset;

} TADDRDESC, *PTADDRDESC;

// Root structure describing the debugee state

typedef struct
{
    TDescriptor idt;                    // Linux idt descriptor
    TDescriptor gdt;                    // Linux gdt descriptor
    PTREGS r;                           // pointer to live registers
    TREGS  r_prev;                      // previous registers (for color coding of changes)
    int nInterrupt;                     // Interrupt that occurred

    int DumpSize;                       // Dx dump value size
    TADDRDESC dataAddr;                 // Data - current display address

    BOOL fCode;                         // Code - is SET CODE ON ?
    TADDRDESC codeAddr;                 // Code - current display address

    BOOL fAltscr;
    BOOL fFaults;
    BOOL fI1Here;
    BOOL fI3Here;
    BOOL fLowercase;
    BOOL fPause;
    BOOL fSymbols;

} TDEB, *PTDEB;

extern TDEB deb;                        // Global data structure

/////////////////////////////////////////////////////////////////
// SCREEN WINDOW STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    // These are set by the user
    BOOL fVisible;                      // Whether a frame is visible or not
    DWORD nLines;                       // How many lines a frame occupies

    // The following are calculated on a fly
    DWORD Top;                          // Top coordinate holding the header line
    DWORD Bottom;                       // Bottom coordinate (inclusive)
    void (*draw)();                     // Function that draws inside frame

} TFRAME, *PTFRAME;

typedef struct
{
    TFRAME r;                           // Register window frame
    TFRAME d;                           // Data window frame
    TFRAME c;                           // Code window frame
    TFRAME h;                           // Command (history) window frame

} TWINDOWS, *PTWINDOWS;

extern TWINDOWS Win;
extern PTWINDOWS pWin;

/////////////////////////////////////////////////////////////////
// DEBUGGER COMMAND STRUCTURE
/////////////////////////////////////////////////////////////////

typedef BOOL (*TFNCOMMAND)(char *args, int subClass);

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
// INTERNAL MOUSE PACKET STRUCTURE
/////////////////////////////////////////////////////////////////
// This is the internal mouse packet structure, device independent

typedef struct
{
    int Xd;                             // Mouse X displacement
    int Yd;                             // Mouse Y displacement
    int buttons;                        // LB | CB | RB
                                        // 2    1    0
} TMPACKET, *PTMPACKET;

/////////////////////////////////////////////////////////////////
// OUTPUT SUBSYSTEM DEFINITION
/////////////////////////////////////////////////////////////////
// This structure is instantiated by every output module

typedef struct
{
    BYTE x, y;                          // Current cursor coordinates
    BYTE sizeX, sizeY;                  // Current screen width and height
    BYTE startX, startY;                // Display start coordinates

    void (*sprint)(char *c);            // Effective print string function
    void (*mouse)(int, int);            // Function that displays mouse cursor

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

extern void PutKey( CHAR Key );

extern int dprint( char *format, ... );
extern BOOL dprinth( int nLineCount, char *format, ... );
extern void dputc(UCHAR c);

extern BOOL CommandExecute( char *pCmd );
extern void HistoryDraw(void);
extern DWORD HistoryDisplay(DWORD hView, int nDir);
extern DWORD HistoryGetTop(void);
extern void HistoryAdd(char *sLine);
extern void ClearHistory(void);

extern BYTE ReadCRTC(int index);
extern void WriteCRTC(int index, int value);
extern int GetByte(WORD sel, DWORD offset);

extern BOOL AddrIsPresent(PTADDRDESC pAddr);
extern BYTE AddrGetByte(PTADDRDESC pAddr);


#endif //  _ICE_H_
