/******************************************************************************
*                                                                             *
*   Module:     command.c                                                     *
*                                                                             *
*   Date:       09/11/00                                                      *
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

        This module contains the main debugger execution loop and definition
        of commands.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/00   Original                                             Goran Devic *
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

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static int iLast;                       // Last command entry index

BOOL Unsupported(char *args, int subClass);

extern BOOL cmdEvaluate(char *args, int subClass);      // eval.c
extern BOOL cmdAltkey  (char *args, int subClass);      // customization.c
extern BOOL cmdCode    (char *args, int subClass);      // customization.c
extern BOOL cmdSet     (char *args, int subClass);      // customization.c
extern BOOL cmdVar     (char *args, int subClass);      // customization.c
extern BOOL cmdMacro   (char *args, int subClass);      // customization.c
extern BOOL cmdLines   (char *args, int subClass);      // customization.c
extern BOOL cmdColor   (char *args, int subClass);      // customization.c
extern BOOL cmdSerial  (char *args, int subClass);      // customization.c
extern BOOL cmdXwin    (char *args, int subClass);      // customization.c
extern BOOL cmdPause   (char *args, int subClass);      // customization.c
extern BOOL cmdXit     (char *args, int subClass);      // flow.c
extern BOOL cmdGo      (char *args, int subClass);      // flow.c
extern BOOL cmdTrace   (char *args, int subClass);      // flow.c
extern BOOL cmdStep    (char *args, int subClass);      // flow.c
extern BOOL cmdZap     (char *args, int subClass);      // flow.c
extern BOOL cmdI1here  (char *args, int subClass);      // flow.c
extern BOOL cmdI3here  (char *args, int subClass);      // flow.c
extern BOOL cmdHboot   (char *args, int subClass);      // flow.c
extern BOOL cmdHalt    (char *args, int subClass);      // flow.c
extern BOOL cmdWd      (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWc      (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWr      (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdCls     (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdRs      (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdFlash   (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdTable   (char *args, int subClass);      // symbols.c
extern BOOL cmdSymbol  (char *args, int subClass);      // symbols.c
extern BOOL cmdReg     (char *args, int subClass);      // registers.c
extern BOOL cmdGdt     (char *args, int subClass);      // sysinfo.c
extern BOOL cmdLdt     (char *args, int subClass);      // sysinfo.c
extern BOOL cmdIdt     (char *args, int subClass);      // sysinfo.c
extern BOOL cmdCpu     (char *args, int subClass);      // sysinfo.c
extern BOOL cmdModule  (char *args, int subClass);      // sysinfo.c
extern BOOL cmdVer     (char *args, int subClass);      // sysinfo.c
extern BOOL cmdProc    (char *args, int subClass);      // sysinfo.c
extern BOOL cmdPage    (char *args, int subClass);      // page.c
extern BOOL cmdPhys    (char *args, int subClass);      // page.c
extern BOOL cmdPeek    (char *args, int subClass);      // page.c
extern BOOL cmdPoke    (char *args, int subClass);      // page.c
extern BOOL cmdPci     (char *args, int subClass);      // pci.c
extern BOOL cmdDdump   (char *args, int subClass);      // data.c
extern BOOL cmdUnasm   (char *args, int subClass);      // code.c
extern BOOL cmdOut     (char *args, int subClass);      // ioport.c
extern BOOL cmdIn      (char *args, int subClass);      // ioport.c
extern BOOL cmdFill    (char *args, int subClass);      // blockops.c
extern BOOL cmdSearch  (char *args, int subClass);      // blockops.c
extern BOOL cmdCompare (char *args, int subClass);      // blockops.c
extern BOOL cmdMove    (char *args, int subClass);      // blockops.c
extern BOOL cmdHelp    (char *args, int subClass);      // command.c

TCommand Cmd[] = {
{    ".",        1, 0, Unsupported,    "Locate current instruction", "ex: .",  0 },
{    "?",        1, 0, cmdEvaluate,    "? expression", "ex: ? ax << 1",   0 },
{    "A",        1, 0, Unsupported,    "Assemble [Address]", "ex: A CS:1236",    0 },
{    "ADDR",     4, 0, Unsupported,    "ADDR [context-handle | task | *]", "ex: ADDR 80FD602C",   0 },
{    "ALTKEY",   6, 0, cmdAltkey,      "ALTKEY [ALT letter | CTRL letter]", "ex: ALTKEY ALT D",   0 },
{    "ALTSCR",   6, 0, Unsupported,    "ALTSCR [MONO | VGA | OFF]", "ex: ALTSCR MONO",    0 },
{    "BC",       2, 0, Unsupported,    "BC list | *", "ex: BC *", 0 },
{    "BD",       2, 0, Unsupported,    "BD list | *", "ex: BD 1,3,4", 0 },
{    "BE",       2, 0, Unsupported,    "BE list | *", "ex: BE 1,3,4", 0 },
{    "BH",       2, 0, Unsupported,    "BH breakpoint history", "ex: BH", 0 },
{    "BL",       2, 0, Unsupported,    "BL list current breakpoints", "ex: BL",   0 },
{    "BPE",      3, 0, Unsupported,    "BPE breakpoint number", "ex: BPE 3",  0 },
{    "BPINT",    5, 0, Unsupported,    "BPINT interrupt-number [IF expression] [DO bp-action]", "ex: BPINT 50",   0 },
{    "BPIO",     4, 0, Unsupported,    "BPIO [-h] port [R|W|RW] [debug register] [IF expression] [DO bp-action]", "ex: BPIO 3DA W",   0 },
{    "BPM",      3, 0, Unsupported,    "BPM[size] address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPM 1234 RW", 0 },
{    "BPMB",     4, 0, Unsupported,    "BPMB address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMB 333 R",   0 },
{    "BPMD",     4, 0, Unsupported,    "BPMD address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMD EDI W",   0 },
{    "BPMW",     4, 0, Unsupported,    "BPMW address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]", "ex: BPMW ESP-6 W", 0 },
{    "BPRW",     4, 0, Unsupported,    "BPRW module-name | code selector [R|W|RW|T|TW] [IF expression] [DO bp-action]", "ex: BPR ESI EDI+32 RW",  0 },
{    "BPT",      3, 0, Unsupported,    "BPT breakpoint number", "ex: BPT 0",  0 },
{    "BPX",      3, 0, Unsupported,    "BPX address [IF expression] [DO bp-action]", "ex: BPX 282FE0",    0 },
{    "BSTAT",    5, 0, Unsupported,    "BSTAT [breakpoint #]", "ex: BSTAT 3", 0 },
{    "C",        1, 0, cmdCompare,     "Compare [-e] address1 L length address2", "ex: C 80000 L 40 EBX",    0 },
{    "CLS",      3, 0, cmdCls,         "CLS clear window", "ex: CLS", 0 },
{    "CODE",     4, 0, cmdCode,        "CODE [ON | OFF]", "ex: CODE OFF", 0 },
{    "COLOR",    5, 0, cmdColor,       "COLOR [normal bold reverse help line | - ]", "ex: COLOR 30 3E 1F 1E 34", 0 },
{    "CPU",      3, 0, cmdCpu,         "CPU [-I]", "ex: CPU", 0 },
{    "D",        1, 0, cmdDdump,       "D [address [L length]]", "ex: D B0000",   0 },
{    "DATA",     4, 0, Unsupported,    "DATA [window-number(0-3)]", "ex: DATA 2", 0 },
{    "DEVICE",   6, 0, Unsupported,    "DEVICE [device-name | address]", "ex: DEVICE HCD0",   0 },
{    "DEX",      3, 0, Unsupported,    "DEX [window-number(0-3)] [expression]", "ex: DEX 2 esp",  0 },
{    "DB",       2, 1, cmdDdump,       "DB [address [L length]]", "ex: DB ESI+EBX",   0 },
{    "DD",       2, 4, cmdDdump,       "DD [address [L length]]", "ex: DD EBX+133",   0 },
{    "DW",       2, 2, cmdDdump,       "DW [address [L length]]", "ex: DW EDI",   0 },
{    "DL",       2, 4, Unsupported,    "DL [address [L length]]", "ex: DL EBX",   0 },
{    "DRIVER",   6, 0, Unsupported,    "DRIVER [driver-name | address]", "ex: DRIVER ne2000.sys", 0 },
{    "DS",       2, 0, Unsupported,    "DS [address [L length]]", "ex: DS EBX",   0 },
{    "DT",       2, 0, Unsupported,    "DT [address [L length]]", "ex: DT EBX",   0 },
{    "E",        1, 0, Unsupported,    "Edit [address] [data]", "ex: E 400000",  0 },
{    "EB",       2, 1, Unsupported,    "EB [address] [data]", "ex: EB 324415",    0 },
{    "EC",       2, 0, Unsupported,    "EC Enable/disable code window", "ex: EC", 0 },
{    "ED",       2, 3, Unsupported,    "ED [address] [data]", "ex: ED 84",    0 },
{    "EL",       2, 0, Unsupported,    "EL [address] [data]", "ex: EL DS:EBX",    0 },
{    "ES",       2, 0, Unsupported,    "ES [address] [data]", "ex: ES DS:EDI",    0 },
{    "ET",       2, 0, Unsupported,    "ET [address] [data]", "ex: ET DS:EBX",    0 },
{    "EW",       2, 2, Unsupported,    "EW [address] [data]", "ex: EW ESP-8", 0 },
{    "EXP",      3, 0, Unsupported,    "EXP [partial-name*]", "ex: EXP GLOB*",    0 },
{    "CSIP",     4, 0, Unsupported,    "CSIP [OFF | [NOT] address address | [NOT] module-name]", "ex: CSIP NOT CS:201000 CS:205fff",  0 },
{    "F",        1, 0, cmdFill,        "Fill address L length data-string", "ex: F EBX L 50 'ABCD'", 0 },
{    "FAULTS",   6, 0, Unsupported,    "FAULTS [ON | OFF]", "ex: FAULTS ON",  0 },
{    "FILE",     4, 0, Unsupported,    "FILE [file-name | *]", "ex: FILE main.c", 0 },
{    "FKEY",     4, 0, Unsupported,    "FKEY [function-key string]", "ex: FKEY F1 DD ESP; G @ESP",    0 },
{    "FLASH",    5, 0, cmdFlash,       "FLASH [ON | OFF]", "ex: FLASH ON",    0 },
{    "FOBJ",     4, 0, Unsupported,    "FOBJ pfile_object", "ex: FOBJ EAX",   0 },
{    "FORMAT",   6, 0, Unsupported,    "FORMAT Change format of data window", "ex: FORMAT",   0 },
{    "G",        1, 0, cmdGo,          "G [=address] [address]", "ex: G 231456",  0 },
{    "GDT",      3, 0, cmdGdt,         "GDT [selector | GDT base-address]", "ex: GDT 28", 0 },
{    "GENINT",   6, 0, Unsupported,    "GENINT [NMI | INT1 | INT3 | int-number]", "ex: GENINT 2", 0 },
{    "H",        1, 0, cmdHelp,        "Help [command]", "ex: H R",  0 },
{    "HALT",     4, 0, cmdHalt,        "HALT System APM Off", "ex: HALT",    0 },
{    "HBOOT",    5, 0, cmdHboot,       "HBOOT System boot (total reset)", "ex: HBOOT",    0 },
{    "HEAP",     4, 0, Unsupported,    "HEAP [-l] [FREE | mod | sel]", "ex: HEAP GDI",    0 },
{    "HERE",     4, 0, Unsupported,    "HERE Got to current cursor line", "ex: HERE", 0 },
{    "I",        1, 1, cmdIn,          "I port", "ex: I 21",  0 },
{    "I1HERE",   6, 0, cmdI1here,      "I1HERE [ON | OFF | KERNEL]", "ex: I1HERE ON",  0 },
{    "I3HERE",   6, 0, cmdI3here,      "I3HERE [ON | OFF | KERNEL]", "ex: I3HERE ON",  0 },
{    "IB",       2, 1, cmdIn,          "IB port", "ex: IB 3DA",   0 },
{    "ID",       2, 4, cmdIn,          "ID port", "ex: ID DX",    0 },
{    "IW",       2, 2, cmdIn,          "IW port", "ex: IW DX",    0 },
{    "IDT",      3, 0, cmdIdt,         "IDT [int-number | IDT base-address]", "ex: IDT 21",   0 },
{    "LDT",      3, 0, cmdLdt,         "LDT [selector | LDT table selector]", "ex: LDT 45",   0 },
{    "LINES",    5, 0, cmdLines,       "LINES [25 | 43 | 50 | 60]", "ex: LINES 43",   0 },
{    "LOCALS",   6, 0, Unsupported,    "LOCALS", "ex: LOCALS",    0 },
{    "M",        1, 0, cmdMove,        "Move source-address L length dest-address", "ex: M 4000 L 80 8000",    0 },
{    "MACRO",    5, 0, cmdMacro,       "MACRO [macro-name] | [[*] | [= \"macro-body\"]]", "ex: MACRO Oops = \"i3here off; genint 3;\"",   0 },
{    "MODULE",   6, 0, cmdModule,      "MODULE [module-name | partial-name*]", "ex: MODULE",    0 },
{    "O",        1, 1, cmdOut,         "O port value", "ex: O 21 FF", 0 },
{    "OB",       2, 1, cmdOut,         "OB port value", "ex: OB 21 FF",   0 },
{    "OBJDIR",   6, 0, Unsupported,    "OBJDIR [object-directory]", "ex: OBJDIR Driver", 0 },
{    "OW",       2, 2, cmdOut,         "OW port value", "ex: OW DX AX",   0 },
{    "OD",       2, 4, cmdOut,         "OD port value", "ex: OD DX EAX",  0 },
{    "P",        1, 0, Unsupported,    "P [RET]", "ex: P",    0 },
{    "PAGE",     4, 0, cmdPage,        "PAGE [address [L length]]", "ex: PAGE DS:0 L 20", 0 },
{    "PAGEIN",   6, 0, Unsupported,    "PAGEIN address", "ex: PAGEIN 401000", 0 },
{    "PAUSE",    5, 0, cmdPause,       "PAUSE [ON | OFF]", "ex: PAUSE OFF",   0 },
{    "PCI",      3, 0, cmdPci,         "PCI [-raw] [-extended] [-terse] [-b] [-w] [-d] [bus device function]", "ex: PCI", 0 },
{    "PEEK",     4, 0, cmdPeek,        "PEEK[size] address", "ex: PEEK F8000000",    0 },
{    "PEEKB",    5, 0, cmdPeek,        "PEEK address", "ex: PEEKB F8000000",    0 },
{    "PEEKD",    5, 2, cmdPeek,        "PEEKD address", "ex: PEEKD F8000000",    0 },
{    "PEEKW",    5, 1, cmdPeek,        "PEEKW address", "ex: PEEKW F8000000",    0 },
{    "PHYS",     4, 0, cmdPhys,        "PHYS physical-address", "ex: PHYS A0000", 0 },
{    "POKE",     4, 0, cmdPoke,        "POKE[size] address value", "ex: POKE F8000000 AA", 0 },
{    "POKEB",    5, 0, cmdPoke,        "POKEB address value", "ex: POKEB F8000000 AA", 0 },
{    "POKED",    5, 2, cmdPoke,        "POKED address value", "ex: POKED F8000000 12345678", 0 },
{    "POKEW",    5, 1, cmdPoke,        "POKEW address value", "ex: POKEW F8000000 55AA", 0 },
{    "PRN",      3, 0, Unsupported,    "PRN [LPTx | COMx]", "ex: PRN LPT1",   0 },
{    "PROC",     4, 0, cmdProc,        "PROC [-xo] [task-name]", "ex: PROC -x Explorer",  0 },
{    "QUERY",    5, 0, Unsupported,    "QUERY [[-x] address] [process-type]", "ex: QUERY PROGMAN",    0 },
{    "R",        1, 0, cmdReg,         "R [-d | register-name | register-name [=] value]", "ex: R EAX=50",    0 },
{    "RS",       2, 0, cmdRs,          "RS Restore program screen", "ex: RS", 0 },
{    "S",        1, 0, cmdSearch,      "Search [-c] address L length data-string", "ex: S 0 L ffffff 'Help',0D,0A",   0 },
{    "SERIAL",   6, 0, cmdSerial,      "SERIAL [ON|VT100 [com-port] [baud-rate] | OFF]", "ex: SERIAL ON 2 19200", 0 },
{    "SET",      3, 0, cmdSet,         "SET [setvariable] [ON | OFF] [value]", "ex: SET FAULTS ON",   0 },
{    "SHOW",     4, 0, Unsupported,    "SHOW [B | start] [L length]", "ex: SHOW 100", 0 },
{    "SRC",      3, 0, Unsupported,    "SRC Toggle between source, mixed & code", "ex: SRC",  0 },
{    "SS",       2, 0, Unsupported,    "SS [line-number] ['search-string']", "ex: SS 40 'if (i==3)'", 0 },
{    "STACK",    5, 0, Unsupported,    "STACK [-v | -r][task-name | thread-type | SS:EBP]", "ex: STACK",  0 },
{    "SYM",      3, 0, cmdSymbol,      "SYM [partial-name* | symbol-name]", "ex: SYM hDC*",   0 },
{    "T",        1, 0, cmdTrace,       "Trace [count]", "ex: T",   0 },
{    "THREAD",   6, 0, Unsupported,    "THREAD [TCB | ID | task-name]", "ex: THREAD", 0 },
{    "TRACE",    5, 0, Unsupported,    "TRACE [B | OFF | start]", "ex: TRACE 50", 0 },
{    "TABLE",    5, 0, cmdTable,       "TABLE [[R] table-name | AUTOON | AUTOOFF]", "ex: TABLE test", 0 },
{    "TABS",     4, 0, Unsupported,    "TABS [1 - 8]", "ex: TABS 4",  0 },
{    "TSS",      3, 0, Unsupported,    "TSS [TSS selector]", "ex: TSS",   0 },
{    "TYPES",    5, 0, Unsupported,    "TYPES [type-name]", "ex: TYPE DWORD", 0 },
{    "U",        1, 0, cmdUnasm,       "Unassemble [address [L length]]", "ex: U EIP-10",  0 },
{    "VAR",      3, 0, cmdVar,         "VAR [variable-name] | [[*] | [= expression]]", "ex: VAR myvalue = eax+20",   0 },
{    "VER",      3, 0, cmdVer,         "VER Display LinIce version", "ex: VER",   0 },
{    "WATCH",    5, 0, Unsupported,    "WATCH address", "ex: WATCH VariableName", 0 },
{    "WC",       2, 0, cmdWc,          "WC [window-size]", "ex: WC 8",    0 },
{    "WD",       2, 0, cmdWd,          "WD [window-size]", "ex: WD 4",    0 },
{    "WF",       2, 0, Unsupported,    "WF [-D] [B | W | D | F | P | *]", "ex: WF",   0 },
{    "WIDTH",    5, 0, Unsupported,    "WIDTH [80-160]", "ex: WIDTH 100", 0 },
{    "WL",       2, 0, Unsupported,    "WL [window-size]", "ex: WL 8",    0 },
{    "WHAT",     4, 0, Unsupported,    "WHAT expression", "ex: WHAT system",  0 },
{    "WR",       2, 0, cmdWr,          "WR Toggle register window", "ex: WR", 0 },
{    "WS",       2, 0, Unsupported,    "WS [window-size]", "ex: WS 8",    0 },
{    "WW",       2, 0, Unsupported,    "WW Toggle watch window", "ex: WW",    0 },
{    "WX",       2, 0, Unsupported,    "WX [D | F | *]", "WX 8",  0 },
{    "X",        1, 0, cmdXit,         "X Return to host debugger or program", "ex: X",   0 },
{    "XFRAME",   6, 0, Unsupported,    "XFRAME [frame address]", "ex: XFRAME EBP",    0 },
{    "XWIN",     4, 0, cmdXwin,        "XWIN", "ex: XWIN",    0 },
{    "ZAP",      3, 0, cmdZap,         "ZAP Zap embeded INT1 or INT3", "ex: ZAP", 0 },
{    NULL,       0, 0, NULL,           NULL, 0 }
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
   "MODULE - Display kernel module list",
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
   "HALT   - System APM Off",
   "HBOOT  - System boot (total reset)",
   " MODE CONTROL",
   "I1HERE - Direct INT1 to SoftICE, globally or kernel only",
   "I3HERE - Direct INT3 to SoftICE, globally or kernel only",
   "ZAP    - Zap embedded INT1 or INT3",
   "FAULTS - Enable/disable SoftICE fault trapping",
   "SET    - Change an internal system variable",
   "VAR    - Change a user variable",
   " CUSTOMIZATION COMMANDS",
   "PAUSE  - Controls display scroll mode",
   "ALTKEY - Set key sequence to invoke window",
   "FKEY   - Display/set function keys",
   "DEX    - Display/assign window data expressions",
   "CODE   - Display instruction bytes in code window",
   "COLOR  - Display/set screen colors",
   "ANSWER - Auto-answer and redirect console to modem",
   "DIAL   - Redirect console to modem",
   "SERIAL - Redirect console to a serial terminal",
   "XWIN   - Redirect console to DGA frame buffer",
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
   "up     - Recall previous command line",
   "down   - Recall next command line",
   "right  - Move cursor right",
   "left   - Move cursor left",
   "BKSP   - Back over last character",
   "HOME   - Start of line",
   "END    - End of line",
   "INS    - Toggle insert mode",
   "DEL    - Delete character",
   "ESC    - Cancel current command",
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

extern char *MacroExpand(char *pCmd);


/******************************************************************************
*                                                                             *
*   BOOL CommandExecute( char *pOrigCmd )                                     *
*                                                                             *
*******************************************************************************
*
*   Executes commands stored in a string. If the command contains a macro,
*   this function is called recursively.
*
*   Where:
*       pOrigCmd is the string with commands. Multiple commands may be separated
*            with ';'
*
*   Returns:
*       FALSE if the command reuqested debugger to continue running
*             the debugee program (such are commands 'g' or 't')
*       TRUE if suggested staying in the debugger
*
******************************************************************************/
BOOL CommandExecute( char *pOrigCmd )
{
    static int nDeep = 0;               // Recursion depth count
    char Command[MAX_STRING];           // Command buffer on the stack
    char *pCmd = Command;               // Pointer to a local buffer
    char *pCmdNext, cDelimiter;
    char *pMacro;                       // Pointer to a macro string (expanded)
    BOOL fInString, fRet = TRUE;
    int i;

    nDeep++;                            // Inside the function, recursion count

    // Copy given command(s) into a local buffer so we can recurse
    strcpy(pCmd, pOrigCmd);

    do
    {
        // Find the first non-space, non-delimiter character of the current command
        while( (*pCmd==' ' || *pCmd==';') && (*pCmd!=0) ) pCmd++;

        // Look for the ";" delimiter, ignore it within a string
        pCmdNext = pCmd;
        fInString = FALSE;
        while( *pCmdNext!=0 && (*pCmdNext!=';' || fInString) )
        {
            if( *pCmdNext=='"' )
                fInString = !fInString;
            pCmdNext++;
        }

        // If the command is empty, return
        if( *pCmd==0 )
            break;

        // Got the first character.. Search all known command keywords from
        // back to front to find a match
        for( i=iLast; i>=0; i--)
        {
            if( strnicmp(pCmd, Cmd[i].sCmd, Cmd[i].nLen)==0
              && !isalnum(pCmd[Cmd[i].nLen]))
                break;
        }

        // Separate current string from the next one
        cDelimiter = *pCmdNext;
        *pCmdNext = 0;

        if( i>= 0 )
        {
            // Find the first non-space character to assign it as a pointer to the first argument
            pCmd += Cmd[i].nLen;
            while( *pCmd==' ' ) pCmd++;

            // Call the command function handler
            fRet = (Cmd[i].pfnCommand)( pCmd, Cmd[i].subClass );
        }
        else
        {
            // Command was not found.  Search all defined macros.
            pMacro = MacroExpand(pCmd);
            if( pMacro != NULL )
            {
                // Recursively submit expanded macro for execution
                // Keep the recursion count so we can terminate if too deep
                if( nDeep <= MAX_MACRO_RECURSE )
                    fRet = CommandExecute(pMacro);
                else
                    fRet = TRUE;
            }
            else
            {
                dprinth(1, "Unknown command or macro");
            }
        }

        // Restore the delimiter character
        *pCmdNext = cDelimiter;

        pCmd = pCmdNext;

    } while( (*pCmd != 0) && fRet );

    nDeep--;                            // Leaving the function, recursion count
    return( fRet );
}


/******************************************************************************
*                                                                             *
*   void CommandBuildHelpIndex()                                              *
*                                                                             *
*******************************************************************************
*
*   Builds a help index at init time.
*
******************************************************************************/
void CommandBuildHelpIndex()
{
    TCommand *pCmd;
    int i, j;

    pCmd = &Cmd[0];
    i = 0;

    // Loop over every entry of command structure...
    while( pCmd->sCmd != NULL )
    {
        // When we are already here, store the last index
        iLast = i;
        j = 0;

        // Search the help string array for the command
        while( sHelp[j] != NULL )
        {
            // If names exactly match, copy the pointer
            if( *(sHelp[j]+pCmd->nLen)==' ' )
            {
                if( strnicmp(sHelp[j], pCmd->sCmd, pCmd->nLen)==0 )
                {
                    pCmd->iHelp = j;
                    break;
                }
            }
            j++;
        }

        i++;
        pCmd++;
    }
}


/******************************************************************************
*                                                                             *
*   BOOL cmdHelp(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Display Help info
*
******************************************************************************/
BOOL cmdHelp(char *args, int subClass)
{
    int nLine = 1;
    int i;

    if( *args )
    {
        // Help on a specific command
        // Search for the specified command in the command array
        i = 0;
        while( Cmd[i].sCmd )
        {
            if( strnicmp(args, Cmd[i].sCmd, strlen(args))==0 )
                break;
            i++;
        }

        // Did we find it?
        if( Cmd[i].sCmd )
        {
            dprinth(nLine++, "%s", sHelp[Cmd[i].iHelp] + 9);
            dprinth(nLine++, "%s", Cmd[i].sSyntax);
            dprinth(nLine++, "%s", Cmd[i].sExample);
        }
        else
        {
            // Nope - user typed invalid command
            dprinth(1, "Command %s is not valid", args);
        }
    }
    else
    {
        // General help - list all commands and their short descriptions
        i = 0;
        do
        {
            if(dprinth(nLine++, "%s", sHelp[i++])==FALSE)
                break;

        } while( sHelp[i] != NULL );
    }

    return(TRUE);
}


/******************************************************************************
*                                                                             *
*   BOOL Unsupported(char *args, int subClass)                                *
*                                                                             *
*******************************************************************************
*
*   Unsupported command stub
*
******************************************************************************/
BOOL Unsupported(char *args, int subClass)
{
    dprinth(1, "Not yet implemented.");

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   int GetOnOff(char *args)                                                  *
*                                                                             *
*******************************************************************************
*
*   Parser helper: Returns [ON | OFF] state + extra
*       ON = 1
*       OFF = 2
*       <none> = 3
*       KERNEL = 4
*   For any other token, it prints the syntax error and returns 0.
*
******************************************************************************/
int GetOnOff(char *args)
{
    int len;

    while( *args==' ') args++;
    len = strlen(args);

    if( len==0 )
        return( 3 );                    // No token - end of argument
    if( len==2 && strnicmp(args, "on", 2)==0 )
        return( 1 );                    // ON
    if( len==3 && strnicmp(args, "off", 3)==0 )
        return( 2 );                    // OFF
    if( len==6 && strnicmp(args, "kernel", 6)==0 )
        return( 4 );                    // KERNEL

    dprinth(1, "Syntax error");

    return( 0 );
}
