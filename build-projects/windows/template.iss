; =============================================================================
; template.iss
; -----------------------------------------------------------------------------
; configuration template for Inno Setup installation project
;
; 2020-06-30 info@artsoft.org
; =============================================================================

[Setup]
AppName=_PRG_NAME_
AppVerName=_PRG_NAME_ _PRG_VERSION_
AppPublisher=Artsoft Entertainment
AppPublisherURL=https://www.artsoft.org/
AppSupportURL=https://www.artsoft.org/_PRG_BASENAME_/
AppUpdatesURL=https://www.artsoft.org/_PRG_BASENAME_/

ArchitecturesInstallIn64BitMode=_PRG_ARCH_
ArchitecturesAllowed=_PRG_ARCH_

DefaultDirName={pf}\_PRG_NAME_
DefaultGroupName=_PRG_NAME_
;LicenseFile="_PRG_DIR_\COPYING.txt"
;InfoBeforeFile="_PRG_DIR_\INSTALL.txt"
;InfoAfterFile="_PRG_DIR_\README.txt"
UninstallDisplayIcon={app}\_PRG_EXE_
Compression=lzma
SolidCompression=yes

OutputBaseFilename=_SETUP_EXE_
OutputDir=.

[Files]
Source: "_PRG_DIR_\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Tasks]
Name: "desktopicon"; Description: "Create a &Desktop icon"; GroupDescription: "Additional icons:"
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"

[Icons]
Name: "{group}\_PRG_NAME_"; Filename: "{app}\_PRG_EXE_"
Name: "{group}\_PRG_NAME_ on the Web"; Filename: "{app}\_PRG_BASENAME_.url"
Name: "{userdesktop}\_PRG_NAME_"; Filename: "{app}\_PRG_EXE_"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\_PRG_NAME_"; Filename: "{app}\_PRG_EXE_"; Tasks: quicklaunchicon

; This dynamically generates a Windows internet shortcut file. Unfortunately,
; this file is not removed when the package is uninstalled, leaving an empty
; program directory with just that internet shortcut file. Using a static file
; does not cause this problem.
;[INI]
;Filename: "{app}\_PRG_BASENAME_.url"; Section: "InternetShortcut"; Key: "URL"; String: "https://www.artsoft.org/_PRG_BASENAME_/"

[Run]
Filename: "{app}\_PRG_EXE_"; Description: "Launch _PRG_NAME_"; Flags: nowait postinstall skipifsilent
