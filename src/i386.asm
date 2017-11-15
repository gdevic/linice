
; Int 0     Divide error
; Int 1     Debug exception
; Int 3     Breakpoint
; Int 6     Invalid opcode
; Int 8     Double fault
; Int 10    Invalid TSS
; Int 13    General protection fault
; Int 33    Keyboard interrupt

global  Interrupt_0
global  Interrupt_1
global  Interrupt_3
global  Interrupt_6
global  Interrupt_8
global  Interrupt_10
global  Interrupt_13
global  Interrupt_33


global  GetIDT
global  SetIDT             
global  IssueInt3
global  outp
global  inp
global  DisableInterrupts
global  EnableInterrupts

extern  DebInterruptHandler


SEL_ICE_CS      equ     0x10
SEL_ICE_DS      equ     0x18
SEL_ICE_SS      equ     0x18
SEL_ICE_ES      equ     0x18
SEL_ICE_FS      equ     0x18
SEL_ICE_GS      equ     0   


;------------------------------------------------------------------------------
; Set of new (injected) exception handlers - we simply hook all of them :-)
; and later figure out if we are going to handle them or not
;------------------------------------------------------------------------------

; Define interrupt handler stub for ints without error code

%macro int_handler_no_ec 1
        push    dword 0         ; Fake error code
        pushad                  ; Save all general registers
        mov     ecx, %1         ; Get the interrupt number
        jmp     IntCommon       ; Jump to the common handler
%endmacro

; Define interrupt handler stub for ints with error code

%macro int_handler 1
        pushad                  ; Save all general registers
        mov     ecx, %1         ; Get the interrupt number
        jmp     IntCommon       ; Jump to the common handler
%endmacro

; Define common interrupt handler stubs

Interrupt_0:  int_handler_no_ec   0
Interrupt_1:  int_handler_no_ec   1
Interrupt_3:  int_handler_no_ec   3
Interrupt_6:  int_handler_no_ec   6
Interrupt_8:  int_handler         8
Interrupt_10: int_handler         10
Interrupt_13: int_handler         13
Interrupt_33: int_handler_no_ec   33


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
;
;   ecx = int#

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

        cld                     ; Set the direction bit
        push    ecx             
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


GetIDT:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        sidt    [edx]
        pop     ebp
        ret

SetIDT:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        lidt    [edx]
        pop     ebp
        ret

outp:
        push    ebp
        mov     ebp, esp
        mov     eax, [ebp+8]
        mov     edx, [ebp+12]
        out     dx, al
        pop     ebp
        ret

inp:
        push    ebp
        mov     ebp, esp
        mov     edx, [ebp+8]
        xor     eax, eax
        in      al, dx
        pop     ebp
        ret

DisableInterrupts:
        cli
        ret

EnableInterrupts:
        sti
        ret

IssueInt3:
        int3
        ret

