[Setup]
AppName=Yuna App
AppVersion=1.0.0
DefaultDirName={pf}\YunaApp
DefaultGroupName=YunaApp
UninstallDisplayIcon={app}\Yuna.exe
Compression=lzma2
SolidCompression=yes
OutputDir=installer_output
OutputBaseFilename=YunaSetup

[Files]
; Tempatkan Yuna.exe hasil build-mu
Source: "dist\app\Yuna.exe"; DestDir: "{app}"  
; Kalau ada file DLL tambahan atau asset, tambahkan di sini:
; Source: "dist\app*.dll"; DestDir: "{app}"

[Icons]
; Buat shortcut di Start Menu dan Desktop (opsional)
Name: "{group}\Yuna App Image Upscale"; Filename: "{app}\Yuna.exe"
Name: "{commondesktop}\Yuna App Image Upscale"; Filename: "{app}\Yuna.exe"

[Run]
; Opsional: jalankan aplikasi setelah instalasi selesai
Filename: "{app}\Yuna.exe"; Description: "Jalankan Yuna App Image Upscale"; Flags: postinstall nowait skipifsilent