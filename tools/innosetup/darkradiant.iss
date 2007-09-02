; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=DarkRadiant
AppVerName=DarkRadiant 0.9.1
AppPublisher=The Dark Mod
AppPublisherURL=http://www.thedarkmod.com
AppSupportURL=http://www.thedarkmod.com
AppUpdatesURL=http://www.thedarkmod.com
DefaultDirName={pf}\DarkRadiant
DefaultGroupName=DarkRadiant 0.9.1
OutputDir=C:\workspace\darkradiant\tools\innosetup
OutputBaseFilename=darkradiant-0.9.1
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\workspace\darkradiant\install\darkradiant.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\workspace\darkradiant\install\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\DarkRadiant"; Filename: "{app}\darkradiant.exe";
Name: "{group}\{cm:UninstallProgram,DarkRadiant}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\DarkRadiant"; Filename: "{app}\darkradiant.exe"; Tasks: desktopicon

