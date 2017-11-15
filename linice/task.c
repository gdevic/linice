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

        Task switch hook functions.

    We need to hook the task switcher, or scheduler, so we know which process
    is running.

                                  -   -   -

    The other problem that this hook solves is the kernel actually wipes out
    CPU debug registers on a task switch. This would prevent us to use them
    even for the kernel modules. Since our hook is running after that happens,
    we are able to restore the state of DR registers.

                                  -   -   -

    The way we hook task switch function is as follows:
    - there is a function in linux:process.c
    void __switch_to(struct task_struct *prev_p, struct task_struct *next_p)

    that does the switching of a register state. We know the first few bytes
    of that code, so we insert a call to our stub that makes our handler's
    return address pushed on the stack, so the __switch_to() will return
    to it on a way out.


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


/******************************************************************************
*                                                                             *
*   External functions                                                        *
*                                                                             *
******************************************************************************/

extern void SetDebugReg(TSysreg * pSys);
extern void InsertTaskSwitchHandler();

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern DWORD switchto;                  // Address of the kernel's __switch_to()

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

DWORD TaskSwitchOrigRet;                // Temp variable for the asm code
BYTE switch_to_header[6] = { 0, };      // Code bytes from the start of __switch_to()

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
*   BOOL TestHookSwitch(void)
*                                                                             *
*******************************************************************************
*
*   Tests the code bytes for the task switch hook function.
*
*   Returns:
*       TRUE - Expected code bytes there
*       FALSE - Unexpected code bytes - we should not load!
*
******************************************************************************/
BOOL TestHookSwitch(void)
{
#ifdef SIM
    return( TRUE );
#endif // SIM

    // The code that we expect at the beginning of the __switch_to() is:
    //
    // 55               push ebp                +0
    // BD xx xx xx xx   mov  ebp, <xxx>         +1
    // 57               push edi                +6
    // ...
    // Make sure the function is correct by comparing the code bytes

    if( switchto && *(BYTE *)(switchto+0)==0x55 && *(BYTE *)(switchto+1)==0xBD && *(BYTE *)(switchto+6)==0x57 )
    {
        return( TRUE );
    }
    return( FALSE );    // The address of the "switchto" was incorrect, we cannot hook!
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
    // Arm the __switch_to() to jump into our stub function.

    // Store away the original 5 bytes from those locations

    switch_to_header[0] = *(BYTE *)(switchto+0);
    switch_to_header[1] = *(BYTE *)(switchto+1);
    switch_to_header[2] = *(BYTE *)(switchto+2);
    switch_to_header[3] = *(BYTE *)(switchto+3);
    switch_to_header[4] = *(BYTE *)(switchto+4);
    switch_to_header[5] = *(BYTE *)(switchto+5);

    // Hook a call to our stub function

    *(BYTE *)(switchto+0) = 0x90;       // 90   NOP
    *(BYTE *)(switchto+1) = 0xE8;       // E8   CALL <relative>
    *(DWORD*)(switchto+2) = (DWORD)InsertTaskSwitchHandler - (switchto+1) - 5;
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

    if( switch_to_header[0] )
    {
        *(BYTE *)(switchto+0) = switch_to_header[0];
        *(BYTE *)(switchto+1) = switch_to_header[1];
        *(BYTE *)(switchto+2) = switch_to_header[2];
        *(BYTE *)(switchto+3) = switch_to_header[3];
        *(BYTE *)(switchto+4) = switch_to_header[4];
        *(BYTE *)(switchto+5) = switch_to_header[5];
    }
}
