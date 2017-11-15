;
;   Copyright (c) 2000 Goran Devic
;
;==============================================================================
; Define exported data and functions
;==============================================================================

global  IceIntHandlers

global  GetIDT
global  GetGDT
global  outp
global  inp
global  ReadCRTC
global  WriteCRTC
global  DisableInterrupts
global  EnableInterrupts
global  IssueInt3
global  GetByte
global  memset_w
global  HaltCpu
global  GetSysreg

;==============================================================================
; We need the following from the other files
;==============================================================================

extern  DebInterruptHandler


SEL_ICE_CS      equ     0x10
SEL_ICE_DS      equ     0x18
SEL_ICE_SS      equ     0x18
SEL_ICE_ES      equ     0x18
SEL_ICE_FS      equ     0x18
SEL_ICE_GS      equ     0   


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


Intr00:      int_handler 0x00, 0
Intr01:      int_handler 0x01, 0
Intr02:      int_handler 0x02, 0
Intr03:      int_handler 0x03, 0
Intr04:      int_handler 0x04, 0
Intr05:      int_handler 0x05, 0
Intr06:      int_handler 0x06, 0
Intr07:      int_handler 0x07, 0

Intr08:      int_handler 0x08, 1
Intr09:      int_handler 0x09, 0
Intr0A:      int_handler 0x0A, 1
Intr0B:      int_handler 0x0B, 1
Intr0C:      int_handler 0x0C, 1
Intr0D:      int_handler 0x0D, 1
Intr0E:      int_handler 0x0E, 1
Intr0F:      int_handler 0x0F, 0

Intr10:      int_handler 0x10, 0
Intr11:      int_handler 0x11, 0
Intr12:      int_handler 0x12, 0
Intr13:      int_handler 0x13, 0
Intr14:      int_handler 0x14, 0
Intr15:      int_handler 0x15, 0
Intr16:      int_handler 0x16, 0
Intr17:      int_handler 0x17, 0

Intr18:      int_handler 0x18, 0
Intr19:      int_handler 0x19, 0
Intr1A:      int_handler 0x1A, 0
Intr1B:      int_handler 0x1B, 0
Intr1C:      int_handler 0x1C, 0
Intr1D:      int_handler 0x1D, 0
Intr1E:      int_handler 0x1E, 0
Intr1F:      int_handler 0x1F, 0


Intr20:      int_handler 0x20, 0
Intr21:      int_handler 0x21, 0
Intr22:      int_handler 0x22, 0
Intr23:      int_handler 0x23, 0
Intr24:      int_handler 0x24, 0
Intr25:      int_handler 0x25, 0
Intr26:      int_handler 0x26, 0
Intr27:      int_handler 0x27, 0

Intr28:      int_handler 0x28, 0
Intr29:      int_handler 0x29, 0
Intr2A:      int_handler 0x2A, 0
Intr2B:      int_handler 0x2B, 0
Intr2C:      int_handler 0x2C, 0
Intr2D:      int_handler 0x2D, 0
Intr2E:      int_handler 0x2E, 0
Intr2F:      int_handler 0x2F, 0

; Table containing the addresses of our interrupt handlers

IceIntHandlers:
    dd  Intr00, Intr01, Intr02, Intr03, Intr04, Intr05, Intr06, Intr07
    dd  Intr08, Intr09, Intr0A, Intr0B, Intr0C, Intr0D, Intr0E, Intr0F

    dd  Intr10, Intr11, Intr12, Intr13, Intr14, Intr15, Intr16, Intr17
    dd  Intr18, Intr19, Intr1A, Intr1B, Intr1C, Intr1D, Intr1E, Intr1F

    dd  Intr20, Intr21, Intr22, Intr23, Intr24, Intr25, Intr26, Intr27
    dd  Intr28, Intr29, Intr2A, Intr2B, Intr2C, Intr2D, Intr2E, Intr2F


; On the ring-0 stack (from the current TSS):
;
;    (I) Running V86 code:             (II) Running PM code:
;   ___________________________        ______________________________
;   esp from tss * -|-
;              ---- | VM gs
;              ---- | VM fs
;              ---- | VM ds
;              ---- | VM es             esp from tss * -|-
;              ---- | VM ss                        ---- | PM ss
;              VM esp                              PM esp
;              VM eflags                           PM eflags
;              ---- | VM cs                        ---- | PM cs
;              VM eip                              PM eip
;              error_code or 0                     error_code or 0
;   <pushed by us>   .   .   .   .   .   .   .   .   .   .   .   .
;              eax                                 eax
;              ecx                                 ecx
;              edx                                 edx
;              ebx                                 ebx
;              ---                                 ---
;              ebp                                 ebp
;              esi                                 esi
;              edi                                 edi
;              0                                   ---- | PM ds
;              0                                   ---- | PM fs
;              0                                   ---- | PM gs
;              0                                   ---- | PM es

IntCommon:
        xor     eax, eax        ; Save the rest of segment/selector registers
        mov     ax, ds
        push    eax
        mov     ax, fs
        push    eax
        mov     ax, gs
        push    eax
        mov     ax, es
        push    eax

        mov     ax, SEL_ICE_DS  ; Load all selectors with kernel values
        mov     ds, ax
        mov     ax, SEL_ICE_FS
        mov     fs, ax
        mov     ax, SEL_ICE_GS
        mov     gs, ax
        mov     ax, SEL_ICE_ES
        mov     es, ax

; Push the address of the register structure that we just set up
        mov     eax, esp
        push    eax

; Push interrupt number on the stack and call our C handler

        push    ecx             
        cld                     ; Set the direction bit
        call    DebInterruptHandler
        add     esp, 8

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
;   void GetIDT( TDescriptor * )
;
;   Stores the 6-byte IDT descriptor to the given location.
;
;   Where:
;       [ebp + 8]   pointer to a 6-byte descriptor buffer
;
;==============================================================================
GetIDT:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        sidt    [edx]
        pop     ebp
        ret

;==============================================================================
;
;   void GetGDT( TDescriptor * )
;
;   Stores the 6-byte GDT descriptor to the given location.
;
;   Where:
;       [ebp + 8]   pointer to a 6-byte descriptor buffer
;
;==============================================================================
GetGDT:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        sgdt    [edx]
        pop     ebp
        ret

;==============================================================================
;
;   void outp(WORD wPort, BYTE bValue)
;
;   Outputs a byte value to a word sized port.
;
;   Where:
;       [ebp + 8]   port number
;       [ebp +12]   value
;
;==============================================================================
outp:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        mov     eax, [ebp+12]
        out     dx, al
        pop     ebp
        ret

;==============================================================================
;
;   BYTE inp(WORD wPort)
;
;   Inputs a byte value from a word sized port.
;
;   Where:
;       [ebp + 8]   port number
;
;==============================================================================
inp:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        xor     eax, eax
        in      al, dx
        pop     ebp
        ret

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
;   void DisableInterrupts(void)
;
;==============================================================================
DisableInterrupts:
        cli
        ret

;==============================================================================
;
;   void EnableInterrupts(void)
;
;==============================================================================
EnableInterrupts:
        sti
        ret

;==============================================================================
;
;   void IssueInt3(void)
;
;==============================================================================
IssueInt3:
        int3
        ret


;==============================================================================
;
;   DWORD GetByte( DWORD offset )
;
;   Reads a byte from a memory location offset
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
;   HaltCpu()
;
;==============================================================================
HaltCpu:
        cli
        halt


;==============================================================================
;
;   void GetSysreg( TSysreg * pSys )
;
;   Reads in system registers
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

