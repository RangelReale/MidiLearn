[Setup]
AppName=MIDILearn
AppVerName=MIDILearn 0.4
DefaultDirName={pf}\MIDILearn
DefaultGroupName=MIDILearn
SourceDir=..\..
OutputBaseFilename=midilearn_0.4_win32_install
OutputDir=release\win32

[Dirs]
Name: "{app}\data"

[Files]
Source: "bin\Release\midilearn.exe"; DestDir: "{app}"
Source: "README"; DestDir: "{app}"; DestName: "README.txt"
Source: "data\*"; DestDir: "{app}\data"; Excludes: "Makefile.*,.svn"; Flags: recursesubdirs

[Icons]
Name: "{group}\MIDILearn"; Filename: "{app}\midilearn.exe"; WorkingDir: "{app}"
Name: "{group}\README"; Filename: "{app}\README.txt"; WorkingDir: "{app}"
Name: "{group}\Uninstall MIDILearn"; Filename: "{uninstallexe}"

