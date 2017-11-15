;==============================================================================
;                                                                             |
;   File: i386.asm                                                            |
;                                                                             |
;   Date: 03/11/01                                                            |
;                                                                             |
;   Copyright (c) 2000 - 2001 Goran Devic                                     |
;                                                                             |
;   Author:     Goran Devic                                                   |
;                                                                             |
;==============================================================================
;
;   Module description:
;
;       This module contains all assembly code
;
;==============================================================================
; Define exported data and functions
;==============================================================================

global  IceIntHandlers
global  GetByte

;==============================================================================
; External definitions that this module uses
;==============================================================================

extern  InterruptHandler

;==============================================================================
; Constants
;==============================================================================

SEL_ICE_CS      equ     010h
SEL_ICE_DS      equ     018h

;==============================================================================
; Data
;==============================================================================

TempEIP:        dd      0
TempESP:        dd      0

;==============================================================================
;
;           I N T E R R U P T   H A N D L E R S 
;
;==============================================================================

; Define interrupt handler stubs taking care to abstract error code on the stack

%macro int_handler 2
%if %2 == 0
        push    dword 0         ; Fake error code if parameter 2 is zero (no EC)
%endif
        pushad                  ; Save all general registers
        mov     ecx, %1         ; Get the interrupt number; parameter 1
        jmp     IntCommon       ; Jump to the common handler
%endmacro

; CPU traps and faults

Intr00:      int_handler 000h, 0
Intr01:      int_handler 001h, 0
Intr02:      int_handler 002h, 0
Intr03:      int_handler 003h, 0
Intr04:      int_handler 004h, 0
Intr05:      int_handler 005h, 0
Intr06:      int_handler 006h, 0
Intr07:      int_handler 007h, 0

Intr08:      int_handler 008h, 1
Intr09:      int_handler 009h, 0
Intr0A:      int_handler 00Ah, 1
Intr0B:      int_handler 00Bh, 1
Intr0C:      int_handler 00Ch, 1
Intr0D:      int_handler 00Dh, 1
Intr0E:      int_handler 00Eh, 1
Intr0F:      int_handler 00Fh, 0

Intr10:      int_handler 010h, 0
Intr11:      int_handler 011h, 0
Intr12:      int_handler 012h, 0
Intr13:      int_handler 013h, 0
Intr14:      int_handler 014h, 0
Intr15:      int_handler 015h, 0
Intr16:      int_handler 016h, 0
Intr17:      int_handler 017h, 0

Intr18:      int_handler 018h, 0
Intr19:      int_handler 019h, 0
Intr1A:      int_handler 01Ah, 0
Intr1B:      int_handler 01Bh, 0
Intr1C:      int_handler 01Ch, 0
Intr1D:      int_handler 01Dh, 0
Intr1E:      int_handler 01Eh, 0
Intr1F:      int_handler 01Fh, 0

; Interrupts from PIC

Intr20:      int_handler 020h, 0
Intr21:      int_handler 021h, 0
Intr22:      int_handler 022h, 0
Intr23:      int_handler 023h, 0
Intr24:      int_handler 024h, 0
Intr25:      int_handler 025h, 0
Intr26:      int_handler 026h, 0
Intr27:      int_handler 027h, 0

Intr28:      int_handler 028h, 0
Intr29:      int_handler 029h, 0
Intr2A:      int_handler 02Ah, 0
Intr2B:      int_handler 02Bh, 0
Intr2C:      int_handler 02Ch, 0
Intr2D:      int_handler 02Dh, 0
Intr2E:      int_handler 02Eh, 0
Intr2F:      int_handler 02Fh, 0

; SYSCALL ineterrupt

Intr80:      int_handler 0x80, 0

; Table containing the addresses of our interrupt handlers

IceIntHandlers:
    dd  Intr00, Intr01, Intr02, Intr03, Intr04, Intr05, Intr06, Intr07
    dd  Intr08, Intr09, Intr0A, Intr0B, Intr0C, Intr0D, Intr0E, Intr0F

    dd  Intr10, Intr11, Intr12, Intr13, Intr14, Intr15, Intr16, Intr17
    dd  Intr18, Intr19, Intr1A, Intr1B, Intr1C, Intr1D, Intr1E, Intr1F

    dd  Intr20, Intr21, Intr22, Intr23, Intr24, Intr25, Intr26, Intr27
    dd  Intr28, Intr29, Intr2A, Intr2B, Intr2C, Intr2D, Intr2E, Intr2F

    times 080h-030h  dd Intr03
    dd Intr80
    times 100h-081h dd Intr03


; On the ring-0 stack:
;
;      (I) Running V86 code:             (II) Running PM code RING3:      (III) Running PM code RING0:
;     ___________________________        ______________________________   ______________________________
;     esp from tss * -|-
; 54             ---- | VM gs
; 50             ---- | VM fs
; 4C             ---- | VM ds
; 48             ---- | VM es             esp from tss * -|-              esp is from interrupted kernel (*)
; 44             ---- | VM ss                        ---- | PM ss                                    
; 40             VM esp                              PM esp                                          
; 3C             VM eflags                           PM eflags                        PM eflags
; 38             ---- | VM cs                        ---- | PM cs                     ---- | PM cs
; 34             VM eip                              PM eip                           PM eip
; 30             error_code or 0                     error_code or 0                  error_code or 0
;     <pushed by us>   .   .   .   .   .   .   .   .   .   .   .   .    .   .   .   .   .   .   .   .
; 2C             eax                                 eax                              eax
; 28  p          ecx                                 ecx                              ecx
; 24  u          edx                                 edx                              edx
; 20  s          ebx                                 ebx                              ebx
; 1C  h          ---                                 ---                              ---
; 18  a          ebp                                 ebp                              ebp
; 14  d          esi                                 esi                              esi
; 10   _______   edi                                 edi                              edi
; 0C             0       *                           ---- | PM ds                     ---- | PM ds
; 08             0       *                           ---- | PM fs                     ---- | PM fs
; 04             0       *                           ---- | PM gs                     ---- | PM gs
; ebp->          0       *                           ---- | PM es                     ---- | PM es
;     <copied by us>   .   .   .   .   .   .   .   .   .   .   .   .    .   .   .   .   .   .   .   .
;                ---- | VM ss                        ---- | PM ss                     ---- | PM ss
;                VM esp                              PM esp                           PM esp (*)
;
;
IntCommon:
        xor     eax, eax        ; Push the rest of segment/selector registers
        mov     ax, ds
        push    eax
        mov     ax, fs
        push    eax
        mov     ax, gs
        push    eax
        mov     ax, es
        push    eax

        mov     ax, SEL_ICE_DS  ; Load all selectors with kernel data selector
        mov     ds, ax
        mov     ax, SEL_ICE_DS
        mov     fs, ax
        mov     ax, SEL_ICE_DS
        mov     gs, ax
        mov     ax, SEL_ICE_DS
        mov     es, ax

; Depending on the mode of execution of the interrupted code, we need to do some entry adjustments
        mov     ebp, esp
        bt      dword [ebp+03Ch], 17            ; VM mode ?
        jnc     not_VM

        mov     eax, [ebp+04Ch]                 ; VM ds
        mov     [ebp+00Ch], eax
        mov     eax, [ebp+050h]                 ; VM fs
        mov     [ebp+008h], eax
        mov     eax, [ebp+054h]                 ; VM gs
        mov     [ebp+004h], eax
        mov     eax, [ebp+048h]                 ; VM es
        mov     [ebp+000h], eax
        push    dword [ebp+044h]                ; VM ss
        push    dword [ebp+040h]                ; VM esp
        jmp     entry
not_VM:
        bt      dword [ebp+038h], 0             ; Ring 3 code ?
        jnc     ring0
        push    dword [ebp+044h]                ; PM3 ss
        push    dword [ebp+040h]                ; PM3 esp
        jmp     entry
ring0:
        mov     ax, ss
        push    eax                             ; PM0 ss
        lea     eax, [ebp+040h]
        push    eax                             ; PM0 esp
entry:
; Push the address of the register structure that we just set up
        mov     eax, esp
        push    eax

; Push interrupt number on the stack and call our C handler

        push    ecx             
        cld                                     ; Set the direction bit
        call    InterruptHandler
        add     esp, 8

; Depending on the mode of execution of the interrupted code, we need to do some exit adjustments
        mov     ebp, esp
        bt      dword [ebp+03Ch], 17            ; VM mode ?
        jnc     exit_not_VM

        mov     eax, [ebp+00Ch]
        mov     [ebp+04Ch], eax                 ; VM ds
        mov     eax, [ebp+008h]
        mov     [ebp+050h], eax                 ; VM fs
        mov     eax, [ebp+004h]
        mov     [ebp+054h], eax                 ; VM gs
        mov     eax, [ebp+000h]
        mov     [ebp+048h], eax                 ; VM es
        pop     dword [ebp+040h]                ; VM esp
        pop     dword [ebp+044h]                ; VM ss
        jmp     exit
exit_not_VM:
        bt      dword [ebp+038h], 0             ; Ring 3 code ?
        jnc     exit_ring0
        pop     dword [ebp+040h]                ; PM3 esp
        pop     dword [ebp+044h]                ; PM3 ss
        jmp     exit
exit_ring0:
        ; In the ring 0, we are allowed to change esp only, not the ss. So if the esp has been
        ; changed, we need to use some trickery to end up where we want to be..
        ; We use the fact that in the kernel mode DS==SS==known value
        pop     dword [TempESP]                 ; PM0 esp
        add     esp, 4                          ; skip ss

        pop     ebx             ; Restore segment/selector registers
        mov     es, bx
        pop     ebx
        mov     gs, bx
        pop     ebx
        mov     fs, bx
        pop     ebx
        mov     ds, bx          ; DS should really be equal to SS

        popad                   ; Restore all general purpose registers
        add     esp, 4          ; Skip over the error code
        pop     dword [TempEIP] ; Pick up the return eip
        add     esp, 4          ; Skip over the PM0 cs (we should be in the same cs, anyways)
        popfd                   ; Pop eflags
        mov     esp, [TempESP]  ; Reload new esp
        jmp     dword [TempEIP] ; And jump to where we came from

exit:
; Return execution to the interrupted program

        pop     ebx             ; Restore segment/selector registers
        mov     es, bx
        pop     ebx
        mov     gs, bx
        pop     ebx
        mov     fs, bx
        pop     ebx
        mov     ds, bx

        popad                   ; Restore all general purpose registers
        add     esp, 4          ; Skip over the error code value
        iretd                   ; Return to the interrupted task


;==============================================================================
;
;           M I S C E L L A N E O U S    R O U T I N E S
;
;==============================================================================

;==============================================================================
;
;   BYTE ReadCRTC(WORD wBaseCRTC, BYTE index)
;
;   This VGA helper function reads a CRTC value from the specified index.
;
;   Where:
;       [ebp + 8]   base port number
;       [ebp +12]   byte index of a CRTC register
;
;==============================================================================
ReadCRTC:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        mov     eax, [ebp+12]
        out     dx, al
        inc     dx
        in      al, dx
        pop     ebp
        ret

;==============================================================================
;
;   void WriteCRTC(WORD wBaseCRTC, BYTE index, BYTE value)
;
;   This VGA helper function writes a CRTC value to the specified index register
;
;   Where:
;       [ebp + 8]   base port number
;       [ebp +12]   byte index of a CRTC register
;       [ebp +16]   new value
;
;==============================================================================
WriteCRTC:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        mov     eax, [ebp+12]
        out     dx, al
        inc     dx
        mov     eax, [ebp+16]
        out     dx, al
        pop     ebp
        ret
        
;==============================================================================
;
;   int GetByte( DWORD absOffset )
;
;   Reads a byte from an absolute memory location offset
;   Returns a BYTE if the address is present, or value greater than 0xFF
;   address is not present
;
;   Where:
;       [ebp + 8 ]        offset
;
;==============================================================================
GetByte:
        push    ebp
        mov     ebp, esp

        mov     ebx, [ebp+8]           ; Get the offset

        ; Get the byte from the memory, possibly page faulting
        ; if the memory address was not valid.  Anyhow, since
        ; we installed PF handler, we will return 0xFFFFFFFF
        ; in EAX register in that case

        xor     eax, eax
        mov     al, [ebx]
;        mov     al, gs:[ebx]
        nop
        nop
        nop

        pop     ebp
        ret


;==============================================================================
;
;   memset_w( BYTE *target, WORD filler, DWORD len )
;
;   This function is used from VGA text driver to fill in character/attribute
;   words.
;
;   Where:
;       [ebp+8]         target address
;       [ebp+12]        word filler
;       [ebp+16]        length IN WORDS !!
;
;==============================================================================
memset_w:
        push    ebp
        mov     ebp, esp

        mov     ax, ds
        mov     es, ax
        mov     edi, [ebp+8]
        mov     eax, [ebp+12]
        mov     ecx, [ebp+16]
        cld
        rep     stosw

        pop     ebp
        ret


;==============================================================================
;
;   void GetSysreg( TSysreg * pSys )
;
;   Reads in the rest of system registers
;
;   Where:
;       [ebp + 8 ]        sysreg array
;
;==============================================================================
GetSysreg:
        push    ebp
        mov     ebp, esp

        mov     ebx, [ebp + 8]
        mov     eax, cr0
        mov     [ebx+0], eax
        mov     eax, cr2
        mov     [ebx+4], eax
        mov     eax, cr3
        mov     [ebx+8], eax
        mov     eax, cr4
        mov     [ebx+12], eax

        mov     eax, dr0
        mov     [ebx+16], eax
        mov     eax, dr1
        mov     [ebx+20], eax
        mov     eax, dr2
        mov     [ebx+24], eax
        mov     eax, dr3
        mov     [ebx+28], eax
        mov     eax, dr6
        mov     [ebx+32], eax
        mov     eax, dr7
        mov     [ebx+36], eax

        pop     ebp
        ret

