/******************************************************************************
*                                                                             *
*   Module:     ice.h                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/27/2000                                                    *
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

#include "intel.h"  

/******************************************************************************
*                                                                             *
*   Linux kernel Extern functions                                             *
*                                                                             *
******************************************************************************/

extern int printk(char *, ...);

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/
             
#define SEL_ICE_CS      0x10
#define SEL_ICE_DS      0x18
#define SEL_ICE_SS      0x18
#define SEL_ICE_ES      0x18
#define SEL_ICE_FS      0   
#define SEL_ICE_GS      0   

/////////////////////////////////////////////////////////////////
// Define structure that holds debugee state after an interrupt
/////////////////////////////////////////////////////////////////

typedef struct
{
    DWORD       pmES;
    DWORD       pmGS;
    DWORD       pmFS;
    DWORD       pmDS;
    DWORD       edi;
    DWORD       esi;
    DWORD       ebp;
    DWORD       temp;
    DWORD       ebx;
    DWORD       edx;
    DWORD       ecx;
    DWORD       eax;
    DWORD       ErrorCode;
    DWORD       eip;
    DWORD       pmCS;
    DWORD       eflags;
    DWORD       esp;
    DWORD       SS;
    DWORD       vmES;
    DWORD       vmDS;
    DWORD       vmFS;
    DWORD       vmGS;
} TRegs;


//--------------------------------------------------------
// Modes of dumping data
//--------------------------------------------------------

typedef enum tagDDUMP
{
    DD_BYTE,
    DD_WORD,
    DD_DWORD,
    DD_SHORTF,
    DD_LONGF,
    DD_TERAF

} TDDUMP;

typedef enum tagDCODE
{
    DC_ASM,
    DC_SOURCE,
    DC_MIXED

} TDCODE;


// Screen window structure

typedef struct
{
    // These are kept as set by the user
    BOOL        fVisible;               // Whether a window is visible or not
    DWORD       nLines;                 // How many lines a window occupies

    // The following are calculated on a fly
    DWORD       yTop;                   // Top coordinate holding the header line
    DWORD       yBottom;                // Bottom coordinate (inclusive)
} TWnd;

/////////////////////////////////////////////////////////////////
// THE MAIN DEBUGGER STRUCTURE
/////////////////////////////////////////////////////////////////

typedef struct
{
    TRegs       *r;                     // Pointer to a register structure
    TRegs       rPrev;                  // Previous registers (so we know to color code changed)

    TSS_Struct  *pTss;                  // Pointer to a TSS structure

    TDescriptor idt;                    // interrupt descriptor table
    TIDT_Gate   *pIdt;                  // Pointer to actual IDT table (stays constant)

    TDescriptor gdt;                    // global descriptor table
    TGDT_Gate   *pGdt;                  // Pointer to actual GDT table

    DWORD       nInterrupt;             // Current interrupt number

    BOOL        fRunningIce;            // Are we running Ice or debugee?

    BOOL        fInt1Here;              // Break on INT1
    BOOL        fInt3Here;              // Break on INT3
    BOOL        fSetCode;               // "SET CODE ON"

    TWnd        wr;                     // Register window record
    TWnd        wd;                     // Data window record
    TWnd        wc;                     // Code window record
    TWnd        wcmd;                   // Command window record
    DWORD       nLines;                 // Total number of lines on a display

    TDDUMP      dumpMode;               // Mode of dumping data
    WORD        dumpSel;                // Selector to use to dump data
    DWORD       dumpOffset;             // Offset of the data to dump

    TDCODE      codeMode;               // Mode of disassembling
    WORD        codeSel;                // Selector to use to disassemble
    DWORD       codeOffset;             // Offset to use to disassemble

} TDeb;

extern TDeb deb;                        // Debugee state structure


/******************************************************************************
*                                                                             *
*   Extern functions                                                          *
*                                                                             *
******************************************************************************/

extern void HookDebuger(void);

extern void SaveIDT(void);
extern void RestoreIDT(void);

extern void EnterDebugger(void);
extern void Deb_Keyboard_Handler(void);

extern void VgaInit(void);
extern void vga_putchar(char);


//--------------------------------------------------------
// Main print function and its control codes
//--------------------------------------------------------

#define DP_SETWRITEATTR         0x10    // [1] Sets the const. writing attribute
#define DP_SETCURSOR            0x11    // [2] Sets the location
#define DP_SETLOCATTR           0x12    // [4] Sets an arbitrary attribute patch
#define DP_CLS                  0x13    // [0] Clears the screen
#define DP_SAVEBACKGROUND       0x14    // [0] Saves user background
#define DP_RESTOREBACKGROUND    0x15    // [0] Restores saved background
#define DP_SETSCROLLREGION      0x16
#define DP_SETLINES             0x17
#define DP_SCROLLUP             0x18
#define DP_SCROLLDOWN           0x19
#define DP_SAVEXY               0x1A
#define DP_RESTOREXY            0x1B

extern int dprint( char *format, ... );
#define dputc(ascii)    (pfnPutChar)(ascii)

typedef void (*TPUTCHAR)( char );

extern TPUTCHAR pfnPutChar;

extern int nCharsWritten;

//--------------------------------------------------------
// Define keyboard codes
//--------------------------------------------------------

extern BOOL CheckHotKey(void);
extern WORD GetKey( BOOL fBlock );

#define F1            0x80
#define F2            0x81
#define F3            0x82
#define F4            0x83
#define F5            0x84
#define F6            0x85
#define F7            0x86
#define F8            0x87
#define F9            0x88
#define F10           0x89
#define F11           0x8A
#define F12           0x8B

#define SF1           0x90
#define SF2           0x91
#define SF3           0x92
#define SF4           0x93
#define SF5           0x94
#define SF6           0x95
#define SF7           0x96
#define SF8           0x97
#define SF9           0x98
#define SF10          0x99
#define SF11          0x9A
#define SF12          0x9B

#define BACKSPACE     '\b'
#define TAB           '\t'
#define ENTER         '\n'
#define ESC           27
#define NUMLOCK       18
#define SCROLL        19
#define HOME          20
#define UP            21
#define PGUP          22
#define LEFT          23
#define RIGHT         24
#define END           25
#define DOWN          26
#define PGDN          28
#define INS           29
#define DEL           30

#define CTRL        0x100             // Add this to ASCII if Ctrl key is pressed
#define ALT         0x200             // Add this to ASCII if Alt key is pressed


/******************************************************************************
*
*   This structure is used to pass parameters and options to the
*   line disassembler.
*
******************************************************************************/
typedef struct
{
    DWORD dwFlags;              // Generic flags (described below)
    WORD wSel;                  // Selector to use to fetch code
    BYTE *bpTarget;             // Target pointer to disassemble
    BYTE *szDisasm;             // String where to put ascii result
    BYTE *pCode;                // Buffer where to store code bytes

    BYTE bAsciiLen;             // Length of the ascii result
    BYTE bInstrLen;             // Instruction lenght in bytes

    int nDisplacement;          // Scanner: possible constant displacement
    int nScanEnum;              // Scanner: specific flags SCAN_*

} TDisassembler;

// dwFlags contains a set of boolean flags with the following functionality

#define DIS_DATA32          0x0001  // Data size 16/32 bits (0/1)
#define   DIS_GETDATASIZE(flags) ((flags)&DIS_DATA32)
#define DIS_ADDRESS32       0x0002  // Address size 16/32 bits (0/1)
#define   DIS_GETADDRSIZE(flags) (((flags)&DIS_ADDRESS32)?1:0)

#define DIS_SEGOVERRIDE     0x0004  // Default segment has been overriden

#define DIS_REP             0x0100  // Return: REP prefix found (followed by..)
#define DIS_REPNE           0x0200  // Return: REPNE prefix found
#define   DIS_GETREPENUM(flags)  (((flags)>>8)&3)
#define DIS_ILLEGALOP       0x8000  // Return: illegal opcode



#endif //  _ICE_H_
