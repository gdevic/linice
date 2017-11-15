;==============================================================================
;                                                                             |
;   File: endasm.asm                                                          |
;                                                                             |
;   Date: 08/09/04                                                            |
;                                                                             |
;   Copyright (c) 2000-2005 Goran Devic                                       |
;                                                                             |
;   Author:     Goran Devic                                                   |
;                                                                             ;
;   This program is free software; you can redistribute it and/or modify      ;
;   it under the terms of the GNU General Public License as published by      ;
;   the Free Software Foundation; either version 2 of the License, or         ;
;   (at your option) any later version.                                       ;
;                                                                             ;
;   This program is distributed in the hope that it will be useful,           ;
;   but WITHOUT ANY WARRANTY; without even the implied warranty of            ;
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             ;
;   GNU General Public License for more details.                              ;
;                                                                             ;
;   You should have received a copy of the GNU General Public License         ;
;   along with this program; if not, write to the Free Software               ;
;   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   ;
;                                                                             ;
;==============================================================================
;
;   Module description:
;
;       This module contains the assembly source code that needs to reside
;       at the end of the module, outside the checksum code section
;
;       This is necessary since this code is being modified.
;
;==============================================================================
; Define exported data and functions
;==============================================================================

;==============================================================================
;
;   Hook the task switcher: insert our handler address so the switcher will return to it
;
;   Look the file task.c for the detailed description on how the task switcher hook is operated.
;
;==============================================================================
global  TaskSwitchHookBuffer
global  TaskSwitchHookBufferKernelCodeLine

extern  SwitchHandler                   ; Our inserted task switcher handler
extern  TaskSwitchNewRet


AsmTaskSwitchHandler:
        pushad                          ; Save all registers
        pushf
        call    SwitchHandler           ; Run our task switch function next
        popf
        popad
        ret

TaskSwitchHookBuffer:
        push    dword AsmTaskSwitchHandler      ; Insert our handler onto the return stack

TaskSwitchHookBufferKernelCodeLine:
        ; Buffer initially stores 16 NOP's over which we embed original instruction from the "switch_to"
        db      90h, 90h, 90h, 90h, 90h, 90h, 90h, 90h,  90h, 90h, 90h, 90h, 90h, 90h, 90h, 90h

        jmp     [TaskSwitchNewRet]              ; Continue into the __switch_to...

