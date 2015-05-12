!define S_NAME "RemoteHelp"
AutoCloseWindow True
SilentInstall silent
WindowIcon off
XPStyle on
Icon "../tvnserver/res/idle.ico"
Name "${S_NAME}"
OutFile "${S_NAME}.exe"
RequestExecutionLevel user
SetCompressor /solid lzma

!include UAC.nsh
!include nsProcess.nsh
!include x64.nsh
!include WinVer.nsh
!include macros.nsh

Section
${IfNot} ${AtLeastWinXP}
 MessageBox MB_ICONSTOP|MB_OK "Windows XP or newer is required. Sorry"
 Quit
${EndIf}

InitPluginsDir
SetOutPath "$PLUGINSDIR"

File "..\Release\lcremotehelp.exe"
File "lcremotehelp.ini"

Call install_dfmirage

ExecWait "$PLUGINSDIR\lcremotehelp.exe"
Sleep 1000

SetOutPath "$TEMP"
SectionEnd

Function install_dfmirage
${!defineifexist} HAVE_DFMIRAGE_1_1 dfmirage_1.1.68.0
${!defineifexist} HAVE_DFMIRAGE_2_0 dfmirage_2.0.105.0

!ifdef HAVE_DFMIRAGE_1_1 & HAVE_DFMIRAGE_2_0
 File /r "dfmirage_1.1.68.0"
 File /r "dfmirage_2.0.105.0"
 File "devcon.exe"
 File "devcon64.exe"

 ${IfNot} ${AtLeastWin8}
  var /global devcon
  ${If} ${RunningX64}
   StrCpy $devcon "$PLUGINSDIR\devcon64.exe"
  ${Else}
   StrCpy $devcon "$PLUGINSDIR\devcon.exe"
  ${EndIf}

  nsExec::ExecToStack '"$devcon" find dfmirage'
  Pop $0
  Pop $1
  ${StrContains} $0 "Mirage Driver" $1
  StrCmp $0 "Mirage Driver" skip_driver_install
  !insertmacro Init "application"
  ${If} ${AtLeastWinVista}
   nsExec::Exec '"$devcon" install "$PLUGINSDIR\dfmirage_2.0.105.0\dfmirage.inf" dfmirage'
  ${Else}
   ${If} ${RunningX64}
    nsExec::Exec '"$devcon" install "$PLUGINSDIR\dfmirage_2.0.105.0\dfmirage.inf" dfmirage'
   ${Else}
    nsExec::Exec '"$devcon" install "$PLUGINSDIR\dfmirage_1.1.68.0\dfmirage.inf" dfmirage'
   ${EndIf}
  ${EndIf}
  skip_driver_install:
 ${EndIf}
!endif
FunctionEnd