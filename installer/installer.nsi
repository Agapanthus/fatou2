# TODO: Not working...


##############################################
# Start

!define PRODUCT_NAME "Fatou"
!define PRODUCT_VERSION  "v0.1.0"
!define PRODUCT_WEB_SITE "http://fenchel.dev/fatou"
!define PRODUCT_DIR_REGKEY "Software\fenchel\fatou"
!define PRODUCT_PUBLISHER "Eric Skaliks"
!define PRODUCT_GROUP "fenchel"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

!define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_DIR_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"

!define MEMENTO_REGISTRY_ROOT ${PRODUCT_UNINST_ROOT_KEY}
!define MEMENTO_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"

ManifestDPIAware true
Unicode true

Var ReinstallType
Var UninstallLog


##############################################
# General

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"

# define name of installer
OutFile .\fatou-${PRODUCT_VERSION}-installer.exe

# define installation directory
#InstallDir $DESKTOP
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"

InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
SetCompressor lzma

ShowInstDetails show
ShowUnInstDetails show
SetOverwrite ifdiff

# perform a CRC on the installer before allowing an install
CRCCheck On
BrandingText "${PRODUCT_NAME}"

##############################################
# Modern UI

!include helpers\nsProcess.nsh
!include WinVer.nsh
!include MUI2.nsh
!include Memento.nsh

# MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "favicon.ico"
!define MUI_UNICON "favicon.ico"
!define MUI_COMPONENTSPAGE_SMALLDESC

; Banner (welcome and finish page) for installer
!define MUI_WELCOMEFINISHPAGE_BITMAP "branding.bmp"
; Banner for uninstaller
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "branding.bmp"


# Installer pages
; Welcome page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageFastUpdatePre
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME

; Optional update page
Page custom PageReinstall PageLeaveReinstall

; License page
!define MUI_LICENSEPAGE_BUTTON $(^NextBtn)
!define MUI_LICENSEPAGE_TEXT_BOTTOM "$(License_NextText)"
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageSkipPre
!insertmacro MUI_PAGE_LICENSE "../public/license.txt"

; Components page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageComponentsPre
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageDirectoryPre
!insertmacro MUI_PAGE_DIRECTORY

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageFastUpdatePre
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION AppExecAs
!define MUI_FINISHPAGE_LINK "$(Link_VisitWebsite)"
!define MUI_FINISHPAGE_LINK_LOCATION "https://fenchel.dev/fatou"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH


# Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH


###### languages

!addincludedir "languages"
!define VLC_LANGFILE_FALLBACK "EnglishExtra.nsh"

; Language files (first language is default)
!insertmacro MUI_LANGUAGE "English"
!insertmacro LANGFILE_INCLUDE "EnglishExtra.nsh"
!insertmacro MUI_LANGUAGE "German"
!insertmacro LANGFILE_INCLUDE_WITHDEFAULT "GermanExtra.nsh" ${VLC_LANGFILE_FALLBACK}

# Reserve files for solid compression
!insertmacro MUI_RESERVEFILE_LANGDLL

InstType $Name_InstTypeRecommended
InstType $Name_InstTypeMinimum
InstType $Name_InstTypeFull

!include helpers\install.nsh
!include helpers\utils.nsh

##############################################
# core installer

${MementoSection} "$(Name_Section01)" SEC01
  SectionIn 1 2 3 RO
  SetShellVarContext all
  SetOutPath "$INSTDIR"

  DetailPrint "$(Detail_CheckProcesses)"
  Call CheckRunningProcesses

  ; Remove previous version first, if this is update
  ${If} $ReinstallType == "1"
    FileOpen $UninstallLog "$INSTDIR\uninstall.log" r
    UninstallLoop:
      ClearErrors
      FileRead $UninstallLog $R0
      IfErrors UninstallEnd
      Push $R0
      Call TrimNewLines
      Pop $R0
      Delete "$INSTDIR\$R0"
      Goto UninstallLoop
    UninstallEnd:
    FileClose $UninstallLog
    Delete "$INSTDIR\uninstall.log"
    Delete "$INSTDIR\uninstall.exe"
    Push "\"
    Call RemoveEmptyDirs
  ${EndIf}

  !insertmacro OpenUninstallLog
  
  !insertmacro InstallFile fatou.exe

  ; All dlls, paks
  !insertmacro InstallFile *.dll
  !insertmacro InstallFile *.pak
  !insertmacro InstallFile *.dat
  !insertmacro InstallFile *.bin
  !insertmacro InstallFile *.json

  ; subfolders
  !insertmacro InstallFolder locales
  !insertmacro InstallFolder swiftshader

  !insertmacro CloseUninstallLog


  WriteRegStr HKCR Applications\fatou.exe "" ""
  WriteRegStr HKCR Applications\fatou.exe "FriendlyAppName" "${PRODUCT_NAME}"

${MementoSectionEnd}


${MementoSection} "$(Name_Section02b)" SEC02b
  SectionIn 1 2 3
  CreateShortCut "$DESKTOP\Fatou.lnk" \
    "$INSTDIR\fatou.exe" ""
${MementoSectionEnd}


${MementoSection} "$(Name_Section07)" SEC07
  SectionIn 1 3
  !insertmacro MacroAllExtensions AddContextMenu
  !insertmacro AddContextMenuExt "Directory"
${MementoSectionEnd}


#${MementoUnselectedSection} "$(Name_Section08)" SEC08
#  !insertmacro delprefs
#${MementoSectionEnd}


${MementoSectionDone}


; Installer section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "$(Desc_Section01)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02b} "$(Desc_Section02b)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC07} "$(Desc_Section07)"
  #!insertmacro MUI_DESCRIPTION_TEXT ${SEC08} "$(Desc_Section08)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

 
### Start function
Function .onInit
  SetRegView 64

  ${MementoSectionRestore}

  !include "x64.nsh"
  ${Unless} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "This version of Fatou only runs on 64-bit operating systems."
    Quit
  ${EndUnless}

  ${If} ${AtLeastWinXP}
      ${If} ${IsWinXP}
      ${AndIf} ${AtMostServicePack} 1
          Goto WinTooOld
      ${Endif}
  ${Else}
      Goto WinTooOld
  ${Endif}


  ReadRegStr $INSTDIR HKLM "${PRODUCT_DIR_REGKEY}" "InstallDir"
  StrCmp $INSTDIR "" 0 WinOk
  StrCpy $INSTDIR "$@PROGRAMFILES@\VideoLAN\VLC"
  Goto WinOk

WinTooOld:
    MessageBox MB_OK|MB_ICONSTOP "This version of Fatou only runs on Windows XP SP2 and newer."
    Quit

WinOk:

  ; See if previous version exists
  Call ReadPreviousVersion


  ${If} $PreviousVersion == ""
    StrCpy $PerformUpdate 0
  ${Else}
    Push "${VERSION}"
    Push $PreviousVersion
    Call VersionCompare

    ${If} $PreviousVersionState != "newer"
      StrCpy $PerformUpdate 0
    ${EndIf}
  ${EndIf}


  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd
 

;;; Page to upgrade / downgrade or customize the installation
Function PageReinstall
  ${If} $PreviousVersion == ""
    Abort
  ${EndIf}

  ${If} $PerformUpdate == 1
    StrCpy $ReinstallType 1
    Abort
  ${EndIf}

  nsDialogs::Create /NOUNLOAD 1018
  Pop $0

  ${If} $PreviousVersionState == "newer"

    !insertmacro MUI_HEADER_TEXT "$(Reinstall_Headline)" "$(Reinstall_HeadlineInstall)"
    nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "$(Reinstall_MessageOlder)"
    Pop $R0
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "$(Reinstall_OptionUpgrade)"
    Pop $ReinstallUninstallBtn
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "$(Reinstall_OptionAdvanced)"
    Pop $R0

    ${If} $ReinstallType == ""
      StrCpy $ReinstallType 1
    ${EndIf}

  ${ElseIf} $PreviousVersionState == "older"

    !insertmacro MUI_HEADER_TEXT "$(Reinstall_Headline)" "$(Reinstall_HeadlineInstall)"
    nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "$(Reinstall_MessageNewer)"
    Pop $R0
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "$(Reinstall_OptionDowngrade)"
    Pop $ReinstallUninstallBtn
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "$(Reinstall_OptionAdvanced)"
    Pop $R0

    ${If} $ReinstallType == ""
      StrCpy $ReinstallType 1
    ${EndIf}

  ${ElseIf} $PreviousVersionState == "same"

    !insertmacro MUI_HEADER_TEXT "$(Reinstall_Headline)" "$(Reinstall_HeadlineMaintenance)"
    nsDialogs::CreateItem /NOUNLOAD STATIC ${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 0 0 100% 40 "$(Reinstall_MessageSame)"
    Pop $R0
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_VCENTER}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS}|${WS_GROUP}|${WS_TABSTOP} 0 10 55 100% 30 "$(Reinstall_OptionComponents)"
    Pop $R0
    nsDialogs::CreateItem /NOUNLOAD BUTTON ${BS_AUTORADIOBUTTON}|${BS_TOP}|${BS_MULTILINE}|${WS_VISIBLE}|${WS_CHILD}|${WS_CLIPSIBLINGS} 0 10 85 100% 50 "$(Reinstall_OptionUninstall)"
    Pop $ReinstallUninstallBtn

    ${If} $ReinstallType == ""
      StrCpy $ReinstallType 2
    ${EndIf}

  ${Else}

    MessageBox MB_ICONSTOP "Unknown value of PreviousVersionState, aborting" /SD IDOK
    Abort

  ${EndIf}

  ${If} $ReinstallType == "1"
    SendMessage $ReinstallUninstallBtn ${BM_SETCHECK} 1 0
  ${Else}
    SendMessage $R0 ${BM_SETCHECK} 1 0
  ${EndIf}

  nsDialogs::Show

FunctionEnd



Function PageLeaveReinstall

  SendMessage $ReinstallUninstallBtn ${BM_GETCHECK} 0 0 $R0
  ${If} $R0 == 1
    ; Option to uninstall old version selected
    StrCpy $ReinstallType 1
  ${Else}
    ; Custom up/downgrade or add/remove/reinstall
    StrCpy $ReinstallType 2
  ${EndIf}

  ${If} $ReinstallType == 1

    ${If} $PreviousVersionState == "same"

      Call RunUninstaller
      Quit

    ${EndIf}

  ${EndIf}

FunctionEnd


Function RunUninstaller
  ReadRegStr $R1 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  ${If} $R1 == ""
    Return
  ${EndIf}

  ;Run uninstaller
  HideWindow
  ClearErrors

  ExecWait '"$R1" _?=$INSTDIR'

  IfErrors no_remove_uninstaller

  IfFileExists "$INSTDIR\uninstall.exe" 0 no_remove_uninstaller
    Delete "$R1"
    RMDir $INSTDIR

  no_remove_uninstaller:
FunctionEnd


Function .OnInstSuccess
    ${MementoSectionSave}
FunctionEnd



# End function
Section -Post
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "InstallDir" $INSTDIR
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Version" "${VERSION}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\fatou.exe"
 
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayIcon" "$INSTDIR\fatou.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd



;;;;;;;;;;;;;;;;;;;;;;;;
; Uninstaller sections ;
;;;;;;;;;;;;;;;;;;;;;;;;
 
Section "un.$Name_Section91" SEC91
  SectionIn 1 2 3 RO
  SetShellVarContext all
 
  !insertmacro MacroAllExtensions DeleteContextMenu
  !insertmacro MacroAllExtensions UnRegisterExtensionSection
  !insertmacro DeleteContextMenuExt "Directory"
 
  FileOpen $UninstallLog "$INSTDIR\uninstall.log" r
  UninstallLoop:
    ClearErrors
    FileRead $UninstallLog $R0
    IfErrors UninstallEnd
    Push $R0
    Call un.TrimNewLines
    Pop $R0
    Delete "$INSTDIR\$R0"
    Goto UninstallLoop
  UninstallEnd:
  FileClose $UninstallLog
  Delete "$INSTDIR\uninstall.log"
  Delete "$INSTDIR\uninstall.exe"
  Push "\"
  Call un.RemoveEmptyDirs
  RMDir "$INSTDIR"
 
  ###### registry
 
  DeleteRegKey HKCR Applications\fatou.exe

  DeleteRegKey HKLM \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
 
  Delete "$DESKTOP\fatou.lnk"
 
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM ${PRODUCT_UNINST_ROOT_KEY}

  SetAutoClose false
SectionEnd
 
Section /o "un.$(Name_Section92)" SEC92
  !insertmacro delprefs
SectionEnd

; Uninstaller section descriptions
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC91} $Desc_Section91
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC92} $Desc_Section92
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
 
Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK \
    "$(^Name) was successfully removed from your computer."
FunctionEnd
 
Function un.onInit
  SetRegView 64
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd