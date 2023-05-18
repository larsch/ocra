@ECHO ON
for %%I in ("%~dp0.") do for %%J in ("%%~dpI.") do set ParentFolderName=%%~dpnxJ
cd %ParentFolderName%/tasks
powershell Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))  && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

cmd.exe /C choco install Containers Microsoft-Hyper-V --source windowsfeatures
cmd.exe /C choco install docker-desktop -y
cmd.exe /C choco install vscode -y
cmd.exe /C choco install microsoft-windows-terminal -y
cmd.exe /C choco install firefox -y
cmd.exe /C choco install filezilla -y
cmd.exe /C refreshenv