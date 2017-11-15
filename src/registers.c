/******************************************************************************
*                                                                             *
*   Module:     registers.c                                                   *
*                                                                             *
*   Date:       11/15/00                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains code that prints and edits registers

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/15/00   Original                                             Goran Devic *
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

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct
{
    DWORD x;
    DWORD y;
    
} TRegfield;

static const TRegfield regfield[23] = {
{4, 0}, {19, 0}, {34, 0}, {49, 0}, {64, 0}, 
{4, 1}, {19, 1}, {34, 1}, {49, 1},
{60, 1}, {62, 1}, {64, 1}, {66, 1}, {68, 1}, {70, 1}, {72, 1}, {74, 1}, 
{3, 2}, {13, 2}, {23, 2}, {33, 2}, {43, 2}, {53, 2} };


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void PrintRegisters(void)
{
    dprint("EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X   ESI=%08X\n",
            deb.r->eax, deb.r->ebx, deb.r->ecx, deb.r->edx, deb.r->esi );

    dprint("EDI=%08X   EBP=%08X   ESP=%08X   EIP=%08X   %c %c %c %c %c %c %c %c\n",
            deb.r->edi, deb.r->ebp, deb.r->esp, deb.r->eip,
            (deb.r->eflags & OF_MASK)? 'O' : 'o',
            (deb.r->eflags & DF_MASK)? 'D' : 'd',
            (deb.r->eflags & IF_MASK)? 'I' : 'i',
            (deb.r->eflags & SF_MASK)? 'S' : 's',
            (deb.r->eflags & ZF_MASK)? 'Z' : 'z',
            (deb.r->eflags & AF_MASK)? 'A' : 'a',
            '?',
            (deb.r->eflags & CF_MASK)? 'C' : 'c' );

    dprint("CS=%04X   DS=%04X   SS=%04X   ES=%04X   FS=%04X   GS=%04X\n",
            deb.r->pmCS, deb.r->pmDS, deb.r->SS, deb.r->pmES, deb.r->pmFS, deb.r->pmGS );
}    

