/******************************************************************************
*                                                                             *
*   Module:     Sim.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/08/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is the main simulator source code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/08/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <windows.h>                    // Include Windows support
#include <stdio.h>                      // Include standard I/O header file
#include <malloc.h>                     // Include memory operation defines
#include <commctrl.h>                   // Include common controls
#include <fcntl.h>                      // Include IO functions
#include <sys/types.h>                  // Include file functions

#include "Globals.h"                    // Include global definitions

#include "sim.h"                        // Include sim header file
#include "ice-keycode.h"                // Include private keycode map

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

char *pPageOffset = NULL;               // Mapped kernel memory

TDescriptor GDT;
TDescriptor IDT;

WORD sel_ice_ds = 0;                    // Place to patch code
int SysFunctionStub() {return(0);};

unsigned long lin_sys_call_table[256];  // System call table
unsigned long *sys = lin_sys_call_table;// Pointer to system call table

BYTE port_KBD_STATUS = 0;
BYTE port_KBD_DATA = 0;

HANDLE hThread = NULL;                  // Handle to the debugger thread
DWORD dwThreadID;                       // Thread ID number

TREGS Regs;
extern TDEB deb;

unsigned int opt = OPT_VERBOSE;         // Default options
int nVerbose = 1;                       // Loader verbose level
char sTitle[80];                        // Buffer for window title

char sConfig[] = "sim.conf.txt";        // Configuration file

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static OPENFILENAME File;               // Open file name structure
static char sFile[512];                 // Temporary file path name


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void ConfigurationLoad()
{
    FILE *fp;

    fp = fopen(sConfig, "r");
    if(fp)
    {
        fscanf(fp, "Regs.eax = %08X\n", &Regs.eax);
        fscanf(fp, "Regs.ebx = %08X\n", &Regs.ebx);
        fscanf(fp, "Regs.ecx = %08X\n", &Regs.ecx);
        fscanf(fp, "Regs.edx = %08X\n", &Regs.edx);
        fscanf(fp, "Regs.esi = %08X\n", &Regs.esi);
        fscanf(fp, "Regs.edi = %08X\n", &Regs.edi);
        fscanf(fp, "Regs.esp = %08X\n", &Regs.esp);
        fscanf(fp, "Regs.ebp = %08X\n", &Regs.ebp);
        fscanf(fp, "Regs.eip = %08X\n", &Regs.eip);

        fscanf(fp, "Regs.cs = %04X\n", &Regs.cs);
        fscanf(fp, "Regs.ds = %04X\n", &Regs.ds);
        fscanf(fp, "Regs.es = %04X\n", &Regs.es);
        fscanf(fp, "Regs.fs = %04X\n", &Regs.fs);
        fscanf(fp, "Regs.gs = %04X\n", &Regs.gs);
        fscanf(fp, "Regs.ss = %04X\n", &Regs.ss);

        fclose(fp);
    }
}

void ConfigurationSave()
{
    FILE *fp;

    fp = fopen(sConfig, "w");

    if(fp)
    {
        fprintf(fp, "Regs.eax = %08X\n", Regs.eax);
        fprintf(fp, "Regs.ebx = %08X\n", Regs.ebx);
        fprintf(fp, "Regs.ecx = %08X\n", Regs.ecx);
        fprintf(fp, "Regs.edx = %08X\n", Regs.edx);
        fprintf(fp, "Regs.esi = %08X\n", Regs.esi);
        fprintf(fp, "Regs.edi = %08X\n", Regs.edi);
        fprintf(fp, "Regs.esp = %08X\n", Regs.esp);
        fprintf(fp, "Regs.ebp = %08X\n", Regs.ebp);
        fprintf(fp, "Regs.eip = %08X\n", Regs.eip);

        fprintf(fp, "Regs.cs = %04X\n", Regs.cs);
        fprintf(fp, "Regs.ds = %04X\n", Regs.ds);
        fprintf(fp, "Regs.es = %04X\n", Regs.es);
        fprintf(fp, "Regs.fs = %04X\n", Regs.fs);
        fprintf(fp, "Regs.gs = %04X\n", Regs.gs);
        fprintf(fp, "Regs.ss = %04X\n", Regs.ss);

        fclose(fp);
    }
}


void SimInit()
{
    int i;

    // Initialize SIM environment
    //=================================================================

    pPageOffset = VirtualAlloc(NULL, SIM_PAGE_OFFSET_SIZE, MEM_COMMIT, PAGE_READWRITE);
    //pPageOffset = malloc(SIM_PAGE_OFFSET_SIZE);

    GDT.base = (DWORD) pPageOffset;
    GDT.limit = 0xFF;

    IDT.base = (DWORD) pPageOffset;
    IDT.limit = 0xFF;

    // Initialize installer opt (options)

    opt = OPT_VERBOSE;

    // Initialize open file name structure for the symbol files

    ZeroMemory( &File, sizeof(OPENFILENAME) );
    File.lStructSize = sizeof(OPENFILENAME);
    File.hwndOwner = hSim;
    File.lpstrFilter = "Symbol table files (*.sym)\0*.sym\0"\
                       "All files (*.*)\0*.*\0\0";
    File.lpstrFile = sFile;
    File.nMaxFile = sizeof(sFile);
    File.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    File.lpstrDefExt = "sym";
    File.lpstrTitle = "Select symbol file";

    // Initialize system call numbers callback functions to our catcher
    for(i=0; i<256; i++)
        lin_sys_call_table[i] = (ULONG)SysFunctionStub;

    // Init few system calls that we actually expect
    lin_sys_call_table[__NR_exit]           = (ULONG)SysFunctionStub;
    lin_sys_call_table[__NR_fork]           = (ULONG)SysFunctionStub;
    lin_sys_call_table[__NR_unlink]         = (ULONG)SysFunctionStub;
    lin_sys_call_table[__NR_mknod]          = (ULONG)SysFunctionStub;
    lin_sys_call_table[__NR_init_module]    = (ULONG)SysFunctionStub;
    lin_sys_call_table[__NR_delete_module]  = (ULONG)SysFunctionStub;

    Regs.esp = (DWORD) pPageOffset + 0x1000;
    Regs.ss  = 0x18;
    Regs.ds  = 0x18;
    Regs.eip = 0x0;
    Regs.cs  = 0x10;
    Regs.eflags = 0x203;

    ConfigurationLoad();
}

void SimInstall()
{
    char *pSystemMap = SYSTEM_MAP;

    // Call the Linsym installation function
    if( OptInstall(pSystemMap)==FALSE )
        Message("Install failed");
}

void SimUninstall()
{
    // Call the Linsym uninstall function
    OptUninstall();
}

void SimLoadSymbolFile()
{
    // Open up an OpenFile standard dialog box to select the symbol table
    File.lpstrFilter = "Symbol table files (*.sym)\0*.sym\0All files (*.*)\0*.*\0\0";
    File.lpstrDefExt = "sym";
    File.lpstrTitle = "Select symbol file";

    if( GetOpenFileName( &File ) )
    {
        OptAddSymbolTable(sFile);
    }
}

void SimUnloadSymbolFile()
{
    char *p;                            // Generic pointer

    // Unload a symbol table. User will need to supply the table name as an optional parameter
    // For now we will just open the file selection dialog box in order to select a file

    if( GetOpenFileName( &File ) )
    {
        // Get the table name only: We assume the File has the following form:
        // "[drive]:\{Path\}\[Map].[o.]sym"
        p = strrchr(sFile, '\\') + 1;
        *(char *)(strchr(p, '.')) = 0;
        OptRemoveSymbolTable(p);
    }
}

void SimLoadCapture()
{
    int fd;                             // Capture file descriptor

    // Get the capture file
    File.lpstrFilter = "Capture files (*.bin)\0*.bin\0All files (*.*)\0*.*\0\0";
    File.lpstrDefExt = "bin";
    File.lpstrTitle = "Select capture file";

    if( GetOpenFileName( &File ) )
    {
        // Open the capture file and copy its contents into our virtual memory
        fd = open(File.lpstrFile, O_RDONLY | O_BINARY);
        if(fd>=0)
        {
            LoadModuleFile(fd);
        }
        else
            MessageBox(hSim, "Error opening capture file", "Error", MB_OK);
    }
}

int RunCPU()
{
    int dr;

    // Keep the CPU going...
Loop:
    Regs.eip += 2;                      // Increment by 2 since all assembly instructions are 0 (add eax, al)

    sprintf(sTitle, "Sim: IP=%04X", Regs.eip);
    SetWindowText(hSim, sTitle);

    // Check debug registers
    dr = 0;
    if((deb.sysReg.dr7 & (3<<0)) && !(deb.sysReg.dr7 & (3<<16)) && deb.sysReg.dr[0]==Regs.eip) dr |= 1<<0;
    if((deb.sysReg.dr7 & (3<<2)) && !(deb.sysReg.dr7 & (3<<18)) && deb.sysReg.dr[1]==Regs.eip) dr |= 1<<1;
    if((deb.sysReg.dr7 & (3<<4)) && !(deb.sysReg.dr7 & (3<<20)) && deb.sysReg.dr[2]==Regs.eip) dr |= 1<<2;
    if((deb.sysReg.dr7 & (3<<6)) && !(deb.sysReg.dr7 & (3<<22)) && deb.sysReg.dr[3]==Regs.eip) dr |= 1<<3;

    deb.sysReg.dr6 |= dr;

    // Trap flag has highest priority

    if(Regs.eflags & (1<<8))        // TRAP FLAG
    {
        deb.sysReg.dr6 |= 1<<14;    // Trap happened

        return(1);
    }

    // If debug condition happened, break
    if(dr)
        return(1);

    // Else just keep looping until certain EIP
    if(Regs.eip<100)
        goto Loop;

    return(0);
}


DWORD WINAPI DebuggerThread(int nInt)
{
Execute:
    sprintf(sTitle, "Sim: Int %d", nInt);
    SetWindowText(hSim, sTitle);

    InterruptHandler( nInt, &Regs );

    if( (nInt = RunCPU()) )
        goto Execute;

    SetWindowText(hSim, "Sim: Exit thread");

    ExitThread(TRUE);

    return(0);
}

void CreateSimINT(int nInt)
{
    // Destroy any possible previous thread
    if(hThread)
        TerminateThread(hThread, 0);

    // Create thread to execute the interrupt

    hThread = CreateThread(NULL, 0, DebuggerThread, nInt, 0, &dwThreadID);
}

void SimKey(UINT wCode, DWORD lExt)
{
    // Store the raw key code to be picked up
    port_KBD_DATA = (lExt >> 16) & 0xFF;

    // If the key has been depressed, append 1
    if(lExt & (1<<31))
        port_KBD_DATA |= 0x80;

    // Call linice low-level keyboard handler
    KeyboardHandler();
}
