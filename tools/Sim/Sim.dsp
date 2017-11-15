# Microsoft Developer Studio Project File - Name="Sim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Sim - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sim.mak" CFG="Sim - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sim - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sim - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Sim"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sim - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /I "$(LINICE_ROOT)\include" /I "$(LINICE_ROOT)\bin-2.4" /D "NDEBUG" /D "WIN32" /D "WINDOWS" /D "_MBCS" /D "SIM" /YX /FD /c
# SUBTRACT CPP /Ox
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /libpath:"Linice\linice.lib"

!ELSEIF  "$(CFG)" == "Sim - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /I "$(LINICE_ROOT)\include" /I "$(LINICE_ROOT)\bin-2.4" /D "WINDOWS" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "SIM" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 linice.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"Linice"

!ENDIF 

# Begin Target

# Name "Sim - Win32 Release"
# Name "Sim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(LINICE_ROOT)\linsym\install.c"
# End Source File
# Begin Source File

SOURCE=.\sim.c
# End Source File
# Begin Source File

SOURCE=.\simextension.c
# End Source File
# Begin Source File

SOURCE=.\simface.c
# End Source File
# Begin Source File

SOURCE=.\simgui.c
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\linsym\symbols.c"
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE="$(LINICE_ROOT)\bin\iceface.h"
# End Source File
# Begin Source File

SOURCE=.\sim.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sim.ico
# End Source File
# Begin Source File

SOURCE=.\sim.rc
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter "h"
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
# End Target
# End Project
