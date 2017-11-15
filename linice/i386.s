#==============================================================================
#                                                                             |
#   File: i386.asm                                                            |
#                                                                             |
#   Date: 09/11/00                                                            |
#                                                                             |
#   Copyright (c) 2000-2005 Goran Devic                                       |
#                                                                             |
#   Author:     Goran Devic                                                   |
#                                                                             |
#   This file is translated by Oleg Khudyakov to the gasm syntax.             |
#                                                                             |
#   This program is free software; you can redistribute it and/or modify      ;
#   it under the terms of the GNU General Public License as published by      ;
#   the Free Software Foundation; either version 2 of the License, or         ;
#   (at your option) any later version.                                       ;
#                                                                             ;
#   This program is distributed in the hope that it will be useful,           ;
#   but WITHOUT ANY WARRANTY; without even the implied warranty of            ;
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             ;
#   GNU General Public License for more details.                              ;
#                                                                             ;
#   You should have received a copy of the GNU General Public License         ;
#   along with this program; if not, write to the Free Software               ;
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   ;
#                                                                             ;
#==============================================================================
#
#   Module description:
#
#       This module contains all assembly code
#
#   NOTE: Dont forget to save/restore all used registers except eax!
#
#==============================================================================
# Define exported data and functions
#==============================================================================

.global IceIntHandlers

.global SyscallExecve

.global ReadCRTC
.global WriteCRTC
.global ReadSR
.global WriteSR
.global ReadMdaCRTC
.global WriteMdaCRTC
.global inp
.global getTR
.global SelLAR
.global GetRdtsc
.global FlushTLB

.global MemAccess_START
.global GetByte
.global GetDWORD
.global SetDWORD
.global SetByte
.global MemAccess_FAULT
.global MemAccess_END

.global strtolower
.global memset_w
.global memset_d
.global GetSysreg
.global SetSysreg
.global SetDebugReg
.global Outpb
.global Outpw
.global Outpd
.global Inpb
.global Inpw
.global Inpd

.global InterruptPoll

.global SpinlockTest
.global SpinUntilReset
.global SpinlockSet
.global SpinlockReset

.global GetKernelDS
.global GetKernelCS
.global sel_ice_ds              # This needs to be initialized at module load time!

.global Checksum1               # Protection - two checksum functions
.global Checksum2

#==============================================================================
# External definitions that this module uses
#==============================================================================

.extern InterruptHandler

.extern fStackLevel             # Contains 0 for the first Linice entry on the stack
.extern StackExtraBuffer        # Current extra buffer space on the stack
.extern MaxStackExtraBuffer     # Maximum extra buffer space on the stack

#==============================================================================
#
#           I N T E R R U P T   H A N D L E R S
#
#==============================================================================

.equ            MAGIC_ERROR, 0xCAFEBABE

# Define interrupt handler stubs taking care to abstract error code on the stack

.macro int_handler p1, p2=0
.if \p2 == 0
        pushl   $MAGIC_ERROR
.endif
        subl    $4,%esp         # Space for the chain_address
        pushal                  # Save all general registers
        movl    \p1, %ecx       # Get the interrupt number; parameter 1
        jmp     IntCommon       # Jump to the common handler
.endm


# CPU traps and faults

Intr00:      int_handler $0x000, 0
Intr01:      int_handler $0x001, 0
Intr02:      int_handler $0x002, 0
Intr03:      int_handler $0x003, 0
Intr04:      int_handler $0x004, 0
Intr05:      int_handler $0x005, 0
Intr06:      int_handler $0x006, 0
Intr07:      int_handler $0x007, 0

Intr08:      int_handler $0x008, 1
Intr09:      int_handler $0x009, 0
Intr0A:      int_handler $0x00A, 1
Intr0B:      int_handler $0x00B, 1
Intr0C:      int_handler $0x00C, 1
Intr0D:      int_handler $0x00D, 1
Intr0E:      int_handler $0x00E, 1
Intr0F:      int_handler $0x00F, 0

Intr10:      int_handler $0x010, 0
Intr11:      int_handler $0x011, 0
Intr12:      int_handler $0x012, 0
Intr13:      int_handler $0x013, 0
Intr14:      int_handler $0x014, 0
Intr15:      int_handler $0x015, 0
Intr16:      int_handler $0x016, 0
Intr17:      int_handler $0x017, 0

Intr18:      int_handler $0x018, 0
Intr19:      int_handler $0x019, 0
Intr1A:      int_handler $0x01A, 0
Intr1B:      int_handler $0x01B, 0
Intr1C:      int_handler $0x01C, 0
Intr1D:      int_handler $0x01D, 0
Intr1E:      int_handler $0x01E, 0
Intr1F:      int_handler $0x01F, 0

# Interrupts from PIC

Intr20:      int_handler $0x020, 0
Intr21:      int_handler $0x021, 0
Intr22:      int_handler $0x022, 0
Intr23:      int_handler $0x023, 0
Intr24:      int_handler $0x024, 0
Intr25:      int_handler $0x025, 0
Intr26:      int_handler $0x026, 0
Intr27:      int_handler $0x027, 0

Intr28:      int_handler $0x028, 0
Intr29:      int_handler $0x029, 0
Intr2A:      int_handler $0x02A, 0
Intr2B:      int_handler $0x02B, 0
Intr2C:      int_handler $0x02C, 0
Intr2D:      int_handler $0x02D, 0
Intr2E:      int_handler $0x02E, 0
Intr2F:      int_handler $0x02F, 0

# SYSCALL ineterrupt

Intr80:      int_handler $0x080, 0

# Table containing the addresses of our interrupt handlers

IceIntHandlers:
.long   Intr00, Intr01, Intr02, Intr03, Intr04, Intr05, Intr06, Intr07
.long   Intr08, Intr09, Intr0A, Intr0B, Intr0C, Intr0D, Intr0E, Intr0F

.long   Intr10, Intr11, Intr12, Intr13, Intr14, Intr15, Intr16, Intr17
.long   Intr18, Intr19, Intr1A, Intr1B, Intr1C, Intr1D, Intr1E, Intr1F

.long   Intr20, Intr21, Intr22, Intr23, Intr24, Intr25, Intr26, Intr27
.long   Intr28, Intr29, Intr2A, Intr2B, Intr2C, Intr2D, Intr2E, Intr2F

.rept   0x080-0x030
.long   Intr03
.endr

.long   Intr80

.rept   0x100-0x081
.long   Intr03
.endr

/*
# On the ring-0 stack:
#
#    (I) Running V86 code:             (II) Running PM code RING3:      (III) Running PM code RING0:
#   ___________________________        ______________________________   ______________________________
#   esp from tss * -|-
#              ---- | VM gs
#              ---- | VM fs
#              ---- | VM ds
#              ---- | VM es             esp from tss * -|-              esp is from interrupted kernel (*)
#              ---- | VM ss                        ---- | PM ss
#              VM esp                              PM esp
#              VM eflags                           PM eflags                        PM eflags
#              ---- | VM cs                        ---- | PM cs                     ---- | PM cs
#    ________  VM eip  __________________________  PM eip  _______________________  PM eip
#              error_code or 0                     error_code or 0                  error_code or 0
#              chain_address                       chain_address                    chain_address
#              eax                                 eax                              eax
#   p          ecx                                 ecx                              ecx
#   u          edx                                 edx                              edx
#   s          ebx                                 ebx                              ebx
#   h          ---                                 ---                              ---
#   a          ebp                                 ebp                              ebp
#   d          esi                                 esi                              esi
#    ________  edi  _____________________________  edi  __________________________  edi
#              0       *                           ---- | PM gs                     ---- | PM gs
#              0       *                           ---- | PM fs                     ---- | PM fs
#              0       *                           ---- | PM ds                     ---- | PM ds
#              0       *                           ---- | PM es                     ---- | PM es
#              ---- | VM ss                        ---- | PM ss                     ---- | PM ss
# frame ->     VM esp                              PM esp                           PM esp (*)
*/
        .struct 0
_esp:
        .struct _esp + 4
_ss:
        .struct _ss + 4
_es:
        .struct _es + 4
_ds:
        .struct _ds + 4
_fs:
        .struct _fs + 4
_gs:
        .struct _gs + 4
_genreg:
        .struct _genreg + 4*8
chain_address:
        .struct chain_address + 4
error_code:
        .struct error_code + 4
_eip:
        .struct _eip + 4
_cs:
        .struct _cs + 4
_eflags:
        .struct _eflags + 4
_esp_orig:
        .struct _esp_orig + 4
_ss_orig:
        .struct _ss_orig + 4
_es_orig:
        .struct _es_orig + 4
_ds_orig:
        .struct _ds_orig + 4
_fs_orig:
        .struct _fs_orig + 4
_gs_orig:
        .struct _gs_orig + 4

.text
IntCommon:
        xorl    %eax,%eax       # Push the rest of segment/selector registers
        movw    %gs,%ax
        pushl   %eax
        movw    %fs,%ax
        pushl   %eax
        movw    %ds,%ax
        pushl   %eax
        movw    %es,%ax
        pushl   %eax

        # Since we dont know what is the running kernel data selector at the compile time,
        # we leave the space and fill in this word at the module init time.
        #
        # The next instruction decodes this way:
        # 66 B8 xx yy      mov ax, SEL_ICE_DS
        .byte 0x66,0xB8
sel_ice_ds:
        .word 0                 # Load all selectors with kernel data selector

# We can not reload fs and gs with the default data selector. If we try, we randomly fault (under SuSE).
#       mov     gs, ax          ; Leave FS and GS as-is
#       mov     fs, ax
        movw    %ax,%ds
        movw    %ax,%es

# Depending on the mode of execution of the interrupted code, we need to do some entry adjustments
        subl    $8,%esp                         # Start of the frame
        movl    %esp,%ebp
        testl   $0x20000,_eflags(%ebp)          # 1 << 17
        jz      not_VM

        # --- VM86 ---
        movl    _gs_orig(%ebp),%eax
        movl    %eax,_gs(%ebp)
        movl    _fs_orig(%ebp),%eax
        movl    %eax,_fs(%ebp)
        movl    _ds_orig(%ebp),%eax
        movl    %eax,_ds(%ebp)
        movl    _es_orig(%ebp),%eax
        movl    %eax,_es(%ebp)
ring_3:  # --- VM86 + RING 3 ---
        movl    _ss_orig(%ebp),%eax
        movl    %eax,_ss(%ebp)
        movl    _esp_orig(%ebp),%eax
        movl    %eax,_esp(%ebp)
        jmp     entry
not_VM:
        testl   $1,_cs(%ebp)                    # Code selector is ring 3 ?
        jnz     ring_3

        # --- RING 0 ---
        movw    %ss,%ax
        movl    %eax,_ss(%ebp)
        leal    _esp_orig(%ebp),%eax
        movl    %eax,_esp(%ebp)
entry:
# This semaphore is needed to let interrupts that happen during the Linice run
# simply stack up (down?) onto where we currently are, instead of reserving more extra stack space
        btsl    $0,fStackLevel                  # First level of Linice interrupt handler?
        jc      skip_set_stack                  # If it is not (1), we dont need to allocate extra space

        movl    MaxStackExtraBuffer,%eax        # Get the constant max value
	movl	%eax,StackExtraBuffer
        subl    %eax,%esp                       # And give it that much on the actual stack
skip_set_stack:
# Push the address of the register structure that we just set up.
        pushl   %ebp                            # This is the parameter PTREGS pRegs

# Push interrupt number on the stack and call our C handler
        pushl   %ecx                            # This is the parameter DWORD nInt
        cld                                     # Set the direction bit
        call    InterruptHandler                # on return, eax is the chain address (or NULL)
        addl    $8,%esp                         # Restore esp to what was before the call

        cmpl    %ebp,%esp                       # Did we gave it any extra buffer?
        setzb   fStackLevel                     # Reset the semaphore if we did (first level)
        jz      skip_reset_stack
        addl    StackExtraBuffer,%esp           # Move esp to skip the buffer and position to the pRegs
        movl    %esp,%ebp                       # Make ebp and esp the same at this point
skip_reset_stack:
	movl	%eax,chain_address(%ebp)
        movl    %eax,%ecx                       # Keep the value in ecx register as well

# Depending on the mode of execution of the interrupted code, we need to do some exit adjustments
        testl   $0x20000,_eflags(%ebp)          # 1 << 17
        jz      exit_not_VM

        # --- VM86 ---
        movl    _es(%ebp),%eax
        movl    %eax,_es_orig(%ebp)
        movl    _ds(%ebp),%eax
        movl    %eax,_ds_orig(%ebp)
        movl    _fs(%ebp),%eax
        movl    %eax,_fs_orig(%ebp)
        movl    _gs(%ebp),%eax
        movl    %eax,_gs_orig(%ebp)
exit_ring_3:
        movl    _ss(%ebp),%eax
        movl    %eax,_ss_orig(%ebp)
        movl    _esp(%ebp),%eax
        movl    %eax,_esp_orig(%ebp)
        jmp     exit
exit_not_VM:
        testl   $1,_cs(%ebp)                    # Code is ring 3 ?
        jnz     exit_ring_3
exit_ring0:
        # --- RING 0 ---
exit:
        addl    $2*4,%esp                       # Skip esp/ss as we can't modify them here
        popl    %eax
        movw    %ax,%es
        popl    %eax
        movw    %ax,%ds
        popl    %eax
#       mov     fs, ax                          ; We should not reload FS and GS
        popl    %eax                            # They should be left with their default values
#       mov     gs, ax                          ; If we reload them, we fault under SuSE (?)

# Depending on the chain address and the error code, we need to return the right way
        orl     %ecx,%ecx
        jz      no_chain_return
chain_return:
        movl    error_code(%ebp),%eax
        cmpl    $MAGIC_ERROR, %eax
        je      chain_no_ec
        popal                   # Restore all general purpose registers
        ret                     # Return to the chain address and keep the error code

chain_no_ec:
        popal                   # Restore all general purpose registers
        ret     $4              # Return to the chain address and pop the error code

no_chain_return:
        popal                   # Restore all general purpose registers
        addl    $8,%esp         # Skip the chain_address and the error code value
        iretl                   # Return to the interrupted task

#==============================================================================
#
#           M I S C E L L A N E O U S    R O U T I N E S
#
#==============================================================================

.equ    MISC_INPUT, 0x03CC
.equ    CRTC_INDEX_MONO, 0x03B4
.equ    CRTC_INDEX_COLOR, 0x03D4

.equ    MDA_INDEX, 0x03B4
.equ    MDA_DATA, 0x03B5

#==============================================================================
#
#   GetCRTCAddr
#
#   Returns the CRTC base register
#
#==============================================================================
GetCRTCAddr:
        pushw   %ax
        movw    $MISC_INPUT, %dx
        inb     %dx, %al
        movw    $CRTC_INDEX_MONO, %dx
        testb   $1,%al
        jz      _mono
        movw    $CRTC_INDEX_COLOR, %dx
_mono:  popw    %ax
        ret

#==============================================================================
#
#   BYTE ReadCRTC(BYTE index)
#
#   This VGA helper function reads a CRTC value from a specified CRTC index
#   register.
#
#   Where:
#       [ebp + 8]   byte index of a CRTC register
#
#==============================================================================
ReadCRTC:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        call    GetCRTCAddr
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        inb     %dx, %al
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   void WriteCRTC(int index, int value)
#
#   This VGA helper function writes a CRTC value into a specified CRTC index
#   register
#
#   Where:
#       [ebp + 8]   byte index of a CRTC register
#       [ebp +12]   new value
#
#==============================================================================
WriteCRTC:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        call    GetCRTCAddr
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        movl    12(%ebp),%eax
        outb    %al, %dx
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   BYTE ReadMdaCRTC(BYTE index)
#
#   This MDA helper function reads a CRTC value from a specified CRTC index
#   register.
#
#   Where:
#       [ebp + 8]   byte index of a CRTC register
#
#==============================================================================
ReadMdaCRTC:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movw    $MDA_INDEX, %dx
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        inb     %dx, %al
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   void WriteMdaCRTC(int index, int value)
#
#   This MDA helper function writes a CRTC value into a specified CRTC index
#   register
#
#   Where:
#       [ebp + 8]   byte index of a CRTC register
#       [ebp +12]   new value
#
#==============================================================================
WriteMdaCRTC:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movw    $MDA_INDEX, %dx
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        movl    12(%ebp),%eax
        outb    %al, %dx
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   BYTE ReadSR(BYTE index)
#
#   This helper function reads a Sequencer register.
#
#   Where:
#       [ebp + 8]   byte index of a SR register
#
#==============================================================================
ReadSR:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movw    $0x3C4,%dx
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        inb     %dx, %al
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   void WriteSR(int index, int value)
#
#   This helper function writes a sequencer register
#
#   Where:
#       [ebp + 8]   byte index of a SR register
#       [ebp +12]   new value
#
#==============================================================================
WriteSR:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movw    $0x3C4,%dx
        movl    8(%ebp),%eax
        outb    %al, %dx
        incw    %dx
        movl    12(%ebp),%eax
        outb    %al, %dx
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   BYTE inp( WORD port )
#
#   Where:
#       [ebp + 8]   port index
#   Returns:
#       al - in(port)
#
#==============================================================================
inp:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movw    8(%ebp),%dx
        xorl    %eax,%eax
        inb     %dx, %al
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   DWORD getTR(void)
#
#   Returns:
#       ax - Task Register value
#
#==============================================================================
getTR:
        strw    %ax
        ret
/*
#==============================================================================
#
#   DWORD SelLAR(WORD sel)
#
#   Where:
#       [ebp + 8 ]      selector
#
#   Returns:
#       eax - Access rights
#           0 if the selector was invalid
#
#==============================================================================
*/
SelLAR:
        pushl   %ebp
        movl    %esp,%ebp
        larl    8(%ebp),%eax            # Load access rights of a selector
        jz      _sel_ok                 # Jump forth if the selector was ok
        xorl    %eax,%eax               # Reset to zero for invalid selector
_sel_ok:
        popl    %ebp
        ret

#==============================================================================
#
#   DWORD GetRdtsc(DWORD *[2])
#
#   Where:
#       [ebp + 8 ]      address of the 8-byte buffer to store the value
#
#   Returns:
#       eax - Rdtsc counter - Lower DWORD
#
#==============================================================================
GetRdtsc:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        pushl   %ebx

        movl    8(%ebp),%ebx            # Load the buffer address
        rdtsc                           # Get the time-stamp value is EDX:EAX
        movl    %eax,(%ebx)             # Store the low dword
        movl    %edx,4(%ebx)            # Store the high dword

        popl    %ebx                    # Return with eax - low dword
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   void FlushTLB(void)
#
#   Reloads CR3 thus flushing the TLB.
#
#==============================================================================
FlushTLB:
        pushl   %eax
        movl    %cr3, %eax
        movl    %eax, %cr3
        popl    %eax
        ret


#==============================================================================
#
#   DWORD Checksum1( DWORD start, DWORD len )
#
#   Calculates the sum of the code region.
#
#   Where:
#       [ebp + 8 ]      start
#       [ebp + 12 ]     len
#
#   Returns:
#       eax - SUM value
#
#==============================================================================
Checksum1:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx
        pushl   %ecx
        movl    8(%ebp),%ebx            # Load the starting address
        movl    12(%ebp),%ecx           # Load the len in bytes
        xorl    %eax,%eax               # Starting checksum value
_next_sum1:
        addl    %cs:(%ebx),%eax
        incl    %ebx                    # Next byte
        loop    _next_sum1              # Do it for the whole buffer
        popl    %ecx
        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   int GetByte( WORD sel, DWORD offset )
#
#   Reads a byte from a memory location sel:offset
#   Returns a BYTE if the address is present, or value greater than 0xFF
#   if the address is not present
#
#   Where:
#       [ebp + 8 ]      selector
#       [ebp + 12 ]     offset
#
#==============================================================================
#
#   We do some checks here, and leave others to PF handler and GPF handler
#

.equ    MEMACCESS_PF, 0x0F0FFFFFF       # Memory access caused a page fault
.equ    MEMACCESS_GPF, 0x0F1FFFFFF      # Memory access caused a GP fault
.equ    MEMACCESS_LIM, 0x0F2FFFFFF      # Memory access invalid limit

MemAccess_START:

GetByte:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx
        pushl   %gs

        # Perform a sanity check of the selector value
        movl    $MEMACCESS_GPF, %eax    # Assume failure with the access right
        larw    8(%ebp),%bx             # Load access rights of a selector
        jnz     _bad_selector_rb        # Exit if the selector is invisible or invalid

        movl    $MEMACCESS_LIM, %eax    # Assume failure with the segment limit
        lsll    8(%ebp),%ebx            # Load segment limit into ebx register
        cmpl    12(%ebp),%ebx           # Compare to what we requested
        jb      _bad_selector_rb        # Exit if the limit is exceeded

        movw    8(%ebp),%ax             # Get the selector
        movw    %ax,%gs                 # Store it in the GS
        movl    12(%ebp),%ebx           # Get the offset off that selector

        # Access the memory using the GS selector, possibly causing PF or GPF
        # since we installed handlers, they will return the correct error code

        xorl    %eax,%eax
        movb    %gs:(%ebx),%al
_bad_selector_rb:
        popl    %gs
        popl    %ebx
        popl    %ebp
        ret


#==============================================================================
#
#   unsigned int GetDWORD( WORD sel, DWORD offset )
#
#   Reads a DWORD from a memory location sel:offset
#
#   Where:
#       [ebp + 8 ]      selector
#       [ebp + 12 ]     offset
#
#==============================================================================
GetDWORD:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx
        pushl   %gs

        # Perform a sanity check of the selector value
        movl    $MEMACCESS_GPF, %eax    # Assume failure with the access right
        larw    8(%ebp),%bx             # Load access rights of a selector
        jnz     _bad_selector_rd        # Exit if the selector is invisible or invalid

        movl    $MEMACCESS_LIM, %eax    # Assume failure with the segment limit
        lsll    8(%ebp),%ebx            # Load segment limit into ebx register
        cmpl    12(%ebp),%ebx           # Compare to what we requested
        jb      _bad_selector_rd        # Exit if the limit is exceeded

        movw    8(%ebp),%ax             # Get the selector
        movw    %ax,%gs                 # Store it in the GS
        movl    12(%ebp),%ebx           # Get the offset off that selector

        # Access the memory using the GS selector, possibly causing PF or GPF
        # since we installed handlers, they will return the correct error code

        movl    %gs:(%ebx),%eax
_bad_selector_rd:
        popl    %gs
        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   void SetDWORD( WORD sel, DWORD offset, DWORD dwValue )
#
#   Sets a DWORD to a memory location at sel:offset
#
#   Where:
#       [ebp + 8 ]      selector
#       [ebp + 12 ]     offset
#       [ebp + 16 ]     value
#
#==============================================================================
SetDWORD:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx
        pushl   %gs

        # Perform a sanity check of the selector value
        movl    $MEMACCESS_GPF, %eax    # Assume failure with the access right
        larw    8(%ebp),%bx             # Load access rights of a selector
        jnz     _bad_selector_wd        # Exit if the selector is invisible or invalid

        movl    $MEMACCESS_LIM, %eax    # Assume failure with the segment limit
        lsll    8(%ebp),%ebx            # Load segment limit into ebx register
        cmpl    12(%ebp),%ebx           # Compare to what we requested
        jb      _bad_selector_wd        # Exit if the limit is exceeded

        movw    8(%ebp),%ax             # Get the selector
        movw    %ax,%gs                 # Store it in the GS
        movl    12(%ebp),%ebx           # Get the offset off that selector
        movl    16(%ebp),%eax           # Get the value to write

        # Access the memory using the GS selector, possibly causing PF or GPF
        # since we installed handlers, they will return the correct error code

        movl    %eax,%gs:(%ebx)
_bad_selector_wd:
        popl    %gs
        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   int SetByte( WORD sel, DWORD offset, BYTE byte )
#
#   Writes a byte to a memory location at sel:offset
#
#   Where:
#       [ebp + 8 ]      selector
#       [ebp + 12 ]     offset
#       [ebp + 16 ]     byte value
#
#==============================================================================
SetByte:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx
        pushl   %gs

        # Perform a sanity check of the selector value
        movl    $MEMACCESS_GPF, %eax    # Assume failure with the access right
        larw    8(%ebp),%bx             # Load access rights of a selector
        jnz     _bad_selector_wb        # Exit if the selector is invisible or invalid

        movl    $MEMACCESS_LIM, %eax    # Assume failure with the segment limit
        lsll    8(%ebp),%ebx            # Load segment limit into ebx register
        cmpl    12(%ebp),%ebx           # Compare to what we requested
        jb      _bad_selector_wb        # Exit if the limit is exceeded

        movw    8(%ebp),%ax             # Get the selector
        movw    %ax,%gs                 # Store it in the GS
        movl    12(%ebp),%ebx           # Get the offset off that selector

        movl    16(%ebp),%eax           # Get the byte value

        # Access the memory using the GS selector, possibly causing PF or GPF
        # since we installed handlers, they will return the correct error code

        movb    %al,%gs:(%ebx)              # Store the byte

_bad_selector_wb:
MemAccess_FAULT:
        popl    %gs
        popl    %ebx
        popl    %ebp
        ret

MemAccess_END:
        nop


#==============================================================================
#
#   memset_w( BYTE *target, WORD filler, DWORD len )
#
#   Fills a buffer with words.
#
#   Where:
#       [ebp+8]         target address
#       [ebp+12]        word filler
#       [ebp+16]        length IN WORDS !!
#
#==============================================================================
memset_w:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ecx
        pushl   %edi

        movl    8(%ebp),%edi
        movl    12(%ebp),%eax
        movl    16(%ebp),%ecx
        orl     %ecx,%ecx
        jz      _zero_w
        cld
        rep
        stosw
_zero_w:
        popl    %edi
        popl    %ecx
        popl    %ebp
        ret


#==============================================================================
#
#   memset_d( BYTE *target, DWORD filler, DWORD len )
#
#   Fills a buffer with dwords.
#
#   Where:
#       [ebp+8]         target address
#       [ebp+12]        dword filler
#       [ebp+16]        length IN DWORDS !!
#
#==============================================================================
memset_d:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ecx
        pushl   %edi

        movl    8(%ebp),%edi
        movl    12(%ebp),%eax
        movl    16(%ebp),%ecx
        orl     %ecx,%ecx
        jz      _zero_d
        cld
        rep
        stosl
_zero_d:
        popl    %edi
        popl    %ecx
        popl    %ebp
        ret


#==============================================================================
#
#   strtolower(char *str)
#
#   Where:
#       [ebp+8]         string
#
#==============================================================================
strtolower:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx

        movl    8(%ebp),%ebx
_loop:
        movb    (%ebx),%al
        orb     %al,%al
        jz      _exit
        cmpb    $'A', %al
        jl      _next
        cmpb    $'Z', %al
        jg      _next
        addb    $'a'-'A', %al
        movb    %al,(%ebx)
_next:
        incl    %ebx
        jmp     _loop
_exit:
        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   void GetSysreg( TSysreg * pSys )
#
#   Reads in the rest of system registers
#
#   Where:
#       [ebp + 8 ]        pointer to the SysReg storage
#
#==============================================================================
GetSysreg:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx

        movl    8(%ebp),%ebx
        movl    %cr0, %eax
        movl    %eax,0(%ebx)
        movl    %cr2, %eax
        movl    %eax,4(%ebx)
        movl    %cr3, %eax
        movl    %eax,8(%ebx)
        movl    %cr4, %eax
        movl    %eax,12(%ebx)

        movl    %dr0, %eax
        movl    %eax,16(%ebx)
        movl    %dr1, %eax
        movl    %eax,20(%ebx)
        movl    %dr2, %eax
        movl    %eax,24(%ebx)
        movl    %dr3, %eax
        movl    %eax,28(%ebx)
        movl    %dr6, %eax
        movl    %eax,32(%ebx)
        movl    %dr7, %eax
        movl    %eax,36(%ebx)

        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   void SetSysreg( TSysreg * pSys )
#
#   Writes system registers back. Only registers that matter are written.
#
#   Where:
#       [ebp + 8 ]        pointer to the SysReg storage
#
#==============================================================================
SetSysreg:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx

        movl    8(%ebp),%ebx
        movl    0(%ebx),%eax
        movl    %eax, %cr0
#       mov     eax, [ebx+4 ]   ; No need to change Page Fault Linear Address
#       mov     cr2, eax
#       mov     eax, [ebx+8 ]   ; No need to change PDBR
#       mov     cr3, eax
        movl    12(%ebx),%eax
        movl    %eax, %cr4

        movl    16(%ebx),%eax
        movl    %eax, %dr0
        movl    20(%ebx),%eax
        movl    %eax, %dr1
        movl    24(%ebx),%eax
        movl    %eax, %dr2
        movl    28(%ebx),%eax
        movl    %eax, %dr3
        movl    32(%ebx),%eax
        movl    %eax, %dr6
        movl    36(%ebx),%eax
        movl    %eax, %dr7

        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   void SetDebugReg( TSysreg * pSys )
#
#   Writes debug registers back.
#
#   Where:
#       [ebp + 8 ]        pointer to the SysReg storage
#
#==============================================================================
SetDebugReg:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ebx

        movl    8(%ebp),%ebx

        movl    16(%ebx),%eax
        movl    %eax, %dr0
        movl    20(%ebx),%eax
        movl    %eax, %dr1
        movl    24(%ebx),%eax
        movl    %eax, %dr2
        movl    28(%ebx),%eax
        movl    %eax, %dr3
        movl    32(%ebx),%eax
        movl    %eax, %dr6
        movl    36(%ebx),%eax
        movl    %eax, %dr7

        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   void Outpb( DWORD port, DWORD value)
#   void Outpw( DWORD port, DWORD value)
#   void Outpd( DWORD port, DWORD value)
#
#   Where:
#       [ebp + 8 ]        port
#       [ebp + 12 ]       value
#
#==============================================================================
Outpb:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        pushl   %eax
        movl    8(%ebp),%edx
        movl    12(%ebp),%eax
        outb    %al,%dx
        popl    %eax
        popl    %edx
        popl    %ebp
        ret

Outpw:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        pushl   %eax
        movl    8(%ebp),%edx
        movl    12(%ebp),%eax
        outw    %ax,%dx
        popl    %eax
        popl    %edx
        popl    %ebp
        ret

Outpd:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        pushl   %eax
        movl    8(%ebp),%edx
        movl    12(%ebp),%eax
        outl    %eax,%dx
        popl    %eax
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   DWORD Inpb( DWORD port )
#   DWORD Inpw( DWORD port )
#   DWORD Inpd( DWORD port )
#
#   Where:
#       [ebp + 8 ]      port
#   Returns:
#       eax             value
#
#==============================================================================
Inpb:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        xorl    %eax,%eax
        inb     %dx, %al
        popl    %edx
        popl    %ebp
        ret

Inpw:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        xorl    %eax,%eax
        inw     %dx, %ax
        popl    %edx
        popl    %ebp
        ret

Inpd:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        xorl    %eax,%eax
        inl     %dx, %eax
        popl    %edx
        popl    %ebp
        ret


#==============================================================================
#
#   void InterruptPoll(void)
#
#==============================================================================
InterruptPoll:
        hlt
        ret

#==============================================================================
#
#   Kernel code and data information functions
#
#==============================================================================
GetKernelDS:
        movw    %ds,%ax
        ret

GetKernelCS:
        movw    %cs,%ax
        ret

#==============================================================================
#
#   Spinlock functions:
#
#   DWORD SpinlockTest(DWORD *pSpinlock);
#   DWORD SpinUntilReset(DWORD *pSpinlock);
#   void  SpinlockSet(DWORD *pSpinlock);
#   void  SpinlockReset(DWORD *pSpinlock);
#
#==============================================================================

SpinUntilReset:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
_spin1: cmpl    $0,(%edx)
        jnz     _spin1
        popl    %edx
        popl    %ebp
        ret

SpinlockSet:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        movl    $1,(%edx)
        popl    %edx
        popl    %ebp
        ret

SpinlockReset:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        movl    $0,(%edx)
        popl    %edx
        popl    %ebp
        ret

SpinlockTest:
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edx
        movl    8(%ebp),%edx
        movl    (%edx),%eax
        popl    %edx
        popl    %ebp
        ret

#==============================================================================
#
#   DWORD Checksum2( DWORD start, DWORD end )
#
#   Calculates the sum of the code region. This function is intentionally
#   written differently from Checksum1 and resides somewhat away from it.
#
#   Where:
#       [ebp + 8 ]      start
#       [ebp + 12 ]     end
#
#   Returns:
#       eax - SUM value
#
#==============================================================================
Checksum2:
        pushl   %ebp
        movl    %esp,%ebp
        xorl    %eax,%eax               # Starting checksum value
        pushl   %ebx
        pushl   %edx
        movl    8(%ebp),%ebx            # Load the starting address
        movl    12(%ebp),%edx           # Load the ending address
_next_sum2:
        addl    %cs:(%ebx),%eax
        addl    $1,%ebx                 # Next byte
        cmpl    %edx,%ebx               # Did we reach the end?
        jnz     _next_sum2              # Jump back if we did not
        popl    %edx
        popl    %ebx
        popl    %ebp
        ret

#==============================================================================
#
#   Handler for the execve system call hook.
#
#   We do it this way since we need to preserve the registers on the stack
#   within the original call and our hook.
#
#   Look in process.h:
#       asmlinkage int sys_execve(struct pt_regs regs)
#
#==============================================================================
.extern sys_execve
.extern SyscallExecveHook

# Calling our handler _before_ the original execve

SyscallExecve:
        call    SyscallExecveHook
        jmp     *sys_execve
