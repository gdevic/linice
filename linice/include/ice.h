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

        This header file contains global LinIce data structures

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

#ifdef SIM
#pragma warning( disable : 4133 )
#endif

#include "intel.h"                      // Include Intel x86 specific defines
#include "ice-limits.h"                 // Include our limits
#include "ice-symbols.h"                // Include symbol file defines
#include "ice-ioctl.h"                  // Include symbol table defines
#include "memaccess.h"                  // Include memory access header file
#include "lists.h"                      // Include lists support header file
#include "eval.h"                       // Include expression evaluator header file

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
    // Define the constant state of the machine

    BOOL fSmp;                          // Is this machine SMP enabled?
    BOOL fIoApic;                       // Are we using IO APIC?
    TIOAPIC0 IoApic0;                   // Initial content of the first IO APIC register
    TIOAPIC1 IoApic1;                   // Initial content of the second IO APIC register
    TIOAPICREDIR IoApicRedir[MAX_IOAPICREG]; // Initial content of IO APIC redirection registers

    // Define the pseudo-constant state -- that changes only if the user manually modifies the state

    WCHAR BreakKey;                     // Key combination for break
    char keyFn[4][12][MAX_STRING];      // Key assignments for function keys
                                        // Fn:0 Shift:1 Alt:2 Control:3
    BYTE col[6];                        // Color attributes (1-5)
#   define COL_NORMAL          1
#   define COL_BOLD            2
#   define COL_REVERSE         3
#   define COL_HELP            4
#   define COL_LINE            5

    ENUM eSrc;                          // 0=Source off; 1=Source on; 2=Mixed:
#   define SRC_OFF          0
#   define SRC_ON           1
#   define SRC_MIXED        2

    BOOL fCode;                         // Code - is SET CODE ON?
    BOOL fAltscr;                       // Is alternate screen enabled?
    BOOL fFaults;                       // Is break on faults enables?
    BOOL fI1Here;                       // Break on INT1
    BOOL fI1Kernel;                     // Break on INT1 only in kernel code
    BOOL fI3Here;                       // Break on INT3
    BOOL fI3Kernel;                     // Break on INT3 only in kernel code
    BOOL fLowercase;                    // Display lowercase
    BOOL fSymbols;                      //
    BOOL fFlash;                        // Restore screen during P and T commands
    BOOL fPause;                        // Pause after a screenful of scrolling info
    BOOL fOvertype;                     // Cursor shape is overtype? (or insert)
    BOOL fCodeEdit;                     // Cursor is in the code window
    UINT nCodeEditY;                    // Effective code edit cursor Y offset within the code window
    TADDRDESC CodeEditAddress;          // Address selected by the code edit cursor
    UINT FrameX;                        // X-Window frame origin X coordinate
    UINT FrameY;                        // X-Window frame origin X coordinate
    UINT nFont;                         // Font number for hi-res display
    UINT nTabs;                         // Tabs value for source display

    BOOL fRedraw;                       // Request to redraw the screen after the current command

    // Define the Linice high-level data structures and pointers

    TSYMTAB *pSymTab;                   // Linked list of symbol tables
    TSYMTAB *pSymTabCur;                // Pointer to the current symbol table

    UINT nSymbolBufferSize;             // Symbol buffer size
    UINT nSymbolBufferAvail;            // Symbol buffer size available
    BYTE *hSymbolBufferHeap;            // Handle to symbol buffer memory pool heap

    UINT nHistorySize;                  // History buffer size
    BYTE *hHistoryBufferHeap;           // Handle to a history buffer memory pool heap

    UINT nXDrawSize;                    // DrawSize parameter
    BYTE *pXDrawBuffer;                 // Backstore buffer pointer, if XInitPacket accepted
    BYTE *pXFrameBuffer;                // Mapped X frame buffer

    UINT nVars;                         // Number of user variables
    UINT nMacros;                       // Number of macros
    BYTE *hHeap;                        // Handle to internal memory pool heap

    TLIST Watch;                        // Watch window data list
    TLIST Local;                        // Locals window data list
    TLIST Stack;                        // Stack window data list

    UINT nDumpSize;                     // Last Dx dump value size
    TADDRDESC dataAddr;                 // Data - current display address

    DWORD error;                        // Error code
    DWORD memaccess;                    // Last memaccess code, or memaccess error code
    BOOL fStamp;                        // Last memory stamp (checksum) result

    // Define the particular state of the machine at the time of break

    UINT nScheduleKbdBreakTimeout;      // Schedule break into the ICE within that many time slices
    BOOL fRunningIce;                   // Is ICE running?
    BOOL fDelayedArm;                   // Delayed breakpoint arm is requested
    BOOL fTrace;                        // Trace command in progress
    BOOL fSrcTrace;                     // Source is on and we issued a trace command (T)
    UINT nTraceCount;                   // Single trace instr. count to go
    BOOL fStep;                         // Step command in progress
    BOOL fStepRet;                      // Single step until RET instruction (P RET)
    BOOL fSrcStep;                      // Source is on and we issued a step command (P)

    // Define the current context evaluated at the time of break

    TDescriptor idt;                    // Linux idt descriptor
    TDescriptor gdt;                    // Linux gdt descriptor
    PTREGS r;                           // pointer to live registers
    TREGS  r_prev;                      // previous registers (for color coding of changes)
    TSysreg sysReg;                     // System registers

    UINT cpu;                           // CPU number that the debugger uses
    UINT nInterrupt;                    // Interrupt that occurred
    int bpIndex;                        // Index of the breakpoint that hit (default -1)

    TSYMFNSCOPE *pFnScope;              // Pointer to a current function scope descriptor
    TSYMFNLIN *pFnLin;                  // Pointer to a current function line descriptor
    TSYMSOURCE *pSource;                // Current source file in the code window
    char *pSrcEipLine;                  // Pointer to the current source line
    TADDRDESC codeTopAddr;              // Address of the top machine code instr. in the code window
    TADDRDESC codeBottomAddr;           // Address of the last line
    WORD codeFileTopLine;               // Line number of the top of code window source file
    WORD codeFileEipLine;               // Line number of the current EIP in the windows source file
    WORD codeFileXoffset;               // X offset of the source code text

    // Statistics and timing variables

    UINT nIntsPass[0x40];               // Number of interrupts received
    UINT nIntsIce[0x40];                // While debugger run

    // Timers - decremented by the timer interrupt down to zero
    //  0 - serial polling
    //  1 - cursor carret blink
    UINT timer[2];

} TDEB, *PTDEB;

extern TDEB deb;                        // Global data structure

/////////////////////////////////////////////////////////////////
// SCREEN WINDOW STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    // These are set by the user
    BOOL fVisible;                      // Whether a frame is visible or not
    UINT nLines;                        // How many lines a frame occupies

    // The following are calculated on a fly
    UINT Top;                           // Top coordinate holding the header line
    UINT Bottom;                        // Bottom coordinate (inclusive)

} TFRAME, *PTFRAME;

typedef struct
{
    TFRAME r;                           // Register window frame
    TFRAME l;                           // Locals window frame
    TFRAME w;                           // Watch window frame
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
    UINT nLen;                          // Length of the command string in characters
    int subClass;                       // Optional command argument (subclass)
    TFNCOMMAND pfnCommand;              // Pointer to a function for the command
    char *sSyntax;                      // Syntax string
    char *sExample;                     // Example string
    UINT iHelp;                         // Index to description string

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

extern WCHAR GetKey( BOOL fBlock );     // Read a key code from the input queue

extern void PutKey( WCHAR Key );

extern int dprint( char *format, ... );
extern BOOL dprinth( int nLineCount, char *format, ... );
extern int PrintLine(char *format,...);
extern void dputc(UCHAR c);

extern void RegDraw(BOOL fForce);
extern void LocalsDraw(BOOL fForce);
extern void WatchDraw(BOOL fForce);
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
extern BYTE ReadSR(int index);
extern void WriteSR(int index, int value);
extern BYTE ReadMdaCRTC(int index);
extern void WriteMdaCRTC(int index, int value);
extern int GetByte(WORD sel, DWORD offset);
extern DWORD GetDWORD(WORD sel, DWORD offset);
extern DWORD SetByte(WORD sel, DWORD offset, BYTE value);
extern void memset_w(void *dest, WORD data, int size);
extern void memset_d(void *dest, DWORD data, int size);
extern WORD getTR(void);
extern WORD GetKernelCS(void);
extern WORD GetKernelDS(void);
extern char *mallocHeap(BYTE *pHeap, UINT size);
extern void freeHeap(BYTE *pHeap, void *mPtr );

extern DWORD SelLAR(WORD Sel);
extern BOOL AddrIsPresent(PTADDRDESC pAddr);
extern BYTE AddrGetByte(PTADDRDESC pAddr);
extern DWORD AddrGetDword(PTADDRDESC pAddr);
extern DWORD AddrSetByte(PTADDRDESC pAddr, BYTE value, BOOL fForce);
extern BOOL VerifyRange(PTADDRDESC pAddr, DWORD dwSize);
extern BOOL VerifySelector(WORD Sel);

// Command parser helpers:
extern BOOL Expression(DWORD *value, char *sExpr, char **psNext );
extern WORD evalSel;               // Selector result of the expression (optional)
extern DWORD GetDec(char **psString);

extern int GetOnOff(char *args);

extern void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType);
extern TSYMSOURCE *SymTabFindSource(TSYMTAB *pSymTab, WORD fileID);
extern TSYMTYPEDEF *SymTabFindTypedef(TSYMTAB *pSymTab, WORD fileID);

extern DWORD SymLinNum2Address(DWORD line);
extern TSYMFNLIN *SymAddress2FnLin(WORD wSel, DWORD dwOffset);
extern char *SymFnLin2Line(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress);
extern char *SymFnLin2LineExact(WORD *pLineNumber, TSYMFNLIN *pFnLin, DWORD dwAddress);
extern TSYMFNSCOPE *SymAddress2FnScope(WORD wSel, DWORD dwOffset);

extern void SetSymbolContext(WORD wSel, DWORD dwOffset);

#endif //  _ICE_H_

