; P Client - Rust Injector Setup Script
; Generated for Inno Setup Compiler

#define MyAppName "P Client"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Rust Injector"
#define MyAppExeName "DownloadGUI.exe"
#define MyAppAssocName "P Client"
#define MyAppAssocExt ".pclient"
#define MyAppDescription "Rust Game Injector - P Client"

[Setup]
AppId={{D4E5F6A7-B8C9-0123-4567-890ABCDEF345}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE.txt
OutputDir=installer_output
OutputBaseFilename=PClient-Setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
WizardImageFile=installer_wizard.bmp
WizardSmallImageFile=installer_small.bmp
SetupIconFile=icon.ico
UninstallDisplayIcon={app}\RustInjector.exe
UninstallDisplayName={#MyAppName} {#MyAppVersion}
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "bin\x64\Release\DownloadGUI.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall
Source: "icon.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\RustInjector.exe"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\RustInjector.exe"; Tasks: desktopicon

[Run]
Filename: "{tmp}\{#MyAppExeName}"; Description: "Downloading P Client files..."; StatusMsg: "Downloading P Client files..."; Flags: runhidden waituntilterminated
Filename: "{app}\RustInjector.exe"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: "{#MyAppAssocExt}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocName}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "{#MyAppAssocName}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppDescription}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "{#MyAppAssocName}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\RustInjector.exe,0"
Root: HKCR; Subkey: "{#MyAppAssocName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\RustInjector.exe"" ""%1"""

[Code]
function InitializeSetup(): Boolean;
begin
  // Check if running on 64-bit Windows
  if IsWin64 then
    Result := True
  else begin
    MsgBox('P Client requires 64-bit Windows.', mbError, MB_OK);
    Result := False;
  end;
end;
