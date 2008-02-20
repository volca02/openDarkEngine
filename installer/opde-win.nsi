; The script to gracefully install OpenDarkEngine under Microsoft Windows.
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

ShowInstDetails show
ShowUninstDetails show
XPStyle on
SetCompressor /SOLID /FINAL lzma

; The name of the installer
Name "opde"
Caption "OpenDarkEngine"
BrandingText "OpenDarkEngine Team"

; The file to write
OutFile "opdeSetup.exe"

; The default installation directory
InstallDir C:\Games\OpenDarkEngine

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_opde" "Install_Dir"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
;--------------------------------
;Version Information

  VIProductVersion "0.2.1.0"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "OpenDarkEngine"
  ;VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
  VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "OpenDarkEngine Team"
  ;VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© OpenDarkEngine Team"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "OpenDarkEngine"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "0.2.1"

;--------------------------------
; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "OpenDarkEngine (required)"

  SectionIn RO
  
  SetOutPath $INSTDIR
  File opde.exe
  File *.cfg
  SetOutPath $INSTDIR\Scripts
  File /r /x .svn ..\Scripts\*.*
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_opde "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "DisplayName" "NSIS opde"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\opde" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\OpenDarkEngine"
  CreateShortCut "$SMPROGRAMS\OpenDarkEngine\OpenDarkEngine (TDP, TG).lnk" "$INSTDIR\opde.exe" "-TG" "$INSTDIR\opde.exe" 0
  CreateShortCut "$SMPROGRAMS\OpenDarkEngine\OpenDarkEngine (TMA).lnk" "$INSTDIR\opde.exe" "-TMA" "$INSTDIR\opde.exe" 0
  CreateShortCut "$SMPROGRAMS\OpenDarkEngine\OpenDarkEngine (SS2).lnk" "$INSTDIR\opde.exe" "-SS2" "$INSTDIR\opde.exe" 0
  CreateShortCut "$SMPROGRAMS\OpenDarkEngine\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
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
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\OpenDarkEngine\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\OpenDarkEngine"
  RMDir /r "$INSTDIR\Scripts\"
  RMDir "$INSTDIR"
  

SectionEnd
