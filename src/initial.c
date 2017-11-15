/******************************************************************************
*                                                                             *
*   Module:     initial.c                                                     *
*                                                                             *
*   Date:       10/26/00                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains initial load/unload code

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/26/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <linux/module.h>               // Include module header file

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int errno = 0;                          // Needed by the C library

TDeb deb;                               // Debugee state structure

TVIDEO video;                           // video redirections

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void *malloc(size_t size)
{
    return( NULL );
}    


/******************************************************************************
*                                                                             *
*   int init_module(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   This function is called when a module gets loaded.
*
*   Returns:
*       0 to load module
*       non-zero if a module should not be loaded
*
******************************************************************************/


extern TIDT_Gate ice_idt[0x30];
extern TDescriptor ice_idt_descriptor;


int init_module(void)
{
    printk("<1>LinIce Copyright 2000 by Goran Devic\n");

#if DBG
    printk("<1>Debug build\n");
    ASSERT(sizeof(TSS_Struct) == 104);
    ASSERT(sizeof(TDescriptor) == 6);
#endif

#if 0
    InitIceIdt();
    printk("<1> IceIntHandlers: %08X\n", IceIntHandlers);
    printk("<1>   [0] %08X\n", IceIntHandlers[0]);
    printk("<1>   [1] %08X\n", IceIntHandlers[1]);
    printk("<1>   [2] %08X\n", IceIntHandlers[2]);

    {
        TIDT_Gate *p;
        int i;
        
        p = GET_DESC_BASE(&ice_idt_descriptor);
        printk("<1> IDT: %08X:%04X\n", p, ice_idt_descriptor.limit);

        for( i=1; i<0x30; i++)
        {
            DecodeIDT(p++);
        }
    }
#endif
//return( 1 );

    // Initialize redirection structures to VGA text display

    VgaInit(&video);

    // Initialize deb data structure

    deb.fInt1Here = FALSE;
    deb.fInt3Here = TRUE;

    // Store the current kernel IDT descriptor record so the 
    // hook function can use it
    GetIDT(&deb.idt);

    // Initializes debugger private IDT
    InitIceIdt();

    // Hook the debugee IDT to break on selected interrupts/faults/traps
    HookDebuger();

printk("<1> INT3\n");
    // Issue INT 3 to initially stop inside the debugger
    IssueInt3();

    return( 0 );
}    


/******************************************************************************
*                                                                             *
*   void cleanup_module(void)                                                 *
*                                                                             *
*******************************************************************************
*
*   This function is called when a module gets unloaded.
*
******************************************************************************/
void cleanup_module(void)
{
    // Unhook all the interrupts on exit.
    UnhookDebuger();

    printk("<1>LinIce unloaded.\n");
}    

