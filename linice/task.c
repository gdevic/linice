/******************************************************************************
*                                                                             *
*   Module:     task.c                                                        *
*                                                                             *
*   Date:       04/30/04                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        Task switch hook functions.

    We need to hook the task switcher, or scheduler, so we know which process
    is running.

                                  -   -   -

    The other problem that this hook solves is the kernel actually wipes out
    CPU debug registers on a task switch. This would prevent us to use them
    even for the kernel modules.

                                  -   -   -

    The way we hook task switch function is as follows:
    - there is a function in linux:process.c
    void __switch_to(struct task_struct *prev_p, struct task_struct *next_p)

    that does the switching of a register state. We know the address of that
    function (kernel 2.4 only), so we copy a single CPU instruction from that
    address into our internal buffer, zapping it with a single INT3 (0xCC)
    (This is followed by a required number of NOPs to cover the "missing"
    instruction.)

    When a task switch happens (100 times per second), INT3 calls our
    standard interrupt handler, where we detect this special address and
    modify the return EIP to our handler only, then simply return from the
    "heavy" INT3 handler.

    On a return, our code in a buffer gets control, and pushes our real
    task switch function onto the stack, proceeds to executing the original
    CPU instruction and continues into the kernel code.

    When the kernel is done with its own task switching, our function gets
    called (since we inserted its address to the stack), and within it, we
    reprogram CPU debug registers that are modified by the kernel task
    switch code.

                                  -   -   -

    These are some samples from the start of the switch_to() code for
    various kernels:


        RED HAT 9.0

        The code that we expect at the beginning of the __switch_to() is:

        55               push ebp                +0
        BD xx xx xx xx   mov  ebp, <xxx>         +1
        57               push edi                +6
        ...

        SuSE 9.0

        The code that we expect at the beginning of the __switch_to() is:

        83 EC 18         sub esp, 18             +0
        89 5C 24 0C      mov [esp+C], ebx        +3
        ...


*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 04/30/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()
#include "disassembler.h"               // Include disassembler


/******************************************************************************
*                                                                             *
*   External functions                                                        *
*                                                                             *
******************************************************************************/

extern void SetDebugReg(TSysreg * pSys);

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD switchto;                  // Address of the kernel's __switch_to()
extern BYTE TaskSwitchHookBufferKernelCodeLine[];

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

DWORD TaskSwitchNewRet = 0;             // Updated continuation address
static BYTE bLen;                       // Temp instruction len


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void SwitchHandler(void)                                                  *
*                                                                             *
*******************************************************************************
*
*   This is the effective handler to the task switcher.
*
******************************************************************************/
void SwitchHandler(void)
{
    // Restore debug registers since the task switch just might have reloaded them

    SetDebugReg(&deb.sysReg);
}

/******************************************************************************
*                                                                             *
*   void HookSwitch(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Hooks the task switch handler stub.
*
******************************************************************************/
void HookSwitch(void)
{
    INFO("HookSwitch()\n");

    // Disassemble the starting location to find the length of the CPU instruction
    bLen = GetInstructionLen(GetKernelCS(), switchto);

    INFO("Patch %d bytes (%02X %02X...) at %08X\n", bLen, *(BYTE *)(switchto+0), *(BYTE *)(switchto+1), (DWORD) switchto);

    // Copy the instruction into our buffer to form the complete handler code
    memcpy((BYTE *) &TaskSwitchHookBufferKernelCodeLine, (BYTE *) switchto, bLen);

    // TODO: kernel 2.6 can be pre-empted. Wrap this with a semaphore

    // Zap the original instruction with the 0xCC (INT3) followed by NOP's
    memset((BYTE *) switchto, 0x90, bLen);
    *(BYTE *)switchto = 0xCC;           // Store one-byte INT3 at the start of it

    // Store the new return address from within the original code
    TaskSwitchNewRet = switchto + bLen;

    INFO("TaskSwitchNewRet = %08X\n", TaskSwitchNewRet);
    INFO("TaskSwitchHookBufferKernelCodeLine = %08X\n", &TaskSwitchHookBufferKernelCodeLine);
}

/******************************************************************************
*                                                                             *
*   void UnHookSwitch(void)                                                   *
*                                                                             *
*******************************************************************************
*
*   Unhooks the task switch hook.
*
******************************************************************************/
void UnHookSwitch(void)
{
    INFO("UnHookSwitch()\n");

    // Copy the original code bytes from our buffer into the kernel
    // Only copy if the code line was not "NOP" (0x90) for the safety

    if( TaskSwitchHookBufferKernelCodeLine[0]!=0x90 )
        memcpy((BYTE *) switchto, (BYTE *) &TaskSwitchHookBufferKernelCodeLine, bLen);
}
