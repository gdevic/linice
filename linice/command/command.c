/******************************************************************************
*                                                                             *
*   Module:     command.c                                                     *
*                                                                             *
*   Date:       09/11/00                                                      *
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

extern BOOL cmdEvaluate     (char *args, int subClass);      // evalex.c
extern BOOL cmdAscii        (char *args, int subClass);      // customization.c
extern BOOL cmdAltkey       (char *args, int subClass);      // customization.c
extern BOOL cmdCode         (char *args, int subClass);      // customization.c
extern BOOL cmdSet          (char *args, int subClass);      // customization.c
extern BOOL cmdVar          (char *args, int subClass);      // customization.c
extern BOOL cmdMacro        (char *args, int subClass);      // customization.c
extern BOOL cmdResize       (char *args, int subClass);      // customization.c
extern BOOL cmdColor        (char *args, int subClass);      // customization.c
extern BOOL cmdSerial       (char *args, int subClass);      // customization.c
extern BOOL cmdPause        (char *args, int subClass);      // customization.c
extern BOOL cmdSrc          (char *args, int subClass);      // customization.c
extern BOOL cmdTabs         (char *args, int subClass);      // customization.c
extern BOOL cmdDisplay      (char *args, int subClass);      // customization.c
extern BOOL cmdBp           (char *args, int subClass);      // breakpoints.c
extern BOOL cmdBl           (char *args, int subClass);      // breakpoints.c
extern BOOL cmdBpet         (char *args, int subClass);      // breakpoints.c
extern BOOL cmdBstat        (char *args, int subClass);      // breakpoints.c
extern BOOL cmdBpx          (char *args, int subClass);      // breakpoints.c
extern BOOL cmdXit          (char *args, int subClass);      // flow.c
extern BOOL cmdGo           (char *args, int subClass);      // flow.c
extern BOOL cmdTrace        (char *args, int subClass);      // flow.c
extern BOOL cmdStep         (char *args, int subClass);      // flow.c
extern BOOL cmdZap          (char *args, int subClass);      // flow.c
extern BOOL cmdI1here       (char *args, int subClass);      // flow.c
extern BOOL cmdI3here       (char *args, int subClass);      // flow.c
extern BOOL cmdHboot        (char *args, int subClass);      // flow.c
extern BOOL cmdHalt         (char *args, int subClass);      // flow.c
extern BOOL cmdCall         (char *args, int subClass);      // flow.c
extern BOOL cmdWd           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdData         (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWc           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWr           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWs           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWl           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdWw           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdCls          (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdRs           (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdFlash        (char *args, int subClass);      // windowcontrol.c
extern BOOL cmdTable        (char *args, int subClass);      // symbolTable.c
extern BOOL cmdSymbol       (char *args, int subClass);      // symbols.c
extern BOOL cmdExport       (char *args, int subClass);      // symbols.c
extern BOOL cmdTypes        (char *args, int subClass);      // types.c
extern BOOL cmdFile         (char *args, int subClass);      // symbols.c
extern BOOL cmdWhat         (char *args, int subClass);      // symbols.c
extern BOOL cmdReg          (char *args, int subClass);      // registers.c
extern BOOL cmdLocals       (char *args, int subClass);      // locals.c
extern BOOL cmdStack        (char *args, int subClass);      // stack.c
extern BOOL cmdWatch        (char *args, int subClass);      // watch.c
extern BOOL cmdGdt          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdLdt          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdIdt          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdCpu          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdModule       (char *args, int subClass);      // sysinfo.c
extern BOOL cmdVer          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdProc         (char *args, int subClass);      // sysinfo.c
extern BOOL cmdTss          (char *args, int subClass);      // sysinfo.c
extern BOOL cmdPage         (char *args, int subClass);      // page.c
extern BOOL cmdPhys         (char *args, int subClass);      // page.c
extern BOOL cmdPeek         (char *args, int subClass);      // page.c
extern BOOL cmdPoke         (char *args, int subClass);      // page.c
extern BOOL cmdPci          (char *args, int subClass);      // pci.c
extern BOOL cmdDdump        (char *args, int subClass);      // data.c
extern BOOL cmdDex          (char *args, int subClass);      // data.c
extern BOOL cmdEdit         (char *args, int subClass);      // data.c
extern BOOL cmdFormat       (char *args, int subClass);      // data.c
extern BOOL cmdUnasm        (char *args, int subClass);      // code.c
extern BOOL cmdDot          (char *args, int subClass);      // code.c
extern BOOL cmdEnterCode    (char *args, int subClass);      // code.c
extern BOOL cmdHere         (char *args, int subClass);      // code.c
extern BOOL cmdOut          (char *args, int subClass);      // ioport.c
extern BOOL cmdIn           (char *args, int subClass);      // ioport.c
extern BOOL cmdFill         (char *args, int subClass);      // blockops.c
extern BOOL cmdSearch       (char *args, int subClass);      // blockops.c
extern BOOL cmdCompare      (char *args, int subClass);      // blockops.c
extern BOOL cmdMove         (char *args, int subClass);      // blockops.c
extern BOOL cmdHelp         (char *args, int subClass);      // command.c
extern BOOL cmdFkey         (char *args, int subClass);      // edlin.c
extern BOOL cmdExtList      (char *args, int subClass);      // extend.c


//    +--Command name
//    |          +--Number of characters in the command name
//    |          |  +--Command subclass number
//    |          |  |  +--Function
//    |          |  |  |               +--Help line for that command
//    |          |  |  |               |

TCommand Cmd[] = {
{    ".",        1, 0, cmdDot,         "Locate current instruction", "ex: .",  0 },
{    ".?",       2, 0, cmdExtList,     "List extended debugger commands", "ex: .?",   0 },
{    "?",        1, 0, cmdEvaluate,    "? expression", "ex: ? ax << 1",   0 },
//{  "A",        1, 0, Unsupported,    "Assemble [Address]", "ex: A CS:1236",    0 },
//{  "ADDR",     4, 0, Unsupported,    "ADDR [context-handle | task | *]", "ex: ADDR 80FD602C",   0 },
{    "ALTKEY",   6, 0, cmdAltkey,      "ALTKEY [ALT letter | CTRL letter]", "ex: ALTKEY ALT D",   0 },
{    "ALTSCR",   6, 3, cmdDisplay,     "ALTSCR [MONO | VGA | XWIN | OFF]", "ex: ALTSCR MONO",    0 },
{    "ASCII",    5, 0, cmdAscii,       "ASCII", "ex: ASCII", 0 },
{    "BC",       2, 0, cmdBp,          "BC list | *", "ex: BC *", 0 },
{    "BD",       2, 1, cmdBp,          "BD list | *", "ex: BD 1,3,4", 0 },
{    "BE",       2, 2, cmdBp,          "BE list | *", "ex: BE 1,3,4", 0 },
//{  "BH",       2, 0, Unsupported,    "BH breakpoint history", "ex: BH", 0 },
{    "BL",       2, 0, cmdBl,          "BL list current breakpoints", "ex: BL",   0 },
{    "BPE",      3, 0, cmdBpet,        "BPE edit breakpoint number", "ex: BPE 3",  0 },
//{  "BPINT",    5, 2, Unsupported,    "BPINT interrupt-number [IF expression] [DO bp-action]", "ex: BPINT 50",   0 },
{    "BPIO",     4, 3, cmdBpx,         "BPIO port [R|W|RW] [debug register] [O] [IF expression] [DO bp-action]", "ex: BPIO 3DA W",   0 },
{    "BPM",      3, 4, cmdBpx,         "BPM[size] address [R|W|RW|X] [debug register] [O] [IF expression] [DO bp-action]", "ex: BPM 1234 RW", 0 },
{    "BPMB",     4, 4, cmdBpx,         "BPMB address [R|W|RW|X] [debug register] [O] [IF expression] [DO bp-action]", "ex: BPMB 333 R",   0 },
{    "BPMD",     4, 7, cmdBpx,         "BPMD address [R|W|RW|X] [debug register] [O] [IF expression] [DO bp-action]", "ex: BPMD EDI W",   0 },
{    "BPMW",     4, 5, cmdBpx,         "BPMW address [R|W|RW|X] [debug register] [O] [IF expression] [DO bp-action]", "ex: BPMW ESP-6 W", 0 },
{    "BPT",      3, 1, cmdBpet,        "BPT template breakpoint number", "ex: BPT 0",  0 },
{    "BPX",      3, 1, cmdBpx,         "BPX address [debug register] [O] [IF expression] [DO bp-action]", "ex: BPX 282FE0",    0 },
{    "BSTAT",    5, 0, cmdBstat,       "BSTAT [breakpoint #]", "ex: BSTAT 3", 0 },
{    "C",        1, 0, cmdCompare,     "Compare [-e] address1 L length address2", "ex: C 80000 L 40 EBX",    0 },
{    "CALL",     4, 0, cmdCall,        "CALL [address]([args[,]])", "ex: CALL funct(eax,0)", 0 },
{    "CLS",      3, 0, cmdCls,         "CLS clear window", "ex: CLS", 0 },
{    "CODE",     4, 0, cmdCode,        "CODE [ON | OFF]", "ex: CODE OFF", 0 },
{    "COLOR",    5, 0, cmdColor,       "COLOR [normal bold reverse help line | - ]", "ex: COLOR 30 3E 1F 1E 34", 0 },
{    "CPU",      3, 0, cmdCpu,         "CPU [s | r]", "ex: CPU", 0 },
{    "D",        1, 0, cmdDdump,       "D [address [L length]]", "ex: D B0000",   0 },
{    "DATA",     4, 0, cmdData,        "DATA [data-window-number]", "ex: DATA 2", 0 },
//{  "DEVICE",   6, 0, Unsupported,    "DEVICE [device-name | address]", "ex: DEVICE /dev/hda",   0 },
{    "DEX",      3, 0, cmdDex,         "DEX [[data-window-number] expression]", "ex: DEX 2 esp",  0 },
{    "DB",       2, 1, cmdDdump,       "DB [address [L length]]", "ex: DB ESI+EBX",   0 },
{    "DD",       2, 4, cmdDdump,       "DD [address [L length]]", "ex: DD EBX+133",   0 },
{    "DW",       2, 2, cmdDdump,       "DW [address [L length]]", "ex: DW EDI",   0 },
{    "E",        1, 0, cmdEdit,        "Edit [address] [data]", "ex: E 400000",  0 },
{    "EB",       2, 1, cmdEdit,        "EB [address] [data]", "ex: EB 324415",    0 },
{    "EC",       2, 0, cmdEnterCode,   "EC Enter/exit code window", "ex: EC", 0 },
{    "ED",       2, 4, cmdEdit,        "ED [address] [data]", "ex: ED 84",    0 },
{    "EW",       2, 2, cmdEdit,        "EW [address] [data]", "ex: EW ESP-8", 0 },
{    "EXP",      3, 0, cmdExport,      "EXP [module[!partial-name]]", "ex: EXP module", 0 },
{    "F",        1, 0, cmdFill,        "Fill address L length data-string", "ex: F EBX L 50 'ABCD'", 0 },
{    "FAULTS",   6, 0, Unsupported,    "FAULTS [ON | OFF]", "ex: FAULTS ON",  0 },
{    "FILE",     4, 0, cmdFile,        "FILE [file-name | *]", "ex: FILE main.c", 0 },
{    "FKEY",     4, 0, cmdFkey,        "FKEY [function-key string]", "ex: FKEY F1 DD ESP; G @ESP",    0 },
{    "FLASH",    5, 0, cmdFlash,       "FLASH [ON | OFF]", "ex: FLASH ON",    0 },
{    "FORMAT",   6, 0, cmdFormat,      "FORMAT Change format of data window", "ex: FORMAT",   0 },
{    "G",        1, 0, cmdGo,          "G [=start address] [end address]", "ex: G 231456",  0 },
{    "GDT",      3, 0, cmdGdt,         "GDT [selector | GDT base-address]", "ex: GDT 28", 0 },
//{  "GENINT",   6, 0, Unsupported,    "GENINT [NMI | INT1 | INT3 | int-number]", "ex: GENINT 2", 0 },
{    "H",        1, 0, cmdHelp,        "Help [command]", "ex: H R",  0 },
{    "HALT",     4, 0, cmdHalt,        "HALT System APM Off", "ex: HALT",    0 },
{    "HBOOT",    5, 0, cmdHboot,       "HBOOT System boot (total reset)", "ex: HBOOT",    0 },
{    "HERE",     4, 0, cmdHere,        "HERE Go to current cursor line", "ex: HERE", 0 },
{    "HELP",     4, 0, cmdHelp,        "Help [command]", "ex: HELP R",  0 },
{    "I",        1, 1, cmdIn,          "I port", "ex: I 21",  0 },
{    "I1HERE",   6, 0, cmdI1here,      "I1HERE [ON | OFF | KERNEL]", "ex: I1HERE ON",  0 },
{    "I3HERE",   6, 0, cmdI3here,      "I3HERE [ON | OFF | KERNEL]", "ex: I3HERE ON",  0 },
{    "IB",       2, 1, cmdIn,          "IB port", "ex: IB 3DA",   0 },
{    "ID",       2, 4, cmdIn,          "ID port", "ex: ID DX",    0 },
{    "IW",       2, 2, cmdIn,          "IW port", "ex: IW DX",    0 },
{    "IDT",      3, 0, cmdIdt,         "IDT [int-number | IDT base-address]", "ex: IDT 21",   0 },
{    "LDT",      3, 0, cmdLdt,         "LDT [selector | LDT table selector]", "ex: LDT 45",   0 },
{    "LINES",    5, 1, cmdResize,      "LINES [Number of lines on a display device]", "ex: LINES 43",   0 },
{    "LOCALS",   6, 0, cmdLocals,      "LOCALS", "ex: LOCALS",    0 },
{    "M",        1, 0, cmdMove,        "Move source-address L length dest-address", "ex: M 4000 L 80 8000",    0 },
{    "MACRO",    5, 0, cmdMacro,       "MACRO [macro-name] | [[*] | [= \"macro-body\"]]", "ex: MACRO Oops = \"i3here off; genint 3;\"",   0 },
{    "MDA",      3, 1, cmdDisplay,     "MDA Switch to Monochrome text output", "ex: MDA",   0 },
{    "MODULE",   6, 0, cmdModule,      "MODULE [module-name | partial-name]", "ex: MODULE",    0 },
{    "O",        1, 1, cmdOut,         "O port value", "ex: O 21 FF", 0 },
{    "OB",       2, 1, cmdOut,         "OB port value", "ex: OB 21 FF",   0 },
{    "OW",       2, 2, cmdOut,         "OW port value", "ex: OW DX AX",   0 },
{    "OD",       2, 4, cmdOut,         "OD port value", "ex: OD DX EAX",  0 },
{    "P",        1, 0, cmdStep,        "P [RET]", "ex: P",    0 },
{    "PAGE",     4, 0, cmdPage,        "PAGE [address [L length]]", "ex: PAGE DS:0 L 20", 0 },
//{  "PAGEIN",   6, 0, Unsupported,    "PAGEIN address", "ex: PAGEIN 401000", 0 },
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
//{  "PRN",      3, 0, Unsupported,    "PRN [LPTx | COMx]", "ex: PRN LPT1", 0 },
{    "PROC",     4, 0, cmdProc,        "PROC [-xo] [task-name]", "ex: PROC -x Explorer", 0 },
//{  "QUERY",    5, 0, Unsupported,    "QUERY [[-x] address]", "ex: QUERY eip", 0 },
{    "R",        1, 0, cmdReg,         "R [-d | register-name | register-name [=] value]", "ex: R EAX=50", 0 },
{    "RS",       2, 0, cmdRs,          "RS Restore program screen", "ex: RS", 0 },
{    "S",        1, 0, cmdSearch,      "Search [-c] address L length data-string", "ex: S 0 L ffffff 'Help',0D,0A", 0 },
{    "SERIAL",   6, 0, cmdSerial,      "SERIAL [ON|VT100 [com-port] [baud-rate] | OFF]", "ex: SERIAL ON 2 19200", 0 },
{    "SET",      3, 0, cmdSet,         "SET [setvariable] [ON | OFF] [value]", "ex: SET FAULTS ON",   0 },
//{  "SHOW",     4, 0, Unsupported,    "SHOW [B | start] [L length]", "ex: SHOW 100", 0 },
{    "SRC",      3, 0, cmdSrc,         "SRC Toggle between source, mixed & code", "ex: SRC",  0 },
//{  "SS",       2, 0, Unsupported,    "SS [line-number] ['search-string']", "ex: SS 40 'if (i==3)'", 0 },
{    "STACK",    5, 0, cmdStack,       "STACK [-v] [SS:EBP]", "ex: STACK",  0 },
{    "SYM",      3, 0, cmdSymbol,      "SYM [partial-name* | symbol-name]", "ex: SYM get*",   0 },
{    "T",        1, 0, cmdTrace,       "Trace [count]", "ex: T",   0 },
//{  "THREAD",   6, 0, Unsupported,    "THREAD [TCB | ID | task-name]", "ex: THREAD", 0 },
//{  "TRACE",    5, 0, Unsupported,    "TRACE [B | OFF | start]", "ex: TRACE 50", 0 },
{    "TABLE",    5, 0, cmdTable,       "TABLE [[R] table-name] | AUTOON | AUTOOFF | $", "ex: TABLE test", 0 },
{    "TABS",     4, 0, cmdTabs,        "TABS [1 - 8]", "ex: TABS 4",  0 },
{    "TSS",      3, 0, cmdTss,         "TSS [TSS selector]", "ex: TSS",   0 },
{    "TYPES",    5, 0, cmdTypes,       "TYPES [type-name]", "ex: TYPE TagMyType", 0 },
{    "U",        1, 0, cmdUnasm,       "Unassemble [address [L length]]", "ex: U EIP-10",  0 },
{    "VAR",      3, 0, cmdVar,         "VAR [variable-name] | [[*] | [= expression]]", "ex: VAR myvalue = eax+20",   0 },
{    "VER",      3, 0, cmdVer,         "VER Display Linice version", "ex: VER",   0 },
{    "VGA",      3, 0, cmdDisplay,     "VGA Switch to VGA text output", "ex: VGA",   0 },
{    "WATCH",    5, 0, cmdWatch,       "WATCH address", "ex: WATCH VariableName", 0 },
{    "WC",       2, 0, cmdWc,          "WC [code-window-size]", "ex: WC 8",    0 },
{    "WD",       2, 0, cmdWd,          "WD [data-window-size]", "ex: WD 4",    0 },
{    "WIDTH",    5, 0, cmdResize,      "WIDTH [Number of columns on a display device]", "ex: WIDTH 100", 0 },
{    "WL",       2, 0, cmdWl,          "WL [locals-window-size]", "ex: WL 8",    0 },
{    "WHAT",     4, 0, cmdWhat,        "WHAT expression", "ex: WHAT esi",  0 },
{    "WR",       2, 0, cmdWr,          "WR Toggle register window", "ex: WR", 0 },
{    "WS",       2, 0, cmdWs,          "WS [stack-window-size]", "ex: WS 8",    0 },
{    "WW",       2, 0, cmdWw,          "WW Toggle watch window", "ex: WW", 0 },
{    "X",        1, 0, cmdXit,         "X Return to host debugger or program", "ex: X", 0 },
//{  "XG",       2, 0, Unsupported,    "XG [r] address", "ex: XG eip-10", 0 },
//{  "XP",       2, 0, Unsupported,    "XP", "ex: XP", 0 },
//{  "XRSET",    5, 0, Unsupported,    "XRSET", "ex: XRSET", 0 },
//{  "XT",       2, 0, Unsupported,    "XT [r]", "ex: XT", 0 },
//{  "XFRAME",   6, 0, Unsupported,    "XFRAME [frame address]", "ex: XFRAME EBP",    0 },
{    "XWIN",     4, 2, cmdDisplay,     "XWIN Switch to DGA-compatible X-Window display", "ex: XWIN",   0 },
{    "ZAP",      3, 0, cmdZap,         "ZAP Zap embeded INT1 or INT3", "ex: ZAP", 0 },
{    NULL,       0, 0, NULL,           NULL, 0 }
};


char *sHelp[] = {
   " SETTING BREAK POINTS",
   "BPM    - Breakpoint on memory access",
   "BPMB   - Breakpoint on memory access, byte size",
   "BPMW   - Breakpoint on memory access, word size",
   "BPMD   - Breakpoint on memory access, double word size",
   "BPIO   - Breakpoint on I/O port access",
/* "BPINT  - Breakpoint on interrupt", */
   "BPX    - Breakpoint on execution",
   "BSTAT  - Breakpoint Statistics",
   " MANIPULATING BREAK POINTS",
   "BPE    - Edit breakpoint",
   "BPT    - Use breakpoint as a template",
   "BL     - List current breakpoints",
   "BC     - Clear breakpoint",
   "BD     - Disable breakpoint",
   "BE     - Enable breakpoint",
/* "BH     - Breakpoint history", */
   " DISPLAY/CHANGE MEMORY",
   "R      - Display/change register contents",
   "U      - Un-assembles instructions",
   "D      - Display memory",
   "DB     - Display memory, byte size",
   "DW     - Display memory, word size",
   "DD     - Display memory, double word size",
   "E      - Edit memory",
   "EB     - Edit memory, byte size",
   "EW     - Edit memory, word size",
   "ED     - Edit memory, double word size",
   "PEEK   - Read from physical address",
   "PEEKB  - Read from physical address a byte",
   "PEEKW  - Read from physical address a word",
   "PEEKD  - Read from physical address a dword",
   "POKE   - Write to physical address",
   "POKEB  - Write to physical address a byte",
   "POKEW  - Write to physical address a word",
   "POKED  - Write to physical address a dword",
/* "PAGEIN - Load a page into physical memory (note: not always safe)", */
   "H      - Help on the specified function",
   "HELP   - Help on the specified function",
   "?      - Evaluate expression",
   "VER    - Linice version",
   "WATCH  - Add watch variable",
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
   "PAGE   - Display page table information",
   "PHYS   - Display all virtual addresses for physical address",
   "STACK  - Display call stack",
/* "XFRAME - Display active exception frames", */
/* "THREAD - Display thread information", */
/* "ADDR   - Display/change address contexts", */
   "PROC   - Display process information",
/* "QUERY  - Display a processes virtual address space map", */
   "WHAT   - Identify the type of an expression",
/* "DEVICE - Display info about a device", */
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
/* "GENINT - Generate an interrupt", */
   "HALT   - System APM Off",
   "HBOOT  - System boot (total reset)",
   " MODE CONTROL",
   "I1HERE - Direct INT1 to LinICE, globally or kernel only",
   "I3HERE - Direct INT3 to LinICE, globally or kernel only",
   "ZAP    - Zap embedded INT1 or INT3",
   "FAULTS - Enable/disable LinICE fault trapping",
   "SET    - Change an internal system variable",
   "VAR    - Change a user variable",
   " CUSTOMIZATION COMMANDS",
   "PAUSE  - Controls display scroll mode",
   "ALTKEY - Set key sequence to invoke window",
   "FKEY   - Display/set function keys",
   "DEX    - Display/assign window data expressions",
   "CODE   - Display instruction bytes in code window",
   "COLOR  - Display/set screen colors",
   "TABS   - Set/display tab settings",
   "LINES  - Set/display number of lines on screen",
   "WIDTH  - Set/display number of columns on screen",
/* "PRN    - Set printer output port", */
/* "PRINT-SCREEN key - Dump screen to printer", */
   "MACRO  - Define a named macro command",
   " UTILITY COMMANDS",
/* "A      - Assemble code", */
   "S      - Search for data",
   "F      - Fill memory with data",
   "M      - Move data",
   "C      - Compare two data blocks",
   "ASCII  - Prints an ASCII character table",
   "CALL   - Execute a function call",
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
   "WL     - Toggle locals window",
   "WR     - Toggle register window",
   "WS     - Toggle call stack window",
   "WW     - Toggle  watch window",
   "EC     - Enter/exit code window",
   ".      - Locate current instruction",
   " WINDOW CONTROL",
   "VGA    - Switch to a VGA text display",
   "MDA    - Switch to a MDA (Monochrome) text display",
   "XWIN   - Redirect console to a DGA frame buffer",
   "SERIAL - Redirect console to a serial terminal",
   "CLS    - Clear window",
   "RS     - Restore program screen",
   "ALTSCR - Change to alternate display",
   "FLASH  - Restore screen during P and T",
   " SYMBOL/SOURCE COMMANDS",
   "SYM    - Display symbols",
   "EXP    - Display exported symbols from a kernel or a module",
   "SRC    - Toggle between source, mixed & code",
   "TABLE  - Select/remove symbol table",
   "FILE   - Change/display current source file",
/* "SS     - Search source module for string", */
   "TYPES  - List all types, or display type definition",
   "LOCALS - Display locals currently in scope",
/* " BACK TRACE COMMANDS", */
/* "SHOW   - Display from backtrace buffer", */
/* "TRACE  - Enter back trace simulation mode", */
/* "XT     - Step in trace simulation mode", */
/* "XP     - Program step in trace simulation mode", */
/* "XG     - Go to address in trace simulation mode", */
/* "XRSET  - Reset back trace history buffer", */
   " SPECIAL OPERATORS",
   ".      - Preceding a decimal number specifies a line number",
   "@      - Preceding an address specifies indirection",
   NULL
};


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern char *MacroExpand(char *pCmd);
extern int DispatchExtCommand(char *pCommand);

/******************************************************************************
*                                                                             *
*   BOOL EOL(char **ppArg)                                                    *
*                                                                             *
*******************************************************************************
*
*   Given the pointer to argument, advances it past spaces and checks for
*   separators ';' returns if the _logical_ end of command line is reached.
*
*   It adjusts the argument pointer accordingly.
*
*   Where:
*       ppArg is the address of the pointer to argument string
*
*   Returns:
*       TRUE - _logical_ end of line is reached
*       FALSE - another non-space character in the line
*
******************************************************************************/
BOOL EOL(char **ppArg)
{
    char *arg = *ppArg;                 // Get the pointer to argument string

    // Skip the spaces
    while( *arg==' ' ) arg++;

    // if we hit the end of string or a logical line terminator, return TRUE
    if( *arg=='\0' || *arg==';' )
    {
        *ppArg = arg;
        return( TRUE );
    }

    // Else, the end of line has not been reached
    *ppArg = arg;
    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL CommandExecute( char *pOrigCmd )                                     *
*                                                                             *
*******************************************************************************
*
*   Executes commands stored in a string. If the command contains a macro,
*   this function is called recursively.
*
*   We also exit if there was an error evaluating a command.
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
            if( !strnicmp(pCmd, Cmd[i].sCmd, Cmd[i].nLen)
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
                // The last thing we search the extended commands (DOT-commands)
                // Only if the first character is a DOT, though
                if( *pCmd=='.' )
                {
                    return( DispatchExtCommand(pCmd+1) );
                }
                else
                {
                    PostError(ERR_COMMAND, 0);
                    return( TRUE );
                }
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
                if( !strnicmp(sHelp[j], pCmd->sCmd, pCmd->nLen) )
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
            if( !strnicmp(args, Cmd[i].sCmd, strlen(args)) )
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
            PostError(ERR_COMMAND, 0);
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
    PostError(ERR_NOT_IMPLEMENTED, 0);

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
    if( len==2 && !strnicmp(args, "on", 2) )
        return( 1 );                    // ON
    if( len==3 && !strnicmp(args, "off", 3) )
        return( 2 );                    // OFF
    if( len==6 && !strnicmp(args, "kernel", 6) )
        return( 4 );                    // KERNEL

    PostError(ERR_SYNTAX, 0);

    return( 0 );
}
