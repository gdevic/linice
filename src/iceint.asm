;------------------------------------------------------------------------------
; Set of interrupt handlers that are active when debugger is on
;------------------------------------------------------------------------------

global  IceIntHandlers

extern  IceInterrupt


PIC1    equ     0x20
PIC2    equ     0xA0
PICACK  equ     0x20


%macro  int_handler 2
%if %2 == 0
        push    dword 0
%endif
        pushad
        cld                     ; Set the direction bit
        push    dword %1
        call    IceInterrupt
        add     esp, 4
%if %1 >= 0x28
        mov     dx, PIC2        ; Ack PIC 2
        mov     al, PICACK
        out     (dx), al
%endif
%if %1 >= 0x20
        mov     dx, PIC1        ; Ack PIC 1
        mov     al, PICACK
        out     (dx), al
%endif
        popad
%if %2 == 0
        add     esp, 4
%endif
        iretd
%endmacro


; On the ring-0 stack:
; (We will always be running protected mode ring 0)
;______________________________
; ss:esp ->  PM eflags
;            ---- | PM cs
;            PM eip
;            error_code or 0
;            eax
;            ecx
;            edx
;            ebx
;            ---
;            ebp
;            esi
;            edi
;

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


IceIntHandlers:
    dd  Intr00, Intr01, Intr02, Intr03, Intr04, Intr05, Intr06, Intr07
    dd  Intr08, Intr09, Intr0A, Intr0B, Intr0C, Intr0D, Intr0E, Intr0F

    dd  Intr10, Intr11, Intr12, Intr13, Intr14, Intr15, Intr16, Intr17
    dd  Intr18, Intr19, Intr1A, Intr1B, Intr1C, Intr1D, Intr1E, Intr1F

    dd  Intr20, Intr21, Intr22, Intr23, Intr24, Intr25, Intr26, Intr27
    dd  Intr28, Intr29, Intr2A, Intr2B, Intr2C, Intr2D, Intr2E, Intr2F


