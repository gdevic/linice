/******************************************************************************
*                                                                             *
*   Module:     asm.h                                                         *
*                                                                             *
*   Date:       03/16/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This header file includes assembly macros that are too simple to
        deserve their space in a separate asm file.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/16/01   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ASM_H_
#define _ASM_H_

#include <asm/io.h>     // Include defines for I/O, memory access
#include <asm/msr.h>    // Access to machine-specific registers


#define SET_IDT(_idt)    asm("lidt  %0" : : "m" (_idt))
#define SET_GDT(_gdt)    asm("lgdt  %0" : : "m" (_gdt))
#define SET_TR(_tr)      asm("ltr   %0" : : "m" (_tr))
#define SET_LDT(_tr)     asm("lldt  %0" : : "m" (_tr))

#define GET_IDT(_idt)    asm("sidt  %0" : : "m" (_idt))
#define GET_GDT(_gdt)    asm("sgdt  %0" : : "m" (_gdt))
#define GET_TR(_tr)      asm("str   %0" : : "m" (_tr))
#define GET_LDT(_tr)     asm("sldt  %0" : : "m" (_tr))

#define GET_CR0(_reg)    __asm__ __volatile__("movl %%cr0,%0":"=r" (_reg));
#define SET_CR0(_reg)    __asm__ __volatile__("movl %0,%%cr0"::"r" (_reg) );
#define GET_CR2(_reg)    __asm__ __volatile__("movl %%cr2,%0":"=r" (_reg));
#define SET_CR2(_reg)    __asm__ __volatile__("movl %0,%%cr2"::"r" (_reg) );
#define GET_CR3(_reg)    __asm__ __volatile__("movl %%cr3,%0":"=r" (_reg));
#define SET_CR3(_reg)    __asm__ __volatile__("movl %0,%%cr3"::"r" (_reg) );
#define GET_CR4(_reg)    __asm__ __volatile__("movl %%cr4,%0":"=r" (_reg));
#define SET_CR4(_reg)    __asm__ __volatile__("movl %0,%%cr4"::"r" (_reg) );

#define GET_DR0(_reg)    __asm__("movl %%dr0,%0":"=r" (_reg));
#define SET_DR0(_reg)    __asm__("movl %0,%%dr0"::"r" (_reg) );
#define GET_DR1(_reg)    __asm__("movl %%dr1,%0":"=r" (_reg));
#define SET_DR1(_reg)    __asm__("movl %0,%%dr1"::"r" (_reg) );
#define GET_DR2(_reg)    __asm__("movl %%dr2,%0":"=r" (_reg));
#define SET_DR2(_reg)    __asm__("movl %0,%%dr2"::"r" (_reg) );
#define GET_DR3(_reg)    __asm__("movl %%dr3,%0":"=r" (_reg));
#define SET_DR3(_reg)    __asm__("movl %0,%%dr3"::"r" (_reg) );
#define GET_DR6(_reg)    __asm__("movl %%dr6,%0":"=r" (_reg));
#define SET_DR6(_reg)    __asm__("movl %0,%%dr6"::"r" (_reg) );
#define GET_DR7(_reg)    __asm__("movl %%dr7,%0":"=r" (_reg));
#define SET_DR7(_reg)    __asm__("movl %0,%%dr7"::"r" (_reg) );

#define INT1()           __asm__("int $1" ::);
#define INT3()           __asm__("int $3" ::);

#define CLTS()           __asm__("clts" ::);

#define FNCLEX()         __asm__("fnclex" ::);


static unsigned char
INB(unsigned short port) 
{ 
   volatile unsigned char  val;
   __asm__ __volatile__("inb %w1,%0\n\t"
                        : "=a" (val) : "d" (port) );
   return val;
}

static unsigned short
INW(unsigned short port) 
{ 
   volatile unsigned short val;
   __asm__ __volatile__("inw %w1,%w0\n\t"
                        : "=a" (val) : "d" (port) );
   return val;
}

static unsigned int
IND(unsigned short port) 
{ 
   volatile unsigned int val;
   __asm__ __volatile__("inl %w1,%w0\n\t"
                        : "=a" (val) : "d" (port) );
   return val;
}

#define OUTB(port,value) __asm__ __volatile__("outb %b0,%w1\n\t":: "a" (value) , "d" (port) ); }
#define OUTW(port,value) __asm__ __volatile__("outw %w0,%w1\n\t":: "a" (value) , "d" (port) ); }
#define OUTD(port,value) __asm__ __volatile__("outl %w0,%w1\n\t":: "a" (value) , "d" (port) ); }


#endif // _ASM_H_                   
