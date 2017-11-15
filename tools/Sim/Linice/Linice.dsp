# Microsoft Developer Studio Project File - Name="Linice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Linice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Linice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Linice.mak" CFG="Linice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Linice - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Linice - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Linice"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Linice - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /X /I "$(LINICE_ROOT)\linice\include" /I "$(LINICE_ROOT)\include" /I "$(LINICE_ROOT)\bin-2.4" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MODULE" /D "SIM" /FD /c
# SUBTRACT CPP /Ot /Oa /Ow /Og /Oi /Os /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\linice.lib"

!ELSEIF  "$(CFG)" == "Linice - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /X /I "$(LINICE_ROOT)\linice\include" /I "$(LINICE_ROOT)\include" /I "$(LINICE_ROOT)\bin-2.4" /D "DBG" /D "_LIB" /D "MODULE" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "SIM" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"linice.lib"

!ENDIF 

# Begin Target

# Name "Linice - Win32 Release"
# Name "Linice - Win32 Debug"
# Begin Group "include"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\ice-ioctl.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\ice-keycode.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\ice-limits.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\ice-symbols.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\ice-types.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\loader.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\stab_gnu.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\include\stabs.h"
# End Source File
# End Group
# Begin Group "linice"

# PROP Default_Filter "*.c"
# Begin Group "include No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\clib.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\ctype.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\debug.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\disassembler.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\disassemblerdata.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\disassemblerdefines.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\errno.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\font.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\ibm-pc.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\ice.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\intel.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\ioctl.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\lists.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\module-header.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\module-symbols.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\pcihdr.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\stdarg.h"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\include\string.h"
# End Source File
# End Group
# Begin Group "command"

# PROP Default_Filter "c"
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\blockops.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\breakpoints.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\code.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\command.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\customization.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\data.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\debugger.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\disassembler-bytelen.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\disassembler-ea.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\disassembler.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\edlin.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\evalex.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\flow.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\ioport.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\lists.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\locals.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\page.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\pci.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\registers.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\stack.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\sysinfo.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\watch.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\command\windowcontrol.c"
# End Source File
# End Group
# Begin Group "input"

# PROP Default_Filter "c"
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\input\input.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\input\keyboard.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\input\mouse.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\input\vt100.c"
# End Source File
# End Group
# Begin Group "output"

# PROP Default_Filter "c"
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\font.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\lfb.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\mda.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\output.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\vga.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\vt100o.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\output\window.c"
# End Source File
# End Group
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\apic.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\context.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\ctype.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\driver.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\errors.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\extend.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\history.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\init.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\interrupt.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\malloc.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\memaccess.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\messages.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\printf.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\printk.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\proc.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\serial.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\string.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\symbols.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\symbolTable.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\syscall.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\task.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\types.c"
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linice\typesprint.c"
# End Source File
# End Group
# Begin Group "bin"

# PROP Default_Filter "c;h"
# Begin Source File

SOURCE="$(LINICE_ROOT)\bin\iceface.h"
# End Source File
# End Group
# End Target
# End Project
