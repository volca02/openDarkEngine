; The script to gracefully install openDarkEngine under Microsoft Windows.
; 
; This file is part of openDarkEngine project
; Copyright (C) 2005-2006 openDarkEngine team
; 
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
;
; $Id$
;--------------------------------

!include nsDialogs.nsh

ShowInstDetails show
ShowUninstDetails show
XPStyle on
SetCompressor /SOLID /FINAL lzma

Name "opde"
Caption "openDarkEngine"
BrandingText "openDarkEngine Team"

OutFile "opdeSetup.exe"

Var OgrePath
Var PythonPath

InstallDir C:\Games\openDarkEngine

InstallDirRegKey HKLM "Software\NSIS_opde" "Install_Dir"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

;--------------------------------

VIProductVersion "0.2.1.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "openDarkEngine"
;VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "openDarkEngine Team"
;VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© openDarkEngine Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "openDarkEngine"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "0.2.1"

;--------------------------------

Page Custom ShowWelcome LeaveWelcome
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

Function .onInit
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
	    Pop $R0
    
    StrCmp $R0 0 +3
    	MessageBox MB_OK|MB_ICONEXCLAMATION "The openDarkEngine installer is already running."
    	Abort
FunctionEnd

Function un.onInit
   	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
   		Pop $R0
    
    StrCmp $R0 0 +3
    	MessageBox MB_OK|MB_ICONEXCLAMATION "The openDarkEngine uninstaller is already running."
    	Abort
FunctionEnd

Function CheckOgre
    ReadEnvStr $OgrePath OGRE_HOME
    StrCmp $OgrePath "" 0 +3
		MessageBox MB_OK|MB_ICONSTOP "Ogre3D SDK not found. Please make sure Ogre3D SDK (http://www.ogre3d.org) is installed."
		Abort
FunctionEnd	

Function CheckPython
    ReadRegStr $PythonPath HKLM SOFTWARE\Python\PythonCore\2.5\InstallPath ""
    StrCmp $PythonPath "" 0 +3
		MessageBox MB_OK|MB_ICONSTOP "Python not found. Please make sure Python (http://www.python.org) is installed."
		Abort
FunctionEnd	

Function ShowWelcome
		nsDialogs::Create /NOUNLOAD 1018
		Pop $0
	
		${NSD_CreateLabel} 10 10u 100% 80u "Welcome to openDarkEngine installation. Please make sure you have already installed the following components:$\n$\n    - Ogre3D SDK (http://www.ogre3d.org)$\n    - Python (http://www.python.org)"
		Pop $0
	
		nsDialogs::Show
FunctionEnd

Function LeaveWelcome
    Call CheckOgre
    Call CheckPython
FunctionEnd

;--------------------------------

Section "openDarkEngine (required)"	

	SectionIn RO

	SetOutPath $INSTDIR
	File opde.exe
	File *.cfg
	SetOutPath $INSTDIR\Scripts
	File /r /x .svn ..\Scripts\*.*

	WriteRegStr HKLM SOFTWARE\NSIS_opde "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "DisplayName" "NSIS opde"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "NoRepair" 1
	WriteUninstaller "uninstall.exe"
  
SectionEnd

Section "Start Menu Shortcuts"

	SetOutPath $INSTDIR
	CreateDirectory "$SMPROGRAMS\openDarkEngine"
	CreateShortCut "$SMPROGRAMS\openDarkEngine\openDarkEngine (TDP, TG).lnk" "$INSTDIR\opde.exe" "-TG" "$INSTDIR\opde.exe" 0
	CreateShortCut "$SMPROGRAMS\openDarkEngine\openDarkEngine (TMA).lnk" "$INSTDIR\opde.exe" "-TMA" "$INSTDIR\opde.exe" 0
	CreateShortCut "$SMPROGRAMS\openDarkEngine\openDarkEngine (SS2).lnk" "$INSTDIR\opde.exe" "-SS2" "$INSTDIR\opde.exe" 0
	CreateShortCut "$SMPROGRAMS\openDarkEngine\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde"
	DeleteRegKey HKLM SOFTWARE\NSIS_opde

	; Remove files and uninstaller
	Delete $INSTDIR\opde.exe
	Delete $INSTDIR\*.cfg
	Delete $INSTDIR\*.log
	Delete $INSTDIR\uninstall.exe

	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\openDarkEngine\*.*"

	; Remove directories used
	RMDir "$SMPROGRAMS\openDarkEngine"
	RMDir /r "$INSTDIR\Scripts\"
	RMDir "$INSTDIR"


SectionEnd