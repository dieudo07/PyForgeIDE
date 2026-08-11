; Script Inno Setup pour PyForge IDE
#define MyAppName "PyForge IDE"
#define MyAppVersion "3.0"
#define MyAppPublisher "PyForge Team"
#define MyAppExeName "PyForgeIDE.exe"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2
SolidCompression=yes
OutputDir=C:\PyForgeIDE-installer
OutputBaseFilename=PyForgeIDE-Setup-v{#MyAppVersion}
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=C:\PyForgeIDE\resources\icons\pyforge.ico

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le Bureau"; GroupDescription: "Raccourcis:"
Name: "quicklaunch"; Description: "Épingler à la barre des tâches"; GroupDescription: "Raccourcis:"; Flags: unchecked
Name: "associate"; Description: "Associer les fichiers .py avec PyForge IDE"; GroupDescription: "Associations:"; Flags: unchecked

[Files]
Source: "C:\PyForgeIDE\build\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\PyForgeIDE\README.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Désinstaller {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: ".py"; ValueType: string; ValueName: ""; ValueData: "PyForgeIDE.py"; Flags: uninsdeletevalue; Tasks: associate
Root: HKCR; Subkey: "PyForgeIDE.py"; ValueType: string; ValueName: ""; ValueData: "Python Script"; Flags: uninsdeletekey; Tasks: associate
Root: HKCR; Subkey: "PyForgeIDE.py\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associate
Root: HKCR; Subkey: "PyForgeIDE.py\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associate

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Lancer PyForge IDE"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  // Vérifier si Python est installé
  if not FileExists(ExpandConstant('{sys}\..\..\Python\python.exe')) then
  begin
    if MsgBox('Python n''est pas détecté sur votre système.' + #13#10 + 
              'PyForge IDE nécessite Python 3.10+ pour fonctionner.' + #13#10 + #13#10 +
              'Voulez-vous continuer l''installation ?', 
              mbConfirmation, MB_YESNO) = IDNO then
      Result := False;
  end;
end;