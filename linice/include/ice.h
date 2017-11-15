/******************************************************************************
*                                                                             *
*   Module:     ice.h                                                         *
*                                                                             *
*   Date:       04/27/2000                                                    *
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

        This header file contains global Ice data structures

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------        ----------- *
* 04/27/00   Original                                             Goran Devic *
* --------   ---------------------------------------------        ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ICE_H_
#define _ICE_H_

#include "intel.h"                      // Include Intel x86 specific defines
#include "ice-limits.h"                 // Include our limits
#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include symbol table defines

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

/////////////////////////////////////////////////////////////////
// THE MAIN DEBUGGER STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    BYTE *hSymbolBuffer;                // Handle to symbol buffer pool
    DWORD nSymbolBufferSize;            // Symbol buffer size
    DWORD nSymbolBufferAvail;           // Symbol buffer size available
    DWORD nXDrawSize;                   // DrawSize parameter
    BYTE *pXDrawBuffer;                 // Backstore buffer pointer, if XInitPacket accepted
    BYTE *pXFrameBuffer;                // Mapped X frame buffer
    TSYMTAB *pSymTab;                   // Linked list of symbol tables
    TSYMTAB *pSymTabCur;                // Pointer to the current symbol table
    DWORD nVars;                        // Number of user variables
    DWORD nMacros;                      // Number of macros
    BYTE *hHeap;                        // Handle to internal memory heap

    BYTE *pHistoryBuffer;               // Pointer to a history buffer
    DWORD nHistorySize;                 // History buffer size

    BOOL fKbdBreak;                     // Schedule break into the ICE
    BOOL fRunningIce;                   // Is ICE running?
    DWORD eDebuggerState;               // Debugger state number
#define DEB_STATE_BREAK             0   //  Default state
#define DEB_STATE_DELAYED_TRACE     1
#define DEB_STATE_DELAYED_ARM       2
#define MAX_DEBUGGER_STATE          3

    int layout;                         // Keyboard layout
    char keyFn[4][12][MAX_STRING];      // Key assignments for function keys
                                        // Fn:0 Shift:1 Alt:2 Control:3

    BYTE col[6];                        // Color attributes (1-5)
#define COL_NORMAL          1
#define COL_BOLD            2
#define COL_REVERSE         3
#define COL_HELP            4
#define COL_LINE            5

    // Statistics variables
    int nIntsPass[0x40];                // Number of interrupts received
    int nIntsIce[0x40];                 // While debugger run

    // Timers - decremented by the timer interrupt down to zero
    //  0 - serial polling
    //  1 - cursor carret blink
    DWORD timer[2];

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
    TSysreg sysReg;                     // System registers
    BOOL fIoApic;                       // Are we using IO APIC?
    TIOAPIC0 IoApic0;                   // Initial content of the first IO APIC register
    TIOAPIC1 IoApic1;                   // Initial content of the second IO APIC register
    TIOAPICREDIR IoApicRedir[MAX_IOAPICREG];
                                        // Initial content of IO APIC redirection registers
    BOOL fSmp;                          // Is this machine a SMP enabled?
    int cpu;                            // CPU number that the debugger uses
    int nInterrupt;                     // Interrupt that occurred
    int bpIndex;                        // Index of the breakpoint that hit (default -1)

    int DumpSize;                       // Dx dump value size
    TADDRDESC dataAddr;                 // Data - current display address

    BOOL fCode;                         // Code - is SET CODE ON ?
    TADDRDESC codeTopAddr;              // Address of the top machine code instr. in the code window
    TADDRDESC codeBottomAddr;           // Address of the last line
    TSYMFNSCOPE *pFnScope;              // Pointer to a current function scope descriptor
    TSYMFNLIN *pFnLin;                  // Pointer to a current function line descriptor
    TSYMSOURCE *pSource;                // Current source file in the code window
    WORD codeFileTopLine;               // Line number of the top of code window source file
    WORD codeFileEipLine;               // Line number of the current EIP in the windows source file
    WORD codeFileXoffset;               // X offset of the source code text
    int eSrc;                           // 0=Source off; 1=Source on; 2=Mixed

    BOOL fAltscr;
    BOOL fFaults;
    BOOL fI1Here;                       // Break on INT1
    BOOL fI1Kernel;                     // Break on INT1 only in kernel code
    BOOL fI3Here;                       // Break on INT3
    BOOL fI3Kernel;                     // Break on INT3 only in kernel code
    BOOL fLowercase;
    BOOL fSymbols;
    BOOL fFlash;                        // Restore screen during P and T commands
    BOOL fPause;                        // Pause after a screenful of scrolling info
    DWORD dwTabs;                       // Tabs value for source display
    BYTE fOvertype;                     // Cursor shape is overtype? (or insert)
    DWORD FrameX;                       // X-Window frame origin X coordinate
    DWORD FrameY;                       // X-Window frame origin X coordinate
    DWORD nFont;                        // Font number for hi-res display

    BOOL fTrace;                        // Trace command in progress
    BOOL fDelayedTrace;                 // Delayed trace request
    DWORD TraceCount;                   // Single trace instr. count to go

    BOOL fStep;                         // Single logical step in progress (command P)
    BOOL fStepRet;                      // Single step until RET instruction (P RET)

    BOOL fRedraw;                       // Request to redraw the screen after the current command
    DWORD error;                        // Error code

    CHAR BreakKey;                      // Key combination for break

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

} TFRAME, *PTFRAME;

typedef struct
{
    TFRAME r;                           // Register window frame
    TFRAME l;                           // Locals window frame
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
    void (*carret)(BOOL fOn);           // Cursor (carret) callback
    BOOL (*resize)(int, int, int);      // Resize window command

} TOUT, *PTOUT;

extern PTOUT pOut;                      // Pointer to a current output device

#define DP_SAVEBACKGROUND       0x01    // Save background into a private buffer
#define DP_RESTOREBACKGROUND    0x02    // Restore saved background
#define DP_CLS                  0x03    // Clear screen
#define DP_SETCURSORXY          0x04    // Set cursor coordinates (1..x, 1..x)
#define DP_SAVEXY               0x05    // Single-level save XY cursor coords
#define DP_RESTOREXY            0x06    // Single-level restore XY cursor coords
#define DP_SETSCROLLREGIONYY    0x07    // Set top and bottom scroll region
#define _BACKSPACE_             0x08    // Backspace code
#define DP_TAB                  0x09    // Tabulation code
#define _LF_                    0x0A    // Line feed code
#define DP_SETCOLINDEX          0x0B    // Set the current line's color index
//                              0x0C    // Reserved
#define _CR_                    0x0D    // Carriage return code
#define DP_SCROLLUP             0x0E    // Scroll up a scroll region
#define DP_SCROLLDOWN           0x0F    // Scroll down a scroll region
#define DP_SETCURSORSHAPE       0x10    // Set Insert/Overtype cursor shape (1, 2)
#define DP_ENABLE_OUTPUT        0x11    // Enable output driver
#define DP_DISABLE_OUTPUT       0x12    // Disable output driver
#define DP_RIGHTALIGN           0x13    // Right align the rest of the line
#define DP_ESCAPE               0x14    // Escape character, print next ascii code
#define DP_AVAIL                0x15    // First available code


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

extern void PutKey( CHAR Key );

extern int dprint( char *format, ... );
extern BOOL dprinth( int nLineCount, char *format, ... );
extern int PrintLine(char *format,...);
extern void dputc(UCHAR c);

extern void RegDraw(BOOL fForce);
extern void LocalsDraw(BOOL fForce);
extern void DataDraw(BOOL fForce, DWORD newOffset);
extern void CodeDraw(BOOL fForce);
extern void RecalculateDrawWindows();

extern BOOL CommandExecute( char *pCmd );

extern void HistoryDraw(void);
extern DWORD HistoryDisplay(DWORD hView, int nDir);
extern DWORD HistoryGetTop(void);
extern void HistoryAdd(char *sLine);
extern void ClearHistory(void);

extern BYTE ReadCRTC(int index);
extern void WriteCRTC(int index, int value);
extern BYTE ReadMdaCRTC(int index);
extern void WriteMdaCRTC(int index, int value);
extern int GetByte(WORD sel, DWORD offset);
extern DWORD GetDWORD(WORD sel, DWORD offset);
extern void SetByte(WORD sel, DWORD offset, BYTE value);
extern void memset_w(void *dest, WORD data, int size);
extern void memset_d(void *dest, DWORD data, int size);

extern BOOL AddrIsPresent(PTADDRDESC pAddr);
extern BYTE AddrGetByte(PTADDRDESC pAddr);
extern void AddrSetByte(PTADDRDESC pAddr, BYTE value);

// Command parser helpers:
extern BOOL Expression(DWORD *value, char *sExpr, char **psNext );
extern WORD evalSel;               // Selector result of the expression (optional)
extern int Evaluate( char *sExpr, char **psNext );
extern DWORD GetDec(char **psString);

extern int GetOnOff(char *args);

extern char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset);
extern void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType);
extern TSYMSOURCE *SymTabFindSource(TSYMTAB *pSymTab, WORD fileID);

extern BOOL SymbolName2Value(TSYMTAB *pSymTab, DWORD *pValue, char *name);
extern char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset);
extern char *SymAddress2Name(WORD wSel, DWORD dwOffset);
extern DWORD SymLinNum2Address(DWORD line);
extern TSYMFNLIN *SymAddress2FnLin(WORD wSel, DWORD dwOffset);
extern char *SymFnLin2Line(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress);
extern char *SymFnLin2LineExact(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress);
extern TSYMFNSCOPE *SymAddress2FnScope(WORD wSel, DWORD dwOffset);
extern char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset);

#endif //  _ICE_H_

