/******************************************************************************
*                                                                             *
*   Module:     debugger.c                                                    *
*                                                                             *
*   Date:       10/31/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains debugger main loop

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/31/00   Original                                             Goran Devic *
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

extern void GetCommand(int nLine, char *sCmdLine);

extern void PrintRegisters(void);
extern void PrintData(void);
extern void PrintCode(void);

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

BOOL RecalculateWindows();


BOOL Unsupported(char *args);
BOOL CmdEval(char *args);
BOOL CmdHelp(char *args);
BOOL CmdGdt(char *args);
BOOL CmdUnassemble(char *args);
BOOL CmdCpu(char *args);
BOOL CmdCls(char *args);
BOOL CmdWr(char *args);
BOOL CmdWd(char *args);
BOOL CmdWc(char *args);
BOOL CmdColor(char *args);


static char sCmd[160];


static char *sSyntaxError = "Syntax error";


TCommand Cmd[MAX_COMMAND] = {
{    ".",        1, Unsupported,    "Locate current instruction", "ex: .",  0 },
{    "?",        1, CmdEval,        "? expression", "ex: ? ax << 1",   0 },
{    "A",        1, Unsupported,    "A [Address]", "ex: A CS:1236",    0 },
{    "ADDR",     4, Unsupported,    "ADDR [context-handle | task | *]", "ex: ADDR 80FD602C",   0 },
{    "ALTKEY",   6, Unsupported,    "ALTKEY [ALT letter | CTRL letter]", "ex: ALTKEY ALT D",   0 },
{    "ALTSCR",   6, Unsupported,    "ALTSCR [MONO | VGA | OFF]", "ex: ALTSCR MONO",    0 },
{    "BC",       2, Unsupported,    "BC list | *", "ex: BC *", 0 },
{    "BD",       2, Unsupported,    "BD list | *", "ex: BD 1,3,4", 0 },
{    "BE",       2, Unsupported,    "BE list | *", "ex: BE 1,3,4", 0 },
{    "BH",       2, Unsupported,    "BH breakpoint history", "ex: BH", 0 },
{    "BL",       2, Unsupported,    "BL list current breakpoints", "ex: BL",   0 },
{    "BPE",      3, Unsupported,    "BPE breakpoint number", "ex: BPE 3",  0 },
{    "BPINT",    5, Unsupported,    "BPINT interrupt-number [IF expression] [DO bp-action]", "ex: BPINT 50",   0 },
{    "BPIO",     4, Unsupported,    "BPIO [-h] port [R|W|RW] [debug register] [IF expression] [DO bp-action]", "ex: BPIO 3DA W",   0 },
{    "BPM",      3, Unsupported,    "BPM[size] address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPM 1234 RW", 0 },
{    "BPMB",     4, Unsupported,    "BPMB address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMB 333 R",   0 },
{    "BPMD",     4, Unsupported,    "BPMD address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMD EDI W",   0 },
{    "BPMW",     4, Unsupported,    "BPMW address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMW ESP-6 W", 0 },
{    "BPRW",     4, Unsupported,    "BPRW module-name | code selector [R|W|RW|T|TW] [IF expression] [DO bp-action]", "ex: BPR ESI EDI+32 RW",  0 },
{    "BPT",      3, Unsupported,    "BPT breakpoint number", "ex: BPT 0",  0 },
{    "BPX",      3, Unsupported,    "BPX address [IF expression] [DO bp-action]", "ex: BPX 282FE0",    0 },
{    "BSTAT",    5, Unsupported,    "BSTAT [breakpoint #]", "ex: BSTAT 3", 0 },
{    "C",        1, Unsupported,    "C address1 L length address2", "ex: C 80000 L 40 EBX",    0 },
{    "CLS",      3, CmdCls,         "CLS clear window", "ex: CLS", 0 },
{    "CODE",     4, Unsupported,    "CODE [ON | OFF]", "ex: CODE OFF", 0 },
{    "COLOR",    5, CmdColor,       "COLOR normal bold reverse help line", "ex: COLOR 30 3E 1F 1E 34", 0 },
{    "CPU",      3, CmdCpu,         "CPU [-I]", "ex: CPU", 0 },
{    "D",        1, Unsupported,    "D [address [L length]]", "ex: D B0000",   0 },
{    "DATA",     4, Unsupported,    "DATA [window-number(0-3)]", "ex: DATA 2", 0 },
{    "DEVICE",   6, Unsupported,    "DEVICE [device-name | address]", "ex: DEVICE HCD0",   0 },
{    "DEX",      3, Unsupported,    "DEX [window-number(0-3)] [expression]", "ex: DEX 2 esp",  0 },
{    "DB",       2, Unsupported,    "DB [address [L length]]", "ex: DB ESI+EBX",   0 },
{    "DD",       2, Unsupported,    "DD [address [L length]]", "ex: DD EBX+133",   0 },
{    "DW",       2, Unsupported,    "DW [address [L length]]", "ex: DW EDI",   0 },
{    "DL",       2, Unsupported,    "DL [address [L length]]", "ex: DL EBX",   0 },
{    "DRIVER",   6, Unsupported,    "DRIVER [driver-name | address]", "ex: DRIVER ne2000.sys", 0 },
{    "DS",       2, Unsupported,    "DS [address [L length]]", "ex: DS EBX",   0 },
{    "DT",       2, Unsupported,    "DT [address [L length]]", "ex: DT EBX",   0 },
{    "E",        1, Unsupported,    "E [address] [data]", "ex: E 400000",  0 },
{    "EB",       2, Unsupported,    "EB [address] [data]", "ex: EB 324415",    0 },
{    "EC",       2, Unsupported,    "EC Enable/disable code window", "ex: EC", 0 },
{    "ED",       2, Unsupported,    "ED [address] [data]", "ex: ED 84",    0 },
{    "EL",       2, Unsupported,    "EL [address] [data]", "ex: EL DS:EBX",    0 },
{    "ES",       2, Unsupported,    "ES [address] [data]", "ex: ES DS:EDI",    0 },
{    "ET",       2, Unsupported,    "ET [address] [data]", "ex: ET DS:EBX",    0 },
{    "EW",       2, Unsupported,    "EW [address] [data]", "ex: EW ESP-8", 0 },
{    "EXP",      3, Unsupported,    "EXP [partial-name*]", "ex: EXP GLOB*",    0 },
{    "CSIP",     4, Unsupported,    "CSIP [OFF | [NOT] address address | [NOT] module-name]", "ex: CSIP NOT CS:201000 CS:205fff",  0 },
{    "F",        1, Unsupported,    "F address L length data-string", "ex: F EBX L 50 'ABCD'", 0 },
{    "FAULTS",   6, Unsupported,    "FAULTS [ON | OFF]", "ex: FAULTS ON",  0 },
{    "FILE",     4, Unsupported,    "FILE [file-name | *]", "ex: FILE main.c", 0 },
{    "FKEY",     4, Unsupported,    "FKEY [function-key string]", "ex: FKEY F1 DD ESP; G @ESP",    0 },
{    "FLASH",    5, Unsupported,    "FLASH [ON | OFF]", "ex: FLASH ON",    0 },
{    "FOBJ",     4, Unsupported,    "FOBJ pfile_object", "ex: FOBJ EAX",   0 },
{    "FORMAT",   6, Unsupported,    "FORMAT Change format of data window", "ex: FORMAT",   0 },
{    "G",        1, Unsupported,    "G [=address] [address]", "ex: G 231456",  0 },
{    "GDT",      3, CmdGdt,         "GDT [selector | GDT base-address]", "ex: GDT 28", 0 },
{    "GENINT",   6, Unsupported,    "GENINT [NMI | INT1 | INT3 | int-number]", "ex: GENINT 2", 0 },
{    "H",        1, CmdHelp,        "H or Help [command]", "ex: H R",  0 },
{    "HBOOT",    5, Unsupported,    "HBOOT System boot (total reset)", "ex: HBOOT",    0 },
{    "HEAP",     4, Unsupported,    "HEAP [-l] [FREE | mod | sel]", "ex: HEAP GDI",    0 },
{    "HERE",     4, Unsupported,    "HERE Got to current cursor line", "ex: HERE", 0 },
{    "I",        1, Unsupported,    "I port", "ex: I 21",  0 },
{    "I1HERE",   6, Unsupported,    "I1HERE [ON | OFF]", "ex: I1HERE ON",  0 },
{    "I3HERE",   6, Unsupported,    "I3HERE [ON | OFF]", "ex: I3HERE ON",  0 },
{    "IB",       2, Unsupported,    "IB port", "ex: IB 3DA",   0 },
{    "IDT",      3, Unsupported,    "IDT [int-number | IDT base-address]", "ex: IDT 21",   0 },
{    "IW",       2, Unsupported,    "IW port", "ex: IW DX",    0 },
{    "ID",       2, Unsupported,    "ID port", "ex: ID DX",    0 },
{    "LDT",      3, Unsupported,    "LDT [selector | LDT table selector]", "ex: LDT 45",   0 },
{    "LINES",    5, Unsupported,    "LINES [25 | 43 | 50 | 60]", "ex: LINES 43",   0 },
{    "LOCALS",   6, Unsupported,    "LOCALS", "ex: LOCALS",    0 },
{    "M",        1, Unsupported,    "M address1 L length address2", "ex: M 4000 L 80 8000",    0 },
{    "MACRO",    5, Unsupported,    "MACRO [macro-name] | [[*] | [= \"macro-body\"]]", "ex: MACRO Oops = \"i3here off; genint 3;\"",   0 },
{    "MOD",      3, Unsupported,    "MOD [-u | -s]|[partial-name*]", "ex: MOD",    0 },
{    "O",        1, Unsupported,    "O port value", "ex: O 21 FF", 0 },
{    "OB",       2, Unsupported,    "OB port value", "ex: OB 21 FF",   0 },
{    "OBJDIR",   6, Unsupported,    "OBJDIR [object-directory]", "ex: OBJDIR Driver", 0 },
{    "OW",       2, Unsupported,    "OW port value", "ex: OW DX AX",   0 },
{    "OD",       2, Unsupported,    "OD port value", "ex: OD DX EAX",  0 },
{    "P",        1, Unsupported,    "P [RET]", "ex: P",    0 },
{    "PAGE",     4, Unsupported,    "PAGE [address [L length]]", "ex: PAGE DS:0 L 20", 0 },
{    "PAGEIN",   6, Unsupported,    "PAGEIN address", "ex: PAGEIN 401000", 0 },
{    "PAUSE",    5, Unsupported,    "PAUSE [ON | OFF]", "ex: PAUSE OFF",   0 },
{    "PCI",      3, Unsupported,    "PCI [-raw] [bus device function]", "ex: PCI", 0 },
{    "PEEK",     4, Unsupported,    "PEEK[size] address", "ex: PEEKD F8000000",    0 },
{    "PHYS",     4, Unsupported,    "PHYS physical-address", "ex: PHYS A0000", 0 },
{    "POKE",     4, Unsupported,    "POKE[size] address value", "ex: POKED F8000000 12345678", 0 },
{    "PRN",      3, Unsupported,    "PRN [LPTx | COMx]", "ex: PRN LPT1",   0 },
{    "PROC",     4, Unsupported,    "PROC [-xo] [task-name]", "ex: PROC -x Explorer",  0 },
{    "QUERY",    5, Unsupported,    "QUERY [[-x] address] [process-type]", "ex: QUERY PROGMAN",    0 },
{    "R",        1, Unsupported,    "R [-d | register-name | register-name [=] value]", "ex: R EAX=50",    0 },
{    "RS",       2, Unsupported,    "RS Restore program screen", "ex: RS", 0 },
{    "S",        1, Unsupported,    "S [-cu] address L length data-string", "ex: S 0 L ffffff 'Help',0D,0A",   0 },
{    "SERIAL",   6, Unsupported,    "SERIAL [ON|VT100 [com-port] [baud-rate] | OFF]", "ex: SERIAL ON 2 19200", 0 },
{    "SET",      3, Unsupported,    "SET [setvariable] [ON | OFF] [value]", "ex: SET FAULTS ON",   0 },
{    "SHOW",     4, Unsupported,    "SHOW [B | start] [L length]", "ex: SHOW 100", 0 },
{    "SRC",      3, Unsupported,    "SRC Toggle between source, mixed & code", "ex: SRC",  0 },
{    "SS",       2, Unsupported,    "SS [line-number] ['search-string']", "ex: SS 40 'if (i==3)'", 0 },
{    "STACK",    5, Unsupported,    "STACK [-v | -r][task-name | thread-type | SS:EBP]", "ex: STACK",  0 },
{    "SYM",      3, Unsupported,    "SYM [partial-name* | symbol-name]", "ex: SYM hDC*",   0 },
{    "T",        1, Unsupported,    "T [=address] [count]", "ex: T",   0 },
{    "THREAD",   6, Unsupported,    "THREAD [TCB | ID | task-name]", "ex: THREAD", 0 },
{    "TRACE",    5, Unsupported,    "TRACE [B | OFF | start]", "ex: TRACE 50", 0 },
{    "TABLE",    5, Unsupported,    "TABLE [[R] table-name | AUTOON | AUTOOFF]", "ex: TABLE test", 0 },
{    "TABS",     4, Unsupported,    "TABS [1 - 8]", "ex: TABS 4",  0 },
{    "TSS",      3, Unsupported,    "TSS [TSS selector]", "ex: TSS",   0 },
{    "TYPES",    5, Unsupported,    "TYPES [type-name]", "ex: TYPE DWORD", 0 },
{    "U",        1, CmdUnassemble,  "U [address [L length]]", "ex: U EIP-10",  0 },
{    "VER",      3, Unsupported,    "VER Display LinIce version", "ex: VER",   0 },
{    "WATCH",    5, Unsupported,    "WATCH address", "ex: WATCH VariableName", 0 },
{    "WC",       2, CmdWc,          "WC [window-size]", "ex: WC 8",    0 },
{    "WD",       2, CmdWd,          "WD [window-size]", "ex: WD 4",    0 },
{    "WF",       2, Unsupported,    "WF [-D] [B | W | D | F | P | *]", "ex: WF",   0 },
{    "WIDTH",    5, Unsupported,    "WIDTH [80-160]", "ex: WIDTH 100", 0 },
{    "WL",       2, Unsupported,    "WL [window-size]", "ex: WL 8",    0 },
{    "WHAT",     4, Unsupported,    "WHAT expression", "ex: WHAT system",  0 },
{    "WR",       2, CmdWr,          "WR Toggle register window", "ex: WR", 0 },
{    "WS",       2, Unsupported,    "WS [window-size]", "ex: WS 8",    0 },
{    "WW",       2, Unsupported,    "WW Toggle watch window", "ex: WW",    0 },
{    "WX",       2, Unsupported,    "WX [D | F | *]", "WX 8",  0 },
{    "X",        1, Unsupported,    "X Return to host debugger or program", "ex: X",   0 },
{    "XFRAME",   6, Unsupported,    "XFRAME [frame address]", "ex: XFRAME EBP",    0 },
{    "ZAP",      3, Unsupported,    "ZAP Zap embeded INT1 or INT3", "ex: ZAP", 0 }
};


char *sHelp[] = {
   " SETTING BREAK POINTS",
   "BPM    - Breakpoint on memory access",
   "BPMB   - Breakpoint on memory access, byte size",
   "BPMW   - Breakpoint on memory access, word size",
   "BPMD   - Breakpoint on memory access, double word size",
   "BPR    - Breakpoint on memory range",
   "BPIO   - Breakpoint on I/O port access",
   "BPINT  - Breakpoint on interrupt",
   "BPX    - Breakpoint on execution",
   "BMSG   - Breakpoint on Windows message",
   "BSTAT  - Breakpoint Statistics",
   "CSIP   - Set CS:EIP range qualifier",
   " MANIPULATING BREAK POINTS",
   "BPE    - Edit breakpoint",
   "BPT    - Use breakpoint as a template",
   "BL     - List current breakpoints",
   "BC     - Clear breakpoint",
   "BD     - Disable breakpoint",
   "BE     - Enable breakpoint",
   "BH     - Breakpoint history",
   " DISPLAY/CHANGE MEMORY",
   "R      - Display/change register contents",
   "U      - Un-assembles instructions",
   "D      - Display memory",
   "DB     - Display memory, byte size",
   "DW     - Display memory, word size",
   "DD     - Display memory, double word size",
   "DS     - Display memory, short real size",
   "DL     - Display memory, long real size",
   "DT     - Display memory, 10-byte real size",
   "E      - Edit memory",
   "EB     - Edit memory, byte size",
   "EW     - Edit memory, word size",
   "ED     - Edit memory, double word size",
   "ES     - Edit memory, short real size",
   "EL     - Edit memory, long real size",
   "ET     - Edit memory, 10-byte real size",
   "PEEK   - Read from physical address",
   "POKE   - Write to physical address",
   "PAGEIN - Load a page into physical memory (note: not always safe)",
   "H      - Help on the specified function",
   "?      - Evaluate expression",
   "VER    - SoftICE version",
   "WATCH  - Add watch",
   "FORMAT - Change format of data window",
   "DATA   - Change data window",
   " DISPLAY SYSTEM INFORMATION",
   "GDT    - Display global descriptor table",
   "LDT    - Display local descriptor table",
   "IDT    - Display interrupt descriptor Table",
   "TSS    - Display task state segment",
   "CPU    - Display cpu register information",
   "PCI    - Display PCI device information",
   "MOD    - Display windows module list",
   "HEAP   - Display windows global heap",
   "LHEAP  - Display windows local heap",
   "VXD    - Display windows VxD map",
   "TASK   - Display windows task list",
   "VCALL  - Display VxD calls",
   "WMSG   - Display windows messages",
   "PAGE   - Display page table information",
   "PHYS   - Display all virtual addresses for physical address",
   "STACK  - Display call stack",
   "XFRAME - Display active exception frames",
   "MAPV86 - Display v86 memory map",
   "HWND   - Display window handle information",
   "CLASS  - Display window class information",
   "VM     - Display virtual machine information",
   "THREAD - Display thread information",
   "ADDR   - Display/change address Contexts",
   "MAP32  - Display 32 bit section map",
   "PROC   - Display process information",
   "QUERY  - Display a processes virtual address space map",
   "WHAT   - Identify the type of an expression",
   "OBJDIR - Display info about an object directory",
   "DEVICE - Display info about a device",
   "DRIVER - Display info about a driver",
   "FOBJ   - Display info about a file object",
   "IRP    - Display info about a IRP",
   " I/O PORT COMMANDS",
   "I      - Input data from I/O port",
   "IB     - Input data from I/O port, byte size",
   "IW     - Input data from I/O port, word size",
   "ID     - Input data from I/O port, double word size",
   "O      - Output data to I/O port",
   "OB     - Output data to I/O port, byte size",
   "OW     - Output data to I/O port, word size",
   "OD     - Output data to I/O port, double word size",
   " FLOW CONTROL COMMANDS",
   "X      - Return to host debugger or program",
   "G      - Go to address",
   "T      - Single step one instruction",
   "P      - Step skipping calls, Int, etc.",
   "HERE   - Go to current cursor line",
   "EXIT   - Force an exit to current DOS/Windows program",
   "GENINT - Generate an interrupt",
   "HBOOT  - System boot (total reset)",
   " MODE CONTROL",
   "I1HERE - Direct INT1 to SoftICE",
   "I3HERE - Direct INT3 to SoftICE",
   "ZAP    - Zap embedded INT1 or INT3",
   "FAULTS - Enable/disable SoftICE fault trapping",
   "SET    - Change an internal variable",
   " CUSTOMIZATION COMMANDS",
   "PAUSE  - Controls display scroll mode",
   "ALTKEY - Set key sequence to invoke window",
   "FKEY   - Display/set function keys",
   "DEX    - Display/assign window data expressions",
   "CODE   - Display instruction bytes in code window",
   "COLOR  - Display/set screen colors",
   "ANSWER - Auto-answer and redirect console to modem",
   "DIAL   - Redirect console to modem",
   "SERIAL - Redirect console",
   "TABS   - Set/display tab settings",
   "LINES  - Set/display number of lines on screen",
   "WIDTH  - Set/display number of columns on screen",
   "PRN    - Set printer output port",
   "PRINT-SCREEN key - Dump screen to printer",
   "MACRO  - Define a named macro command",
   " UTILITY COMMANDS",
   "A      - Assemble code",
   "S      - Search for data",
   "F      - Fill memory with data",
   "M      - Move data",
   "C      - Compare two data blocks",
   " LINE EDITOR KEY USAGE",
   "      - Recall previous command line",
   "      - Recall next command line",
   "      - Move cursor right",
   "      - Move cursor left",
   "BKSP   - Back over last character",
   "HOME   - Start of line",
   "END    - End of line",
   "INS    - Toggle insert mode",
   "DEL    - Delete character",
   "ESC    - Cancel current command",
   " SCROLLING KEY USAGE",
   "PageUp      - Display previous page of display history",
   "PageDn      - Display next page of display history",
   "Alt-",
   "       - Scroll data window down one line",
   "Alt-",
   "       - Scroll data window up one line",
   "Alt-PageUp  - Scroll data window down one page",
   "Alt-PageDn  - Scroll data window up one page",
   "Ctrl-PageUp - Scroll code window down one page",
   "Ctrl-PageDn - Scroll code window up one page",
   "Ctrl-",
   "      - Scroll code window down one line",
   "Ctrl-",
   "      - Scroll code window up one line",
   " WINDOW COMMANDS",
   "WC     - Toggle code window",
   "WD     - Toggle data window",
   "WF     - Toggle floating point stack window",
   "WL     - Toggle locals window",
   "WR     - Toggle register window",
   "WS     - Toggle call stack window",
   "WW     - Toggle watch window",
   "WX     - Toggle Katmai XMM register window",
   "EC     - Enable/disable code window",
   ".      - Locate current instruction",
   " WINDOW CONTROL",
   "CLS    - Clear window",
   "RS     - Restore program screen",
   "ALTSCR - Change to alternate display",
   "FLASH  - Restore screen during P and T",
   " SYMBOL/SOURCE COMMANDS",
   "SYM    - Display symbols",
   "SYMLOC - Relocate symbol base",
   "EXP    - Display export symbols",
   "SRC    - Toggle between source, mixed & code",
   "TABLE  - Select/remove symbol table",
   "FILE   - Change/display current source file",
   "SS     - Search source module for string",
   "TYPES  - List all types, or display type definition",
   "LOCALS - Display locals currently in scope",
   " BACK TRACE COMMANDS",
   "SHOW   - Display from backtrace buffer",
   "TRACE  - Enter back trace simulation mode",
   "XT     - Step in trace simulation mode",
   "XP     - Program step in trace simulation mode",
   "XG     - Go to address in trace simulation mode",
   "XRSET  - Reset back trace history buffer",
   " SPECIAL OPERATORS",
   ".      - Preceding a decimal number specifies a line number",
   "$      - Preceding an address specifies SEGMENT addressing",
   "#      - Preceding an address specifies SELECTOR addressing",
   "@      - Preceding an address specifies indirection",
   NULL
};



/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL Unsupported(char *args)
{
    printline("UNSUPPORTED %s", args);

    return( TRUE );
}    


BOOL CmdHelp(char *args)
{
#if 0
    DWORD nLine = 0;

    Multiline(TRUE);

    while( sHelp[nLine] != NULL )
    {
        if( printline(sHelp[nLine])==TRUE)
            break;

        nLine++;
    }

    Multiline(FALSE);
#endif
    return( FALSE );
}    


BOOL CmdCls(char *args)
{
    DWORD nLines = deb.wcmd.nLines;

    dprint("%c%c%c", DP_SETCURSOR, 0, deb.wcmd.yTop);

    while( nLines-- )
        dputc('\n');

    ClearHistory();
    PrintCmd(NULL, 0);

    return( TRUE );
}    


BOOL CmdColor(char *args)
{
    DWORD value;
    BYTE bColors[5];
    int i;

    if( *args==0 )
    {
        printline("Colors are %02X %02X %02X %02X %02X",
                deb.colors[0], deb.colors[1], deb.colors[2], deb.colors[3], deb.colors[4] );
    }
    else
    if( *args=='*' )        // color *   will re-initialize colors
    {
        deb.colors[0] = 0x07;
        deb.colors[1] = 0x0B;
        deb.colors[2] = 0x71;
        deb.colors[3] = 0x30;
        deb.colors[4] = 0x02;

        RecalculateWindows();
    }
    else
    {
        nEvalDefaultBase = 16;

        for( i=0; i<5; i++)
        {
            value = nEvaluate(args, &args);
            if( value > 0xFF )
                goto ColSyntaxError;
            bColors[i] = value & 0xFF;
            if( i<4 && *args==0 )
                goto ColSyntaxError;
        }

        if( *args==0 )
        {
            memcpy(&deb.colors, &bColors, 5);
            RecalculateWindows();

            return( TRUE );
        }

ColSyntaxError:
        printline(sSyntaxError);
    }

    return( TRUE );
}    


BOOL CmdW(char *args, TWnd *pWnd)
{
    DWORD value;

    nEvalDefaultBase = 10;

    value = nEvaluate(args, &args);
    if( value > deb.nLines - 2 )
        value = deb.nLines - 2;

    if( value != 0 )
    {
        pWnd->nLines = value;
        pWnd->fVisible = TRUE;
    }
    else
    {
        pWnd->fVisible = !pWnd->fVisible;
    }

    while( RecalculateWindows()==FALSE )
        pWnd->nLines--;

    return( TRUE );
}    

BOOL CmdWr(char *args)
{
    return( CmdW(args, &deb.wr) );
}

BOOL CmdWd(char *args)
{
    return( CmdW(args, &deb.wd) );
}

BOOL CmdWc(char *args)
{
    return( CmdW(args, &deb.wc) );
}


BOOL CmdEval(char *args)
{
    DWORD value;

    nEvalDefaultBase = 16;

    value = nEvaluate(args, &args);

    printline("Hex=%08X  Dec=%08d", value, value);

    return( TRUE );
}


BOOL CmdCpu(char *args)
{
    Multiline(TRUE);

    printline("Processor Registers");
    printline("-------------------");
    printline("CS:EIP=%04X:%08X    SS:ESP=%04X:%08X", 
        deb.r->pmCS, deb.r->eip, deb.r->ss, deb.r->esp );
    printline("EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X",
        deb.r->eax, deb.r->ebx, deb.r->ecx, deb.r->edx );
    printline("ESI=%08X   EDI=%08X   EBP=%08X   EFL=%08X",
        deb.r->esi, deb.r->edi, deb.r->ebp, deb.r->eflags );
    printline("DS=%04X   ES=%04X   FS=%04X   GS=%04X", 
        deb.r->pmDS, deb.r->pmES, deb.r->pmFS, deb.r->pmGS );
    printline("CR0=%08X", deb.sysreg.cr0 );
    printline("CR2=%08X", deb.sysreg.cr2 );
    printline("CR3=%08X", deb.sysreg.cr3 );
    printline("CR4=%08X", deb.sysreg.cr4 );

    printline("DR0=%08X", deb.sysreg.dr0 );
    printline("DR1=%08X", deb.sysreg.dr1 );
    printline("DR2=%08X", deb.sysreg.dr2 );
    printline("DR3=%08X", deb.sysreg.dr3 );
    printline("DR6=%08X", deb.sysreg.dr6 );
    printline("DR7=%08X", deb.sysreg.dr7 );
    printline("EFL=%04X", deb.r->eflags );

    Multiline(FALSE);

    return( TRUE );
}    


BOOL CmdGdt(char *args)
{
    char *sType;
    TGDT_Gate *pGdt;
    int count;
    DWORD limit;
    BOOL fEsc = FALSE;

    // Set the command window header line

    dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSOR, 0, deb.wcmd.yTop - 1);
    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_LINE]);
    dprint("Sel---Type--------Base------Limit-----DPL--Attributes---------------------------\n");
    dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_NORMAL]);
    dputc(DP_RESTOREXY);

    Multiline(TRUE);

    printline("GDTbase=%08X  Limit=%04X", GET_DESC_BASE(&deb.gdt), deb.gdt.limit );

    pGdt = deb.pGdt + 1;

    for( count=1; count < (deb.gdt.limit + 1) / sizeof(TGDT_Gate); count++, pGdt++)
    {
        switch( pGdt->type )
        {
            case DESC_TYPE_LDT:     sType = "LDT       ";  break;
            case DESC_TYPE_TSS32A:
            case DESC_TYPE_TSS32B:  sType = "TSS32     ";  break;
            case DESC_TYPE_DATA:    sType = "Data32    ";  break;
            case DESC_TYPE_EXEC:    sType = "Code32    ";  break;
            default:                sType = "Reserved  ";
        }

        // Get the proper limit

        limit = GET_GDT_LIMIT(pGdt);
        if( pGdt->granularity == 1 )
            limit = limit << 12 | 0xFFF;

        // Print the descriptor value

        fEsc = printline("%04X  %s  %08X  %08X  %d    %s",
                count * sizeof(TGDT_Gate), 
                sType,
                GET_GDT_BASE(pGdt),
                limit,
                pGdt->dpl,
                pGdt->present? "P ":"NP");

        if( fEsc==TRUE )
            break;
    }

    Multiline(FALSE);

    return( TRUE );
}    


/******************************************************************************
*                                                                             *
*   BOOL RecalculateWindows(void)                                             *
*                                                                             *
*******************************************************************************
*
*   Recalculates window sizes and returns the status of the current size
*   requests.
*
*   Returns:
*       TRUE - all requested window 'nLines' are correct
*       FALSE - can not fit windows using those 'nLines' - must retry!
*
******************************************************************************/
BOOL RecalculateWindows()
{
    DWORD nLine;
    
    nLine = 0;

    if( deb.wr.fVisible )
    {
        deb.wr.yTop = 0;
        deb.wr.yBottom = 2;
        nLine = 3;                      // First available line
    }

    if( deb.wd.fVisible )
    {
        nLine++;                        // Skip the header line
        deb.wd.yTop = nLine;
        nLine += deb.wd.nLines;
        deb.wd.yBottom = nLine - 1;     // Last line

        if( nLine >= deb.nLines - 2 )
            return( FALSE );
    }

    if( deb.wc.fVisible )
    {
        nLine++;                        // Skip the header line
        deb.wc.yTop = nLine;
        nLine += deb.wc.nLines;
        deb.wc.yBottom = nLine - 1;     // Last line

        if( nLine >= deb.nLines - 2 )
            return( FALSE );
    }
    //  et cetera...

    // Command window is always visible

    nLine++;                            // Skip the header line
    deb.wcmd.yTop = nLine;
    deb.wcmd.yBottom = deb.nLines - 2;
    deb.wcmd.nLines = deb.wcmd.yBottom - deb.wcmd.yTop;

    if( nLine >= deb.nLines - 2 )
        return( FALSE );

    // Clear the screen and reset the cursor

    dputc(DP_CLS);

    // Set up the autoscroll region of the command buffer

    dprint("%c%c%c", DP_SETSCROLLREGION, deb.wcmd.yTop, deb.wcmd.yBottom );

    // Start painting display...

    if( deb.wr.fVisible )
    {
        PrintRegisters();        
    }

    if( deb.wd.fVisible )
    {
        PrintData();
    }

    if( deb.wc.fVisible )
    {
        PrintCode();
    }

    // Print the content of the command buffer into the command window
    
    PrintCmd(NULL, 0);

    return( TRUE );
}   
 

/******************************************************************************
*                                                                             *
*   void EnterDebugger(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   Debugger main loop
*
******************************************************************************/
void EnterDebugger(void)
{
    BOOL fContinue = TRUE;
    char *p;
    int i;

    dprint("%c", DP_SAVEBACKGROUND);

    // Recalculate window locations based on visibility and number of lines
    // and repaint all windows

    RecalculateWindows();

    // Read system register file

    GetSysreg(&deb.sysreg);

    deb.codeSel    = 0x18;
    deb.codeOffset = deb.r->eip;

    AddHistory("LinIce (C) 2000 by Goran Devic");
    AddHistory("------------------------------");

    printline("The process is %s pid %d", current->comm, current->pid);

    while( TRUE )
    {
        // Print the command window header line

        dprint("%c%c%c%c", DP_SAVEXY, DP_SETCURSOR, 0, deb.wcmd.yTop - 1);
        dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_LINE]);
        dprint("--kernel------------------------------------------------------------------------\n");
        dputc(DP_SETWRITEATTR);dputc(deb.colors[COL_NORMAL]);
        dputc(DP_RESTOREXY);


        GetCommand( deb.nLines - 2, sCmd );

        // Find the first non-space character

        p = sCmd + 1;
        while( (*p==' ') && (*p!=0) ) p++;

        // Got a character.. Search all known command keywords from back to
        // front in order to find a match

        for( i=MAX_COMMAND-1; i>=0; i--)
        {
            if( strnicmp(p, Cmd[i].sCmd, Cmd[i].nLen)==0 )
                break;
        }

        if( i>= 0 )
        {
            // Command found !!!!  Find the first non-space character of argument

            p += Cmd[i].nLen;
            while( (*p==' ') && (*p!=0) ) p++;

            // Call the command function handler

            fContinue = (Cmd[i].pfnCommand)( p );

            if( fContinue==FALSE )
            {
                return;
            }
        }
    }
}    

